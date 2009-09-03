#include "tLSCStabilized.hpp"
#include "NS/PB_LSCPreconditionerFactory.hpp"
#include "NS/PB_InvLSCStrategy.hpp"

// Teuchos includes
#include "Teuchos_RCP.hpp"

// Epetra includes
#include "Epetra_Map.h"
#include "Epetra_CrsMatrix.h"
#include "Epetra_Vector.h"

// Thyra includes
#include "Thyra_EpetraLinearOp.hpp"
#include "Thyra_LinearOpBase.hpp"
#include "Thyra_DefaultBlockedLinearOp.hpp"
#include "Thyra_DefaultIdentityLinearOp.hpp"
#include "Thyra_DefaultZeroLinearOp.hpp"
#include "Thyra_DefaultLinearOpSource.hpp"
#include "Thyra_DefaultPreconditioner.hpp"
#include "Thyra_EpetraThyraWrappers.hpp"
#include "Thyra_DefaultMultipliedLinearOp.hpp"
#include "Thyra_DefaultScaledAdjointLinearOp.hpp"
#include "Thyra_PreconditionerFactoryHelpers.hpp"
#include "Thyra_LinearOpTester.hpp"

#include <vector>

// This whole test rig is based on inverting the matrix
// 
//      [  1  2  1 -1 ]
//  A = [  2  1 -3  1 ]
//      [  1 -3  0  0 ]
//      [ -1  1  0  0 ]
//
// see the matlab file

namespace PB {
namespace Test {

using namespace Teuchos;
using namespace Thyra;
using namespace PB::NS;

void tLSCStabilized::initializeTest()
{
   tolerance_ = 1.0e-13;

   comm = rcp(new Epetra_SerialComm());
}

int tLSCStabilized::runTest(int verbosity,std::ostream & stdstrm,std::ostream & failstrm,int & totalrun)
{
   bool allTests = true;
   bool status;
   int failcount = 0;

   failstrm << "tLSCStabilized";

   status = test_diagonal(verbosity,failstrm);
   PB_TEST_MSG(stdstrm,1,"   \"diagonal\" ... PASSED","   \"diagonal\" ... FAILED");
   allTests &= status;
   failcount += status ? 0 : 1;
   totalrun++;

   status = test_strategy(verbosity,failstrm);
   PB_TEST_MSG(stdstrm,1,"   \"strategy\" ... PASSED","   \"strategy\" ... FAILED");
   allTests &= status;
   failcount += status ? 0 : 1;
   totalrun++;

   status = allTests;
   if(verbosity >= 10) {
      PB_TEST_MSG(failstrm,0,"tLSCStabilized...PASSED","tLSCStablilized...FAILED");
   }
   else {// Normal Operatoring Procedures (NOP)
      PB_TEST_MSG(failstrm,0,"...PASSED","tLSCStabilized...FAILED");
   }

   return failcount;
}

bool tLSCStabilized::test_diagonal(int verbosity,std::ostream & os)
{
   // make sure the preconditioner is working by testing against the identity matrix
   typedef RCP<const Thyra::VectorBase<double> > Vector;
   typedef RCP<const Thyra::VectorSpaceBase<double> > VectorSpace;
   typedef RCP<const Thyra::LinearOpBase<double> > LinearOp;

   bool status = false;
   bool allPassed = true;
   double vec[2];
   double diff = 0.0;

   // build 4x4 matrix with block 2x2 diagonal subblocks
   //
   //            [ 1 0 7 0 ]
   // [ F G ] =  [ 0 2 0 8 ]
   // [ D C ]    [ 5 0 3 0 ]
   //            [ 0 6 0 4 ]
   //

   vec[0] = 1.0; vec[1] = 2.0;
   LinearOp F = PB::Test::DiagMatrix(2,vec);

   vec[0] = 7.0; vec[1] = 8.0;
   LinearOp G = PB::Test::DiagMatrix(2,vec);

   vec[0] = 5.0; vec[1] = 6.0;
   LinearOp D = PB::Test::DiagMatrix(2,vec);

   vec[0] = 3.0; vec[1] = 4.0;
   LinearOp C = PB::Test::DiagMatrix(2,vec);

   vec[0] = 1.0; vec[1] = 0.5;
   LinearOp iF = PB::Test::DiagMatrix(2,vec);

   vec[0] = 0.030303030303030; vec[1] = 0.02205882352941;
   LinearOp iBBt = PB::Test::DiagMatrix(2,vec);

   vec[0] = 0.026041666666667; vec[1] = 0.041666666666667;
   LinearOp aiD = PB::Test::DiagMatrix(2,vec);

   LinearOp A = Thyra::block2x2(F,G,D,C);
 
   const RCP<const Thyra::PreconditionerFactoryBase<double> > precFactory 
         = rcp(new LSCPreconditionerFactory(iF,iBBt,aiD,Teuchos::null));
   RCP<Thyra::PreconditionerBase<double> > prec = Thyra::prec<double>(*precFactory,A);

   // build linear operator
   RCP<const Thyra::LinearOpBase<double> > precOp = prec->getUnspecifiedPrecOp(); 

   const RCP<Epetra_Map> map = rcp(new Epetra_Map(2,0,*comm));
   // construct a couple of vectors
   Epetra_Vector ea(*map),eb(*map);
   Epetra_Vector ef(*map),eg(*map);
   const RCP<const Thyra::VectorBase<double> > x = BlockVector(ea,eb,A->domain());
   const RCP<const Thyra::VectorBase<double> > z = BlockVector(ef,eg,A->domain());
   const RCP<Thyra::VectorBase<double> > y = Thyra::createMember(A->range()); 

   // now checks of the preconditioner (should be exact!)
   /////////////////////////////////////////////////////////////////////////

   // test vector [0 1 1 3]
   ea[0] = 0.0; ea[1] = 1.0; eb[0] = 1.0; eb[1] = 3.0;
   ef[0] =  0.407268709825528; ef[1] =  1.560553633217993;
   eg[0] = -0.058181244260790; eg[1] = -0.265138408304498;
   Thyra::apply(*precOp,NONCONJ_ELE,*x,&*y);
   TEST_ASSERT((diff = PB::Test::Difference(y,z)/Thyra::norm_2(*z))<tolerance_,
               "   tLSCStabilized::test_diagonal " << toString(status) << ":(y=inv(A)*x) != z (|y-z|_2/|z|_2 = "
            << diff << " <= " << tolerance_ << ")\n"
            << "      " << Print("x",x) 
            << "      " << Print("y",y) 
            << "      " << Print("z",z));

   // test vector [-2 4 7 9]
   ea[0] =-2.0; ea[1] = 4.0; eb[0] = 7.0; eb[1] = 9.0;
   ef[0] =  0.850880968778696; ef[1] =  5.181660899653979;
   eg[0] = -0.407268709825528; eg[1] = -0.795415224913495;
   Thyra::apply(*precOp,NONCONJ_ELE,*x,&*y);
   TEST_ASSERT((diff = PB::Test::Difference(y,z)/Thyra::norm_2(*z))<tolerance_,
               "   tLSCStabilized::test_diagonal " << toString(status) << ":(y=inv(A)*x) != z (|y-z|_2/|z|_2 = "
            << diff << " <= " << tolerance_ << ")\n"
            << "      " << Print("x",x) 
            << "      " << Print("y",y) 
            << "      " << Print("z",z));

   // test vector [1 0 0 -5]
   ea[0] = 1.0; ea[1] = 0.0; eb[0] = 0.0; eb[1] =-5.0;
   ef[0] =  1.000000000000000; ef[1] = -1.767589388696655;
   eg[0] =  0.000000000000000; eg[1] =  0.441897347174164;
   Thyra::apply(*precOp,NONCONJ_ELE,*x,&*y);
   TEST_ASSERT((diff = PB::Test::Difference(y,z)/Thyra::norm_2(*z))<tolerance_,
               "   tLSCStabilized::test_diagonal " << toString(status) << ":(y=inv(A)*x) != z (|y-z|_2/|z|_2 = "
            << diff << " <= " << tolerance_ << ")\n"
            << "      " << Print("x",x) 
            << "      " << Print("y",y) 
            << "      " << Print("z",z));

   // test vector [4 -4 6 12]
   ea[0] = 4.0; ea[1] =-4.0; eb[0] = 6.0; eb[1] =12.0;
   ef[0] =  6.443612258953168; ef[1] =  2.242214532871971;
   eg[0] = -0.349087465564738; eg[1] = -1.060553633217993;
   Thyra::apply(*precOp,NONCONJ_ELE,*x,&*y);
   TEST_ASSERT((diff = PB::Test::Difference(y,z)/Thyra::norm_2(*z))<tolerance_,
               "   tLSCStabilized::test_diagonal " << toString(status) << ":(y=inv(A)*x) != z (|y-z|_2/|z|_2 = "
            << diff << " <= " << tolerance_ << ")\n"
            << "      " << Print("x",x) 
            << "      " << Print("y",y) 
            << "      " << Print("z",z));

   return allPassed;
}

bool tLSCStabilized::test_strategy(int verbosity,std::ostream & os)
{
   std::vector<int> indicies(2);
   std::vector<double> row0(2);
   int sz = 5;
   double vec[5];

   bool status = false;
   bool allPassed = true;

   vec[0] = 1.0; vec[1] = 2.0; vec[2] = 3.0; vec[3] = 4.0; vec[4] = 5.0;
   LinearOp F = PB::Test::DiagMatrix(sz,vec);

   vec[0] = 7.0; vec[1] = 8.0; vec[2] = 9.0; vec[3] = 10.0; vec[4] = 11.0;
   LinearOp G = PB::Test::DiagMatrix(sz,vec);

   vec[0] = 5.0; vec[1] = 6.0; vec[2] = 7.0; vec[3] = 8.0; vec[4] = 9.0;
   LinearOp D = PB::Test::DiagMatrix(sz,vec);

   vec[0] = 3.0; vec[1] = 4.0; vec[2] = 5.0; vec[3] = 6.0; vec[4] = 7.0;
   LinearOp C = PB::Test::DiagMatrix(sz,vec);

   vec[0] = 1.0; vec[1] = 1.0/2.0; vec[2] = 1.0/3.0; vec[3] = 1.0/4.0; vec[4] = 1.0/5.0;
   LinearOp iF = PB::Test::DiagMatrix(sz,vec);

   vec[0] = 0.091304347826087;
   vec[1] = 0.090517241379310;
   vec[2] = 0.087646076794658;
   vec[3] = 0.084000000000000;
   vec[4] = 0.080152671755725;
   LinearOp iBQBtmC = PB::Test::DiagMatrix(sz,vec);

   vec[0] = 0.020202020202020;
   vec[1] = 0.032323232323232;
   vec[2] = 0.040404040404040;
   vec[3] = 0.046176046176046;
   vec[4] = 0.050505050505051;
   LinearOp aiD = PB::Test::DiagMatrix(sz,vec);

   LinearOp A = Thyra::block2x2(F,G,D,C);

   comm = rcp(new Epetra_SerialComm());
   const RCP<Epetra_Map> map = rcp(new Epetra_Map(sz,0,*comm));

   Epetra_Vector ea(*map),eb(*map);
   const RCP<const Thyra::VectorBase<double> > x = BlockVector(ea,eb,A->domain());
   const RCP<Thyra::VectorBase<double> > y = Thyra::createMember(A->range()); 

   Teuchos::ParameterList paramList;
   paramList.set("Linear Solver Type","Amesos");
   RCP<PB::InverseFactory> invFact = PB::invFactoryFromParamList(paramList,"Amesos");

   // build F matrix
   const RCP<Epetra_CrsMatrix> ptrMass  = rcp(new Epetra_CrsMatrix(Copy,*map,2));
   row0[0] = 3.0; row0[1] = 9.0; indicies[0] = 0; indicies[1] = 2;
   ptrMass->InsertGlobalValues(0,2,&row0[0],&indicies[0]);
   row0[0] = 1.0; row0[1] = 4.0; indicies[0] = 0; indicies[1] = 1;
   ptrMass->InsertGlobalValues(1,2,&row0[0],&indicies[0]);
   row0[0] = 5.0; row0[1] = 7.0; indicies[0] = 2; indicies[1] = 3;
   ptrMass->InsertGlobalValues(2,2,&row0[0],&indicies[0]);
   row0[0] = 1.0; row0[1] = 6.0; indicies[0] = 2; indicies[1] = 3;
   ptrMass->InsertGlobalValues(3,2,&row0[0],&indicies[0]);
   row0[0] = 7.0; row0[1] = 6.0; indicies[0] = 4; indicies[1] = 1;
   ptrMass->InsertGlobalValues(4,2,&row0[0],&indicies[0]);
   ptrMass->FillComplete();
   LinearOp mass = Thyra::epetraLinearOp(ptrMass,"mass");

   vec[0] = 1.0/3.0; vec[1] = 1.0/4.0; vec[2] = 1.0/5.0; vec[3] = 1.0/6.0; vec[4] = 1.0/7.0;
   LinearOp invMass = PB::Test::DiagMatrix(sz,vec);

   Thyra::LinearOpTester<double> tester;
   tester.set_all_error_tol(5e-4);
   tester.show_all_tests(true);
   std::stringstream ss;
   Teuchos::FancyOStream fos(Teuchos::rcpFromRef(ss),"      |||");

   PB::BlockedLinearOp blkA = PB::toBlockedLinearOp(A);

   // build preconditioner
   vec[0] = 1.0; vec[1] = 0.5; vec[2] = 1.0/3.0; vec[3] = 0.25; vec[4] = 0.2;
   LinearOp p00 = PB::Test::DiagMatrix(sz,vec);

   vec[0] = 0.368351759561589;
   vec[1] = 0.325933832979017;
   vec[2] = 0.295436133965709;
   vec[3] = 0.272240115440115;
   vec[4] = 0.253891252128534;
   LinearOp p01 = PB::Test::DiagMatrix(sz,vec);

   vec[0] = 0; vec[1] = 0; vec[2] = 0; vec[3] = 0; vec[4] = 0;
   LinearOp p10 = PB::Test::DiagMatrix(sz,vec);

   vec[0] = -0.052621679937370;
   vec[1] = -0.081483458244754;
   vec[2] = -0.098478711321903;
   vec[3] = -0.108896046176046;
   vec[4] = -0.115405114603879;
   LinearOp p11 = PB::Test::DiagMatrix(sz,vec);
   LinearOp P = Thyra::block2x2(p00,p01,p10,p11);

   // build inverse strategy
   { 
      bool result;
      PB::NS::LSCPrecondState state;
      PB::NS::InvLSCStrategy iStrat(invFact,mass);
      iStrat.setEigSolveParam(3);
      PB::NS::LSCPreconditionerFactory factory(Teuchos::rcpFromRef(iStrat));
      LinearOp prec = factory.buildPreconditionerOperator(blkA,state);

      // test inverse mass
      ss.str("");
      result = tester.compare( *invMass, *iStrat.getInvMass(blkA,state), &fos );
      TEST_ASSERT(result,
            std::endl << "   tLSCStabilized::test_strategy " << toString(status)
                      << " : Comparing mass operators");
      if(not result || verbosity>=10) 
         os << ss.str(); 

      // test inverse F
      ss.str("");
      result = tester.compare( *iF, *iStrat.getInvF(blkA,state), &fos );
      TEST_ASSERT(result,
            std::endl << "   tLSCStabilized::test_strategy " << toString(status)
                      << " : Comparing F operators");
      if(not result || verbosity>=10) 
         os << ss.str(); 

      // test inverse B*Q*Bt-gamma*C
      ss.str("");
      result = tester.compare( *iBQBtmC, *iStrat.getInvBQBt(blkA,state), &fos );
      TEST_ASSERT(result,
            std::endl << "   tLSCStabilized::test_strategy " << toString(status)
                      << " : Comparing B*Q*Bt-C operators");
      if(not result || verbosity>=10) 
         os << ss.str(); 

      // test alpha*inv(D)
      ss.str("");
      result = tester.compare( *aiD, *iStrat.getInvAlphaD(blkA,state), &fos );
      TEST_ASSERT(result,
            std::endl << "   tLSCStabilized::test_strategy " << toString(status)
                      << " : Comparing alpha*inv(D) operators");
      if(not result || verbosity>=10) 
         os << ss.str(); 

      // test full op
      ss.str("");
      result = tester.compare( *P, *prec, &fos );
      TEST_ASSERT(result,
            std::endl << "   tLSCStabilized::test_strategy " << toString(status)
                      << " : Comparing full op");
      if(not result || verbosity>=10) 
         os << ss.str(); 
   }

   

   return allPassed;
}

} // end namespace Test
} // end namespace PB

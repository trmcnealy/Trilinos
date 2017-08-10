#include <iostream>

// Teuchos
#include <Teuchos_RCP.hpp>
#include <Teuchos_DefaultComm.hpp>
#include "Teuchos_Assert.hpp"
#include "Teuchos_LocalTestingHelpers.hpp"
#include <Teuchos_StandardCatchMacros.hpp>
#include "Teuchos_ParameterList.hpp"

// Xpetra
#include "Xpetra_Map.hpp"
#include "Xpetra_MapFactory.hpp"
#include "Xpetra_Matrix.hpp"
#include "Xpetra_CrsMatrixWrap.hpp"
#include "Xpetra_IO.hpp"
#include "Xpetra_MatrixSplitting.hpp"
#include "Xpetra_RegionalAMG_def.hpp"
#ifdef HAVE_XPETRA_TPETRA
#include "Xpetra_TpetraCrsMatrix.hpp"
#endif
#ifdef HAVE_XPETRA_EPETRA
#include "Xpetra_EpetraCrsMatrix.hpp"
#endif

// Epetra
#ifdef HAVE_MPI
#include "Epetra_MpiComm.h"
#include "mpi.h"
#else
#include "Epetra_SerialComm.h"
#endif
#include "Epetra_Map.h"
#include "Epetra_Vector.h"
#include "Epetra_CrsMatrix.h"
#include "Epetra_LinearProblem.h"


// =========== //
// main driver //
// =========== //

int main(int argc, char* argv[])
{


	typedef double                                      scalar_type;
	typedef int                                         local_ordinal_type;
	typedef int                                         global_ordinal_type;
	typedef scalar_type         												Scalar;
	typedef local_ordinal_type  												LocalOrdinal;
	typedef global_ordinal_type 												GlobalOrdinal;
	typedef KokkosClassic::DefaultNode::DefaultNodeType Node;

	typedef Xpetra::MultiVector<Scalar,LocalOrdinal,GlobalOrdinal,Node> multivector_type;

	#ifdef HAVE_MPI
	  MPI_Init(&argc, &argv);
	  Epetra_MpiComm CommEpetra(MPI_COMM_WORLD);
	#else
	  Epetra_SerialComm CommEpetra;
	#endif

	// Here we create the linear problem
	//
	//   Matrix * LHS = RHS
	//
	// with Matrix arising from a 5-point formula discretization.
  
	TEUCHOS_TEST_FOR_EXCEPT_MSG(argc<4, "\nInvalid name for input matrix and output file\n");


	Teuchos::ParameterList xmlParams;
	Teuchos::ParameterList mueluParams;
	Teuchos::updateParametersFromXmlFile(argv[1], Teuchos::inoutArg(mueluParams));

	Teuchos::RCP<const Teuchos::Comm<int> > comm = Teuchos::DefaultComm<int>::getComm();
	if (CommEpetra.MyPID() == 0)
		std::cout<<"Number of processors: "<<CommEpetra.NumProc()<<std::endl;


	//SplittingDriver
	/*Xpetra::SplittingDriver<Scalar, LocalOrdinal, GlobalOrdinal, Node> driver("node.txt", comm);
	Teuchos::Array<GlobalOrdinal> elementlist = driver.GetGlobalRowMap();
	driver.printInactive();
	Xpetra::MatrixSplitting<Scalar,LocalOrdinal,GlobalOrdinal,Node,Xpetra::UseTpetra> xpetraWrapper( argv[1], argv[2], comm );
	std::string output_file="A_write.mtx";
	xpetraWrapper.writeGlobalMatrix();	
	xpetraWrapper.writeRegionMatrices();*/


	//Teuchos::RCP<Xpetra::Matrix<Scalar, LocalOrdinal, GlobalOrdinal, Node> > A;
	//A = Xpetra::IO<Scalar,LocalOrdinal,GlobalOrdinal,Node>::Read(argv[2], Xpetra::UseTpetra, comm);

	int max_num_levels = 4;
	int coarsening_factor = 3;

	Xpetra::RegionalAMG<Scalar,LocalOrdinal,GlobalOrdinal,Node> preconditioner( argv[2], argv[3], comm, mueluParams, max_num_levels, coarsening_factor );


	Teuchos::RCP<multivector_type> X = Xpetra::MultiVectorFactory< Scalar, LocalOrdinal, GlobalOrdinal, Node >::Build(preconditioner.getDomainMap(), 1) ;
	Teuchos::RCP<multivector_type> Y = Xpetra::MultiVectorFactory< Scalar, LocalOrdinal, GlobalOrdinal, Node >::Build(preconditioner.getRangeMap(), 1) ;
	X->randomize();
	Y->putScalar((scalar_type) 0.0);

	
	preconditioner.apply(*X,*Y);

	#ifdef HAVE_MPI
 	  MPI_Finalize();
	#endif

	return(EXIT_SUCCESS);
}


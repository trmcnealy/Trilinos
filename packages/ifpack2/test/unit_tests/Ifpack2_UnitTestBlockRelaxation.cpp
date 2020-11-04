/*
//@HEADER
// ***********************************************************************
//
//       Ifpack2: Templated Object-Oriented Algebraic Preconditioner Package
//                 Copyright (2009) Sandia Corporation
//
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ***********************************************************************
//@HEADER
*/


/*! \file Ifpack2_UnitTestRelaxation.cpp

\brief Ifpack2 Unit test for the Relaxation template.
*/

#include <iostream>

#include <Teuchos_ConfigDefs.hpp>
#include <Ifpack2_ConfigDefs.hpp>
#include <Teuchos_UnitTestHarness.hpp>
#include <Ifpack2_Version.hpp>

#include <Tpetra_RowMatrix.hpp>
#include <Tpetra_BlockMultiVector.hpp>
#include <Tpetra_BlockCrsMatrix.hpp>

#include <Ifpack2_UnitTestHelpers.hpp>
#include <Ifpack2_BlockRelaxation.hpp>
#include <Ifpack2_SparseContainer.hpp>
#include <Ifpack2_TriDiContainer.hpp>
#include <Ifpack2_BandedContainer.hpp>
#include <Ifpack2_DenseContainer.hpp>
#include <Ifpack2_OverlappingPartitioner.hpp>
#include <Ifpack2_LinearPartitioner.hpp>
#include <Ifpack2_LinePartitioner.hpp>
#include <Ifpack2_ILUT.hpp>
#include <Ifpack2_Factory.hpp>

namespace {
using Tpetra::global_size_t;
typedef tif_utest::Node Node;

//this macro declares the unit-test-class:
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
TEUCHOS_UNIT_TEST_TEMPLATE_3_DECL(Ifpack2BlockRelaxation, Test0, Scalar, LocalOrdinal, GlobalOrdinal)
#else
TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL(Ifpack2BlockRelaxation, Test0, Scalar)
#endif
{
#ifndef TPETRA_ENABLE_TEMPLATE_ORDINALS
using LocalOrdinal = typename Tpetra::Map<>::local_ordinal_type;
using GlobalOrdinal = typename Tpetra::Map<>::global_ordinal_type;
#endif
//we are now in a class method declared by the above macro, and
//that method has these input arguments:
//Teuchos::FancyOStream& out, bool& success

  std::string version = Ifpack2::Version();
  out << "Ifpack2::Version(): " << version << std::endl;

  global_size_t num_rows_per_proc = 5;

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  typedef Tpetra::CrsMatrix<Scalar,LocalOrdinal,GlobalOrdinal,Node> CRS;
  typedef Tpetra::RowMatrix<Scalar,LocalOrdinal,GlobalOrdinal,Node> ROW;
#else
  typedef Tpetra::CrsMatrix<Scalar,Node> CRS;
  typedef Tpetra::RowMatrix<Scalar,Node> ROW;
#endif

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  const Teuchos::RCP<const Tpetra::Map<LocalOrdinal,GlobalOrdinal,Node> > rowmap =
    tif_utest::create_tpetra_map<LocalOrdinal,GlobalOrdinal,Node>(num_rows_per_proc);
  Teuchos::RCP<const CRS> crsmatrix = tif_utest::create_test_matrix<Scalar,LocalOrdinal,GlobalOrdinal,Node>(rowmap);
#else
  const Teuchos::RCP<const Tpetra::Map<Node> > rowmap =
    tif_utest::create_tpetra_map<Node>(num_rows_per_proc);
  Teuchos::RCP<const CRS> crsmatrix = tif_utest::create_test_matrix<Scalar,Node>(rowmap);
#endif

  Ifpack2::BlockRelaxation<ROW> prec(crsmatrix);

  Teuchos::ParameterList params;
  params.set("relaxation: container", "SparseILUT");
  params.set("relaxation: type", "Jacobi");
  params.set("partitioner: type","linear");
  params.set("partitioner: local parts",(int)num_rows_per_proc);

  TEST_NOTHROW(prec.setParameters(params));

  //trivial tests to insist that the preconditioner's domain/range maps are
  //identically those of the matrix:
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  const Tpetra::Map<LocalOrdinal,GlobalOrdinal,Node>* mtx_dom_map_ptr = &*crsmatrix->getDomainMap();
  const Tpetra::Map<LocalOrdinal,GlobalOrdinal,Node>* mtx_rng_map_ptr = &*crsmatrix->getRangeMap();
#else
  const Tpetra::Map<Node>* mtx_dom_map_ptr = &*crsmatrix->getDomainMap();
  const Tpetra::Map<Node>* mtx_rng_map_ptr = &*crsmatrix->getRangeMap();
#endif

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  const Tpetra::Map<LocalOrdinal,GlobalOrdinal,Node>* prec_dom_map_ptr = &*prec.getDomainMap();
  const Tpetra::Map<LocalOrdinal,GlobalOrdinal,Node>* prec_rng_map_ptr = &*prec.getRangeMap();
#else
  const Tpetra::Map<Node>* prec_dom_map_ptr = &*prec.getDomainMap();
  const Tpetra::Map<Node>* prec_rng_map_ptr = &*prec.getRangeMap();
#endif

  TEST_EQUALITY( prec_dom_map_ptr, mtx_dom_map_ptr);
  TEST_EQUALITY( prec_rng_map_ptr, mtx_rng_map_ptr);

  prec.initialize();
  prec.compute();

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  Tpetra::MultiVector<Scalar,LocalOrdinal,GlobalOrdinal,Node> x(rowmap,2), y(rowmap,2);
#else
  Tpetra::MultiVector<Scalar,Node> x(rowmap,2), y(rowmap,2);
#endif
  x.putScalar(1);

  prec.applyMat(x, y);

  Teuchos::ArrayRCP<const Scalar> yview = y.get1dView();

  //Since crsmatrix is a diagonal matrix with 2 on the diagonal,
  //y should be full of 2's now.

  Teuchos::ArrayRCP<Scalar> twos(num_rows_per_proc*2, 2);

  TEST_COMPARE_FLOATING_ARRAYS(yview, twos(), Teuchos::ScalarTraits<Scalar>::eps());

  prec.apply(x, y);

  //y should be full of 0.5's now.

  Teuchos::ArrayRCP<Scalar> halfs(num_rows_per_proc*2, 0.5);

  TEST_COMPARE_FLOATING_ARRAYS(yview, halfs(), Teuchos::ScalarTraits<Scalar>::eps());
}

// Test apply() with x == y.
// When x == y, apply() need to create internally an auxiliary vector, Xcopy.
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
TEUCHOS_UNIT_TEST_TEMPLATE_3_DECL(Ifpack2BlockRelaxation, Test1, Scalar, LocalOrdinal, GlobalOrdinal)
#else
TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL(Ifpack2BlockRelaxation, Test1, Scalar)
#endif
{
#ifndef TPETRA_ENABLE_TEMPLATE_ORDINALS
  using LocalOrdinal = typename Tpetra::Map<>::local_ordinal_type;
  using GlobalOrdinal = typename Tpetra::Map<>::global_ordinal_type;
#endif
  std::string version = Ifpack2::Version();
  out << "Ifpack2::Version(): " << version << std::endl;

  global_size_t num_rows_per_proc = 5;

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  typedef Tpetra::CrsMatrix<Scalar,LocalOrdinal,GlobalOrdinal,Node> CRS;
  typedef Tpetra::RowMatrix<Scalar,LocalOrdinal,GlobalOrdinal,Node> ROW;
#else
  typedef Tpetra::CrsMatrix<Scalar,Node> CRS;
  typedef Tpetra::RowMatrix<Scalar,Node> ROW;
#endif

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  const Teuchos::RCP<const Tpetra::Map<LocalOrdinal,GlobalOrdinal,Node> > rowmap = tif_utest::create_tpetra_map<LocalOrdinal,GlobalOrdinal,Node>(num_rows_per_proc);
  Teuchos::RCP<const CRS> crsmatrix = tif_utest::create_test_matrix<Scalar,LocalOrdinal,GlobalOrdinal,Node>(rowmap);
#else
  const Teuchos::RCP<const Tpetra::Map<Node> > rowmap = tif_utest::create_tpetra_map<Node>(num_rows_per_proc);
  Teuchos::RCP<const CRS> crsmatrix = tif_utest::create_test_matrix<Scalar,Node>(rowmap);
#endif
  Ifpack2::BlockRelaxation<ROW> prec(crsmatrix);

  Teuchos::ParameterList params;
  params.set("relaxation: container", "SparseILUT");
  params.set("relaxation: type", "Jacobi");
  params.set("partitioner: type","linear");
  params.set("partitioner: local parts",(int)num_rows_per_proc);
  prec.setParameters(params);

  prec.initialize();
  prec.compute();

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  Tpetra::MultiVector<Scalar,LocalOrdinal,GlobalOrdinal,Node> x(rowmap,2);
#else
  Tpetra::MultiVector<Scalar,Node> x(rowmap,2);
#endif
  x.putScalar(1);

  prec.apply(x, x);

  //y should be full of 0.5's now.
  {
    Teuchos::ArrayRCP<const Scalar> xview = x.get1dView();
    Teuchos::ArrayRCP<Scalar> halfs(num_rows_per_proc*2, 0.5);
    TEST_COMPARE_FLOATING_ARRAYS(xview, halfs(), Teuchos::ScalarTraits<Scalar>::eps());
  }
}

// Test apply() with x != y  but x and y pointing to the same memory location.
// It is possible to create two different Tpetra vectors pointing to the same memory location by using MultiVector::subView().
// I can't imagine anyone trying to do this but... in this case, apply() need also to create internally an auxiliary vector, Xcopy.
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
TEUCHOS_UNIT_TEST_TEMPLATE_3_DECL(Ifpack2BlockRelaxation, Test2, Scalar, LO, GO)
#else
TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL(Ifpack2BlockRelaxation, Test2, Scalar)
#endif
{
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  typedef Tpetra::Map<LO,GO,Node> map_type;
  typedef Tpetra::CrsMatrix<Scalar,LO,GO,Node> CRS;
  typedef Tpetra::RowMatrix<Scalar,LO,GO,Node> ROW;
  typedef Tpetra::MultiVector<Scalar,LO,GO,Node> MV;
#else
  using LO = typename Tpetra::Map<>::local_ordinal_type;
  using GO = typename Tpetra::Map<>::global_ordinal_type;
  typedef Tpetra::Map<Node> map_type;
  typedef Tpetra::CrsMatrix<Scalar,Node> CRS;
  typedef Tpetra::RowMatrix<Scalar,Node> ROW;
  typedef Tpetra::MultiVector<Scalar,Node> MV;
#endif
  using Teuchos::RCP;
  using std::endl;

  out << "Ifpack2::BlockRelaxation Test2" << endl;

  const Scalar one = Teuchos::ScalarTraits<Scalar>::one ();
  const Scalar two = one + one;

  global_size_t num_rows_per_proc = 5;
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  RCP<const map_type> rowmap = tif_utest::create_tpetra_map<LO,GO,Node> (num_rows_per_proc);
  RCP<const CRS> crsmatrix = tif_utest::create_test_matrix<Scalar,LO,GO,Node> (rowmap);
#else
  RCP<const map_type> rowmap = tif_utest::create_tpetra_map<Node> (num_rows_per_proc);
  RCP<const CRS> crsmatrix = tif_utest::create_test_matrix<Scalar,Node> (rowmap);
#endif
  Ifpack2::BlockRelaxation<ROW> prec (crsmatrix);

  Teuchos::ParameterList params;
  params.set("relaxation: container", "SparseILUT");
  params.set("relaxation: type", "Jacobi");
  params.set("partitioner: type","linear");
  params.set("partitioner: local parts",(int)num_rows_per_proc);
  prec.setParameters(params);

  prec.initialize();
  prec.compute();

  MV y (rowmap,2);
  y.putScalar (one);
  RCP<MV> xrcp = y.subViewNonConst (Teuchos::Range1D (0, 1));
  MV x = *xrcp;

  TEST_INEQUALITY(&x, &y);                                               // vector x and y are different

  x.sync_host ();
  y.sync_host ();
  auto x_lcl_host = x.getLocalViewHost ();
  auto y_lcl_host = x.getLocalViewHost ();

  TEST_EQUALITY( x_lcl_host.data (), y_lcl_host.data () ); // vector x and y are pointing to the same memory location (such test only works if num of local elements != 0)

  prec.apply(x, y);

  //y should be full of 0.5's now.
  {
    MV y_diff (y.getMap (), y.getNumVectors ());
    y_diff.putScalar (one / two);

    typedef typename MV::device_type device_type;
    typedef typename MV::mag_type mag_type;
    typedef Kokkos::View<mag_type*, device_type> norms_type;
    norms_type y_norms ("y_norms", y.getNumVectors ());

    y_diff.update (one, y, -one);
    y_diff.normInf (y_norms);

    auto y_norms_host = Kokkos::create_mirror_view (y_norms);
    Kokkos::deep_copy (y_norms_host, y_norms);

    for (size_t j = 0; j < y.getNumVectors (); ++j) {
      TEST_EQUALITY( y_norms_host(0), Kokkos::Details::ArithTraits<mag_type>::zero () );
    }
  }
}

// Note: Test3 from Ifpack2Relaxation test has been removed.
// Test apply() with "null" x and y. In parallel, it is possible that some nodes do not have any local elements.
// This never worked in Ifpack and won't work here either.

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
TEUCHOS_UNIT_TEST_TEMPLATE_3_DECL(Ifpack2BlockRelaxation, TriDi, Scalar, LocalOrdinal, GlobalOrdinal)
#else
TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL(Ifpack2BlockRelaxation, TriDi, Scalar)
#endif
{
#ifndef TPETRA_ENABLE_TEMPLATE_ORDINALS
  using LocalOrdinal = typename Tpetra::Map<>::local_ordinal_type;
  using GlobalOrdinal = typename Tpetra::Map<>::global_ordinal_type;
#endif
  std::string version = Ifpack2::Version();
  out << "Ifpack2::Version(): " << version << std::endl;

  global_size_t num_rows_per_proc = 5;

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  typedef Tpetra::CrsMatrix<Scalar,LocalOrdinal,GlobalOrdinal,Node> CRS;
  typedef Tpetra::RowMatrix<Scalar,LocalOrdinal,GlobalOrdinal,Node> ROW;
  typedef Tpetra::Vector<Scalar,LocalOrdinal,GlobalOrdinal,Node> Vector;
#else
  typedef Tpetra::CrsMatrix<Scalar,Node> CRS;
  typedef Tpetra::RowMatrix<Scalar,Node> ROW;
  typedef Tpetra::Vector<Scalar,Node> Vector;
#endif

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  const Teuchos::RCP<const Tpetra::Map<LocalOrdinal,GlobalOrdinal,Node> > rowmap = tif_utest::create_tpetra_map<LocalOrdinal,GlobalOrdinal,Node>(num_rows_per_proc);
  Teuchos::RCP<const CRS > crsmatrix = tif_utest::create_test_matrix_variable_blocking<Scalar,LocalOrdinal,GlobalOrdinal,Node>(rowmap);
#else
  const Teuchos::RCP<const Tpetra::Map<Node> > rowmap = tif_utest::create_tpetra_map<Node>(num_rows_per_proc);
  Teuchos::RCP<const CRS > crsmatrix = tif_utest::create_test_matrix_variable_blocking<Scalar,Node>(rowmap);
#endif

  /*** Fill RHS / LHS ***/
  Vector rhs(rowmap), lhs(rowmap), exact_soln(rowmap);
  rhs.putScalar(2.0);
  lhs.putScalar(0.0);
  exact_soln.putScalar(2.0);

  /* Setup Block Relaxation */
  int PartMap[5]={0,0,1,1,1};
  Teuchos::ArrayRCP<int> PMrcp(&PartMap[0],0,5,false);

  Teuchos::ParameterList ilist;
  ilist.set("partitioner: type","user");
  ilist.set("partitioner: map",PMrcp);
  ilist.set("partitioner: local parts",2);
  ilist.set("relaxation: container", "TriDi");
  ilist.set("relaxation: sweeps",1);
  ilist.set("relaxation: type","Gauss-Seidel");

  Ifpack2::BlockRelaxation<ROW> TDRelax(crsmatrix);

  TDRelax.setParameters(ilist);
  TDRelax.initialize();
  TDRelax.compute();
  TDRelax.apply(rhs,lhs);

  TEST_COMPARE_FLOATING_ARRAYS(lhs.get1dView(), exact_soln.get1dView(), 2*Teuchos::ScalarTraits<Scalar>::eps());
}

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
TEUCHOS_UNIT_TEST_TEMPLATE_3_DECL(Ifpack2BlockRelaxation, BandedContainer, Scalar, LocalOrdinal, GlobalOrdinal)
#else
TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL(Ifpack2BlockRelaxation, BandedContainer, Scalar)
#endif
{
#ifndef TPETRA_ENABLE_TEMPLATE_ORDINALS
  using LocalOrdinal = typename Tpetra::Map<>::local_ordinal_type;
  using GlobalOrdinal = typename Tpetra::Map<>::global_ordinal_type;
#endif
  std::string version = Ifpack2::Version();
  out << "Ifpack2::Version(): " << version << std::endl;

  global_size_t num_rows_per_proc = 5;

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  typedef Tpetra::CrsMatrix<Scalar,LocalOrdinal,GlobalOrdinal,Node> CRS;
  typedef Tpetra::RowMatrix<Scalar,LocalOrdinal,GlobalOrdinal,Node> ROW;
  typedef Tpetra::Vector<Scalar,LocalOrdinal,GlobalOrdinal,Node> Vector;
#else
  typedef Tpetra::CrsMatrix<Scalar,Node> CRS;
  typedef Tpetra::RowMatrix<Scalar,Node> ROW;
  typedef Tpetra::Vector<Scalar,Node> Vector;
#endif

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  const Teuchos::RCP<const Tpetra::Map<LocalOrdinal,GlobalOrdinal,Node> > rowmap = tif_utest::create_tpetra_map<LocalOrdinal,GlobalOrdinal,Node>(num_rows_per_proc);
  Teuchos::RCP<const CRS > crsmatrix = tif_utest::create_test_matrix_variable_banded<Scalar,LocalOrdinal,GlobalOrdinal,Node>(rowmap);
#else
  const Teuchos::RCP<const Tpetra::Map<Node> > rowmap = tif_utest::create_tpetra_map<Node>(num_rows_per_proc);
  Teuchos::RCP<const CRS > crsmatrix = tif_utest::create_test_matrix_variable_banded<Scalar,Node>(rowmap);
#endif

  /*** Fill RHS / LHS ***/
  Vector rhs(rowmap), lhs(rowmap), exact_soln(rowmap);
  rhs.putScalar(2.0);
  lhs.putScalar(0.0);
  exact_soln.putScalar(2.0);

  /* Setup Block Relaxation */
  int PartMap[5]={0,0,0,0,0};
  Teuchos::ArrayRCP<int> PMrcp(&PartMap[0],0,5,false);

  Teuchos::ParameterList ilist;
  ilist.set("partitioner: type","user");
  ilist.set("partitioner: map",PMrcp);
  ilist.set("partitioner: local parts",1);
  ilist.set("relaxation: container", "Banded");
  ilist.set("relaxation: sweeps",1);
  ilist.set("relaxation: type","Gauss-Seidel");

  Ifpack2::BlockRelaxation<ROW> TDRelax(crsmatrix);

  TDRelax.setParameters(ilist);
  TDRelax.initialize();
  TDRelax.compute();
  TDRelax.apply(rhs,lhs);

  TEST_COMPARE_FLOATING_ARRAYS(lhs.get1dView(), exact_soln.get1dView(), 2*Teuchos::ScalarTraits<Scalar>::eps());
}


#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
TEUCHOS_UNIT_TEST_TEMPLATE_3_DECL(Ifpack2BlockRelaxation, BlockedBandedContainer, Scalar, LocalOrdinal, GlobalOrdinal)
#else
TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL(Ifpack2BlockRelaxation, BlockedBandedContainer, Scalar)
#endif
{
#ifndef TPETRA_ENABLE_TEMPLATE_ORDINALS
  using LocalOrdinal = typename Tpetra::Map<>::local_ordinal_type;
  using GlobalOrdinal = typename Tpetra::Map<>::global_ordinal_type;
#endif
  std::string version = Ifpack2::Version();
  out << "Ifpack2::Version(): " << version << std::endl;

  global_size_t num_rows_per_proc = 5;

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  typedef Tpetra::CrsMatrix<Scalar,LocalOrdinal,GlobalOrdinal,Node> CRS;
  typedef Tpetra::RowMatrix<Scalar,LocalOrdinal,GlobalOrdinal,Node> ROW;
  typedef Tpetra::Vector<Scalar,LocalOrdinal,GlobalOrdinal,Node> Vector;
#else
  typedef Tpetra::CrsMatrix<Scalar,Node> CRS;
  typedef Tpetra::RowMatrix<Scalar,Node> ROW;
  typedef Tpetra::Vector<Scalar,Node> Vector;
#endif

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  const Teuchos::RCP<const Tpetra::Map<LocalOrdinal,GlobalOrdinal,Node> > rowmap = tif_utest::create_tpetra_map<LocalOrdinal,GlobalOrdinal,Node>(num_rows_per_proc);
  Teuchos::RCP<const CRS > crsmatrix = tif_utest::create_test_matrix_banded_variable_blocking<Scalar,LocalOrdinal,GlobalOrdinal,Node>(rowmap);
#else
  const Teuchos::RCP<const Tpetra::Map<Node> > rowmap = tif_utest::create_tpetra_map<Node>(num_rows_per_proc);
  Teuchos::RCP<const CRS > crsmatrix = tif_utest::create_test_matrix_banded_variable_blocking<Scalar,Node>(rowmap);
#endif

  /*** Fill RHS / LHS ***/
  Vector rhs(rowmap), lhs(rowmap), exact_soln(rowmap);
  rhs.putScalar(2.0);
  lhs.putScalar(0.0);
  exact_soln.putScalar(2.0);

  /* Setup Block Relaxation */
  int PartMap[5]={0,0,1,1,1};
  Teuchos::ArrayRCP<int> PMrcp(&PartMap[0],0,5,false);

  Teuchos::ParameterList ilist;
  ilist.set("partitioner: type","user");
  ilist.set("partitioner: map",PMrcp);
  ilist.set("partitioner: local parts",2);
  ilist.set("relaxation: container", "Banded");
  ilist.set("relaxation: sweeps",1);
  ilist.set("relaxation: type","Gauss-Seidel");

  Ifpack2::BlockRelaxation<ROW> TDRelax(crsmatrix);

  TDRelax.setParameters(ilist);
  TDRelax.initialize();
  TDRelax.compute();
  TDRelax.apply(rhs,lhs);

  TEST_COMPARE_FLOATING_ARRAYS(lhs.get1dView(), exact_soln.get1dView(), 2*Teuchos::ScalarTraits<Scalar>::eps());
}

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
TEUCHOS_UNIT_TEST_TEMPLATE_3_DECL(Ifpack2BlockRelaxation, LinePartition, Scalar, LocalOrdinal, GlobalOrdinal)
#else
TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL(Ifpack2BlockRelaxation, LinePartition, Scalar)
#endif
{
#ifndef TPETRA_ENABLE_TEMPLATE_ORDINALS
  using LocalOrdinal = typename Tpetra::Map<>::local_ordinal_type;
  using GlobalOrdinal = typename Tpetra::Map<>::global_ordinal_type;
#endif
  std::string version = Ifpack2::Version();
  out << "Ifpack2::Version(): " << version << std::endl;

  global_size_t num_rows_per_proc = 5;

  //typedef Tpetra::RowGraph<LocalOrdinal,GlobalOrdinal,Node> RG;
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  typedef Tpetra::CrsMatrix<Scalar,LocalOrdinal,GlobalOrdinal,Node> CRS;
  typedef Tpetra::RowMatrix<Scalar,LocalOrdinal,GlobalOrdinal,Node> ROW;
  typedef Tpetra::Vector<Scalar,LocalOrdinal,GlobalOrdinal,Node> Vector;
  typedef Tpetra::MultiVector<Scalar,LocalOrdinal,GlobalOrdinal,Node> MultiVector;
#else
  typedef Tpetra::CrsMatrix<Scalar,Node> CRS;
  typedef Tpetra::RowMatrix<Scalar,Node> ROW;
  typedef Tpetra::Vector<Scalar,Node> Vector;
  typedef Tpetra::MultiVector<Scalar,Node> MultiVector;
#endif

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  const Teuchos::RCP<const Tpetra::Map<LocalOrdinal,GlobalOrdinal,Node> > rowmap = tif_utest::create_tpetra_map<LocalOrdinal,GlobalOrdinal,Node>(num_rows_per_proc);
  Teuchos::RCP<const CRS > crsmatrix = tif_utest::create_test_matrix_variable_blocking<Scalar,LocalOrdinal,GlobalOrdinal,Node>(rowmap);
#else
  const Teuchos::RCP<const Tpetra::Map<Node> > rowmap = tif_utest::create_tpetra_map<Node>(num_rows_per_proc);
  Teuchos::RCP<const CRS > crsmatrix = tif_utest::create_test_matrix_variable_blocking<Scalar,Node>(rowmap);
#endif

  /*** Fill RHS / LHS ***/
  Vector rhs(rowmap), lhs(rowmap), exact_soln(rowmap);
  rhs.putScalar(2.0);
  lhs.putScalar(0.0);
  exact_soln.putScalar(2.0);

  /* Generate some fake coordinates */
  Teuchos::RCP<MultiVector> coord = rcp(new MultiVector(rowmap,2));
  Teuchos::ArrayRCP<Scalar> x0rcp = coord->getDataNonConst(0);
  Teuchos::ArrayRCP<Scalar> x1rcp = coord->getDataNonConst(1);

  Teuchos::ArrayView<Scalar> x0 = x0rcp();
  Teuchos::ArrayView<Scalar> x1 = x1rcp();
  x0[0]=0; x0[1]=1;   x0[2]=10;  x0[3]=11;  x0[4]=12;
  x1[0]=0; x1[1]=1;   x1[2]=2;   x1[3]=3;   x1[4]=4;


  /* Setup Block Relaxation */
  Teuchos::ParameterList ilist;
  ilist.set("partitioner: type","line");
  ilist.set("partitioner: coordinates",coord);
  ilist.set("partitioner: line detection threshold",0.5);
  ilist.set("relaxation: container", "TriDi");
  ilist.set("relaxation: sweeps",1);
  ilist.set("relaxation: type","Gauss-Seidel");

  Ifpack2::BlockRelaxation<ROW> TDRelax(crsmatrix);

  TDRelax.setParameters(ilist);
  TDRelax.initialize();
  TDRelax.compute();
  TDRelax.apply(rhs,lhs);

  // For debugging:
  // Teuchos::RCP<Ifpack2::Partitioner<RG> > MyPart = TDRelax.getPartitioner();
  // Teuchos::ArrayView<const LocalOrdinal> part = MyPart->nonOverlappingPartition();

  TEST_COMPARE_FLOATING_ARRAYS(lhs.get1dView(), exact_soln.get1dView(), 2*Teuchos::ScalarTraits<Scalar>::eps());
}

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
TEUCHOS_UNIT_TEST_TEMPLATE_3_DECL(Ifpack2BlockRelaxation, OverlappingPartition, Scalar, LocalOrdinal, GlobalOrdinal)
#else
TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL(Ifpack2BlockRelaxation, OverlappingPartition, Scalar)
#endif
{
#ifndef TPETRA_ENABLE_TEMPLATE_ORDINALS
  using LocalOrdinal = typename Tpetra::Map<>::local_ordinal_type;
  using GlobalOrdinal = typename Tpetra::Map<>::global_ordinal_type;
#endif
  // Test BlockRelaxation with user-provided blocks with overlap 1.
  // Convergence of block Gauss-Seidel is compared against that of point Gauss-Seidel,
  // and the test passes if the block residual norm is smaller at each iteration.
  // I've observed that there must be enough interior points for this relationship to hold.
  // Block Gauss-Seidel convergence using LAPACK is compared to block Gauss-Seidel using Amesos2,
  // and the test passes if the residual histories are identical.
  std::string version = Ifpack2::Version();
  out << "Ifpack2::Version(): " << version << std::endl;

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  typedef Tpetra::Map<LocalOrdinal,GlobalOrdinal,Node>                    map_type;
  typedef Tpetra::CrsMatrix<Scalar,LocalOrdinal,GlobalOrdinal,Node>       crs_matrix_type;
  typedef Tpetra::MultiVector<Scalar,LocalOrdinal,GlobalOrdinal,Node>     multivector_type;
  typedef Ifpack2::Preconditioner<Scalar,LocalOrdinal,GlobalOrdinal,Node> ifpack2_prec_type;
#else
  typedef Tpetra::Map<Node>                    map_type;
  typedef Tpetra::CrsMatrix<Scalar,Node>       crs_matrix_type;
  typedef Tpetra::MultiVector<Scalar,Node>     multivector_type;
  typedef Ifpack2::Preconditioner<Scalar,Node> ifpack2_prec_type;
#endif
  typedef Teuchos::ScalarTraits<Scalar> STS;
  Scalar zero = STS::zero(), one = STS::one();

  using Teuchos::RCP;
  using Teuchos::rcp;
  using Teuchos::Array;
  using Teuchos::ArrayRCP;
  using Teuchos::ParameterList;

  global_size_t num_rows_per_proc = 21;  // Must be odd.  For more than 4 processes, you must increase this number
                                         // or the test will fail.
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  const RCP<const map_type> rowmap = tif_utest::create_tpetra_map<LocalOrdinal,GlobalOrdinal,Node>(num_rows_per_proc);
  RCP<const crs_matrix_type > A = tif_utest::create_test_matrix<Scalar,LocalOrdinal,GlobalOrdinal,Node>(rowmap,-one);
#else
  const RCP<const map_type> rowmap = tif_utest::create_tpetra_map<Node>(num_rows_per_proc);
  RCP<const crs_matrix_type > A = tif_utest::create_test_matrix<Scalar,Node>(rowmap,-one);
#endif

  Array<Teuchos::ArrayRCP<LocalOrdinal>> customBlocks;

  int numLocalBlocks = 1 + (num_rows_per_proc-3)/2;
  out << "#local blocks = " << numLocalBlocks << std::endl;

  //out.setOutputToRootOnly(-1);
  //const int myRank = comm->getRank ();
  for (int i=0,j=0; i<numLocalBlocks; ++i) {
    ArrayRCP<LocalOrdinal> block(3);
    block[0] = j++;
    //out << "pid " << myRank << " block[" << i << ",0]="<< block[0] << std::endl;
    block[1] = j++;
    //out << "pid " << myRank << " block[" << i << ",1]="<< block[1] << std::endl;
    block[2] = j;
    //out << "pid " << myRank << " block[" << i << ",2]="<< block[2] << std::endl;
    customBlocks.push_back(block);
  }
  //out.setOutputToRootOnly(0);

  /* Setup Block Relaxation */
  ParameterList bList;  //block GS
  bList.set("partitioner: type", "user");
  bList.set("partitioner: parts", customBlocks);
  bList.set("relaxation: sweeps",1);
  bList.set("relaxation: type","Gauss-Seidel");
  ParameterList amesosList;
  bList.set("Amesos2",amesosList);

  int overlap = 0;
  RCP<ifpack2_prec_type> dbPrec = Ifpack2::Factory::create("BLOCK RELAXATION", A, overlap);
#if defined(HAVE_IFPACK2_AMESOS2)
  RCP<ifpack2_prec_type> sbPrec = Ifpack2::Factory::create("SPARSE BLOCK RELAXATION", A, overlap);
#endif
  RCP<ifpack2_prec_type> pPrec = Ifpack2::Factory::create("RELAXATION", A, overlap);
  ParameterList pList;  //point GS
  pList.set("relaxation: sweeps",1);
  pList.set("relaxation: type","Gauss-Seidel");

  RCP<multivector_type> B,X,Xact,Res;

  B = rcp( new multivector_type(rowmap,1));
  X = rcp( new multivector_type(rowmap,1));
  Xact = rcp( new multivector_type(rowmap,1));
  Res = rcp( new multivector_type(rowmap,1));

  //Xact->randomize();
  Xact->putScalar (Teuchos::ScalarTraits<Scalar>::one ());

  A->apply(*Xact, *B, Teuchos::NO_TRANS, one, zero);
  X->putScalar(zero);

  Array<typename STS::magnitudeType> pNorms(1), dbNorms(1);
#if defined(HAVE_IFPACK2_AMESOS2)
  Array<typename STS::magnitudeType> sbNorms(1);
#endif
  B->norm2(pNorms);
  //out << "||b|| = " << pNorms[0] << std::endl;
  Xact->norm2(pNorms);
  //out << "||xact|| = " << pNorms[0] << std::endl;

  out << "                      point GS, dense block GS";
#if defined(HAVE_IFPACK2_AMESOS2)
  out << ", sparse block GS";
#endif
  out << std::endl;

  for (int i=1; i<6; ++i) {

    pList.set("relaxation: sweeps",i);
    pPrec->setParameters(pList);
    pPrec->initialize();
    pPrec->compute();
    X->putScalar(zero);
    pPrec->apply(*B,*X);

    A->apply(*X, *Res, Teuchos::NO_TRANS, one, zero); //Res = A*X
    Res->update(one,*B,-one);                         //Res = B-Res
    Res->norm2(pNorms);

    bList.set("relaxation: sweeps",i);
    dbPrec->setParameters(bList);
    dbPrec->initialize();
    dbPrec->compute();
    X->putScalar(zero);
    dbPrec->apply(*B,*X);

    A->apply(*X, *Res, Teuchos::NO_TRANS, one, zero); //Res = A*X
    Res->update(one,*B,-one);                         //Res = B-Res
    Res->norm2(dbNorms);

#if defined(HAVE_IFPACK2_AMESOS2)
    sbPrec->setParameters(bList);
    sbPrec->initialize();
    sbPrec->compute();
    X->putScalar(zero);
    sbPrec->apply(*B,*X);

    A->apply(*X, *Res, Teuchos::NO_TRANS, one, zero); //Res = A*X
    Res->update(one,*B,-one);                         //Res = B-Res
    Res->norm2(sbNorms);
#endif

    out.precision(8);
    out << i << " sweeps : ||b-A*x|| = " << pNorms[0] << ", " << dbNorms[0];
#if defined(HAVE_IFPACK2_AMESOS2)
    out << ", " << sbNorms[0];
#endif
    out << std::endl;
    TEST_EQUALITY( dbNorms[0] < pNorms[0], true);
    out << dbNorms[0] << " < " << pNorms[0] << std::endl;
#if defined(HAVE_IFPACK2_AMESOS2)
    TEST_EQUALITY( sbNorms[0] < pNorms[0], true);
    TEST_EQUALITY( dbNorms[0] - sbNorms[0] < 1e-12, true);
#endif
  }

} //OverlappingPartition test

// Macro used inside the unit test below.  It tests for global error,
// and if so, prints each process' error message and quits the test
// early.
//
// 'out' only prints on Process 0.  It's really not OK for other
// processes to print to stdout, but it usually works and we need to
// do it for debugging.
#define IFPACK2BLOCKRELAXATION_REPORT_GLOBAL_ERR( WHAT_STRING ) do { \
  reduceAll<int, int> (*comm, REDUCE_MIN, lclSuccess, outArg (gblSuccess)); \
  TEST_EQUALITY_CONST( gblSuccess, 1 ); \
  if (gblSuccess != 1) { \
    out << WHAT_STRING << " FAILED on one or more processes!" << endl; \
    for (int p = 0; p < numProcs; ++p) { \
      if (myRank == p && lclSuccess != 1) { \
        std::cout << errStrm.str () << std::flush; \
      } \
      comm->barrier (); \
      comm->barrier (); \
      comm->barrier (); \
    } \
    std::cerr << "TEST FAILED; RETURNING EARLY" << endl; \
    return; \
  } \
} while (false)


#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
TEUCHOS_UNIT_TEST_TEMPLATE_3_DECL(Ifpack2BlockRelaxation, TestDiagonalBlockCrsMatrix, Scalar, LO, GO)
#else
TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL(Ifpack2BlockRelaxation, TestDiagonalBlockCrsMatrix, Scalar)
#endif
{
#ifndef TPETRA_ENABLE_TEMPLATE_ORDINALS
  using LO = typename Tpetra::Map<>::local_ordinal_type;
  using GO = typename Tpetra::Map<>::global_ordinal_type;
#endif
  // Create a block diagonal matrix
  // There are 5 blocks, and each one is [2 0 3; 3 2 0; 0 3 2]
  // Solve the linear system A x = ones(15,1)
  // Solution is 0.2*ones(15,1)
  using Teuchos::outArg;
  using Teuchos::RCP;
  using Teuchos::rcp;
  using Teuchos::REDUCE_MIN;
  using Teuchos::reduceAll;
  using std::endl;
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  typedef Tpetra::BlockCrsMatrix<Scalar,LO,GO,Node> block_crs_matrix_type;
  typedef Tpetra::BlockMultiVector<Scalar,LO,GO,Node> BMV;
  typedef Tpetra::RowMatrix<Scalar,LO,GO,Node> row_matrix_type;
  typedef Tpetra::CrsGraph<LO,GO,Node> crs_graph_type;
  typedef Tpetra::MultiVector<Scalar,LO,GO,Node> MV;
#else
  typedef Tpetra::BlockCrsMatrix<Scalar,Node> block_crs_matrix_type;
  typedef Tpetra::BlockMultiVector<Scalar,Node> BMV;
  typedef Tpetra::RowMatrix<Scalar,Node> row_matrix_type;
  typedef Tpetra::CrsGraph<Node> crs_graph_type;
  typedef Tpetra::MultiVector<Scalar,Node> MV;
#endif
  typedef Ifpack2::BlockRelaxation<row_matrix_type> prec_type;
  int lclSuccess = 1;
  int gblSuccess = 1;
  std::ostringstream errStrm; // for error collection

  out << "Ifpack2::BlockRelaxation diagonal block matrix test" << endl;

  RCP<const Teuchos::Comm<int> > comm = Tpetra::getDefaultComm ();
  const int myRank = comm->getRank ();
  const int numProcs = comm->getSize ();

  const int num_rows_per_proc = 5;
  const int blockSize = 3;
  RCP<crs_graph_type> crsgraph =
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    tif_utest::create_diagonal_graph<LO,GO,Node> (num_rows_per_proc);
#else
    tif_utest::create_diagonal_graph<Node> (num_rows_per_proc);
#endif

  RCP<block_crs_matrix_type> bcrsmatrix;
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  bcrsmatrix = Teuchos::rcp_const_cast<block_crs_matrix_type> (tif_utest::create_block_diagonal_matrix<Scalar,LO,GO,Node> (crsgraph, blockSize));
#else
  bcrsmatrix = Teuchos::rcp_const_cast<block_crs_matrix_type> (tif_utest::create_block_diagonal_matrix<Scalar,Node> (crsgraph, blockSize));
#endif

  RCP<prec_type> prec;
  try {
    prec = rcp (new prec_type (bcrsmatrix));
  } catch (std::exception& e) {
    lclSuccess = 0;
    errStrm << "Process " << myRank << ": Preconditioner constructor threw exception: "
            << e.what () << endl;
  }
  IFPACK2BLOCKRELAXATION_REPORT_GLOBAL_ERR( "Preconditioner constructor" );

  Teuchos::ParameterList params;
  params.set ("relaxation: type", "Jacobi");
  params.set ("partitioner: local parts", (LO)bcrsmatrix->getNodeNumRows());
  params.set ("relaxation: container", "Dense");

  try {
    prec->setParameters (params);
  } catch (std::exception& e) {
    lclSuccess = 0;
    errStrm << "Process " << myRank << ": prec->setParameters() threw exception: "
            << e.what () << endl;
  }
  IFPACK2BLOCKRELAXATION_REPORT_GLOBAL_ERR( "prec->setParameters()" );

  try {
    prec->initialize ();
  } catch (std::exception& e) {
    lclSuccess = 0;
    errStrm << "Process " << myRank << ": prec->initialize() threw exception: "
            << e.what () << endl;
  }
  IFPACK2BLOCKRELAXATION_REPORT_GLOBAL_ERR( "prec->initialize()" );

  try {
    prec->compute ();
  } catch (std::exception& e) {
    lclSuccess = 0;
    errStrm << "Process " << myRank << ": prec->compute() threw exception: "
            << e.what () << endl;
  }
  IFPACK2BLOCKRELAXATION_REPORT_GLOBAL_ERR( "prec->compute()" );

  BMV xBlock (*crsgraph->getRowMap (), blockSize, 1);
  BMV yBlock (*crsgraph->getRowMap (), blockSize, 1);
  MV x = xBlock.getMultiVectorView ();
  MV y = yBlock.getMultiVectorView ();
  x.putScalar (Teuchos::ScalarTraits<Scalar>::one ());

  TEST_EQUALITY(x.getMap()->getNodeNumElements(), blockSize*num_rows_per_proc);
  TEST_EQUALITY(y.getMap()->getNodeNumElements(), blockSize*num_rows_per_proc);

  try {
    prec->apply (x, y);
  } catch (std::exception& e) {
    lclSuccess = 0;
    errStrm << "Process " << myRank << ": prec->apply(x, y) threw exception: "
            << e.what () << endl;
  }
  IFPACK2BLOCKRELAXATION_REPORT_GLOBAL_ERR( "prec->apply(x, y)" );

  const Scalar exactSol = 0.2;

  yBlock.sync_host();
  for (int k = 0; k < num_rows_per_proc; ++k) {
    typename BMV::little_host_vec_type ylcl = yBlock.getLocalBlock(k,0);
    Scalar* yb = ylcl.data();
    for (int j = 0; j < blockSize; ++j) {
      TEST_FLOATING_EQUALITY(yb[j],exactSol,1e-14);
    }
  }
}

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
TEUCHOS_UNIT_TEST_TEMPLATE_3_DECL(Ifpack2BlockRelaxation, TestBlockContainers, Scalar, LO, GO)
#else
TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL(Ifpack2BlockRelaxation, TestBlockContainers, Scalar)
#endif
{
#ifndef TPETRA_ENABLE_TEMPLATE_ORDINALS
  using LO = typename Tpetra::Map<>::local_ordinal_type;
  using GO = typename Tpetra::Map<>::global_ordinal_type;
#endif
  // Create a block tridiagonal matrix (compatible with all 4 container types)
  using Teuchos::outArg;
  using Teuchos::RCP;
  using Teuchos::rcp;
  using Teuchos::REDUCE_MIN;
  using Teuchos::reduceAll;
  using std::endl;
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  typedef Tpetra::BlockMultiVector<Scalar,LO,GO,Node> BMV;
  typedef Tpetra::RowMatrix<Scalar,LO,GO,Node> row_matrix_type;
  typedef Tpetra::MultiVector<Scalar,LO,GO,Node> MV;
#else
  typedef Tpetra::BlockMultiVector<Scalar,Node> BMV;
  typedef Tpetra::RowMatrix<Scalar,Node> row_matrix_type;
  typedef Tpetra::MultiVector<Scalar,Node> MV;
#endif
  typedef Ifpack2::BlockRelaxation<row_matrix_type> prec_type;
  int lclSuccess = 1;
  int gblSuccess = 1;
  std::ostringstream errStrm; // for error collection

  out << "Ifpack2::BlockRelaxation tridiagonal block matrix test" << endl;

  RCP<const Teuchos::Comm<int> > comm = Tpetra::getDefaultComm ();
  const int myRank = comm->getRank ();
  const int numProcs = comm->getSize ();

  const int num_rows_per_proc = 8;
  const int blockSize = 3;

  //the (block) graph should be diagonal
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  auto crsgraph = tif_utest::create_banded_graph<LO,GO,Node>(num_rows_per_proc, 2);
#else
  auto crsgraph = tif_utest::create_banded_graph<Node>(num_rows_per_proc, 2);
#endif

  auto bcrsmatrix = Teuchos::rcp(new
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
      Tpetra::BlockCrsMatrix<Scalar,LO,GO,Node>(*crsgraph, blockSize));
#else
      Tpetra::BlockCrsMatrix<Scalar,Node>(*crsgraph, blockSize));
#endif
  
  //Fill in values of the the matrix
  for(LO l_row = 0; (size_t) l_row < bcrsmatrix->getNodeNumRows(); ++l_row)
  {
    const LO * inds;
    Scalar * vals;
    LO numInd;
    bcrsmatrix->getLocalRowView(l_row, inds, vals, numInd);
    for(int k = 0; k < blockSize * blockSize * numInd; k++)
      vals[k] = 0;
    for (LO j = 0; j < numInd; ++j)
    {
      const LO lcl_col = inds[j];
      for(int bc = 0; bc < blockSize; bc++)
      {
        for(int br = 0; br < blockSize; br++)
        {
          //copy the same tridiagonal stencil onto every block,
          //but for off-diagonal blocks scale it down by 0.4
          if(std::abs(bc - br) > 1)
            continue;
          Scalar val = (bc == br) ? 4 : -1;
          if(lcl_col != l_row)
            val *= pow(2, -2.0 - std::abs(lcl_col - l_row));
          vals[j * blockSize * blockSize + bc * blockSize + br] = val;
        }
      }
    }
  }

  typedef Teuchos::ScalarTraits<Scalar> STS;
  typedef typename STS::magnitudeType Magnitude;
  Teuchos::Array<typename STS::magnitudeType> resNorms(2);

  std::vector<const char*> containerTypes = {"Dense", "TriDi", "Banded"};
  //Record the residual norm ||Ax-b|| to make sure the three container types agree
  std::vector<Magnitude> containerNorms;

  for(auto contType : containerTypes)
  {
    RCP<prec_type> prec;
    try {
      prec = rcp (new prec_type (bcrsmatrix));
    } catch (std::exception& e) {
      lclSuccess = 0;
      errStrm << "Process " << myRank << ": Preconditioner constructor threw exception: "
              << e.what () << endl;
    }
    IFPACK2BLOCKRELAXATION_REPORT_GLOBAL_ERR( "Preconditioner constructor" );

    Teuchos::ParameterList params;
    params.set ("relaxation: type", "Jacobi");
    params.set ("partitioner: local parts", (LO)bcrsmatrix->getNodeNumRows());
    params.set ("relaxation: sweeps", 10);
    params.set ("relaxation: container", contType);

    try {
      prec->setParameters (params);
    } catch (std::exception& e) {
      lclSuccess = 0;
      errStrm << "Process " << myRank << ": prec->setParameters() threw exception: "
              << e.what () << endl;
    }
    IFPACK2BLOCKRELAXATION_REPORT_GLOBAL_ERR( "prec->setParameters()" );

    try {
      prec->initialize ();
    } catch (std::exception& e) {
      lclSuccess = 0;
      errStrm << "Process " << myRank << ": prec->initialize() threw exception: "
              << e.what () << endl;
    }
    IFPACK2BLOCKRELAXATION_REPORT_GLOBAL_ERR( "prec->initialize()" );

    try {
      prec->compute ();
    } catch (std::exception& e) {
      lclSuccess = 0;
      errStrm << "Process " << myRank << ": prec->compute() threw exception: "
              << e.what () << endl;
    }
    IFPACK2BLOCKRELAXATION_REPORT_GLOBAL_ERR( "prec->compute()" );

    BMV xBlock (*crsgraph->getRowMap (), blockSize, 2);
    BMV yBlock (*crsgraph->getRowMap (), blockSize, 2);
    MV x = xBlock.getMultiVectorView ();
    MV y = yBlock.getMultiVectorView ();
    {
      for(int v = 0; v < 2; v++)
      {
        auto xdata = x.getDataNonConst(v);
        for(size_t i = 0; i < (size_t) xdata.size(); i++)
          xdata[i] = (Scalar) i;
      }
    }
    
    TEST_EQUALITY(x.getMap()->getNodeNumElements(), blockSize*num_rows_per_proc);
    TEST_EQUALITY(y.getMap()->getNodeNumElements(), blockSize*num_rows_per_proc);

    try {
      prec->apply (x, y);
    } catch (std::exception& e) {
      lclSuccess = 0;
      errStrm << "Process " << myRank << ": prec->apply(x, y) threw exception: "
              << e.what () << endl;
    }
    IFPACK2BLOCKRELAXATION_REPORT_GLOBAL_ERR( "prec->apply(x, y)" );

    bcrsmatrix->apply(x, y, Teuchos::NO_TRANS, 1.0, -1.0);
    y.norm2(resNorms());
    containerNorms.push_back(resNorms[0]);
    containerNorms.push_back(resNorms[1]);
  }
  for(size_t i = 1; i < containerNorms.size(); i++)
  {
    TEST_FLOATING_EQUALITY(containerNorms[0], containerNorms[i], 1e-7);
  }
}

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
TEUCHOS_UNIT_TEST_TEMPLATE_3_DECL(Ifpack2BlockRelaxation, TestBlockContainersDecoupled, Scalar, LO, GO)
#else
TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL(Ifpack2BlockRelaxation, TestBlockContainersDecoupled, Scalar)
#endif
{
#ifndef TPETRA_ENABLE_TEMPLATE_ORDINALS
  using LO = typename Tpetra::Map<>::local_ordinal_type;
  using GO = typename Tpetra::Map<>::global_ordinal_type;
#endif
  // Create a block tridiagonal matrix (compatible with all 4 container types)
  // The values in each block are different (not just a repeated stencil - decoupled blocks must be nonsingular)
  using Teuchos::outArg;
  using Teuchos::RCP;
  using Teuchos::rcp;
  using Teuchos::REDUCE_MIN;
  using Teuchos::reduceAll;
  using std::endl;
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  typedef Tpetra::BlockMultiVector<Scalar,LO,GO,Node> BMV;
  typedef Tpetra::RowMatrix<Scalar,LO,GO,Node> row_matrix_type;
  typedef Tpetra::MultiVector<Scalar,LO,GO,Node> MV;
#else
  typedef Tpetra::BlockMultiVector<Scalar,Node> BMV;
  typedef Tpetra::RowMatrix<Scalar,Node> row_matrix_type;
  typedef Tpetra::MultiVector<Scalar,Node> MV;
#endif
  typedef Ifpack2::BlockRelaxation<row_matrix_type> prec_type;
  int lclSuccess = 1;
  int gblSuccess = 1;
  std::ostringstream errStrm; // for error collection

  out << "Ifpack2::BlockRelaxation decoupled block matrix test" << endl;

  RCP<const Teuchos::Comm<int> > comm = Tpetra::getDefaultComm ();
  const int myRank = comm->getRank ();
  const int numProcs = comm->getSize ();

  const int num_rows_per_proc = 8;
  const int blockSize = 3;

  //the (block) graph should be diagonal
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  auto crsgraph = tif_utest::create_banded_graph<LO,GO,Node>(num_rows_per_proc, 1);
#else
  auto crsgraph = tif_utest::create_banded_graph<Node>(num_rows_per_proc, 1);
#endif

  auto bcrsmatrix = Teuchos::rcp(new
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
      Tpetra::BlockCrsMatrix<Scalar,LO,GO,Node>(*crsgraph, blockSize));
#else
      Tpetra::BlockCrsMatrix<Scalar,Node>(*crsgraph, blockSize));
#endif
  
  //Fill in values of the the matrix
  for(LO l_row = 0; (size_t) l_row < bcrsmatrix->getNodeNumRows(); ++l_row)
  {
    const LO * inds;
    Scalar * vals;
    LO numInd;
    bcrsmatrix->getLocalRowView(l_row, inds, vals, numInd);
    for (LO j = 0; j < numInd; ++j)
    {
      const LO lcl_col = inds[j];
      for(int bc = 0; bc < blockSize; bc++)
      {
        for(int br = 0; br < blockSize; br++)
        {
          Scalar val = 10 / pow(2, std::abs(l_row - lcl_col) + std::abs(bc - br));
          vals[j * blockSize * blockSize + bc * blockSize + br] = val;
        }
      }
    }
  }

  typedef Teuchos::ScalarTraits<Scalar> STS;
  typedef typename STS::magnitudeType Magnitude;
  Teuchos::Array<typename STS::magnitudeType> resNorms(2);

  std::vector<const char*> containerTypes = {"Dense", "TriDi", "Banded"};
  //Record the residual norm ||Ax-b|| to make sure the three container types agree
  std::vector<Magnitude> containerNorms;

  for(auto contType : containerTypes)
  {
    RCP<prec_type> prec;
    try {
      prec = rcp (new prec_type (bcrsmatrix));
    } catch (std::exception& e) {
      lclSuccess = 0;
      errStrm << "Process " << myRank << ": Preconditioner constructor threw exception: "
              << e.what () << endl;
    }
    IFPACK2BLOCKRELAXATION_REPORT_GLOBAL_ERR( "Preconditioner constructor" );

    Teuchos::ParameterList params;
    params.set ("relaxation: type", "Jacobi");
    params.set ("partitioner: local parts", (LO)bcrsmatrix->getNodeNumRows() / 2);
    params.set ("relaxation: sweeps", 10);
    params.set ("relaxation: container", contType);
    params.set ("block relaxation: decouple dofs", true);

    try {
      prec->setParameters (params);
    } catch (std::exception& e) {
      lclSuccess = 0;
      errStrm << "Process " << myRank << ": prec->setParameters() threw exception: "
              << e.what () << endl;
    }
    IFPACK2BLOCKRELAXATION_REPORT_GLOBAL_ERR( "prec->setParameters()" );

    try {
      prec->initialize ();
    } catch (std::exception& e) {
      lclSuccess = 0;
      errStrm << "Process " << myRank << ": prec->initialize() threw exception: "
              << e.what () << endl;
    }
    IFPACK2BLOCKRELAXATION_REPORT_GLOBAL_ERR( "prec->initialize()" );

    try {
      prec->compute ();
    } catch (std::exception& e) {
      lclSuccess = 0;
      errStrm << "Process " << myRank << ": prec->compute() threw exception: "
              << e.what () << endl;
    }
    IFPACK2BLOCKRELAXATION_REPORT_GLOBAL_ERR( "prec->compute()" );

    BMV xBlock (*crsgraph->getRowMap (), blockSize, 2);
    BMV yBlock (*crsgraph->getRowMap (), blockSize, 2);
    MV x = xBlock.getMultiVectorView ();
    MV y = yBlock.getMultiVectorView ();
    {
      for(int v = 0; v < 2; v++)
      {
        auto xdata = x.getDataNonConst(v);
        for(size_t i = 0; i < (size_t) xdata.size(); i++)
          xdata[i] = (Scalar) i;
      }
    }
    
    TEST_EQUALITY(x.getMap()->getNodeNumElements(), blockSize*num_rows_per_proc);
    TEST_EQUALITY(y.getMap()->getNodeNumElements(), blockSize*num_rows_per_proc);
    try {
      prec->apply (x, y);
    } catch (std::exception& e) {
      lclSuccess = 0;
      errStrm << "Process " << myRank << ": prec->apply(x, y) threw exception: "
              << e.what () << endl;
    }
    IFPACK2BLOCKRELAXATION_REPORT_GLOBAL_ERR( "prec->apply(x, y)" );
    bcrsmatrix->apply(x, y, Teuchos::NO_TRANS, 1.0, -1.0);
    y.norm2(resNorms());
    containerNorms.push_back(resNorms[0]);
    containerNorms.push_back(resNorms[1]);
  }
  for(size_t i = 1; i < containerNorms.size(); i++)
  {
    TEST_FLOATING_EQUALITY(containerNorms[0], containerNorms[i], 1e-7);
  }
}

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
TEUCHOS_UNIT_TEST_TEMPLATE_3_DECL(Ifpack2BlockRelaxation, TestContainersDofsDecoupled, Scalar, LO, GO)
#else
TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL(Ifpack2BlockRelaxation, TestContainersDofsDecoupled, Scalar)
#endif
{
#ifndef TPETRA_ENABLE_TEMPLATE_ORDINALS
  using LO = typename Tpetra::Map<>::local_ordinal_type;
  using GO = typename Tpetra::Map<>::global_ordinal_type;
#endif
  // Create a regular CrsMatrix that is (entrywise) identical to the matrix used in the BlockContainersDecoupled test
  using Teuchos::outArg;
  using Teuchos::RCP;
  using Teuchos::rcp;
  using Teuchos::REDUCE_MIN;
  using Teuchos::reduceAll;
  using std::endl;
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  typedef Tpetra::RowMatrix<Scalar,LO,GO,Node> row_matrix_type;
  typedef Tpetra::CrsMatrix<Scalar,LO,GO,Node> crs_matrix_type;
  typedef Tpetra::MultiVector<Scalar,LO,GO,Node> MV;
#else
  typedef Tpetra::RowMatrix<Scalar,Node> row_matrix_type;
  typedef Tpetra::CrsMatrix<Scalar,Node> crs_matrix_type;
  typedef Tpetra::MultiVector<Scalar,Node> MV;
#endif
  typedef Ifpack2::BlockRelaxation<row_matrix_type> prec_type;
  int lclSuccess = 1;
  int gblSuccess = 1;
  std::ostringstream errStrm; // for error collection

  out << "Ifpack2::BlockRelaxation decoupled block matrix test" << endl;

  RCP<const Teuchos::Comm<int> > comm = Tpetra::getDefaultComm ();
  const int myRank = comm->getRank ();
  const int numProcs = comm->getSize ();

  const int nodesPerProc = 8;
  const int dofsPerNode = 3;
  const int rowsPerProc = nodesPerProc * dofsPerNode;

  auto INVALID = Teuchos::OrdinalTraits<Tpetra::global_size_t>::invalid();

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  auto rowmap = Teuchos::rcp(new Tpetra::Map<LO, GO, Node>(INVALID, rowsPerProc, 0, comm));  //0 is indexBase
#else
  auto rowmap = Teuchos::rcp(new Tpetra::Map<Node>(INVALID, rowsPerProc, 0, comm));  //0 is indexBase
#endif

  const size_t maxNumEntPerRow = dofsPerNode * 3;     //Node graph is tridiagonal, but graph within nodes is complete

  auto matrix = Teuchos::rcp(new crs_matrix_type(rowmap, maxNumEntPerRow));

  //Fill in the matrix one row at a time (on each process)
  Teuchos::Array<GO> colInds;
  Teuchos::Array<Scalar> values;

  for(LO lclRowInd = 0; lclRowInd < rowsPerProc; lclRowInd++)
  {
    colInds.clear();
    values.clear();
    GO gblRowInd = rowmap->getGlobalElement(lclRowInd);
    for (LO blk = 0; blk < 3; blk++)  //3 is degree of each node (tridiagonal graph)
    {
      GO gblBlockCol = (gblRowInd / dofsPerNode + blk - 1);
      if(gblBlockCol < 0 || gblBlockCol >= nodesPerProc * comm->getSize())
        continue;
      for(LO col = 0; col < dofsPerNode; col++)
      {
        GO gblCol = gblBlockCol * dofsPerNode + col;
        LO blockDiff = std::abs(blk - 1);
        LO rowDiff = std::abs(gblRowInd % dofsPerNode - gblCol % dofsPerNode);
        values.push_back(10 / pow(2, blockDiff + rowDiff));
        colInds.push_back(gblCol);
      }
    }
    matrix->insertGlobalValues(gblRowInd, colInds(), values());
  }
  matrix->fillComplete ();

  typedef Teuchos::ScalarTraits<Scalar> STS;
  typedef typename STS::magnitudeType Magnitude;
  Teuchos::Array<typename STS::magnitudeType> resNorms(2);

  std::vector<const char*> containerTypes = {"Dense", "TriDi", "Banded"};
  //Record the residual norm ||Ax-b|| to make sure the three container types agree
  std::vector<Magnitude> containerNorms;

  for(auto contType : containerTypes)
  {
    RCP<prec_type> prec;
    try {
      prec = rcp (new prec_type (matrix));
    } catch (std::exception& e) {
      lclSuccess = 0;
      errStrm << "Process " << myRank << ": Preconditioner constructor threw exception: "
              << e.what () << endl;
    }
    IFPACK2BLOCKRELAXATION_REPORT_GLOBAL_ERR( "Preconditioner constructor" );

    Teuchos::ParameterList params;
    params.set ("relaxation: type", "Jacobi");
    params.set ("partitioner: local parts", (LO) matrix->getNodeNumRows() / 2 / dofsPerNode);
    params.set ("relaxation: sweeps", 10);
    params.set ("relaxation: container", contType);
    params.set ("block relaxation: decouple dofs", true);
    params.set ("partitioner: PDE equations", dofsPerNode);

    try {
      prec->setParameters (params);
    } catch (std::exception& e) {
      lclSuccess = 0;
      errStrm << "Process " << myRank << ": prec->setParameters() threw exception: "
              << e.what () << endl;
    }
    IFPACK2BLOCKRELAXATION_REPORT_GLOBAL_ERR( "prec->setParameters()" );

    try {
      prec->initialize ();
    } catch (std::exception& e) {
      lclSuccess = 0;
      errStrm << "Process " << myRank << ": prec->initialize() threw exception: "
              << e.what () << endl;
    }
    IFPACK2BLOCKRELAXATION_REPORT_GLOBAL_ERR( "prec->initialize()" );

    try {
      prec->compute ();
    } catch (std::exception& e) {
      lclSuccess = 0;
      errStrm << "Process " << myRank << ": prec->compute() threw exception: "
              << e.what () << endl;
    }
    IFPACK2BLOCKRELAXATION_REPORT_GLOBAL_ERR( "prec->compute()" );

    MV x(matrix->getRowMap(), 2);
    MV y(matrix->getRowMap(), 2);
    for(int v = 0; v < 2; v++)
    {
      auto xdata = x.getDataNonConst(v);
      for(size_t i = 0; i < (size_t) xdata.size(); i++)
        xdata[i] = (Scalar) i;
    }
    TEST_EQUALITY(x.getMap()->getNodeNumElements(), rowsPerProc);
    TEST_EQUALITY(y.getMap()->getNodeNumElements(), rowsPerProc);
    try {
      prec->apply (x, y);
    } catch (std::exception& e) {
      lclSuccess = 0;
      errStrm << "Process " << myRank << ": prec->apply(x, y) threw exception: "
              << e.what () << endl;
    }
    IFPACK2BLOCKRELAXATION_REPORT_GLOBAL_ERR( "prec->apply(x, y)" );
    matrix->apply(x, y, Teuchos::NO_TRANS, 1.0, -1.0);
    y.norm2(resNorms());
    containerNorms.push_back(resNorms[0]);
    containerNorms.push_back(resNorms[1]);
  }
  for(size_t i = 1; i < containerNorms.size(); i++)
  {
    TEST_FLOATING_EQUALITY(containerNorms[0], containerNorms[i], 1e-7);
  }
}

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
TEUCHOS_UNIT_TEST_TEMPLATE_3_DECL(Ifpack2BlockRelaxation, TestLowerTriangularBlockCrsMatrix, Scalar, LO, GO)
#else
TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL(Ifpack2BlockRelaxation, TestLowerTriangularBlockCrsMatrix, Scalar)
#endif
{
#ifndef TPETRA_ENABLE_TEMPLATE_ORDINALS
  using LO = typename Tpetra::Map<>::local_ordinal_type;
  using GO = typename Tpetra::Map<>::global_ordinal_type;
#endif
  // Create a block lower triangular matrix
  // Matrix is [2*I_5 0 0; 3*I_5 2*I_ 0; I_5 3*I_5 2*I_5]
  // Solve the linear system A x = ones(15,1)
  // Solution is [0.5*ones(5,1); -0.25*ones(5,1); 0.625*ones(5,1)]
  using Teuchos::outArg;
  using Teuchos::RCP;
  using Teuchos::REDUCE_MIN;
  using Teuchos::reduceAll;
  using std::endl;
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  typedef Tpetra::BlockCrsMatrix<Scalar,LO,GO,Node> block_crs_matrix_type;
  typedef Tpetra::CrsGraph<LO,GO,Node> crs_graph_type;
  typedef Tpetra::RowMatrix<Scalar,LO,GO,Node> row_matrix_type;
  typedef Tpetra::BlockMultiVector<Scalar,LO,GO,Node> BMV;
  typedef Tpetra::MultiVector<Scalar,LO,GO,Node> MV;
#else
  typedef Tpetra::BlockCrsMatrix<Scalar,Node> block_crs_matrix_type;
  typedef Tpetra::CrsGraph<Node> crs_graph_type;
  typedef Tpetra::RowMatrix<Scalar,Node> row_matrix_type;
  typedef Tpetra::BlockMultiVector<Scalar,Node> BMV;
  typedef Tpetra::MultiVector<Scalar,Node> MV;
#endif
  typedef Ifpack2::BlockRelaxation<row_matrix_type> prec_type;
  int lclSuccess = 1;
  int gblSuccess = 1;
  std::ostringstream errStrm; // for error collection

  out << "Ifpack2::BlockRelaxation lower triangular BlockCrsMatrix test" << endl;

  RCP<const Teuchos::Comm<int> > comm = Tpetra::getDefaultComm();
  const int myRank = comm->getRank ();
  const int numProcs = comm->getSize ();

  const size_t num_rows_per_proc = 3;
  RCP<crs_graph_type> crsgraph;
  try {
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    crsgraph = tif_utest::create_dense_local_graph<LO, GO, Node> (num_rows_per_proc);
#else
    crsgraph = tif_utest::create_dense_local_graph<Node> (num_rows_per_proc);
#endif
  } catch (std::exception& e) {
    lclSuccess = 0;
    errStrm << "Process " << myRank << ": create_dense_local_graph threw exception: "
            << e.what () << endl;
  }
  IFPACK2BLOCKRELAXATION_REPORT_GLOBAL_ERR( "create_dense_local_graph" );

  const int blockSize = 5;
  RCP<block_crs_matrix_type> bcrsmatrix;
  try {
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    bcrsmatrix = Teuchos::rcp_const_cast<block_crs_matrix_type> (tif_utest::create_triangular_matrix<Scalar, LO, GO, Node, true> (crsgraph, blockSize));
#else
    bcrsmatrix = Teuchos::rcp_const_cast<block_crs_matrix_type> (tif_utest::create_triangular_matrix<Scalar, Node, true> (crsgraph, blockSize));
#endif
  } catch (std::exception& e) {
    lclSuccess = 0;
    errStrm << "Process " << myRank << ": create_triangular_matrix threw exception: "
            << e.what () << endl;
  }
  IFPACK2BLOCKRELAXATION_REPORT_GLOBAL_ERR( "create_triangular_matrix" );

  RCP<prec_type> prec;
  try {
    prec = rcp (new prec_type (bcrsmatrix));
  } catch (std::exception& e) {
    lclSuccess = 0;
    errStrm << "Process " << myRank << ": Preconditioner constructor threw exception: "
            << e.what () << endl;
  }
  IFPACK2BLOCKRELAXATION_REPORT_GLOBAL_ERR( "Preconditioner constructor" );

  Teuchos::ParameterList params;
  params.set ("relaxation: type", "Gauss-Seidel");
  params.set ("partitioner: local parts", static_cast<LO> (bcrsmatrix->getNodeNumRows ()));
  params.set ("relaxation: container", "Dense");

  try {
    prec->setParameters (params);
  } catch (std::exception& e) {
    lclSuccess = 0;
    errStrm << "Process " << myRank << ": prec->setParameters() threw exception: "
            << e.what () << endl;
  }
  IFPACK2BLOCKRELAXATION_REPORT_GLOBAL_ERR( "prec->setParameters()" );

  try {
    prec->initialize ();
  } catch (std::exception& e) {
    lclSuccess = 0;
    errStrm << "Process " << myRank << ": prec->initialize() threw exception: "
            << e.what () << endl;
  }
  IFPACK2BLOCKRELAXATION_REPORT_GLOBAL_ERR( "prec->initialize()" );

  try {
    prec->compute ();
  } catch (std::exception& e) {
    lclSuccess = 0;
    errStrm << "Process " << myRank << ": prec->compute() threw exception: "
            << e.what () << endl;
  }
  IFPACK2BLOCKRELAXATION_REPORT_GLOBAL_ERR( "prec->compute()" );

  BMV xBlock (* (crsgraph->getRowMap ()), blockSize, 1);
  BMV yBlock (* (crsgraph->getRowMap ()), blockSize, 1);
  MV x = xBlock.getMultiVectorView ();
  MV y = yBlock.getMultiVectorView ();
  x.putScalar (Teuchos::ScalarTraits<Scalar>::one ());

  TEST_EQUALITY( x.getMap()->getNodeNumElements (), blockSize * num_rows_per_proc );
  TEST_EQUALITY( y.getMap ()->getNodeNumElements (), blockSize * num_rows_per_proc );

  try {
    prec->apply (x, y);
  } catch (std::exception& e) {
    lclSuccess = 0;
    errStrm << "Process " << myRank << ": prec->apply(x, y) threw exception: "
            << e.what () << endl;
  }
  IFPACK2BLOCKRELAXATION_REPORT_GLOBAL_ERR( "prec->apply(x, y)" );

  Teuchos::Array<Scalar> exactSol(num_rows_per_proc);
  exactSol[0] = 0.5;
  exactSol[1] = -0.25;
  exactSol[2] = 0.625;

  yBlock.sync_host();
  for (size_t k = 0; k < num_rows_per_proc; ++k) {
    LO lcl_row = k;
    typename BMV::little_host_vec_type ylcl = yBlock.getLocalBlock(lcl_row,0);
    Scalar* yb = ylcl.data();
    for (int j = 0; j < blockSize; ++j) {
      TEST_FLOATING_EQUALITY(yb[j],exactSol[k],1e-14);
    }
  }
}

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
TEUCHOS_UNIT_TEST_TEMPLATE_3_DECL(Ifpack2BlockRelaxation, TestUpperTriangularBlockCrsMatrix, Scalar, LocalOrdinal, GlobalOrdinal)
#else
TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL(Ifpack2BlockRelaxation, TestUpperTriangularBlockCrsMatrix, Scalar)
#endif
{
#ifndef TPETRA_ENABLE_TEMPLATE_ORDINALS
  using LocalOrdinal = typename Tpetra::Map<>::local_ordinal_type;
  using GlobalOrdinal = typename Tpetra::Map<>::global_ordinal_type;
#endif
  // Create a block lower triangular matrix
  // Matrix is [2*I_5 3*I_5 I_5; 0 2*I_5 3*I_5; 0 0 2*I_5]
  // Solve the linear system A x = ones(15,1)
  // Solution is [0.625*ones(5,1); -0.25*ones(5,1); 0.5*ones(5,1)]
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  typedef Tpetra::BlockCrsMatrix<Scalar,LocalOrdinal,GlobalOrdinal,Node> block_crs_matrix_type;
  typedef Tpetra::RowMatrix<Scalar,LocalOrdinal,GlobalOrdinal,Node> row_matrix_type;
  typedef Tpetra::BlockMultiVector<Scalar,LocalOrdinal,GlobalOrdinal,Node> BMV;
  typedef Tpetra::MultiVector<Scalar,LocalOrdinal,GlobalOrdinal,Node> MV;
#else
  typedef Tpetra::BlockCrsMatrix<Scalar,Node> block_crs_matrix_type;
  typedef Tpetra::RowMatrix<Scalar,Node> row_matrix_type;
  typedef Tpetra::BlockMultiVector<Scalar,Node> BMV;
  typedef Tpetra::MultiVector<Scalar,Node> MV;
#endif

  out << "Ifpack2::Version(): " << Ifpack2::Version () << std::endl;

  const int num_rows_per_proc = 3;
  const int blockSize = 5;

  auto comm = Tpetra::getDefaultComm();
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  Teuchos::RCP<Tpetra::CrsGraph<LocalOrdinal,GlobalOrdinal,Node> > crsgraph =
    tif_utest::create_dense_local_graph<LocalOrdinal,GlobalOrdinal,Node>(num_rows_per_proc);
#else
  Teuchos::RCP<Tpetra::CrsGraph<Node> > crsgraph =
    tif_utest::create_dense_local_graph<Node>(num_rows_per_proc);
#endif
  Teuchos::RCP<block_crs_matrix_type> bcrsmatrix =
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    Teuchos::rcp_const_cast<block_crs_matrix_type> (tif_utest::create_triangular_matrix<Scalar,LocalOrdinal,GlobalOrdinal,Node,false> (crsgraph, blockSize));
#else
    Teuchos::rcp_const_cast<block_crs_matrix_type> (tif_utest::create_triangular_matrix<Scalar,Node,false> (crsgraph, blockSize));
#endif

//  Teuchos::RCP<Teuchos::FancyOStream> wrappedStream = Teuchos::getFancyOStream (Teuchos::rcpFromRef (std::cout));
//  bcrsmatrix->describe (*wrappedStream, Teuchos::VERB_EXTREME);
//  bcrsmatrix->getGraph()->describe (*wrappedStream, Teuchos::VERB_EXTREME);

  Ifpack2::BlockRelaxation<row_matrix_type> prec (bcrsmatrix);

  Teuchos::ParameterList params;
  params.set("relaxation: container", "Dense");
  params.set("relaxation: type", "Symmetric Gauss-Seidel");
  params.set("partitioner: local parts", (LocalOrdinal)bcrsmatrix->getNodeNumRows());
  prec.setParameters(params);

  prec.initialize();
  TEST_NOTHROW(prec.compute());

  BMV xBlock (*crsgraph->getRowMap (), blockSize, 1);
  BMV yBlock (*crsgraph->getRowMap (), blockSize, 1);
  MV x = xBlock.getMultiVectorView ();
  MV y = yBlock.getMultiVectorView ();
  x.putScalar (Teuchos::ScalarTraits<Scalar>::one ());

  TEST_EQUALITY(x.getMap()->getNodeNumElements(), blockSize*num_rows_per_proc);
  TEST_EQUALITY(y.getMap()->getNodeNumElements(), blockSize*num_rows_per_proc);
  TEST_NOTHROW(prec.apply(x, y));

  Teuchos::Array<Scalar> exactSol(num_rows_per_proc);
  exactSol[0] = 0.625;
  exactSol[1] = -0.25;
  exactSol[2] = 0.5;

  yBlock.sync_host();
  for (int k = 0; k < num_rows_per_proc; ++k) {
    typename BMV::little_host_vec_type ylcl = yBlock.getLocalBlock(k,0);
    auto yb = ylcl.data();
    for (int j = 0; j < blockSize; ++j) {
      TEST_FLOATING_EQUALITY(yb[j],exactSol[k],1e-14);
    }
  }
}



#define UNIT_TEST_GROUP_SCALAR_ORDINAL(Scalar,LocalOrdinal,GlobalOrdinal) \
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  TEUCHOS_UNIT_TEST_TEMPLATE_3_INSTANT( Ifpack2BlockRelaxation, Test0,                             Scalar, LocalOrdinal,GlobalOrdinal) \
#else
  TEUCHOS_UNIT_TEST_TEMPLATE_1_INSTANT( Ifpack2BlockRelaxation, Test0,                             Scalar) \
#endif
  TEUCHOS_UNIT_TEST_TEMPLATE_3_INSTANT( Ifpack2BlockRelaxation, Test1,                             Scalar, LocalOrdinal,GlobalOrdinal) \
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  TEUCHOS_UNIT_TEST_TEMPLATE_3_INSTANT( Ifpack2BlockRelaxation, Test2,                             Scalar, LocalOrdinal,GlobalOrdinal) \
#else
  TEUCHOS_UNIT_TEST_TEMPLATE_1_INSTANT( Ifpack2BlockRelaxation, Test2,                             Scalar) \
#endif
  TEUCHOS_UNIT_TEST_TEMPLATE_3_INSTANT( Ifpack2BlockRelaxation, TriDi,                             Scalar, LocalOrdinal,GlobalOrdinal) \
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  TEUCHOS_UNIT_TEST_TEMPLATE_3_INSTANT( Ifpack2BlockRelaxation, LinePartition,                     Scalar, LocalOrdinal,GlobalOrdinal) \
#else
  TEUCHOS_UNIT_TEST_TEMPLATE_1_INSTANT( Ifpack2BlockRelaxation, LinePartition,                     Scalar) \
#endif
  TEUCHOS_UNIT_TEST_TEMPLATE_3_INSTANT( Ifpack2BlockRelaxation, OverlappingPartition,              Scalar, LocalOrdinal,GlobalOrdinal) \
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  TEUCHOS_UNIT_TEST_TEMPLATE_3_INSTANT( Ifpack2BlockRelaxation, BandedContainer,                   Scalar, LocalOrdinal,GlobalOrdinal) \
#else
  TEUCHOS_UNIT_TEST_TEMPLATE_1_INSTANT( Ifpack2BlockRelaxation, BandedContainer,                   Scalar) \
#endif
  TEUCHOS_UNIT_TEST_TEMPLATE_3_INSTANT( Ifpack2BlockRelaxation, BlockedBandedContainer,            Scalar, LocalOrdinal,GlobalOrdinal) \
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  TEUCHOS_UNIT_TEST_TEMPLATE_3_INSTANT( Ifpack2BlockRelaxation, TestDiagonalBlockCrsMatrix,        Scalar, LocalOrdinal,GlobalOrdinal) \
#else
  TEUCHOS_UNIT_TEST_TEMPLATE_1_INSTANT( Ifpack2BlockRelaxation, TestDiagonalBlockCrsMatrix,        Scalar) \
#endif
  TEUCHOS_UNIT_TEST_TEMPLATE_3_INSTANT( Ifpack2BlockRelaxation, TestBlockContainers,               Scalar, LocalOrdinal,GlobalOrdinal) \
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  TEUCHOS_UNIT_TEST_TEMPLATE_3_INSTANT( Ifpack2BlockRelaxation, TestBlockContainersDecoupled,      Scalar, LocalOrdinal,GlobalOrdinal) \
#else
  TEUCHOS_UNIT_TEST_TEMPLATE_1_INSTANT( Ifpack2BlockRelaxation, TestBlockContainersDecoupled,      Scalar) \
#endif
  TEUCHOS_UNIT_TEST_TEMPLATE_3_INSTANT( Ifpack2BlockRelaxation, TestContainersDofsDecoupled,       Scalar, LocalOrdinal,GlobalOrdinal) \
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  TEUCHOS_UNIT_TEST_TEMPLATE_3_INSTANT( Ifpack2BlockRelaxation, TestLowerTriangularBlockCrsMatrix, Scalar, LocalOrdinal,GlobalOrdinal) \
#else
  TEUCHOS_UNIT_TEST_TEMPLATE_1_INSTANT( Ifpack2BlockRelaxation, TestLowerTriangularBlockCrsMatrix, Scalar) \
#endif
  TEUCHOS_UNIT_TEST_TEMPLATE_3_INSTANT( Ifpack2BlockRelaxation, TestUpperTriangularBlockCrsMatrix, Scalar, LocalOrdinal,GlobalOrdinal)

// FIXME (mfh 11 Apr 2018) Test for other Scalar types at least.

typedef Tpetra::MultiVector<>::scalar_type default_scalar_type;
typedef Tpetra::MultiVector<>::local_ordinal_type default_local_ordinal_type;
typedef Tpetra::MultiVector<>::global_ordinal_type default_global_ordinal_type;

UNIT_TEST_GROUP_SCALAR_ORDINAL(default_scalar_type, default_local_ordinal_type, default_global_ordinal_type)

} // namespace (anonymous)



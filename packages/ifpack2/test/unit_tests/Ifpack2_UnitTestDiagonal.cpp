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


/*! \file Ifpack2_UnitTestDiagonal.cpp

\brief Ifpack2 Unit test for the Diagonal template.
*/


#include <Teuchos_ConfigDefs.hpp>
#include <Ifpack2_ConfigDefs.hpp>
#include <Teuchos_UnitTestHarness.hpp>
#include <Ifpack2_Version.hpp>
#include <iostream>

#include <Ifpack2_UnitTestHelpers.hpp>
#include <Ifpack2_Diagonal.hpp>

namespace {
using Tpetra::global_size_t;
typedef tif_utest::Node Node;

//this macro declares the unit-test-class:
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
TEUCHOS_UNIT_TEST_TEMPLATE_3_DECL(Ifpack2Diagonal, Test0, Scalar, LocalOrdinal, GlobalOrdinal)
#else
TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL(Ifpack2Diagonal, Test0, Scalar)
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
  const Teuchos::RCP<const Tpetra::Map<LocalOrdinal,GlobalOrdinal,Node> > rowmap = tif_utest::create_tpetra_map<LocalOrdinal,GlobalOrdinal,Node>(num_rows_per_proc);
#else
  const Teuchos::RCP<const Tpetra::Map<Node> > rowmap = tif_utest::create_tpetra_map<Node>(num_rows_per_proc);
#endif

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  Teuchos::RCP<const Tpetra::CrsMatrix<Scalar,LocalOrdinal,GlobalOrdinal,Node> > crsmatrix = tif_utest::create_test_matrix<Scalar,LocalOrdinal,GlobalOrdinal,Node>(rowmap);
#else
  Teuchos::RCP<const Tpetra::CrsMatrix<Scalar,Node> > crsmatrix = tif_utest::create_test_matrix<Scalar,Node>(rowmap);
#endif

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  Ifpack2::Diagonal<Tpetra::RowMatrix<Scalar,LocalOrdinal,GlobalOrdinal,Node> > prec(crsmatrix);
#else
  Ifpack2::Diagonal<Tpetra::RowMatrix<Scalar,Node> > prec(crsmatrix);
#endif

  prec.initialize();
  //trivial tests to insist that the preconditioner's domain/range maps are
  //the same as those of the matrix:
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  const Tpetra::Map<LocalOrdinal,GlobalOrdinal,Node>& mtx_dom_map = *crsmatrix->getDomainMap();
  const Tpetra::Map<LocalOrdinal,GlobalOrdinal,Node>& mtx_rng_map = *crsmatrix->getRangeMap();
#else
  const Tpetra::Map<Node>& mtx_dom_map = *crsmatrix->getDomainMap();
  const Tpetra::Map<Node>& mtx_rng_map = *crsmatrix->getRangeMap();
#endif

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  const Tpetra::Map<LocalOrdinal,GlobalOrdinal,Node>& prec_dom_map = *prec.getDomainMap();
  const Tpetra::Map<LocalOrdinal,GlobalOrdinal,Node>& prec_rng_map = *prec.getRangeMap();
#else
  const Tpetra::Map<Node>& prec_dom_map = *prec.getDomainMap();
  const Tpetra::Map<Node>& prec_rng_map = *prec.getRangeMap();
#endif

  TEST_EQUALITY( prec_dom_map.isSameAs(mtx_dom_map), true );
  TEST_EQUALITY( prec_rng_map.isSameAs(mtx_rng_map), true );

  prec.compute();

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  Tpetra::MultiVector<Scalar,LocalOrdinal,GlobalOrdinal,Node> x(rowmap,2), y(rowmap,2);
#else
  Tpetra::MultiVector<Scalar,Node> x(rowmap,2), y(rowmap,2);
#endif
  x.putScalar(1);

  prec.apply(x, y);

  Teuchos::ArrayRCP<const Scalar> yview = y.get1dView();

  //y should be full of 0.5's now.

  Teuchos::ArrayRCP<Scalar> halfs(num_rows_per_proc*2, 0.5);

  TEST_COMPARE_FLOATING_ARRAYS(yview, halfs(), Teuchos::ScalarTraits<Scalar>::eps());
}

//this macro declares the unit-test-class:
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
TEUCHOS_UNIT_TEST_TEMPLATE_3_DECL(Ifpack2Diagonal, Test1, Scalar, LocalOrdinal, GlobalOrdinal)
#else
TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL(Ifpack2Diagonal, Test1, Scalar)
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
  const Teuchos::RCP<const Tpetra::Map<LocalOrdinal,GlobalOrdinal,Node> > rowmap = tif_utest::create_tpetra_map<LocalOrdinal,GlobalOrdinal,Node>(num_rows_per_proc);
#else
  const Teuchos::RCP<const Tpetra::Map<Node> > rowmap = tif_utest::create_tpetra_map<Node>(num_rows_per_proc);
#endif

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  Teuchos::RCP<const Tpetra::CrsMatrix<Scalar,LocalOrdinal,GlobalOrdinal,Node> > crsmatrix = tif_utest::create_test_matrix<Scalar,LocalOrdinal,GlobalOrdinal,Node>(rowmap);
#else
  Teuchos::RCP<const Tpetra::CrsMatrix<Scalar,Node> > crsmatrix = tif_utest::create_test_matrix<Scalar,Node>(rowmap);
#endif

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  Teuchos::RCP<Tpetra::Vector<Scalar,LocalOrdinal,GlobalOrdinal,Node> > invdiag =
        Teuchos::rcp(new Tpetra::Vector<Scalar,LocalOrdinal,GlobalOrdinal,Node>(rowmap));
#else
  Teuchos::RCP<Tpetra::Vector<Scalar,Node> > invdiag =
        Teuchos::rcp(new Tpetra::Vector<Scalar,Node>(rowmap));
#endif

  crsmatrix->getLocalDiagCopy(*invdiag);
  invdiag->reciprocal(*invdiag);

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  Teuchos::RCP<const Tpetra::Vector<Scalar,LocalOrdinal,GlobalOrdinal,Node> > const_invdiag = invdiag;
#else
  Teuchos::RCP<const Tpetra::Vector<Scalar,Node> > const_invdiag = invdiag;
#endif

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  typedef Tpetra::CrsMatrix<Scalar,LocalOrdinal,GlobalOrdinal,Node> TCrsMatrix;
  typedef Tpetra::Vector<Scalar,LocalOrdinal,GlobalOrdinal,Node> TVector;
#else
  typedef Tpetra::CrsMatrix<Scalar,Node> TCrsMatrix;
  typedef Tpetra::Vector<Scalar,Node> TVector;
#endif

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  Teuchos::RCP<Ifpack2::Preconditioner<Scalar,LocalOrdinal,GlobalOrdinal,Node> > prec =
#else
  Teuchos::RCP<Ifpack2::Preconditioner<Scalar,Node> > prec =
#endif
     Ifpack2::createDiagonalPreconditioner<TCrsMatrix,TVector>(invdiag);

  prec->initialize();
  prec->compute();

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  Tpetra::MultiVector<Scalar,LocalOrdinal,GlobalOrdinal,Node> x(rowmap,2), y(rowmap,2);
#else
  Tpetra::MultiVector<Scalar,Node> x(rowmap,2), y(rowmap,2);
#endif
  x.putScalar(1);

  prec->apply(x, y);

  Teuchos::ArrayRCP<const Scalar> yview = y.get1dView();

  //y should be full of 0.5's now.

  Teuchos::ArrayRCP<Scalar> halfs(num_rows_per_proc*2, 0.5);

  TEST_COMPARE_FLOATING_ARRAYS(yview, halfs(), Teuchos::ScalarTraits<Scalar>::eps());
}

#define UNIT_TEST_GROUP_SC_LO_GO(Scalar,LocalOrdinal,GlobalOrdinal) \
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  TEUCHOS_UNIT_TEST_TEMPLATE_3_INSTANT( Ifpack2Diagonal, Test0, Scalar, LocalOrdinal,GlobalOrdinal) \
#else
  TEUCHOS_UNIT_TEST_TEMPLATE_1_INSTANT( Ifpack2Diagonal, Test0, Scalar) \
#endif
  TEUCHOS_UNIT_TEST_TEMPLATE_3_INSTANT( Ifpack2Diagonal, Test1, Scalar, LocalOrdinal,GlobalOrdinal)

#include "Ifpack2_ETIHelperMacros.h"

IFPACK2_ETI_MANGLING_TYPEDEFS()

// Test all enabled combinations of Scalar (SC), LocalOrdinal (LO),
// and GlobalOrdinal (GO) types, where Scalar is real.

IFPACK2_INSTANTIATE_SLG_REAL( UNIT_TEST_GROUP_SC_LO_GO )

} // namespace (anonymous)


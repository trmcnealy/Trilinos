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


/*! \file Ifpack2_UnitTestILUT.cpp

\brief Ifpack2 Unit test for the ILUT template.
*/


#include <Teuchos_ConfigDefs.hpp>
#include <Ifpack2_ConfigDefs.hpp>
#include <Teuchos_UnitTestHarness.hpp>
#include <Ifpack2_Version.hpp>
#include <iostream>

#include <Ifpack2_UnitTestHelpers.hpp>
#include <Ifpack2_ILUT.hpp>

#include <type_traits>

namespace {
using Tpetra::global_size_t;
typedef tif_utest::Node Node;

//this macro declares the unit-test-class:
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
TEUCHOS_UNIT_TEST_TEMPLATE_3_DECL(Ifpack2ILUT, Test0, Scalar, LocalOrdinal, GlobalOrdinal)
#else
TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL(Ifpack2ILUT, Test0, Scalar)
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
  Ifpack2::ILUT<Tpetra::RowMatrix<Scalar,LocalOrdinal,GlobalOrdinal,Node> > prec(crsmatrix);
#else
  Ifpack2::ILUT<Tpetra::RowMatrix<Scalar,Node> > prec(crsmatrix);
#endif

  Teuchos::ParameterList params;
  params.set("fact: ilut level-of-fill", 1.0);
  params.set("fact: drop tolerance", 0.0);

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

  TEST_EQUALITY( prec_dom_map_ptr, mtx_dom_map_ptr );
  TEST_EQUALITY( prec_rng_map_ptr, mtx_rng_map_ptr );

  prec.initialize();
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

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
TEUCHOS_UNIT_TEST_TEMPLATE_3_DECL(Ifpack2ILUT, Test1, Scalar, LocalOrdinal, GlobalOrdinal)
#else
TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL(Ifpack2ILUT, Test1, Scalar)
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
  Teuchos::RCP<const Tpetra::CrsMatrix<Scalar,LocalOrdinal,GlobalOrdinal,Node> > crsmatrix = tif_utest::create_test_matrix3<Scalar,LocalOrdinal,GlobalOrdinal,Node>(rowmap);
#else
  Teuchos::RCP<const Tpetra::CrsMatrix<Scalar,Node> > crsmatrix = tif_utest::create_test_matrix3<Scalar,Node>(rowmap);
#endif

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  Ifpack2::ILUT<Tpetra::RowMatrix<Scalar,LocalOrdinal,GlobalOrdinal,Node> > prec(crsmatrix);
#else
  Ifpack2::ILUT<Tpetra::RowMatrix<Scalar,Node> > prec(crsmatrix);
#endif

  Teuchos::ParameterList params;
  params.set("fact: ilut level-of-fill", 6.0);
  params.set("fact: drop tolerance", 0.0);

  TEST_NOTHROW(prec.setParameters(params));

  prec.initialize();
  prec.compute();

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  Tpetra::MultiVector<Scalar,LocalOrdinal,GlobalOrdinal,Node> x(rowmap,2), y(rowmap,2);
#else
  Tpetra::MultiVector<Scalar,Node> x(rowmap,2), y(rowmap,2);
#endif
  x.putScalar(1);

  crsmatrix->apply(x,y);
  prec.apply(y, x);

  Teuchos::ArrayRCP<const Scalar> xview = x.get1dView();

  //x should be full of 1's now.

  Teuchos::ArrayRCP<Scalar> ones(num_rows_per_proc*2, 1);

  TEST_COMPARE_FLOATING_ARRAYS(xview, ones(), 2*Teuchos::ScalarTraits<Scalar>::eps());
}

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
TEUCHOS_UNIT_TEST_TEMPLATE_3_DECL(Ifpack2ILUT, Test2, Scalar, LocalOrdinal, GlobalOrdinal)
#else
TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL(Ifpack2ILUT, Test2, Scalar)
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

  global_size_t num_rows_per_proc = 10;

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  const Teuchos::RCP<const Tpetra::Map<LocalOrdinal,GlobalOrdinal,Node> > rowmap = tif_utest::create_tpetra_map<LocalOrdinal,GlobalOrdinal,Node>(num_rows_per_proc);
#else
  const Teuchos::RCP<const Tpetra::Map<Node> > rowmap = tif_utest::create_tpetra_map<Node>(num_rows_per_proc);
#endif

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  Teuchos::RCP<const Tpetra::CrsMatrix<Scalar,LocalOrdinal,GlobalOrdinal,Node> > crsmatrix = tif_utest::create_banded_matrix<Scalar,LocalOrdinal,GlobalOrdinal,Node>(rowmap, 5);
#else
  Teuchos::RCP<const Tpetra::CrsMatrix<Scalar,Node> > crsmatrix = tif_utest::create_banded_matrix<Scalar,Node>(rowmap, 5);
#endif

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  Ifpack2::ILUT<Tpetra::RowMatrix<Scalar,LocalOrdinal,GlobalOrdinal,Node> > prec(crsmatrix);
#else
  Ifpack2::ILUT<Tpetra::RowMatrix<Scalar,Node> > prec(crsmatrix);
#endif

  Teuchos::ParameterList params;
  params.set("fact: ilut level-of-fill", 0.5);
  params.set("fact: drop tolerance", 0.0);

  // Fill < 1.0 not allowed
  TEST_THROW(prec.setParameters(params), std::runtime_error);

  size_t numFill0, numFill1, numFill2;
  params.set("fact: ilut level-of-fill", 1.0);
  TEST_NOTHROW(prec.setParameters(params));
  prec.initialize();
  prec.compute();
  numFill0 = prec.getNodeNumEntries();
  TEST_EQUALITY(numFill0, crsmatrix->getNodeNumEntries());

  params.set("fact: ilut level-of-fill", 1.5);
  TEST_NOTHROW(prec.setParameters(params));
  prec.initialize();
  prec.compute();
  numFill1 = prec.getNodeNumEntries();

  params.set("fact: ilut level-of-fill", 2.0);
  TEST_NOTHROW(prec.setParameters(params));
  prec.initialize();
  prec.compute();
  numFill2 = prec.getNodeNumEntries();

  TEST_ASSERT(numFill0 < numFill1);
  TEST_ASSERT(numFill1 < numFill2);
}

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
TEUCHOS_UNIT_TEST_TEMPLATE_3_DECL(Ifpack2ILUT, Test3, Scalar, LocalOrdinal, GlobalOrdinal)
#else
TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL(Ifpack2ILUT, Test3, Scalar)
#endif
{
#ifndef TPETRA_ENABLE_TEMPLATE_ORDINALS
  using LocalOrdinal = typename Tpetra::Map<>::local_ordinal_type;
  using GlobalOrdinal = typename Tpetra::Map<>::global_ordinal_type;
#endif
  const global_size_t num_rows_per_proc = 10;
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  auto rowMap = tif_utest::create_tpetra_map<LocalOrdinal, GlobalOrdinal, Node> (num_rows_per_proc);
  auto A = tif_utest::create_banded_matrix<Scalar, LocalOrdinal, GlobalOrdinal, Node> (rowMap, 5);
#else
  auto rowMap = tif_utest::create_tpetra_map<Node> (num_rows_per_proc);
  auto A = tif_utest::create_banded_matrix<Scalar, Node> (rowMap, 5);
#endif

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  using prec_type = Ifpack2::ILUT<Tpetra::RowMatrix<Scalar,LocalOrdinal,GlobalOrdinal,Node> >;
#else
  using prec_type = Ifpack2::ILUT<Tpetra::RowMatrix<Scalar,Node> >;
#endif
  using mag_type = decltype (prec_type (A).getAbsoluteThreshold ());

  // Check that setParameters does not change any parameters until it
  // checks that all parameters are valid.
  {
    prec_type prec (A);
    {
      Teuchos::ParameterList params;
      params.set ("fact: ilut level-of-fill", 1.5); // valid, not default
      params.set ("fact: absolute threshold", 0.3); // valid, not default
      params.set ("fact: relative threshold", 0.25); // valid, not default
      params.set ("fact: relax value", 0.20); // valid, not default
      params.set ("fact: drop tolerance", 0.15); // valid, not default
      TEST_NOTHROW( prec.setParameters (params) );
    }
    {
      Teuchos::ParameterList params;
      params.set ("fact: ilut level-of-fill", 0.9); // invalid
      params.set ("fact: absolute threshold", 0.25); // valid, not default
      params.set ("fact: relative threshold", 0.2); // valid, not default
      params.set ("fact: relax value", 0.15); // valid, not default
      params.set ("fact: drop tolerance", 0.1); // valid, not default

      TEST_THROW( prec.setParameters (params), std::runtime_error );
      // Fill level is double, not mag_type, because it depends on
      // LocalOrdinal, not on Scalar.
      TEST_EQUALITY( prec.getLevelOfFill (), double (1.5) ); // this is properly double
      TEST_EQUALITY( prec.getAbsoluteThreshold (), mag_type (0.3) );
      TEST_EQUALITY( prec.getRelativeThreshold (), mag_type (0.25) );
      TEST_EQUALITY( prec.getRelaxValue (), mag_type (0.20) );
      TEST_EQUALITY( prec.getDropTolerance (), mag_type (0.15) );
    }
  }

  // Repeat above test, but set parameters using mag_type, not double
  // (except for "fact: ilut level-of-fill", which is always double).
  if (! std::is_same<mag_type, double>::value) {
    prec_type prec (A);
    {
      Teuchos::ParameterList params;
      params.set ("fact: ilut level-of-fill", 1.5); // valid, not default
      params.set ("fact: absolute threshold", mag_type (0.3)); // valid, not default
      params.set ("fact: relative threshold", mag_type (0.25)); // valid, not default
      params.set ("fact: relax value", mag_type (0.20)); // valid, not default
      params.set ("fact: drop tolerance", mag_type (0.15)); // valid, not default
      TEST_NOTHROW( prec.setParameters (params) );
    }
    {
      Teuchos::ParameterList params;
      params.set ("fact: ilut level-of-fill", 0.9); // invalid
      params.set ("fact: absolute threshold", mag_type (0.25)); // valid, not default
      params.set ("fact: relative threshold", mag_type (0.2)); // valid, not default
      params.set ("fact: relax value", mag_type (0.15)); // valid, not default
      params.set ("fact: drop tolerance", mag_type (0.1)); // valid, not default

      TEST_THROW( prec.setParameters (params), std::runtime_error );
      // Fill level is double, not mag_type, because it depends on
      // LocalOrdinal, not on Scalar.
      TEST_EQUALITY( prec.getLevelOfFill (), double (1.5) );
      TEST_EQUALITY( prec.getAbsoluteThreshold (), mag_type (0.3) );
      TEST_EQUALITY( prec.getRelativeThreshold (), mag_type (0.25) );
      TEST_EQUALITY( prec.getRelaxValue (), mag_type (0.20) );
      TEST_EQUALITY( prec.getDropTolerance (), mag_type (0.15) );
    }
  }
}


#define UNIT_TEST_GROUP_SC_LO_GO(Scalar,LocalOrdinal,GlobalOrdinal) \
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  TEUCHOS_UNIT_TEST_TEMPLATE_3_INSTANT( Ifpack2ILUT, Test0, Scalar, LocalOrdinal,GlobalOrdinal) \
#else
  TEUCHOS_UNIT_TEST_TEMPLATE_1_INSTANT( Ifpack2ILUT, Test0, Scalar) \
#endif
  TEUCHOS_UNIT_TEST_TEMPLATE_3_INSTANT( Ifpack2ILUT, Test1, Scalar, LocalOrdinal,GlobalOrdinal) \
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  TEUCHOS_UNIT_TEST_TEMPLATE_3_INSTANT( Ifpack2ILUT, Test2, Scalar, LocalOrdinal,GlobalOrdinal) \
#else
  TEUCHOS_UNIT_TEST_TEMPLATE_1_INSTANT( Ifpack2ILUT, Test2, Scalar) \
#endif
  TEUCHOS_UNIT_TEST_TEMPLATE_3_INSTANT( Ifpack2ILUT, Test3, Scalar, LocalOrdinal,GlobalOrdinal)

#include "Ifpack2_ETIHelperMacros.h"

IFPACK2_ETI_MANGLING_TYPEDEFS()

// Test all enabled combinations of Scalar (SC), LocalOrdinal (LO),
// and GlobalOrdinal (GO) types, where Scalar is real.

IFPACK2_INSTANTIATE_SLG_REAL( UNIT_TEST_GROUP_SC_LO_GO )

} // namespace (anonymous)

// @HEADER
//
// ***********************************************************************
//
//             Xpetra: A linear algebra interface package
//                  Copyright 2012 Sandia Corporation
//
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
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
// Questions? Contact
//                    Jonathan Hu       (jhu@sandia.gov)
//                    Andrey Prokopenko (aprokop@sandia.gov)
//                    Ray Tuminaro      (rstumin@sandia.gov)
//
// ***********************************************************************
//
// @HEADER
#include <Teuchos_UnitTestHarness.hpp>
#include <Teuchos_Tuple.hpp>
#include <Teuchos_CommHelpers.hpp>

#include "Xpetra_ConfigDefs.hpp"
#include "Xpetra_DefaultPlatform.hpp"

#ifdef HAVE_XPETRA_TPETRA
#include "Xpetra_TpetraMap.hpp"
#endif

#ifdef HAVE_XPETRA_EPETRA
#include "Xpetra_EpetraMap.hpp"
#endif

#include "Xpetra_StridedMapFactory.hpp"
#include "Xpetra_StridedMap.hpp"

namespace {
  using Teuchos::RCP;
  using Teuchos::rcp;
  using Xpetra::global_size_t;
  using Xpetra::DefaultPlatform;
  using Teuchos::Comm;

  bool testMpi = true;
  double errorTolSlack = 1e+1;

  TEUCHOS_STATIC_SETUP()
  {
    Teuchos::CommandLineProcessor &clp = Teuchos::UnitTestRepository::getCLP();
    clp.addOutputSetupOptions(true);
    clp.setOption(
        "test-mpi", "test-serial", &testMpi,
        "Test MPI (if available) or force test of serial.  In a serial build,"
        " this option is ignored and a serial comm is always used." );
    clp.setOption(
        "error-tol-slack", &errorTolSlack,
        "Slack off of machine epsilon used to check test results" );
  }

  RCP<const Comm<int> > getDefaultComm()
  {
    if (testMpi) {
      return DefaultPlatform::getDefaultPlatform().getComm();
    }
    return rcp(new Teuchos::SerialComm<int>());
  }

  //
  // UNIT TESTS
  //


  ////
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  TEUCHOS_UNIT_TEST_TEMPLATE_4_DECL( StridedMapFactory, CreateStridedMap1, M, LO, GO, N )
#else
  TEUCHOS_UNIT_TEST_TEMPLATE_2_DECL( StridedMapFactory, CreateStridedMap1, M, N )
#endif
  {
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    typedef Xpetra::StridedMap<LO,GO,N> SM;
#else
    using LO = typename Tpetra::Map<>::local_ordinal_type;
    using GO = typename Tpetra::Map<>::global_ordinal_type;
    typedef Xpetra::StridedMap<N> SM;
#endif

    // create a comm
    Teuchos::RCP<const Teuchos::Comm<int> > comm = getDefaultComm();
    const int numImages = comm->getSize();

    // test constructor for Xpetra::StridedMaps
    M testMap(1,0,comm);
    Xpetra::UnderlyingLib lib = testMap.lib();

    for (int indexBase = 0; indexBase < 2; indexBase++) {

      GO offset = 111;

      // constructor calls: (num global elements, index base)
      global_size_t numGlobalElements = 120 * numImages;
      size_t numLocalElements = 120;
      std::vector<size_t> stridedInfo;
      stridedInfo.push_back(3);
      stridedInfo.push_back(4);
      stridedInfo.push_back(5);

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
      Teuchos::RCP<SM> map = Xpetra::StridedMapFactory<LO,GO,N>::Build(lib, numGlobalElements, indexBase, stridedInfo, comm, -1, offset);
#else
      Teuchos::RCP<SM> map = Xpetra::StridedMapFactory<N>::Build(lib, numGlobalElements, indexBase, stridedInfo, comm, -1, offset);
#endif

      TEST_EQUALITY_CONST( map->getFixedBlockSize(), 12 );
      TEST_EQUALITY_CONST( map->isStrided(), true );
      TEST_EQUALITY_CONST( map->isBlocked(), true );
      TEST_EQUALITY_CONST( map->getMinAllGlobalIndex(), indexBase + offset );
      TEST_EQUALITY_CONST( map->getMaxAllGlobalIndex(), indexBase + offset + Teuchos::as<GO>(numGlobalElements) - 1);
      TEST_EQUALITY_CONST( map->isContiguous(), false);
      TEST_EQUALITY_CONST( map->getNodeNumElements() % 12 , 0);

      //Teuchos::RCP<SM> emap2 = Teuchos::null;
      for(size_t k=0; k<stridedInfo.size(); k++) {
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
        Teuchos::RCP<SM> map2 = Xpetra::StridedMapFactory<LO,GO,N>::Build(lib, numGlobalElements, indexBase, stridedInfo, comm, k, offset);
#else
        Teuchos::RCP<SM> map2 = Xpetra::StridedMapFactory<N>::Build(lib, numGlobalElements, indexBase, stridedInfo, comm, k, offset);
#endif
        TEST_EQUALITY_CONST( map2->getFixedBlockSize(), 12 );
        TEST_EQUALITY_CONST( map2->isStrided(), true );
        TEST_EQUALITY_CONST( map2->isBlocked(), true );
        TEST_EQUALITY_CONST( map2->isContiguous(), false);
        TEST_EQUALITY_CONST( map2->getNodeNumElements() % stridedInfo[k] , 0);
        TEST_EQUALITY_CONST( map2->getNodeNumElements(), numLocalElements / map2->getFixedBlockSize() * stridedInfo[k] );
      }

    }
  }
  // TODO add test routines for remaining constructors of StridedMapFactory

  ////
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  TEUCHOS_UNIT_TEST_TEMPLATE_4_DECL( StridedMapFactory, CreateStridedMap2, M, LO, GO, N )
#else
  TEUCHOS_UNIT_TEST_TEMPLATE_2_DECL( StridedMapFactory, CreateStridedMap2, M, N )
#endif
  {
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    typedef Xpetra::StridedMap<LO,GO,N> SM;
#else
    using LO = typename Tpetra::Map<>::local_ordinal_type;
    using GO = typename Tpetra::Map<>::global_ordinal_type;
    typedef Xpetra::StridedMap<N> SM;
#endif

    // create a comm
    Teuchos::RCP<const Teuchos::Comm<int> > comm = getDefaultComm();
    const int numImages = comm->getSize();

    // test constructor for Xpetra::StridedMaps
    M testMap(1,0,comm);
    Xpetra::UnderlyingLib lib = testMap.lib();

    GO offset = 111;

    // constructor calls: (num global elements, index base)
    global_size_t numGlobalElements = 120 * numImages;
    size_t numLocalElements = 120;
    std::vector<size_t> stridedInfo;
    stridedInfo.push_back(3);
    stridedInfo.push_back(4);
    stridedInfo.push_back(5);

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    Teuchos::RCP<SM> map = Xpetra::StridedMapFactory<LO,GO,N>::Build(lib, numGlobalElements, 0, stridedInfo, comm, -1, offset);
#else
    Teuchos::RCP<SM> map = Xpetra::StridedMapFactory<N>::Build(lib, numGlobalElements, 0, stridedInfo, comm, -1, offset);
#endif

    TEST_EQUALITY_CONST( map->getFixedBlockSize(), 12 );
    TEST_EQUALITY_CONST( map->isStrided(), true );
    TEST_EQUALITY_CONST( map->isBlocked(), true );
    TEST_EQUALITY_CONST( map->getMinAllGlobalIndex(), offset );
    TEST_EQUALITY_CONST( map->getMaxAllGlobalIndex(), offset + Teuchos::as<GO>(numGlobalElements) - 1);
    TEST_EQUALITY_CONST( map->isContiguous(), false);
    TEST_EQUALITY_CONST( map->getNodeNumElements() % 12 , 0);

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    Teuchos::RCP<SM> map2 = Xpetra::StridedMapFactory<LO,GO,N>::Build(map, 0);
#else
    Teuchos::RCP<SM> map2 = Xpetra::StridedMapFactory<N>::Build(map, 0);
#endif
    TEST_EQUALITY_CONST( map2->getFixedBlockSize(), 12 );
    TEST_EQUALITY_CONST( map2->isStrided(), true );
    TEST_EQUALITY_CONST( map2->isBlocked(), true );
    TEST_EQUALITY_CONST( map2->getMinAllGlobalIndex(), offset );
    TEST_EQUALITY_CONST( map2->getMaxAllGlobalIndex(), offset + Teuchos::as<GO>(numGlobalElements) - 10 );
    TEST_EQUALITY_CONST( map2->isContiguous(), false);
    TEST_EQUALITY_CONST( map2->getNodeNumElements() % 3 , 0);
    TEST_EQUALITY_CONST( map2->getNodeNumElements(), numLocalElements / map2->getFixedBlockSize() * stridedInfo[0]);

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    Teuchos::RCP<SM> map3 = Xpetra::StridedMapFactory<LO,GO,N>::Build(map, 1);
#else
    Teuchos::RCP<SM> map3 = Xpetra::StridedMapFactory<N>::Build(map, 1);
#endif
    TEST_EQUALITY_CONST( map3->getFixedBlockSize(), 12 );
    TEST_EQUALITY_CONST( map3->isStrided(), true );
    TEST_EQUALITY_CONST( map3->isBlocked(), true );
    TEST_EQUALITY_CONST( map3->getMinAllGlobalIndex(), offset + 3 );
    TEST_EQUALITY_CONST( map3->getMaxAllGlobalIndex(), offset + Teuchos::as<GO>(numGlobalElements) - 6 );
    TEST_EQUALITY_CONST( map3->isContiguous(), false);
    TEST_EQUALITY_CONST( map3->getNodeNumElements() % 4 , 0);
    TEST_EQUALITY_CONST( map3->getNodeNumElements(), numLocalElements / map3->getFixedBlockSize() * stridedInfo[1]);

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    Teuchos::RCP<SM> map4 = Xpetra::StridedMapFactory<LO,GO,N>::Build(map, 2);
#else
    Teuchos::RCP<SM> map4 = Xpetra::StridedMapFactory<N>::Build(map, 2);
#endif
    TEST_EQUALITY_CONST( map4->getFixedBlockSize(), 12 );
    TEST_EQUALITY_CONST( map4->isStrided(), true );
    TEST_EQUALITY_CONST( map4->isBlocked(), true );
    TEST_EQUALITY_CONST( map4->getMinAllGlobalIndex(), offset + 7 );
    TEST_EQUALITY_CONST( map4->getMaxAllGlobalIndex(), offset + Teuchos::as<GO>(numGlobalElements) - 1 );
    TEST_EQUALITY_CONST( map4->isContiguous(), false);
    TEST_EQUALITY_CONST( map4->getNodeNumElements() % 5 , 0);
    TEST_EQUALITY_CONST( map4->getNodeNumElements(), numLocalElements / map4->getFixedBlockSize() * stridedInfo[2]);
  }

  //
  // INSTANTIATIONS
  //
#ifdef HAVE_XPETRA_TPETRA

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  #define XPETRA_TPETRA_TYPES( LO, GO, N) \
    typedef typename Xpetra::TpetraMap<LO,GO,N> M##LO##GO##N;
#else
  #define XPETRA_TPETRA_TYPES(N) \
    typedef typename Xpetra::TpetraMap<N> M##LO##GO##N;
#endif

#endif

#ifdef HAVE_XPETRA_EPETRA

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  #define XPETRA_EPETRA_TYPES( LO, GO, N) \
#else
  #define XPETRA_EPETRA_TYPES(N) \
#endif
    typedef typename Xpetra::EpetraMapT<GO,N> M##LO##GO##N;

#endif

// List of tests (which run both on Epetra and Tpetra)
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
#define XP_MAP_INSTANT(LO,GO,N) \
    TEUCHOS_UNIT_TEST_TEMPLATE_4_INSTANT( StridedMapFactory, CreateStridedMap1, M##LO##GO##N , LO, GO, N) \
    TEUCHOS_UNIT_TEST_TEMPLATE_4_INSTANT( StridedMapFactory, CreateStridedMap2, M##LO##GO##N , LO, GO, N)
#else
#define XP_MAP_INSTANT(N) \
    TEUCHOS_UNIT_TEST_TEMPLATE_2_INSTANT( StridedMapFactory, CreateStridedMap1, M##LO##GO##N, N) \
    TEUCHOS_UNIT_TEST_TEMPLATE_2_INSTANT( StridedMapFactory, CreateStridedMap2, M##LO##GO##N, N)
#endif

#if defined(HAVE_XPETRA_TPETRA)

#include <TpetraCore_config.h>
#include <TpetraCore_ETIHelperMacros.h>

TPETRA_ETI_MANGLING_TYPEDEFS()
// no ordinal types as scalar for testing as some tests use ScalarTraits::eps...
TPETRA_INSTANTIATE_LGN ( XPETRA_TPETRA_TYPES )
TPETRA_INSTANTIATE_LGN ( XP_MAP_INSTANT )

#endif

#if defined(HAVE_XPETRA_EPETRA)

#include "Xpetra_Map.hpp" // defines EpetraNode
typedef Xpetra::EpetraNode EpetraNode;
#ifndef XPETRA_EPETRA_NO_32BIT_GLOBAL_INDICES
XPETRA_EPETRA_TYPES(int,int,EpetraNode)
XP_MAP_INSTANT(int,int,EpetraNode)
#endif
#ifndef XPETRA_EPETRA_NO_64BIT_GLOBAL_INDICES
typedef long long LongLong;
XPETRA_EPETRA_TYPES(int,LongLong,EpetraNode)
XP_MAP_INSTANT(int,LongLong,EpetraNode)
#endif
#endif


} // end namespace




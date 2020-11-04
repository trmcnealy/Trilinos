// @HEADER
//
// ***********************************************************************
//
//        MueLu: A package for multigrid based preconditioning
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
#include <Teuchos_DefaultComm.hpp>

#include "MueLu_TestHelpers_kokkos.hpp"
#include "MueLu_Version.hpp"

#include "MueLu_LWGraph_kokkos.hpp"
#include "MueLu_Level.hpp"
#include "MueLu_AmalgamationInfo_kokkos.hpp"
#include "MueLu_AmalgamationFactory_kokkos.hpp"
#include "MueLu_CoalesceDropFactory_kokkos.hpp"

namespace MueLuTests {

  // Little utility to generate a LWGraph_kokkos.
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  template<class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node>
  Teuchos::RCP<MueLu::LWGraph_kokkos<LocalOrdinal, GlobalOrdinal, Node> >
  gimmeLWGraph(const Teuchos::RCP<Xpetra::Matrix<Scalar,LocalOrdinal,GlobalOrdinal,Node> >& A,
               Teuchos::RCP<MueLu::AmalgamationInfo_kokkos<LocalOrdinal, GlobalOrdinal, Node> >& amalgInfo) {
#else
  template<class Scalar, class Node>
  Teuchos::RCP<MueLu::LWGraph_kokkos<Node> >
  gimmeLWGraph(const Teuchos::RCP<Xpetra::Matrix<Scalar,Node> >& A,
               Teuchos::RCP<MueLu::AmalgamationInfo_kokkos<Node> >& amalgInfo) {
#endif
#   include "MueLu_UseShortNames.hpp"

    Level level;

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    TestHelpers_kokkos::TestFactory<SC,LO,GO,NO>::createSingleLevelHierarchy(level);
#else
    TestHelpers_kokkos::TestFactory<SC,NO>::createSingleLevelHierarchy(level);
#endif
    level.Set("A", A);

    RCP<AmalgamationFactory_kokkos> amalgFact = rcp(new AmalgamationFactory_kokkos());
    RCP<CoalesceDropFactory_kokkos> dropFact  = rcp(new CoalesceDropFactory_kokkos());
    dropFact->SetFactory("UnAmalgamationInfo", amalgFact);

    level.Request("Graph",              dropFact.get());
    level.Request("UnAmalgamationInfo", amalgFact.get());

    level.Request(*dropFact);
    dropFact->Build(level);

    auto graph = level.Get<RCP<LWGraph_kokkos> >("Graph", dropFact.get());
    amalgInfo = level.Get<RCP<AmalgamationInfo_kokkos> >("UnAmalgamationInfo", amalgFact.get());
    level.Release("UnAmalgamationInfo", amalgFact.get());
    level.Release("Graph",              dropFact.get());
    return graph;
  }

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  TEUCHOS_UNIT_TEST_TEMPLATE_4_DECL(LWGraph_kokkos, CreateLWGraph, Scalar, LocalOrdinal, GlobalOrdinal, Node)
#else
  TEUCHOS_UNIT_TEST_TEMPLATE_2_DECL(LWGraph_kokkos, CreateLWGraph, Scalar, Node)
#endif
  {
#   include "MueLu_UseShortNames.hpp"
#ifndef TPETRA_ENABLE_TEMPLATE_ORDINALS
    using LocalOrdinal = typename Tpetra::Map<>::local_ordinal_type;
    using GlobalOrdinal = typename Tpetra::Map<>::global_ordinal_type;
#endif
    MUELU_TESTING_SET_OSTREAM;
    MUELU_TESTING_LIMIT_SCOPE(Scalar,GlobalOrdinal,Node);
    out << "version: " << MueLu::Version() << std::endl;

    if (TestHelpers_kokkos::Parameters::getLib() == Xpetra::UseEpetra) {
      out << "skipping test for linAlgebra==UseEpetra" << std::endl;
      return;
    }

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    RCP<Matrix> A = TestHelpers_kokkos::TestFactory<SC, LO, GO, NO>::Build1DPoisson(16);
#else
    RCP<Matrix> A = TestHelpers_kokkos::TestFactory<SC, NO>::Build1DPoisson(16);
#endif

    RCP<AmalgamationInfo_kokkos> amalgInfo;
    RCP<LWGraph_kokkos> graph = gimmeLWGraph(A, amalgInfo);

    const int numRanks = graph->GetComm()->getSize();

    TEST_EQUALITY(graph != Teuchos::null,                true);
    if(numRanks == 1) {
      TEST_EQUALITY(graph->GetNodeNumVertices() == 16,   true);
    } else if(numRanks == 4) {
      TEST_EQUALITY(graph->GetNodeNumVertices() == 4,    true);
    }
  } // CreateLWGraph

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  TEUCHOS_UNIT_TEST_TEMPLATE_4_DECL(LWGraph_kokkos, LocalGraphData, Scalar, LocalOrdinal, GlobalOrdinal, Node)
#else
  TEUCHOS_UNIT_TEST_TEMPLATE_2_DECL(LWGraph_kokkos, LocalGraphData, Scalar, Node)
#endif
  {
#   include "MueLu_UseShortNames.hpp"
#ifndef TPETRA_ENABLE_TEMPLATE_ORDINALS
    using LocalOrdinal = typename Tpetra::Map<>::local_ordinal_type;
    using GlobalOrdinal = typename Tpetra::Map<>::global_ordinal_type;
#endif
    MUELU_TESTING_SET_OSTREAM;
    MUELU_TESTING_LIMIT_SCOPE(Scalar,GlobalOrdinal,Node);
    out << "version: " << MueLu::Version() << std::endl;

    if (TestHelpers_kokkos::Parameters::getLib() == Xpetra::UseEpetra) {
      out << "skipping test for linAlgebra==UseEpetra" << std::endl;
      return;
    }

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    RCP<Matrix> A = TestHelpers_kokkos::TestFactory<SC, LO, GO, NO>::Build1DPoisson(16);
#else
    RCP<Matrix> A = TestHelpers_kokkos::TestFactory<SC, NO>::Build1DPoisson(16);
#endif

    RCP<AmalgamationInfo_kokkos> amalgInfo;
    RCP<LWGraph_kokkos> graph = gimmeLWGraph(A, amalgInfo);

    // const int numRanks = graph->GetComm()->getSize();
    // const int myRank   = graph->GetComm()->getRank();

    using row_map_type = typename LWGraph_kokkos::local_graph_type::row_map_type;
    using entries_type = typename LWGraph_kokkos::local_graph_type::entries_type;

    row_map_type rowPtrs = graph->getRowPtrs();
    entries_type entries = graph->getEntries();
    typename row_map_type::HostMirror rowPtrsHost = Kokkos::create_mirror_view(rowPtrs);
    typename entries_type::HostMirror entriesHost = Kokkos::create_mirror_view(entries);
    Kokkos::deep_copy(rowPtrsHost, rowPtrs);
    Kokkos::deep_copy(entriesHost, entries);

    // Check that the graph is not null
    TEST_EQUALITY(graph != Teuchos::null, true);

    // Since this is a scalar problem and no entries are dropped
    // we can simply compare the value from LWGraph with those in A
    TEST_EQUALITY(graph->GetNodeNumVertices() == A->getNodeNumRows(), true);
    TEST_EQUALITY(graph->GetNodeNumEdges() == A->getNodeNumEntries(), true);
    Array<LO> indices(3);
    Array<SC> values(3);
    size_t numEntries;
    bool chk_rowPtr = true;
    bool chk_colInd = true;
    for(size_t rowIdx = 0; rowIdx < A->getNodeNumRows(); ++rowIdx) {
      A->getLocalRowCopy(rowIdx, indices(), values(), numEntries);
      if(rowPtrsHost(rowIdx + 1) - rowPtrsHost(rowIdx) != numEntries) {chk_rowPtr = false;}
      for(size_t entryIdx = 0; entryIdx < numEntries; ++entryIdx) {
        if(entriesHost(rowPtrsHost(rowIdx) + entryIdx) != indices[entryIdx]) {chk_colInd = false;}
      }
    }
    TEST_EQUALITY(chk_rowPtr, true);
    TEST_EQUALITY(chk_colInd, true);
  } // LocalGraphData

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
#define MUELU_ETI_GROUP(SC, LO, GO, NO) \
  TEUCHOS_UNIT_TEST_TEMPLATE_4_INSTANT(LWGraph_kokkos, CreateLWGraph,  SC, LO, GO, NO) \
  TEUCHOS_UNIT_TEST_TEMPLATE_4_INSTANT(LWGraph_kokkos, LocalGraphData, SC, LO, GO, NO)
#else
#define MUELU_ETI_GROUP(SC, NO) \
  TEUCHOS_UNIT_TEST_TEMPLATE_2_INSTANT(LWGraph_kokkos, CreateLWGraph,  SC, NO) \
  TEUCHOS_UNIT_TEST_TEMPLATE_2_INSTANT(LWGraph_kokkos, LocalGraphData, SC, NO)
#endif

#include <MueLu_ETI_4arg.hpp>

} // namespace MueLuTests

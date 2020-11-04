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
#include <MueLu_TestHelpers.hpp>
#include <MueLu_Version.hpp>

#include <MueLu_CoalesceDropFactory.hpp>
#include <MueLu_PreDropFunctionConstVal.hpp>
#include <MueLu_Graph.hpp>

namespace MueLuTests {


#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  TEUCHOS_UNIT_TEST_TEMPLATE_4_DECL(CoalesceDropFactory, Constructor, Scalar, LocalOrdinal, GlobalOrdinal, Node)
#else
  TEUCHOS_UNIT_TEST_TEMPLATE_2_DECL(CoalesceDropFactory, Constructor, Scalar, Node)
#endif
  {
#   include <MueLu_UseShortNames.hpp>
#ifndef TPETRA_ENABLE_TEMPLATE_ORDINALS
    using LocalOrdinal = typename Tpetra::Map<>::local_ordinal_type;
    using GlobalOrdinal = typename Tpetra::Map<>::global_ordinal_type;
#endif
    MUELU_TESTING_SET_OSTREAM;
    MUELU_TESTING_LIMIT_SCOPE(Scalar,GlobalOrdinal,Node);
    out << "version: " << MueLu::Version() << std::endl;

    RCP<CoalesceDropFactory> coalesceDropFact = rcp(new CoalesceDropFactory());
    TEST_EQUALITY(coalesceDropFact != Teuchos::null, true);

  } //Constructor

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  TEUCHOS_UNIT_TEST_TEMPLATE_4_DECL(CoalesceDropFactory, Build, Scalar, LocalOrdinal, GlobalOrdinal, Node)
#else
  TEUCHOS_UNIT_TEST_TEMPLATE_2_DECL(CoalesceDropFactory, Build, Scalar, Node)
#endif
  {
#   include <MueLu_UseShortNames.hpp>
#ifndef TPETRA_ENABLE_TEMPLATE_ORDINALS
    using LocalOrdinal = typename Tpetra::Map<>::local_ordinal_type;
    using GlobalOrdinal = typename Tpetra::Map<>::global_ordinal_type;
#endif
    MUELU_TESTING_SET_OSTREAM;
    MUELU_TESTING_LIMIT_SCOPE(Scalar,GlobalOrdinal,Node);
    out << "version: " << MueLu::Version() << std::endl;

    RCP<const Teuchos::Comm<int> > comm = Parameters::getDefaultComm();

    Level fineLevel;
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    TestHelpers::TestFactory<SC,LO,GO,NO>::createSingleLevelHierarchy(fineLevel);
#else
    TestHelpers::TestFactory<SC,NO>::createSingleLevelHierarchy(fineLevel);
#endif

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    RCP<Matrix> A = TestHelpers::TestFactory<SC,LO,GO,NO>::Build1DPoisson(36);
#else
    RCP<Matrix> A = TestHelpers::TestFactory<SC,NO>::Build1DPoisson(36);
#endif
    fineLevel.Set("A", A);

    CoalesceDropFactory coalesceDropFact;
    fineLevel.Request("Graph",&coalesceDropFact);
    fineLevel.Request("DofsPerNode", &coalesceDropFact);

    coalesceDropFact.Build(fineLevel);

    RCP<GraphBase> graph = fineLevel.Get<RCP<GraphBase> >("Graph", &coalesceDropFact);
    LO myDofsPerNode = fineLevel.Get<LO>("DofsPerNode", &coalesceDropFact);
    TEST_EQUALITY(Teuchos::as<int>(myDofsPerNode) == 1, true);

    const RCP<const Map> myImportMap = graph->GetImportMap(); // < note that the ImportMap is built from the column map of the matrix A WITHOUT dropping!
    const RCP<const Map> myDomainMap = graph->GetDomainMap();

    TEST_EQUALITY(myImportMap->getMaxAllGlobalIndex(), 35);
    TEST_EQUALITY(myImportMap->getMinAllGlobalIndex(), 0);
    TEST_EQUALITY(myImportMap->getMinLocalIndex(),0);
    TEST_EQUALITY(myImportMap->getGlobalNumElements(),Teuchos::as<size_t>(36 + (comm->getSize()-1)*2));

    TEST_EQUALITY(myDomainMap->getMaxAllGlobalIndex(), 35);
    TEST_EQUALITY(myDomainMap->getMinAllGlobalIndex(), 0);
    TEST_EQUALITY(myDomainMap->getMinLocalIndex(),0);
    TEST_EQUALITY(myDomainMap->getGlobalNumElements(),36);
  } //Build

  // TODO remove this
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  TEUCHOS_UNIT_TEST_TEMPLATE_4_DECL(CoalesceDropFactory, PreDrop, Scalar, LocalOrdinal, GlobalOrdinal, Node)
#else
  TEUCHOS_UNIT_TEST_TEMPLATE_2_DECL(CoalesceDropFactory, PreDrop, Scalar, Node)
#endif
  {
#   include <MueLu_UseShortNames.hpp>
#ifndef TPETRA_ENABLE_TEMPLATE_ORDINALS
    using LocalOrdinal = typename Tpetra::Map<>::local_ordinal_type;
    using GlobalOrdinal = typename Tpetra::Map<>::global_ordinal_type;
#endif
    MUELU_TESTING_SET_OSTREAM;
    MUELU_TESTING_LIMIT_SCOPE(Scalar,GlobalOrdinal,Node);
    out << "version: " << MueLu::Version() << std::endl;

    Level fineLevel;
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    TestHelpers::TestFactory<SC,LO,GO,NO>::createSingleLevelHierarchy(fineLevel);
#else
    TestHelpers::TestFactory<SC,NO>::createSingleLevelHierarchy(fineLevel);
#endif

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    RCP<Matrix> A = TestHelpers::TestFactory<SC,LO,GO,NO>::Build1DPoisson(3);
#else
    RCP<Matrix> A = TestHelpers::TestFactory<SC,NO>::Build1DPoisson(3);
#endif
    fineLevel.Set("A", A);
    A->describe(out,Teuchos::VERB_EXTREME);

    CoalesceDropFactory dropFact = CoalesceDropFactory();
    dropFact.SetVerbLevel(MueLu::Extreme);
    dropFact.SetPreDropFunction(rcp(new PreDropFunctionConstVal(0.00001)));

    fineLevel.Request("Graph", &dropFact);

    dropFact.Build(fineLevel);

    fineLevel.print(out);
    RCP<GraphBase> graph = fineLevel.Get<RCP<GraphBase> >("Graph", &dropFact);

    std::cout << graph->GetDomainMap()->getGlobalNumElements() << std::endl;
    graph->print(out, MueLu::Debug);

    //    TEST_EQUALITY(1 == 0, true);

  } //PreDrop

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  TEUCHOS_UNIT_TEST_TEMPLATE_4_DECL(CoalesceDropFactory, AmalgamationBasic, Scalar, LocalOrdinal, GlobalOrdinal, Node)
#else
  TEUCHOS_UNIT_TEST_TEMPLATE_2_DECL(CoalesceDropFactory, AmalgamationBasic, Scalar, Node)
#endif
  {
#ifndef TPETRA_ENABLE_TEMPLATE_ORDINALS
    using LocalOrdinal = typename Tpetra::Map<>::local_ordinal_type;
    using GlobalOrdinal = typename Tpetra::Map<>::global_ordinal_type;
#endif
    // unit test for block size 3.
    // lightweight wrap = false
#   include <MueLu_UseShortNames.hpp>
    MUELU_TESTING_SET_OSTREAM;
    MUELU_TESTING_LIMIT_SCOPE(Scalar,GlobalOrdinal,Node);
    out << "version: " << MueLu::Version() << std::endl;

    RCP<const Teuchos::Comm<int> > comm = Parameters::getDefaultComm();

    Level fineLevel;
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    TestHelpers::TestFactory<SC,LO,GO,NO>::createSingleLevelHierarchy(fineLevel);
#else
    TestHelpers::TestFactory<SC,NO>::createSingleLevelHierarchy(fineLevel);
#endif

    int blockSize=3;

    int nx = blockSize*comm->getSize();
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    RCP<Matrix> A = TestHelpers::TestFactory<SC,LO,GO,NO>::Build1DPoisson(nx);
#else
    RCP<Matrix> A = TestHelpers::TestFactory<SC,NO>::Build1DPoisson(nx);
#endif
    A->SetFixedBlockSize(blockSize, 0);
    fineLevel.Set("A", A);
    CoalesceDropFactory dropFact = CoalesceDropFactory();
    dropFact.SetParameter("lightweight wrap",Teuchos::ParameterEntry(false));
    fineLevel.Request("Graph", &dropFact);
    fineLevel.Request("DofsPerNode", &dropFact);

    dropFact.Build(fineLevel);

    fineLevel.print(out);
    RCP<GraphBase> graph = fineLevel.Get<RCP<GraphBase> >("Graph", &dropFact);
    LO myDofsPerNode = fineLevel.Get<LO>("DofsPerNode", &dropFact);
    TEST_EQUALITY(Teuchos::as<int>(graph->GetDomainMap()->getGlobalNumElements()) == comm->getSize(), true);
    TEST_EQUALITY(Teuchos::as<int>(myDofsPerNode) == blockSize, true);
    bool bCorrectGraph = false;
    if (comm->getSize() == 1 && graph->getNeighborVertices(0).size() == 1) {
      bCorrectGraph = true;
    } else {
      if (comm->getRank() == 0 || comm->getRank() == comm->getSize()-1) {
        if (graph->getNeighborVertices(0).size() == 2) bCorrectGraph = true;
      }
      else {
        if (graph->getNeighborVertices(0).size() == blockSize) bCorrectGraph = true;
      }
    }
    TEST_EQUALITY(bCorrectGraph, true);

    const RCP<const Map> myImportMap = graph->GetImportMap(); // < note that the ImportMap is built from the column map of the matrix A WITHOUT dropping!
    const RCP<const Map> myDomainMap = graph->GetDomainMap();

    TEST_EQUALITY(myImportMap->getMaxAllGlobalIndex(), comm->getSize()-1);
    TEST_EQUALITY(myImportMap->getMinAllGlobalIndex(), 0);
    TEST_EQUALITY(myImportMap->getMinLocalIndex(),0);
    TEST_EQUALITY(myImportMap->getGlobalNumElements(),Teuchos::as<size_t>(comm->getSize()+2*(comm->getSize()-1))); //u,v and p
    if (comm->getSize()>1) {
      size_t numLocalRowMapElts = graph->GetNodeNumVertices();
      size_t numLocalImportElts = myImportMap->getNodeNumElements();
      if (comm->getRank() == 0 || comm->getRank() == comm->getSize()-1) {
        TEST_EQUALITY(Teuchos::as<bool>(numLocalImportElts==numLocalRowMapElts+1), true);
      } else {
        TEST_EQUALITY(Teuchos::as<bool>(numLocalImportElts==numLocalRowMapElts+2), true);
      }
    }
    if (comm->getSize()>1) {
      size_t numLocalRowMapElts = graph->GetNodeNumVertices();
      size_t maxLocalIndex = myImportMap->getMaxLocalIndex();
      if (comm->getRank() == 0 || comm->getRank() == comm->getSize()-1) {
        TEST_EQUALITY(Teuchos::as<bool>(maxLocalIndex==numLocalRowMapElts*blockSize-2), true);
      } else {
        TEST_EQUALITY(Teuchos::as<bool>(maxLocalIndex==numLocalRowMapElts*blockSize-1), true);
      }
    }

    TEST_EQUALITY(myDomainMap->getMaxAllGlobalIndex(), comm->getSize()-1);
    TEST_EQUALITY(myDomainMap->getMinAllGlobalIndex(), 0);
    TEST_EQUALITY(myDomainMap->getMinLocalIndex(),0);
    TEST_EQUALITY(myDomainMap->getMaxLocalIndex(),0);
    TEST_EQUALITY(myDomainMap->getGlobalNumElements(),Teuchos::as<size_t>(comm->getSize()));
    TEST_EQUALITY(Teuchos::as<bool>(myDomainMap->getNodeNumElements()==1), true);
  } // AmalgamationBasic

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  TEUCHOS_UNIT_TEST_TEMPLATE_4_DECL(CoalesceDropFactory, AmalgamationStrided, Scalar, LocalOrdinal, GlobalOrdinal, Node)
#else
  TEUCHOS_UNIT_TEST_TEMPLATE_2_DECL(CoalesceDropFactory, AmalgamationStrided, Scalar, Node)
#endif
  {
#ifndef TPETRA_ENABLE_TEMPLATE_ORDINALS
    using LocalOrdinal = typename Tpetra::Map<>::local_ordinal_type;
    using GlobalOrdinal = typename Tpetra::Map<>::global_ordinal_type;
#endif
    // unit test for block size 3 using a strided map
    // lightweight wrap = false
#   include <MueLu_UseShortNames.hpp>
    MUELU_TESTING_SET_OSTREAM;
    MUELU_TESTING_LIMIT_SCOPE(Scalar,GlobalOrdinal,Node);
    out << "version: " << MueLu::Version() << std::endl;

    RCP<const Teuchos::Comm<int> > comm = Parameters::getDefaultComm();

    Level fineLevel;
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    TestHelpers::TestFactory<SC,LO,GO,NO>::createSingleLevelHierarchy(fineLevel);
#else
    TestHelpers::TestFactory<SC,NO>::createSingleLevelHierarchy(fineLevel);
#endif

    int blockSize = 3;
    int nx = blockSize*comm->getSize();
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    RCP<Matrix> A = TestHelpers::TestFactory<SC,LO,GO,NO>::Build1DPoisson(nx);
#else
    RCP<Matrix> A = TestHelpers::TestFactory<SC,NO>::Build1DPoisson(nx);
#endif

    std::vector<size_t> stridingInfo;
    stridingInfo.push_back(Teuchos::as<size_t>(blockSize));
    LocalOrdinal stridedBlockId = -1;

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    RCP<const Xpetra::StridedMap<LocalOrdinal, GlobalOrdinal, Node> > stridedRangeMap = Xpetra::StridedMapFactory<LocalOrdinal, GlobalOrdinal, Node>::Build(
#else
    RCP<const Xpetra::StridedMap<Node> > stridedRangeMap = Xpetra::StridedMapFactory<Node>::Build(
#endif
                                                  A->getRangeMap(),
                                                  stridingInfo,
                                                  stridedBlockId,
                                                  0 /*offset*/
                                                  );
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    RCP<const Map> stridedDomainMap = Xpetra::StridedMapFactory<LocalOrdinal, GlobalOrdinal, Node>::Build(
#else
    RCP<const Map> stridedDomainMap = Xpetra::StridedMapFactory<Node>::Build(
#endif
                                            A->getDomainMap(),
                                            stridingInfo,
                                            stridedBlockId,
                                            0 /*offset*/
                                            );

    if(A->IsView("stridedMaps") == true) A->RemoveView("stridedMaps");
    A->CreateView("stridedMaps", stridedRangeMap, stridedDomainMap);

    fineLevel.Set("A", A);
    CoalesceDropFactory dropFact = CoalesceDropFactory();
    dropFact.SetParameter("lightweight wrap",Teuchos::ParameterEntry(false));
    fineLevel.Request("Graph", &dropFact);
    fineLevel.Request("DofsPerNode", &dropFact);

    dropFact.Build(fineLevel);
    fineLevel.print(out);
    RCP<GraphBase> graph = fineLevel.Get<RCP<GraphBase> >("Graph", &dropFact);
    LO myDofsPerNode = fineLevel.Get<LO>("DofsPerNode", &dropFact);
    TEST_EQUALITY(Teuchos::as<int>(graph->GetDomainMap()->getGlobalNumElements()) == comm->getSize(), true);
    TEST_EQUALITY(Teuchos::as<int>(myDofsPerNode) == blockSize, true);
    bool bCorrectGraph = false;
    if (comm->getSize() == 1 && graph->getNeighborVertices(0).size() == 1) {
      bCorrectGraph = true;
    } else {
      if (comm->getRank() == 0 || comm->getRank() == comm->getSize()-1) {
        if (graph->getNeighborVertices(0).size() == 2) bCorrectGraph = true;
      }
      else {
        if (graph->getNeighborVertices(0).size() == blockSize) bCorrectGraph = true;
      }
    }
    TEST_EQUALITY(bCorrectGraph, true);

    const RCP<const Map> myImportMap = graph->GetImportMap(); // < note that the ImportMap is built from the column map of the matrix A WITHOUT dropping!
    const RCP<const Map> myDomainMap = graph->GetDomainMap();

    TEST_EQUALITY(myImportMap->getMaxAllGlobalIndex(), comm->getSize()-1);
    TEST_EQUALITY(myImportMap->getMinAllGlobalIndex(), 0);
    TEST_EQUALITY(myImportMap->getMinLocalIndex(),0);
    TEST_EQUALITY(myImportMap->getGlobalNumElements(),Teuchos::as<size_t>(comm->getSize()+2*(comm->getSize()-1)));
    if (comm->getSize()>1) {
      size_t numLocalRowMapElts = graph->GetNodeNumVertices();
      size_t numLocalImportElts = myImportMap->getNodeNumElements();
      if (comm->getRank() == 0 || comm->getRank() == comm->getSize()-1) {
        TEST_EQUALITY(Teuchos::as<bool>(numLocalImportElts==numLocalRowMapElts+1), true);
      } else {
        TEST_EQUALITY(Teuchos::as<bool>(numLocalImportElts==numLocalRowMapElts+2), true);
      }
    }
    if (comm->getSize()>1) {
      size_t numLocalRowMapElts = graph->GetNodeNumVertices();
      size_t maxLocalIndex = myImportMap->getMaxLocalIndex();
      if (comm->getRank() == 0 || comm->getRank() == comm->getSize()-1) {
        TEST_EQUALITY(Teuchos::as<bool>(maxLocalIndex==numLocalRowMapElts*blockSize-2), true);
      } else {
        TEST_EQUALITY(Teuchos::as<bool>(maxLocalIndex==numLocalRowMapElts*blockSize-1), true);
      }
    }

    TEST_EQUALITY(myDomainMap->getMaxAllGlobalIndex(), comm->getSize()-1);
    TEST_EQUALITY(myDomainMap->getMinAllGlobalIndex(), 0);
    TEST_EQUALITY(myDomainMap->getMinLocalIndex(),0);
    TEST_EQUALITY(myDomainMap->getMaxLocalIndex(),0);
    TEST_EQUALITY(myDomainMap->getGlobalNumElements(),Teuchos::as<size_t>(comm->getSize()));
    TEST_EQUALITY(Teuchos::as<bool>(myDomainMap->getNodeNumElements()==1), true);
  } // AmalgamationStrided

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  TEUCHOS_UNIT_TEST_TEMPLATE_4_DECL(CoalesceDropFactory, AmalgamationStrided2, Scalar, LocalOrdinal, GlobalOrdinal, Node)
#else
  TEUCHOS_UNIT_TEST_TEMPLATE_2_DECL(CoalesceDropFactory, AmalgamationStrided2, Scalar, Node)
#endif
  {
#ifndef TPETRA_ENABLE_TEMPLATE_ORDINALS
    using LocalOrdinal = typename Tpetra::Map<>::local_ordinal_type;
    using GlobalOrdinal = typename Tpetra::Map<>::global_ordinal_type;
#endif
    // unit test for block size 3 = (2,1). wrap block 0
    // lightweight wrap = false
#   include <MueLu_UseShortNames.hpp>
    MUELU_TESTING_SET_OSTREAM;
    MUELU_TESTING_LIMIT_SCOPE(Scalar,GlobalOrdinal,Node);
    out << "version: " << MueLu::Version() << std::endl;

    RCP<const Teuchos::Comm<int> > comm = Parameters::getDefaultComm();
    Xpetra::UnderlyingLib lib = TestHelpers::Parameters::getLib();

    // create strided map information
    std::vector<size_t> stridingInfo;
    stridingInfo.push_back(Teuchos::as<size_t>(2));
    stridingInfo.push_back(Teuchos::as<size_t>(1));
    LocalOrdinal stridedBlockId = 0;

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    RCP<const StridedMap> dofMap = Xpetra::StridedMapFactory<LocalOrdinal, GlobalOrdinal, Node>::Build(lib, 3*comm->getSize(), 0,
#else
    RCP<const StridedMap> dofMap = Xpetra::StridedMapFactory<Node>::Build(lib, 3*comm->getSize(), 0,
#endif
                                  stridingInfo, comm,
                                  stridedBlockId /*blockId*/, 0 /*offset*/);

    /////////////////////////////////////////////////////

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    Teuchos::RCP<Matrix> mtx = TestHelpers::TestFactory<SC,LO,GO,NO>::BuildTridiag(dofMap, 2.0, -1.0, -1.0);
#else
    Teuchos::RCP<Matrix> mtx = TestHelpers::TestFactory<SC,NO>::BuildTridiag(dofMap, 2.0, -1.0, -1.0);
#endif

    Level fineLevel;
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    TestHelpers::TestFactory<SC,LO,GO,NO>::createSingleLevelHierarchy(fineLevel);
#else
    TestHelpers::TestFactory<SC,NO>::createSingleLevelHierarchy(fineLevel);
#endif

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    RCP<const Xpetra::StridedMap<LocalOrdinal, GlobalOrdinal, Node> > stridedRangeMap = Xpetra::StridedMapFactory<LocalOrdinal, GlobalOrdinal, Node>::Build(
#else
    RCP<const Xpetra::StridedMap<Node> > stridedRangeMap = Xpetra::StridedMapFactory<Node>::Build(
#endif
                                                  mtx->getRangeMap(),
                                                  stridingInfo,
                                                  stridedBlockId,
                                                  0 /*offset*/
                                                  );
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    RCP<const Map> stridedDomainMap = Xpetra::StridedMapFactory<LocalOrdinal, GlobalOrdinal, Node>::Build(
#else
    RCP<const Map> stridedDomainMap = Xpetra::StridedMapFactory<Node>::Build(
#endif
                                            mtx->getDomainMap(),
                                            stridingInfo,
                                            stridedBlockId,
                                            0 /*offset*/
                                            );

    if(mtx->IsView("stridedMaps") == true) mtx->RemoveView("stridedMaps");
    mtx->CreateView("stridedMaps", stridedRangeMap, stridedDomainMap);     // should have holes in these maps

    fineLevel.Set("A", mtx);
    CoalesceDropFactory dropFact = CoalesceDropFactory();
    dropFact.SetParameter("lightweight wrap",Teuchos::ParameterEntry(false));
    fineLevel.Request("Graph", &dropFact);
    fineLevel.Request("DofsPerNode", &dropFact);

    dropFact.Build(fineLevel);

    fineLevel.print(out);
    RCP<GraphBase> graph = fineLevel.Get<RCP<GraphBase> >("Graph", &dropFact);

    LO myDofsPerNode = fineLevel.Get<LO>("DofsPerNode", &dropFact);
    TEST_EQUALITY(Teuchos::as<int>(graph->GetDomainMap()->getGlobalNumElements()) == comm->getSize(), true);
    TEST_EQUALITY(Teuchos::as<int>(myDofsPerNode) == 3, true);
    bool bCorrectGraph = false;
    if (comm->getSize() == 1 && graph->getNeighborVertices(0).size() == 1) {
      bCorrectGraph = true;
    } else {
      if (comm->getRank() == 0 || comm->getRank() == comm->getSize()-1) {
        if (graph->getNeighborVertices(0).size() == 2) bCorrectGraph = true;
      }
      else {
        if (graph->getNeighborVertices(0).size() == 3) bCorrectGraph = true;
      }
    }
    TEST_EQUALITY(bCorrectGraph, true);

    const RCP<const Map> myImportMap = graph->GetImportMap(); // < note that the ImportMap is built from the column map of the matrix A WITHOUT dropping!
    const RCP<const Map> myDomainMap = graph->GetDomainMap();

    TEST_EQUALITY(myImportMap->getMaxAllGlobalIndex(), comm->getSize()-1);
    TEST_EQUALITY(myImportMap->getMinAllGlobalIndex(), 0);
    TEST_EQUALITY(myImportMap->getMinLocalIndex(),0);
    TEST_EQUALITY(myImportMap->getGlobalNumElements(),Teuchos::as<size_t>(comm->getSize()+2*(comm->getSize()-1)));
    if (comm->getSize()>1) {
      size_t numLocalRowMapElts = graph->GetNodeNumVertices();
      size_t numLocalImportElts = myImportMap->getNodeNumElements();
      if (comm->getRank() == 0 || comm->getRank() == comm->getSize()-1) {
        TEST_EQUALITY(Teuchos::as<bool>(numLocalImportElts==numLocalRowMapElts+1), true);
      } else {
        TEST_EQUALITY(Teuchos::as<bool>(numLocalImportElts==numLocalRowMapElts+2), true);
      }
    }
    if (comm->getSize()>1) {
      size_t numLocalRowMapElts = graph->GetNodeNumVertices();
      size_t maxLocalIndex = myImportMap->getMaxLocalIndex();
      if (comm->getRank() == 0 || comm->getRank() == comm->getSize()-1) {
        TEST_EQUALITY(Teuchos::as<bool>(maxLocalIndex==numLocalRowMapElts*3-2), true);
      } else {
        TEST_EQUALITY(Teuchos::as<bool>(maxLocalIndex==numLocalRowMapElts*3-1), true);
      }
    }

    TEST_EQUALITY(myDomainMap->getMaxAllGlobalIndex(), comm->getSize()-1);
    TEST_EQUALITY(myDomainMap->getMinAllGlobalIndex(), 0);
    TEST_EQUALITY(myDomainMap->getMinLocalIndex(),0);
    TEST_EQUALITY(myDomainMap->getMaxLocalIndex(),0);
    TEST_EQUALITY(myDomainMap->getGlobalNumElements(),Teuchos::as<size_t>(comm->getSize()));
    TEST_EQUALITY(Teuchos::as<bool>(myDomainMap->getNodeNumElements()==1), true);
  } // AmalgamationStrided2

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  TEUCHOS_UNIT_TEST_TEMPLATE_4_DECL(CoalesceDropFactory, AmalgamationStridedOffset, Scalar, LocalOrdinal, GlobalOrdinal, Node)
#else
  TEUCHOS_UNIT_TEST_TEMPLATE_2_DECL(CoalesceDropFactory, AmalgamationStridedOffset, Scalar, Node)
#endif
  {
#ifndef TPETRA_ENABLE_TEMPLATE_ORDINALS
    using LocalOrdinal = typename Tpetra::Map<>::local_ordinal_type;
    using GlobalOrdinal = typename Tpetra::Map<>::global_ordinal_type;
#endif
    // unit test for block size 9 = (2,3,4). wrap block 1.
    // lightweight wrap = false
#   include <MueLu_UseShortNames.hpp>
    MUELU_TESTING_SET_OSTREAM;
    MUELU_TESTING_LIMIT_SCOPE(Scalar,GlobalOrdinal,Node);
    out << "version: " << MueLu::Version() << std::endl;

    RCP<const Teuchos::Comm<int> > comm = Parameters::getDefaultComm();
    Xpetra::UnderlyingLib lib = TestHelpers::Parameters::getLib();

    // create strided map information
    std::vector<size_t> stridingInfo;
    stridingInfo.push_back(Teuchos::as<size_t>(2));
    stridingInfo.push_back(Teuchos::as<size_t>(3));
    stridingInfo.push_back(Teuchos::as<size_t>(4));
    LocalOrdinal stridedBlockId = 1;
    GlobalOrdinal offset = 19;

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    RCP<const StridedMap> dofMap = Xpetra::StridedMapFactory<LocalOrdinal, GlobalOrdinal, Node>::Build(lib, 9*comm->getSize(), 0,
#else
    RCP<const StridedMap> dofMap = Xpetra::StridedMapFactory<Node>::Build(lib, 9*comm->getSize(), 0,
#endif
                                  stridingInfo, comm,
                                  stridedBlockId, offset);

    /////////////////////////////////////////////////////

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    Teuchos::RCP<Matrix> mtx = TestHelpers::TestFactory<SC,LO,GO,NO>::BuildTridiag(dofMap, 2.0, -1.0, -3.0);
#else
    Teuchos::RCP<Matrix> mtx = TestHelpers::TestFactory<SC,NO>::BuildTridiag(dofMap, 2.0, -1.0, -3.0);
#endif

    Level fineLevel;
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    TestHelpers::TestFactory<SC,LO,GO,NO>::createSingleLevelHierarchy(fineLevel);
#else
    TestHelpers::TestFactory<SC,NO>::createSingleLevelHierarchy(fineLevel);
#endif

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    RCP<const Map> stridedRangeMap = Xpetra::StridedMapFactory<LocalOrdinal, GlobalOrdinal, Node>::Build(
#else
    RCP<const Map> stridedRangeMap = Xpetra::StridedMapFactory<Node>::Build(
#endif
                                                  mtx->getRangeMap(),
                                                  stridingInfo,
                                                  stridedBlockId,
                                                  offset
                                                  );
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    RCP<const Map> stridedDomainMap = Xpetra::StridedMapFactory<LocalOrdinal, GlobalOrdinal, Node>::Build(
#else
    RCP<const Map> stridedDomainMap = Xpetra::StridedMapFactory<Node>::Build(
#endif
                                            mtx->getDomainMap(),
                                            stridingInfo,
                                            stridedBlockId,
                                            offset
                                            );

    if(mtx->IsView("stridedMaps") == true) mtx->RemoveView("stridedMaps");
    mtx->CreateView("stridedMaps", stridedRangeMap, stridedDomainMap);

    fineLevel.Set("A", mtx);
    CoalesceDropFactory dropFact = CoalesceDropFactory();
    dropFact.SetParameter("lightweight wrap",Teuchos::ParameterEntry(false));
    fineLevel.Request("Graph", &dropFact);
    fineLevel.Request("DofsPerNode", &dropFact);

    dropFact.Build(fineLevel);

    fineLevel.print(out);
    RCP<GraphBase> graph = fineLevel.Get<RCP<GraphBase> >("Graph", &dropFact);

    LO myDofsPerNode = fineLevel.Get<LO>("DofsPerNode", &dropFact);
    TEST_EQUALITY(Teuchos::as<int>(graph->GetDomainMap()->getGlobalNumElements()) == comm->getSize(), true);
    TEST_EQUALITY(Teuchos::as<int>(myDofsPerNode) == 9, true);
    bool bCorrectGraph = false;
    if (comm->getSize() == 1 && graph->getNeighborVertices(0).size() == 1) {
      bCorrectGraph = true;
    } else {
      if (comm->getRank() == 0 || comm->getRank() == comm->getSize()-1) {
        if (graph->getNeighborVertices(0).size() == 2) bCorrectGraph = true;
      }
      else {
        if (graph->getNeighborVertices(0).size() == 3) bCorrectGraph = true;
      }
    }
    TEST_EQUALITY(bCorrectGraph, true);

    const RCP<const Map> myImportMap = graph->GetImportMap(); // < note that the ImportMap is built from the column map of the matrix A WITHOUT dropping!
    const RCP<const Map> myDomainMap = graph->GetDomainMap();

    TEST_EQUALITY(myImportMap->getMaxAllGlobalIndex(), comm->getSize()-1);
    TEST_EQUALITY(myImportMap->getMinAllGlobalIndex(), 0);
    TEST_EQUALITY(myImportMap->getMinLocalIndex(),0);
    TEST_EQUALITY(myImportMap->getGlobalNumElements(),Teuchos::as<size_t>(comm->getSize()+2*(comm->getSize()-1)));
    if (comm->getSize()>1) {
      size_t numLocalRowMapElts = graph->GetNodeNumVertices();
      size_t numLocalImportElts = myImportMap->getNodeNumElements();
      if (comm->getRank() == 0 || comm->getRank() == comm->getSize()-1) {
        TEST_EQUALITY(Teuchos::as<bool>(numLocalImportElts==numLocalRowMapElts+1), true);
      } else {
        TEST_EQUALITY(Teuchos::as<bool>(numLocalImportElts==numLocalRowMapElts+2), true);
      }
    }
    if (comm->getSize()>1) {
      size_t numLocalRowMapElts = graph->GetNodeNumVertices();
      size_t maxLocalIndex = myImportMap->getMaxLocalIndex();
      if (comm->getRank() == 0 || comm->getRank() == comm->getSize()-1) {
        TEST_EQUALITY(Teuchos::as<bool>(maxLocalIndex==numLocalRowMapElts*3-2), true);
      } else {
        TEST_EQUALITY(Teuchos::as<bool>(maxLocalIndex==numLocalRowMapElts*3-1), true);
      }
    }

    TEST_EQUALITY(myDomainMap->getMaxAllGlobalIndex(), comm->getSize()-1);
    TEST_EQUALITY(myDomainMap->getMinAllGlobalIndex(), 0);
    TEST_EQUALITY(myDomainMap->getMinLocalIndex(),0);
    TEST_EQUALITY(myDomainMap->getMaxLocalIndex(),0);
    TEST_EQUALITY(myDomainMap->getGlobalNumElements(),Teuchos::as<size_t>(comm->getSize()));
    TEST_EQUALITY(Teuchos::as<bool>(myDomainMap->getNodeNumElements()==1), true);
  } // AmalgamationStridedOffset

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  TEUCHOS_UNIT_TEST_TEMPLATE_4_DECL(CoalesceDropFactory, AmalgamationLightweight, Scalar, LocalOrdinal, GlobalOrdinal, Node)
#else
  TEUCHOS_UNIT_TEST_TEMPLATE_2_DECL(CoalesceDropFactory, AmalgamationLightweight, Scalar, Node)
#endif
  {
#ifndef TPETRA_ENABLE_TEMPLATE_ORDINALS
    using LocalOrdinal = typename Tpetra::Map<>::local_ordinal_type;
    using GlobalOrdinal = typename Tpetra::Map<>::global_ordinal_type;
#endif
    // unit test for block size 3
    // lightweight wrap = true
#   include <MueLu_UseShortNames.hpp>
    MUELU_TESTING_SET_OSTREAM;
    MUELU_TESTING_LIMIT_SCOPE(Scalar,GlobalOrdinal,Node);
    out << "version: " << MueLu::Version() << std::endl;

    RCP<const Teuchos::Comm<int> > comm = Parameters::getDefaultComm();
    Xpetra::UnderlyingLib lib = TestHelpers::Parameters::getLib();

    Level fineLevel;
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    TestHelpers::TestFactory<SC,LO,GO,NO>::createSingleLevelHierarchy(fineLevel);
#else
    TestHelpers::TestFactory<SC,NO>::createSingleLevelHierarchy(fineLevel);
#endif

    int blockSize=3;

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    RCP<const Map> dofMap = Xpetra::MapFactory<LocalOrdinal, GlobalOrdinal, Node>::Build(lib, blockSize*comm->getSize(), 0, comm);
    Teuchos::RCP<Matrix> mtx = TestHelpers::TestFactory<SC,LO,GO,NO>::BuildTridiag(dofMap, 2.0, -1.0, -1.0);
#else
    RCP<const Map> dofMap = Xpetra::MapFactory<Node>::Build(lib, blockSize*comm->getSize(), 0, comm);
    Teuchos::RCP<Matrix> mtx = TestHelpers::TestFactory<SC,NO>::BuildTridiag(dofMap, 2.0, -1.0, -1.0);
#endif
    mtx->SetFixedBlockSize(blockSize, 0);
    fineLevel.Set("A", mtx);

    CoalesceDropFactory dropFact = CoalesceDropFactory();
    dropFact.SetParameter("lightweight wrap",Teuchos::ParameterEntry(true));
    fineLevel.Request("Graph", &dropFact);
    fineLevel.Request("DofsPerNode", &dropFact);

    dropFact.Build(fineLevel);

    RCP<GraphBase> graph = fineLevel.Get<RCP<GraphBase> >("Graph", &dropFact);
    LO myDofsPerNode = fineLevel.Get<LO>("DofsPerNode", &dropFact);
    TEST_EQUALITY(Teuchos::as<int>(graph->GetDomainMap()->getGlobalNumElements()) == comm->getSize(), true);
    TEST_EQUALITY(Teuchos::as<int>(myDofsPerNode) == blockSize, true);
    bool bCorrectGraph = false;
    if (comm->getSize() == 1 && graph->getNeighborVertices(0).size() == 1) {
      bCorrectGraph = true;
    } else {
      if (comm->getRank() == 0 || comm->getRank() == comm->getSize()-1) {
        if (graph->getNeighborVertices(0).size() == 2) bCorrectGraph = true;
      }
      else {
        if (graph->getNeighborVertices(0).size() == blockSize) bCorrectGraph = true;
      }
    }
    TEST_EQUALITY(bCorrectGraph, true);

    const RCP<const Map> myImportMap = graph->GetImportMap(); // < note that the ImportMap is built from the column map of the matrix A WITHOUT dropping!
    const RCP<const Map> myDomainMap = graph->GetDomainMap();

    TEST_EQUALITY(myImportMap->getMaxAllGlobalIndex(), comm->getSize()-1);
    TEST_EQUALITY(myImportMap->getMinAllGlobalIndex(), 0);
    TEST_EQUALITY(myImportMap->getMinLocalIndex(),0);
    TEST_EQUALITY(myImportMap->getGlobalNumElements(),Teuchos::as<size_t>(comm->getSize()+2*(comm->getSize()-1)));
    if (comm->getSize()>1) {
      size_t numLocalRowMapElts = graph->GetNodeNumVertices();
      size_t numLocalImportElts = myImportMap->getNodeNumElements();
      if (comm->getRank() == 0 || comm->getRank() == comm->getSize()-1) {
        TEST_EQUALITY(Teuchos::as<bool>(numLocalImportElts==numLocalRowMapElts+1), true);
      } else {
        TEST_EQUALITY(Teuchos::as<bool>(numLocalImportElts==numLocalRowMapElts+2), true);
      }
    }
    if (comm->getSize()>1) {
      size_t numLocalRowMapElts = graph->GetNodeNumVertices();
      size_t maxLocalIndex = myImportMap->getMaxLocalIndex();
      if (comm->getRank() == 0 || comm->getRank() == comm->getSize()-1) {
        TEST_EQUALITY(Teuchos::as<bool>(maxLocalIndex==numLocalRowMapElts*blockSize-2), true);
      } else {
        TEST_EQUALITY(Teuchos::as<bool>(maxLocalIndex==numLocalRowMapElts*blockSize-1), true);
      }
    }

    TEST_EQUALITY(myDomainMap->getMaxAllGlobalIndex(), comm->getSize()-1);
    TEST_EQUALITY(myDomainMap->getMinAllGlobalIndex(), 0);
    TEST_EQUALITY(myDomainMap->getMinLocalIndex(),0);
    TEST_EQUALITY(myDomainMap->getMaxLocalIndex(),0);
    TEST_EQUALITY(myDomainMap->getGlobalNumElements(),Teuchos::as<size_t>(comm->getSize()));
    TEST_EQUALITY(Teuchos::as<bool>(myDomainMap->getNodeNumElements()==1), true);
  } // AmalgamationLightweight

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  TEUCHOS_UNIT_TEST_TEMPLATE_4_DECL(CoalesceDropFactory, AmalgamationLightweightDrop, Scalar, LocalOrdinal, GlobalOrdinal, Node)
#else
  TEUCHOS_UNIT_TEST_TEMPLATE_2_DECL(CoalesceDropFactory, AmalgamationLightweightDrop, Scalar, Node)
#endif
  {
#ifndef TPETRA_ENABLE_TEMPLATE_ORDINALS
    using LocalOrdinal = typename Tpetra::Map<>::local_ordinal_type;
    using GlobalOrdinal = typename Tpetra::Map<>::global_ordinal_type;
#endif
    // unit test for block size 1
    // lightweight wrap = true
    // drop small values
#   include <MueLu_UseShortNames.hpp>
    MUELU_TESTING_SET_OSTREAM;
    MUELU_TESTING_LIMIT_SCOPE(Scalar,GlobalOrdinal,Node);
    out << "version: " << MueLu::Version() << std::endl;

    RCP<const Teuchos::Comm<int> > comm = Parameters::getDefaultComm();
    Xpetra::UnderlyingLib lib = TestHelpers::Parameters::getLib();

    Level fineLevel;
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    TestHelpers::TestFactory<SC,LO,GO,NO>::createSingleLevelHierarchy(fineLevel);
#else
    TestHelpers::TestFactory<SC,NO>::createSingleLevelHierarchy(fineLevel);
#endif

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    RCP<const Map> dofMap = Xpetra::MapFactory<LocalOrdinal, GlobalOrdinal, Node>::Build(lib, 3*comm->getSize(), 0, comm);
    Teuchos::RCP<Matrix> mtx = TestHelpers::TestFactory<SC,LO,GO,NO>::BuildTridiag(dofMap, 1.0, -1.0, -0.0001);
#else
    RCP<const Map> dofMap = Xpetra::MapFactory<Node>::Build(lib, 3*comm->getSize(), 0, comm);
    Teuchos::RCP<Matrix> mtx = TestHelpers::TestFactory<SC,NO>::BuildTridiag(dofMap, 1.0, -1.0, -0.0001);
#endif
    mtx->SetFixedBlockSize(1, 0);
    fineLevel.Set("A", mtx);

    CoalesceDropFactory dropFact = CoalesceDropFactory();
    dropFact.SetParameter("lightweight wrap",Teuchos::ParameterEntry(true));
    dropFact.SetParameter("aggregation: drop tol",Teuchos::ParameterEntry(0.5));
    fineLevel.Request("Graph", &dropFact);
    fineLevel.Request("DofsPerNode", &dropFact);

    dropFact.Build(fineLevel);

    RCP<GraphBase> graph = fineLevel.Get<RCP<GraphBase> >("Graph", &dropFact);
    LO myDofsPerNode = fineLevel.Get<LO>("DofsPerNode", &dropFact);
    TEST_EQUALITY(Teuchos::as<int>(graph->GetDomainMap()->getGlobalNumElements()) == 3*comm->getSize(), true);
    TEST_EQUALITY(Teuchos::as<int>(myDofsPerNode) == 1, true);
    bool bCorrectGraph = false;
    if (comm->getSize() == 1 && graph->getNeighborVertices(0).size() == 1) {
      bCorrectGraph = true;
    } else {
      if (comm->getRank() == 0 ) {
        if (graph->getNeighborVertices(0).size() == 1) bCorrectGraph = true;
      }
      else {
        if (graph->getNeighborVertices(0).size() == 2) bCorrectGraph = true;
      }
    }
    TEST_EQUALITY(bCorrectGraph, true);

    const RCP<const Map> myImportMap = graph->GetImportMap(); // < note that the ImportMap is built from the column map of the matrix A WITHOUT dropping!
    const RCP<const Map> myDomainMap = graph->GetDomainMap();

    TEST_EQUALITY(myImportMap->getMaxAllGlobalIndex(), 3*comm->getSize()-1);
    TEST_EQUALITY(myImportMap->getMinAllGlobalIndex(), 0);
    TEST_EQUALITY(myImportMap->getMinLocalIndex(),0);
    TEST_EQUALITY(myImportMap->getGlobalNumElements(),Teuchos::as<size_t>(3*comm->getSize()+(comm->getSize()-1)*2));

    TEST_EQUALITY(myDomainMap->getMaxAllGlobalIndex(), 3*comm->getSize()-1);
    TEST_EQUALITY(myDomainMap->getMinAllGlobalIndex(), 0);
    TEST_EQUALITY(myDomainMap->getMinLocalIndex(),0);
    TEST_EQUALITY(myDomainMap->getGlobalNumElements(),Teuchos::as<size_t>(3*comm->getSize()));
    TEST_EQUALITY(Teuchos::as<bool>(myDomainMap->getNodeNumElements()==3), true);
  } // AmalgamationLightweightDrop

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  TEUCHOS_UNIT_TEST_TEMPLATE_4_DECL(CoalesceDropFactory, AmalgamationStridedLW, Scalar, LocalOrdinal, GlobalOrdinal, Node)
#else
  TEUCHOS_UNIT_TEST_TEMPLATE_2_DECL(CoalesceDropFactory, AmalgamationStridedLW, Scalar, Node)
#endif
  {
#ifndef TPETRA_ENABLE_TEMPLATE_ORDINALS
    using LocalOrdinal = typename Tpetra::Map<>::local_ordinal_type;
    using GlobalOrdinal = typename Tpetra::Map<>::global_ordinal_type;
#endif
    // unit test for block size 3 using a strided map
    // lightweight wrap = true
#   include <MueLu_UseShortNames.hpp>
    MUELU_TESTING_SET_OSTREAM;
    MUELU_TESTING_LIMIT_SCOPE(Scalar,GlobalOrdinal,Node);
    out << "version: " << MueLu::Version() << std::endl;

    RCP<const Teuchos::Comm<int> > comm = Parameters::getDefaultComm();

    Level fineLevel;
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    TestHelpers::TestFactory<SC,LO,GO,NO>::createSingleLevelHierarchy(fineLevel);
#else
    TestHelpers::TestFactory<SC,NO>::createSingleLevelHierarchy(fineLevel);
#endif

    int blockSize=3;

    int nx = blockSize*comm->getSize();
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    RCP<Matrix> A = TestHelpers::TestFactory<SC,LO,GO,NO>::Build1DPoisson(nx);
#else
    RCP<Matrix> A = TestHelpers::TestFactory<SC,NO>::Build1DPoisson(nx);
#endif

    std::vector<size_t> stridingInfo;
    stridingInfo.push_back(Teuchos::as<size_t>(blockSize));
    LocalOrdinal stridedBlockId = -1;

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    RCP<const Xpetra::StridedMap<LocalOrdinal, GlobalOrdinal, Node> > stridedRangeMap = Xpetra::StridedMapFactory<LocalOrdinal, GlobalOrdinal, Node>::Build(
#else
    RCP<const Xpetra::StridedMap<Node> > stridedRangeMap = Xpetra::StridedMapFactory<Node>::Build(
#endif
                                                  A->getRangeMap(),
                                                  stridingInfo,
                                                  stridedBlockId,
                                                  0 /*offset*/
                                                  );
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    RCP<const Map> stridedDomainMap = Xpetra::StridedMapFactory<LocalOrdinal, GlobalOrdinal, Node>::Build(
#else
    RCP<const Map> stridedDomainMap = Xpetra::StridedMapFactory<Node>::Build(
#endif
                                            A->getDomainMap(),
                                            stridingInfo,
                                            stridedBlockId,
                                            0 /*offset*/
                                            );

    if(A->IsView("stridedMaps") == true) A->RemoveView("stridedMaps");
    A->CreateView("stridedMaps", stridedRangeMap, stridedDomainMap);

    fineLevel.Set("A", A);
    CoalesceDropFactory dropFact = CoalesceDropFactory();
    dropFact.SetParameter("lightweight wrap",Teuchos::ParameterEntry(true));
    fineLevel.Request("Graph", &dropFact);
    fineLevel.Request("DofsPerNode", &dropFact);

    dropFact.Build(fineLevel);

    fineLevel.print(out);
    RCP<GraphBase> graph = fineLevel.Get<RCP<GraphBase> >("Graph", &dropFact);
    LO myDofsPerNode = fineLevel.Get<LO>("DofsPerNode", &dropFact);
    TEST_EQUALITY(Teuchos::as<int>(graph->GetDomainMap()->getGlobalNumElements()) == comm->getSize(), true);
    TEST_EQUALITY(Teuchos::as<int>(myDofsPerNode) == blockSize, true);
    bool bCorrectGraph = false;
    if (comm->getSize() == 1 && graph->getNeighborVertices(0).size() == 1) {
      bCorrectGraph = true;
    } else {
      if (comm->getRank() == 0 || comm->getRank() == comm->getSize()-1) {
        if (graph->getNeighborVertices(0).size() == 2) bCorrectGraph = true;
      }
      else {
        if (graph->getNeighborVertices(0).size() == blockSize) bCorrectGraph = true;
      }
    }
    TEST_EQUALITY(bCorrectGraph, true);

    const RCP<const Map> myImportMap = graph->GetImportMap(); // < note that the ImportMap is built from the column map of the matrix A WITHOUT dropping!
    const RCP<const Map> myDomainMap = graph->GetDomainMap();

    TEST_EQUALITY(myImportMap->getMaxAllGlobalIndex(), comm->getSize()-1);
    TEST_EQUALITY(myImportMap->getMinAllGlobalIndex(), 0);
    TEST_EQUALITY(myImportMap->getMinLocalIndex(),0);
    TEST_EQUALITY(myImportMap->getGlobalNumElements(),Teuchos::as<size_t>(comm->getSize()+2*(comm->getSize()-1)));
    if (comm->getSize()>1) {
      size_t numLocalRowMapElts = graph->GetNodeNumVertices();
      size_t numLocalImportElts = myImportMap->getNodeNumElements();
      if (comm->getRank() == 0 || comm->getRank() == comm->getSize()-1) {
        TEST_EQUALITY(Teuchos::as<bool>(numLocalImportElts==numLocalRowMapElts+1), true);
      } else {
        TEST_EQUALITY(Teuchos::as<bool>(numLocalImportElts==numLocalRowMapElts+2), true);
      }
    }
    if (comm->getSize()>1) {
      size_t numLocalRowMapElts = graph->GetNodeNumVertices();
      size_t maxLocalIndex = myImportMap->getMaxLocalIndex();
      if (comm->getRank() == 0 || comm->getRank() == comm->getSize()-1) {
        TEST_EQUALITY(Teuchos::as<bool>(maxLocalIndex==numLocalRowMapElts*blockSize-2), true);
      } else {
        TEST_EQUALITY(Teuchos::as<bool>(maxLocalIndex==numLocalRowMapElts*blockSize-1), true);
      }
    }

    TEST_EQUALITY(myDomainMap->getMaxAllGlobalIndex(), comm->getSize()-1);
    TEST_EQUALITY(myDomainMap->getMinAllGlobalIndex(), 0);
    TEST_EQUALITY(myDomainMap->getMinLocalIndex(),0);
    TEST_EQUALITY(myDomainMap->getMaxLocalIndex(),0);
    TEST_EQUALITY(myDomainMap->getGlobalNumElements(),Teuchos::as<size_t>(comm->getSize()));
    TEST_EQUALITY(Teuchos::as<bool>(myDomainMap->getNodeNumElements()==1), true);
  } // AmalgamationStridedLW

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  TEUCHOS_UNIT_TEST_TEMPLATE_4_DECL(CoalesceDropFactory, AmalgamationStrided2LW, Scalar, LocalOrdinal, GlobalOrdinal, Node)
#else
  TEUCHOS_UNIT_TEST_TEMPLATE_2_DECL(CoalesceDropFactory, AmalgamationStrided2LW, Scalar, Node)
#endif
  {
#ifndef TPETRA_ENABLE_TEMPLATE_ORDINALS
    using LocalOrdinal = typename Tpetra::Map<>::local_ordinal_type;
    using GlobalOrdinal = typename Tpetra::Map<>::global_ordinal_type;
#endif
    // unit test for block size 3 = (2,1). wrap block 0
    // lightweight wrap = true
#   include <MueLu_UseShortNames.hpp>
    MUELU_TESTING_SET_OSTREAM;
    MUELU_TESTING_LIMIT_SCOPE(Scalar,GlobalOrdinal,Node);
    out << "version: " << MueLu::Version() << std::endl;

    RCP<const Teuchos::Comm<int> > comm = Parameters::getDefaultComm();
    Xpetra::UnderlyingLib lib = TestHelpers::Parameters::getLib();

    // create strided map information
    std::vector<size_t> stridingInfo;
    stridingInfo.push_back(Teuchos::as<size_t>(2));
    stridingInfo.push_back(Teuchos::as<size_t>(1));
    LocalOrdinal stridedBlockId = 0;

    int blockSize=3;

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    RCP<const StridedMap> dofMap = Xpetra::StridedMapFactory<LocalOrdinal, GlobalOrdinal, Node>::Build(lib, blockSize*comm->getSize(), 0,
#else
    RCP<const StridedMap> dofMap = Xpetra::StridedMapFactory<Node>::Build(lib, blockSize*comm->getSize(), 0,
#endif
                                  stridingInfo, comm,
                                  stridedBlockId /*blockId*/, 0 /*offset*/);

    /////////////////////////////////////////////////////

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    Teuchos::RCP<Matrix> mtx = TestHelpers::TestFactory<SC,LO,GO,NO>::BuildTridiag(dofMap, 2.0, -1.0, -1.0);
#else
    Teuchos::RCP<Matrix> mtx = TestHelpers::TestFactory<SC,NO>::BuildTridiag(dofMap, 2.0, -1.0, -1.0);
#endif

    Level fineLevel;
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    TestHelpers::TestFactory<SC,LO,GO,NO>::createSingleLevelHierarchy(fineLevel);
#else
    TestHelpers::TestFactory<SC,NO>::createSingleLevelHierarchy(fineLevel);
#endif

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    RCP<const Xpetra::StridedMap<LocalOrdinal, GlobalOrdinal, Node> > stridedRangeMap = Xpetra::StridedMapFactory<LocalOrdinal, GlobalOrdinal, Node>::Build(
#else
    RCP<const Xpetra::StridedMap<Node> > stridedRangeMap = Xpetra::StridedMapFactory<Node>::Build(
#endif
                                                  mtx->getRangeMap(),
                                                  stridingInfo,
                                                  stridedBlockId,
                                                  0 /*offset*/
                                                  );
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    RCP<const Map> stridedDomainMap = Xpetra::StridedMapFactory<LocalOrdinal, GlobalOrdinal, Node>::Build(
#else
    RCP<const Map> stridedDomainMap = Xpetra::StridedMapFactory<Node>::Build(
#endif
                                            mtx->getDomainMap(),
                                            stridingInfo,
                                            stridedBlockId,
                                            0 /*offset*/
                                            );
    if(mtx->IsView("stridedMaps") == true) mtx->RemoveView("stridedMaps");
    mtx->CreateView("stridedMaps", stridedRangeMap, stridedDomainMap);

    fineLevel.Set("A", mtx);
    CoalesceDropFactory dropFact = CoalesceDropFactory();
    dropFact.SetParameter("lightweight wrap",Teuchos::ParameterEntry(true));
    fineLevel.Request("Graph", &dropFact);
    fineLevel.Request("DofsPerNode", &dropFact);

    dropFact.Build(fineLevel);

    fineLevel.print(out);
    RCP<GraphBase> graph = fineLevel.Get<RCP<GraphBase> >("Graph", &dropFact);

    LO myDofsPerNode = fineLevel.Get<LO>("DofsPerNode", &dropFact);
    TEST_EQUALITY(Teuchos::as<int>(graph->GetDomainMap()->getGlobalNumElements()) == comm->getSize(), true);
    TEST_EQUALITY(Teuchos::as<int>(myDofsPerNode) == blockSize, true);
    bool bCorrectGraph = false;
    if (comm->getSize() == 1 && graph->getNeighborVertices(0).size() == 1) {
      bCorrectGraph = true;
    } else {
      if (comm->getRank() == 0 || comm->getRank() == comm->getSize()-1) {
        if (graph->getNeighborVertices(0).size() == 2) bCorrectGraph = true;
      }
      else {
        if (graph->getNeighborVertices(0).size() == blockSize) bCorrectGraph = true;
      }
    }
    TEST_EQUALITY(bCorrectGraph, true);

    const RCP<const Map> myImportMap = graph->GetImportMap(); // < note that the ImportMap is built from the column map of the matrix A WITHOUT dropping!
    const RCP<const Map> myDomainMap = graph->GetDomainMap();

    TEST_EQUALITY(myImportMap->getMaxAllGlobalIndex(), comm->getSize()-1);
    TEST_EQUALITY(myImportMap->getMinAllGlobalIndex(), 0);
    TEST_EQUALITY(myImportMap->getMinLocalIndex(),0);
    TEST_EQUALITY(myImportMap->getGlobalNumElements(),Teuchos::as<size_t>(comm->getSize()+2*(comm->getSize()-1)));
    if (comm->getSize()>1) {
      size_t numLocalRowMapElts = graph->GetNodeNumVertices();
      size_t numLocalImportElts = myImportMap->getNodeNumElements();
      if (comm->getRank() == 0 || comm->getRank() == comm->getSize()-1) {
        TEST_EQUALITY(Teuchos::as<bool>(numLocalImportElts==numLocalRowMapElts+1), true);
      } else {
        TEST_EQUALITY(Teuchos::as<bool>(numLocalImportElts==numLocalRowMapElts+2), true);
      }
    }
    if (comm->getSize()>1) {
      size_t numLocalRowMapElts = graph->GetNodeNumVertices();
      size_t maxLocalIndex = myImportMap->getMaxLocalIndex();
      if (comm->getRank() == 0 || comm->getRank() == comm->getSize()-1) {
        TEST_EQUALITY(Teuchos::as<bool>(maxLocalIndex==numLocalRowMapElts*blockSize-2), true);
      } else {
        TEST_EQUALITY(Teuchos::as<bool>(maxLocalIndex==numLocalRowMapElts*blockSize-1), true);
      }
    }

    TEST_EQUALITY(myDomainMap->getMaxAllGlobalIndex(), comm->getSize()-1);
    TEST_EQUALITY(myDomainMap->getMinAllGlobalIndex(), 0);
    TEST_EQUALITY(myDomainMap->getMinLocalIndex(),0);
    TEST_EQUALITY(myDomainMap->getMaxLocalIndex(),0);
    TEST_EQUALITY(myDomainMap->getGlobalNumElements(),Teuchos::as<size_t>(comm->getSize()));
    TEST_EQUALITY(Teuchos::as<bool>(myDomainMap->getNodeNumElements()==1), true);
  } // AmalgamationStrided2LW

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  TEUCHOS_UNIT_TEST_TEMPLATE_4_DECL(CoalesceDropFactory, AmalgamationStridedOffsetLW, Scalar, LocalOrdinal, GlobalOrdinal, Node)
#else
  TEUCHOS_UNIT_TEST_TEMPLATE_2_DECL(CoalesceDropFactory, AmalgamationStridedOffsetLW, Scalar, Node)
#endif
  {
#ifndef TPETRA_ENABLE_TEMPLATE_ORDINALS
    using LocalOrdinal = typename Tpetra::Map<>::local_ordinal_type;
    using GlobalOrdinal = typename Tpetra::Map<>::global_ordinal_type;
#endif
    // unit test for block size 9 = (2,3,4). wrap block 1.
    // lightweight wrap = true
#   include <MueLu_UseShortNames.hpp>
    MUELU_TESTING_SET_OSTREAM;
    MUELU_TESTING_LIMIT_SCOPE(Scalar,GlobalOrdinal,Node);
    out << "version: " << MueLu::Version() << std::endl;

    RCP<const Teuchos::Comm<int> > comm = Parameters::getDefaultComm();
    Xpetra::UnderlyingLib lib = TestHelpers::Parameters::getLib();

    // create strided map information
    std::vector<size_t> stridingInfo;
    stridingInfo.push_back(Teuchos::as<size_t>(2));
    stridingInfo.push_back(Teuchos::as<size_t>(3));
    stridingInfo.push_back(Teuchos::as<size_t>(4));
    LocalOrdinal stridedBlockId = 1;
    GlobalOrdinal offset = 19;

    int blockSize=9;

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    RCP<const StridedMap> dofMap = Xpetra::StridedMapFactory<LocalOrdinal, GlobalOrdinal, Node>::Build(lib, blockSize*comm->getSize(), 0,
#else
    RCP<const StridedMap> dofMap = Xpetra::StridedMapFactory<Node>::Build(lib, blockSize*comm->getSize(), 0,
#endif
                                  stridingInfo, comm,
                                  stridedBlockId, offset);

    /////////////////////////////////////////////////////

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    Teuchos::RCP<Matrix> mtx = TestHelpers::TestFactory<SC,LO,GO,NO>::BuildTridiag(dofMap, 2.0, -1.0, -3.0);
#else
    Teuchos::RCP<Matrix> mtx = TestHelpers::TestFactory<SC,NO>::BuildTridiag(dofMap, 2.0, -1.0, -3.0);
#endif

    Level fineLevel;
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    TestHelpers::TestFactory<SC,LO,GO,NO>::createSingleLevelHierarchy(fineLevel);
#else
    TestHelpers::TestFactory<SC,NO>::createSingleLevelHierarchy(fineLevel);
#endif

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    RCP<const Map> stridedRangeMap = Xpetra::StridedMapFactory<LocalOrdinal, GlobalOrdinal, Node>::Build(
#else
    RCP<const Map> stridedRangeMap = Xpetra::StridedMapFactory<Node>::Build(
#endif
                                                  mtx->getRangeMap(),
                                                  stridingInfo,
                                                  stridedBlockId,
                                                  offset
                                                  );
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    RCP<const Map> stridedDomainMap = Xpetra::StridedMapFactory<LocalOrdinal, GlobalOrdinal, Node>::Build(
#else
    RCP<const Map> stridedDomainMap = Xpetra::StridedMapFactory<Node>::Build(
#endif
                                            mtx->getDomainMap(),
                                            stridingInfo,
                                            stridedBlockId,
                                            offset
                                            );

    if(mtx->IsView("stridedMaps") == true) mtx->RemoveView("stridedMaps");
    mtx->CreateView("stridedMaps", stridedRangeMap, stridedDomainMap);

    fineLevel.Set("A", mtx);
    CoalesceDropFactory dropFact = CoalesceDropFactory();
    dropFact.SetParameter("lightweight wrap",Teuchos::ParameterEntry(true));
    fineLevel.Request("Graph", &dropFact);
    fineLevel.Request("DofsPerNode", &dropFact);

    dropFact.Build(fineLevel);

    fineLevel.print(out);
    RCP<GraphBase> graph = fineLevel.Get<RCP<GraphBase> >("Graph", &dropFact);

    LO myDofsPerNode = fineLevel.Get<LO>("DofsPerNode", &dropFact);
    TEST_EQUALITY(Teuchos::as<int>(graph->GetDomainMap()->getGlobalNumElements()) == comm->getSize(), true);
    TEST_EQUALITY(Teuchos::as<int>(myDofsPerNode) == blockSize, true);
    bool bCorrectGraph = false;
    if (comm->getSize() == 1 && graph->getNeighborVertices(0).size() == 1) {
      bCorrectGraph = true;
    } else {
      if (comm->getRank() == 0 || comm->getRank() == comm->getSize()-1) {
        if (graph->getNeighborVertices(0).size() == 2) bCorrectGraph = true;
      }
      else {
        if (graph->getNeighborVertices(0).size() == 3) bCorrectGraph = true;
      }
    }
    TEST_EQUALITY(bCorrectGraph, true);

    const RCP<const Map> myImportMap = graph->GetImportMap(); // < note that the ImportMap is built from the column map of the matrix A WITHOUT dropping!
    const RCP<const Map> myDomainMap = graph->GetDomainMap();

    TEST_EQUALITY(myImportMap->getMaxAllGlobalIndex(), comm->getSize()-1);
    TEST_EQUALITY(myImportMap->getMinAllGlobalIndex(), 0);
    TEST_EQUALITY(myImportMap->getMinLocalIndex(),0);
    TEST_EQUALITY(myImportMap->getGlobalNumElements(),Teuchos::as<size_t>(comm->getSize()+2*(comm->getSize()-1)));
    if (comm->getSize()>1) {
      size_t numLocalRowMapElts = graph->GetNodeNumVertices();
      size_t numLocalImportElts = myImportMap->getNodeNumElements();
      if (comm->getRank() == 0 || comm->getRank() == comm->getSize()-1) {
        TEST_EQUALITY(Teuchos::as<bool>(numLocalImportElts==numLocalRowMapElts+1), true);
      } else {
        TEST_EQUALITY(Teuchos::as<bool>(numLocalImportElts==numLocalRowMapElts+2), true);
      }
    }

    TEST_EQUALITY(myDomainMap->getMaxAllGlobalIndex(), comm->getSize()-1);
    TEST_EQUALITY(myDomainMap->getMinAllGlobalIndex(), 0);
    TEST_EQUALITY(myDomainMap->getMinLocalIndex(),0);
    TEST_EQUALITY(myDomainMap->getMaxLocalIndex(),0);
    TEST_EQUALITY(myDomainMap->getGlobalNumElements(),Teuchos::as<size_t>(comm->getSize()));
    TEST_EQUALITY(Teuchos::as<bool>(myDomainMap->getNodeNumElements()==1), true);
  } // AmalgamationStridedOffsetLW

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  TEUCHOS_UNIT_TEST_TEMPLATE_4_DECL(CoalesceDropFactory, AmalgamationDroppingLW, Scalar, LocalOrdinal, GlobalOrdinal, Node)
#else
  TEUCHOS_UNIT_TEST_TEMPLATE_2_DECL(CoalesceDropFactory, AmalgamationDroppingLW, Scalar, Node)
#endif
  {
#ifndef TPETRA_ENABLE_TEMPLATE_ORDINALS
    using LocalOrdinal = typename Tpetra::Map<>::local_ordinal_type;
    using GlobalOrdinal = typename Tpetra::Map<>::global_ordinal_type;
#endif
    // unit test for block size 3 = (3)
    // drop small entries
    // lightweight wrap = true
#   include <MueLu_UseShortNames.hpp>
    MUELU_TESTING_SET_OSTREAM;
    MUELU_TESTING_LIMIT_SCOPE(Scalar,GlobalOrdinal,Node);
    out << "version: " << MueLu::Version() << std::endl;

    RCP<const Teuchos::Comm<int> > comm = Parameters::getDefaultComm();
    Xpetra::UnderlyingLib lib = TestHelpers::Parameters::getLib();

    Level fineLevel;
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    TestHelpers::TestFactory<SC,LO,GO,NO>::createSingleLevelHierarchy(fineLevel);
#else
    TestHelpers::TestFactory<SC,NO>::createSingleLevelHierarchy(fineLevel);
#endif

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    RCP<const Map> dofMap = Xpetra::MapFactory<LocalOrdinal, GlobalOrdinal, Node>::Build(lib, 3*comm->getSize(), 0, comm);
    Teuchos::RCP<Matrix> mtx = TestHelpers::TestFactory<SC,LO,GO,NO>::BuildTridiag(dofMap, 2.0, -1.0, 0.00001);
#else
    RCP<const Map> dofMap = Xpetra::MapFactory<Node>::Build(lib, 3*comm->getSize(), 0, comm);
    Teuchos::RCP<Matrix> mtx = TestHelpers::TestFactory<SC,NO>::BuildTridiag(dofMap, 2.0, -1.0, 0.00001);
#endif
    mtx->SetFixedBlockSize(3, 0);
    fineLevel.Set("A", mtx);

    CoalesceDropFactory dropFact = CoalesceDropFactory();
    dropFact.SetParameter("lightweight wrap",Teuchos::ParameterEntry(true));
    dropFact.SetParameter("aggregation: drop tol",Teuchos::ParameterEntry(1.0));
    fineLevel.Request("Graph", &dropFact);
    fineLevel.Request("DofsPerNode", &dropFact);

    dropFact.Build(fineLevel);

    fineLevel.print(out);
    RCP<GraphBase> graph = fineLevel.Get<RCP<GraphBase> >("Graph", &dropFact);

    LO myDofsPerNode = fineLevel.Get<LO>("DofsPerNode", &dropFact);
    TEST_EQUALITY(Teuchos::as<int>(graph->GetDomainMap()->getGlobalNumElements()) == comm->getSize(), true);
    TEST_EQUALITY(Teuchos::as<int>(myDofsPerNode) == 3, true);
    TEST_EQUALITY(graph->getNeighborVertices(0).size(), 1);

    const RCP<const Map> myImportMap = graph->GetImportMap(); // < note that the ImportMap is built from the column map of the matrix A WITHOUT dropping!
    const RCP<const Map> myDomainMap = graph->GetDomainMap();

    TEST_EQUALITY(myImportMap->getMaxAllGlobalIndex(), comm->getSize()-1);
    TEST_EQUALITY(myImportMap->getMinAllGlobalIndex(), 0);
    TEST_EQUALITY(myImportMap->getMinLocalIndex(),0);
    TEST_EQUALITY(myImportMap->getGlobalNumElements(),Teuchos::as<size_t>(comm->getSize()+2*(comm->getSize()-1)));
    if (comm->getSize()>1) {
      size_t numLocalRowMapElts = graph->GetNodeNumVertices();
      size_t numLocalImportElts = myImportMap->getNodeNumElements();
      if (comm->getRank() == 0 || comm->getRank() == comm->getSize()-1) {
        TEST_EQUALITY(Teuchos::as<bool>(numLocalImportElts==numLocalRowMapElts+1), true);
      } else {
        TEST_EQUALITY(Teuchos::as<bool>(numLocalImportElts==numLocalRowMapElts+2), true);
      }
    }

    TEST_EQUALITY(myDomainMap->getMaxAllGlobalIndex(), comm->getSize()-1);
    TEST_EQUALITY(myDomainMap->getMinAllGlobalIndex(), 0);
    TEST_EQUALITY(myDomainMap->getMaxLocalIndex(),0);
    TEST_EQUALITY(myDomainMap->getMinLocalIndex(),0);
    TEST_EQUALITY(myDomainMap->getGlobalNumElements(),Teuchos::as<size_t>(comm->getSize()));
    TEST_EQUALITY(Teuchos::as<bool>(myDomainMap->getNodeNumElements()==1), true);

  } // AmalgamationDroppingLW

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  TEUCHOS_UNIT_TEST_TEMPLATE_4_DECL(CoalesceDropFactory, AmalgamationStridedOffsetDropping2LW, Scalar, LocalOrdinal, GlobalOrdinal, Node)
#else
  TEUCHOS_UNIT_TEST_TEMPLATE_2_DECL(CoalesceDropFactory, AmalgamationStridedOffsetDropping2LW, Scalar, Node)
#endif
  {
#ifndef TPETRA_ENABLE_TEMPLATE_ORDINALS
    using LocalOrdinal = typename Tpetra::Map<>::local_ordinal_type;
    using GlobalOrdinal = typename Tpetra::Map<>::global_ordinal_type;
#endif
    // unit test for block size 9 = (2,3,4). wrap block 1.
    // drop small entries
    // lightweight wrap = true
#   include <MueLu_UseShortNames.hpp>
    MUELU_TESTING_SET_OSTREAM;
    MUELU_TESTING_LIMIT_SCOPE(Scalar,GlobalOrdinal,Node);
    out << "version: " << MueLu::Version() << std::endl;

    RCP<const Teuchos::Comm<int> > comm = Parameters::getDefaultComm();
    Xpetra::UnderlyingLib lib = TestHelpers::Parameters::getLib();

    // create strided map information
    std::vector<size_t> stridingInfo;
    stridingInfo.push_back(Teuchos::as<size_t>(2));
    stridingInfo.push_back(Teuchos::as<size_t>(3));
    stridingInfo.push_back(Teuchos::as<size_t>(4));
    LocalOrdinal stridedBlockId = 1;
    GlobalOrdinal offset = 19;

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    RCP<const StridedMap> dofMap = Xpetra::StridedMapFactory<LocalOrdinal, GlobalOrdinal, Node>::Build(lib, 9*comm->getSize(), 0,
#else
    RCP<const StridedMap> dofMap = Xpetra::StridedMapFactory<Node>::Build(lib, 9*comm->getSize(), 0,
#endif
                                  stridingInfo, comm,
                                  stridedBlockId, offset);

    /////////////////////////////////////////////////////

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    Teuchos::RCP<Matrix> mtx = TestHelpers::TestFactory<SC,LO,GO,NO>::BuildTridiag(dofMap, 2.0, 1.0, 0.0001);
#else
    Teuchos::RCP<Matrix> mtx = TestHelpers::TestFactory<SC,NO>::BuildTridiag(dofMap, 2.0, 1.0, 0.0001);
#endif

    Level fineLevel;
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    TestHelpers::TestFactory<SC,LO,GO,NO>::createSingleLevelHierarchy(fineLevel);
#else
    TestHelpers::TestFactory<SC,NO>::createSingleLevelHierarchy(fineLevel);
#endif

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    RCP<const Map> stridedRangeMap = Xpetra::StridedMapFactory<LocalOrdinal, GlobalOrdinal, Node>::Build(
#else
    RCP<const Map> stridedRangeMap = Xpetra::StridedMapFactory<Node>::Build(
#endif
                                                  mtx->getRangeMap(),
                                                  stridingInfo,
                                                  stridedBlockId,
                                                  offset
                                                  );
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    RCP<const Map> stridedDomainMap = Xpetra::StridedMapFactory<LocalOrdinal, GlobalOrdinal, Node>::Build(
#else
    RCP<const Map> stridedDomainMap = Xpetra::StridedMapFactory<Node>::Build(
#endif
                                            mtx->getDomainMap(),
                                            stridingInfo,
                                            stridedBlockId,
                                            offset
                                            );

    if(mtx->IsView("stridedMaps") == true) mtx->RemoveView("stridedMaps");
    mtx->CreateView("stridedMaps", stridedRangeMap, stridedDomainMap);

    fineLevel.Set("A", mtx);
    CoalesceDropFactory dropFact = CoalesceDropFactory();
    dropFact.SetParameter("lightweight wrap",Teuchos::ParameterEntry(true));
    dropFact.SetParameter("aggregation: drop tol",Teuchos::ParameterEntry(0.3));

    fineLevel.Request("Graph", &dropFact);
    fineLevel.Request("DofsPerNode", &dropFact);

    dropFact.Build(fineLevel);

    fineLevel.print(out);
    RCP<GraphBase> graph = fineLevel.Get<RCP<GraphBase> >("Graph", &dropFact);

    LO myDofsPerNode = fineLevel.Get<LO>("DofsPerNode", &dropFact);
    TEST_EQUALITY(Teuchos::as<int>(graph->GetDomainMap()->getGlobalNumElements()) == comm->getSize(), true);
    TEST_EQUALITY(Teuchos::as<int>(myDofsPerNode) == 9, true);
    bool bCorrectGraph = false;
    if (comm->getSize() == 1 && graph->getNeighborVertices(0).size() == 1) {
      bCorrectGraph = true;
    } else {
      if (comm->getRank() == 0) {
        if (graph->getNeighborVertices(0).size() == 1) bCorrectGraph = true;
      }
      else {
        if (graph->getNeighborVertices(0).size() == 2) bCorrectGraph = true;
      }
    }
    TEST_EQUALITY(bCorrectGraph, true);

    const RCP<const Map> myImportMap = graph->GetImportMap(); // < note that the ImportMap is built from the column map of the matrix A WITHOUT dropping!
    const RCP<const Map> myDomainMap = graph->GetDomainMap();

    TEST_EQUALITY(myImportMap->getMaxAllGlobalIndex(), comm->getSize()-1);
    TEST_EQUALITY(myImportMap->getMinAllGlobalIndex(), 0);
    TEST_EQUALITY(myImportMap->getMinLocalIndex(),0);
    TEST_EQUALITY(myImportMap->getGlobalNumElements(),Teuchos::as<size_t>(comm->getSize()+2*(comm->getSize()-1)));
    if (comm->getSize()>1) {
      size_t numLocalRowMapElts = graph->GetNodeNumVertices();
      size_t numLocalImportElts = myImportMap->getNodeNumElements();
      if (comm->getRank() == 0 || comm->getRank() == comm->getSize()-1) {
        TEST_EQUALITY(Teuchos::as<bool>(numLocalImportElts==numLocalRowMapElts+1), true);
      } else {
        TEST_EQUALITY(Teuchos::as<bool>(numLocalImportElts==numLocalRowMapElts+2), true);
      }
    }
    TEST_EQUALITY(myDomainMap->getMaxAllGlobalIndex(), comm->getSize()-1);
    TEST_EQUALITY(myDomainMap->getMinAllGlobalIndex(), 0);
    TEST_EQUALITY(myDomainMap->getMinLocalIndex(),0);
    TEST_EQUALITY(myDomainMap->getGlobalNumElements(),Teuchos::as<size_t>(comm->getSize()));
    TEST_EQUALITY(Teuchos::as<bool>(myDomainMap->getNodeNumElements()==1), true);
  } // AmalgamationStridedOffsetDropping2LW


#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  TEUCHOS_UNIT_TEST_TEMPLATE_4_DECL(CoalesceDropFactory, DistanceLaplacian, Scalar, LocalOrdinal, GlobalOrdinal, Node)
#else
  TEUCHOS_UNIT_TEST_TEMPLATE_2_DECL(CoalesceDropFactory, DistanceLaplacian, Scalar, Node)
#endif
  {
#   include <MueLu_UseShortNames.hpp>
#ifndef TPETRA_ENABLE_TEMPLATE_ORDINALS
    using LocalOrdinal = typename Tpetra::Map<>::local_ordinal_type;
    using GlobalOrdinal = typename Tpetra::Map<>::global_ordinal_type;
#endif
    typedef Teuchos::ScalarTraits<SC> STS;
    typedef typename STS::magnitudeType real_type;
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    typedef Xpetra::MultiVector<real_type,LO,GO,NO> RealValuedMultiVector;
#else
    typedef Xpetra::MultiVector<real_type,NO> RealValuedMultiVector;
#endif

    MUELU_TESTING_SET_OSTREAM;
    MUELU_TESTING_LIMIT_SCOPE(Scalar,GlobalOrdinal,Node);
    out << "version: " << MueLu::Version() << std::endl;

    RCP<const Teuchos::Comm<int> > comm = Parameters::getDefaultComm();

    Level fineLevel;
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    TestHelpers::TestFactory<SC,LO,GO,NO>::createSingleLevelHierarchy(fineLevel);
#else
    TestHelpers::TestFactory<SC,NO>::createSingleLevelHierarchy(fineLevel);
#endif

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    RCP<Matrix> A = TestHelpers::TestFactory<SC,LO,GO,NO>::Build1DPoisson(36);
#else
    RCP<Matrix> A = TestHelpers::TestFactory<SC,NO>::Build1DPoisson(36);
#endif
    fineLevel.Set("A", A);

    Teuchos::ParameterList galeriList;
    galeriList.set("nx", Teuchos::as<GlobalOrdinal>(36));
    RCP<RealValuedMultiVector> coordinates = Galeri::Xpetra::Utils::CreateCartesianCoordinates<SC,LO,GO,Map,RealValuedMultiVector>("1D", A->getRowMap(), galeriList);
    fineLevel.Set("Coordinates", coordinates);

    CoalesceDropFactory coalesceDropFact;
    // We're dropping all the interior off-diagonal entries.
    // dx = 1/36
    // L_ij = -36
    // L_ii = 72
    // criterion for dropping is |L_ij|^2 <= tol^2 * |L_ii*L_jj|
    coalesceDropFact.SetParameter("aggregation: drop tol",Teuchos::ParameterEntry(0.51));
    coalesceDropFact.SetParameter("aggregation: drop scheme",Teuchos::ParameterEntry(std::string("distance laplacian")));
    fineLevel.Request("Graph",&coalesceDropFact);
    fineLevel.Request("DofsPerNode", &coalesceDropFact);

    coalesceDropFact.Build(fineLevel);

    RCP<GraphBase> graph = fineLevel.Get<RCP<GraphBase> >("Graph", &coalesceDropFact);
    LO myDofsPerNode = fineLevel.Get<LO>("DofsPerNode", &coalesceDropFact);
    TEST_EQUALITY(Teuchos::as<int>(myDofsPerNode) == 1, true);

    const RCP<const Map> myImportMap = graph->GetImportMap(); // < note that the ImportMap is built from the column map of the matrix A WITHOUT dropping!
    const RCP<const Map> myDomainMap = graph->GetDomainMap();

    TEST_EQUALITY(myImportMap->getMaxAllGlobalIndex(), 35);
    TEST_EQUALITY(myImportMap->getMinAllGlobalIndex(), 0);
    TEST_EQUALITY(myImportMap->getMinLocalIndex(),0);
    TEST_EQUALITY(myImportMap->getGlobalNumElements(),Teuchos::as<size_t>(36 + (comm->getSize()-1)*2));

    TEST_EQUALITY(myDomainMap->getMaxAllGlobalIndex(), 35);
    TEST_EQUALITY(myDomainMap->getMinAllGlobalIndex(), 0);
    TEST_EQUALITY(myDomainMap->getMinLocalIndex(),0);
    TEST_EQUALITY(myDomainMap->getGlobalNumElements(),36);

    TEST_EQUALITY(graph->GetGlobalNumEdges(),40);

  } // DistanceLaplacian


#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  TEUCHOS_UNIT_TEST_TEMPLATE_4_DECL(CoalesceDropFactory, DistanceLaplacianCut, Scalar, LocalOrdinal, GlobalOrdinal, Node)
#else
  TEUCHOS_UNIT_TEST_TEMPLATE_2_DECL(CoalesceDropFactory, DistanceLaplacianCut, Scalar, Node)
#endif
  {
#   include <MueLu_UseShortNames.hpp>
#ifndef TPETRA_ENABLE_TEMPLATE_ORDINALS
    using LocalOrdinal = typename Tpetra::Map<>::local_ordinal_type;
    using GlobalOrdinal = typename Tpetra::Map<>::global_ordinal_type;
#endif
    typedef Teuchos::ScalarTraits<SC> STS;
    typedef typename STS::magnitudeType real_type;
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    typedef Xpetra::MultiVector<real_type,LO,GO,NO> RealValuedMultiVector;
#else
    typedef Xpetra::MultiVector<real_type,NO> RealValuedMultiVector;
#endif

    MUELU_TESTING_SET_OSTREAM;
    MUELU_TESTING_LIMIT_SCOPE(Scalar,GlobalOrdinal,Node);
    out << "version: " << MueLu::Version() << std::endl;

    RCP<const Teuchos::Comm<int> > comm = Parameters::getDefaultComm();

    Level fineLevel;
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    TestHelpers::TestFactory<SC,LO,GO,NO>::createSingleLevelHierarchy(fineLevel);
#else
    TestHelpers::TestFactory<SC,NO>::createSingleLevelHierarchy(fineLevel);
#endif

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    RCP<Matrix> A = TestHelpers::TestFactory<SC,LO,GO,NO>::Build1DPoisson(36);
#else
    RCP<Matrix> A = TestHelpers::TestFactory<SC,NO>::Build1DPoisson(36);
#endif
    fineLevel.Set("A", A);

    Teuchos::ParameterList galeriList;
    galeriList.set("nx", Teuchos::as<GlobalOrdinal>(36));
    RCP<RealValuedMultiVector> coordinates = Galeri::Xpetra::Utils::CreateCartesianCoordinates<SC,LO,GO,Map,RealValuedMultiVector>("1D", A->getRowMap(), galeriList);
    // Now we doctor the coordinates so that the off-diagonal pair row 0 will want to keep (0,1) and row 1 will want to drop (1,0)
    if(!comm->getRank()) {
      auto vals = coordinates->getDataNonConst(0);
      vals[0] = vals[0] - 2000*36;
    }

    fineLevel.Set("Coordinates", coordinates);

    CoalesceDropFactory coalesceDropFact;
    // We're dropping all the interior off-diagonal entries.
    // dx = 1/36
    // L_ij = -36
    // L_ii = 72
    // criterion for dropping is |L_ij|^2 <= tol^2 * |L_ii*L_jj|
    coalesceDropFact.SetParameter("aggregation: drop tol",Teuchos::ParameterEntry(8.0));
    coalesceDropFact.SetParameter("aggregation: drop scheme",Teuchos::ParameterEntry(std::string("distance laplacian")));
    coalesceDropFact.SetParameter("aggregation: distance laplacian algo",Teuchos::ParameterEntry(std::string("scaled cut")));
    fineLevel.Request("Graph",&coalesceDropFact);
    fineLevel.Request("DofsPerNode", &coalesceDropFact);

    coalesceDropFact.Build(fineLevel);

    RCP<GraphBase> graph = fineLevel.Get<RCP<GraphBase> >("Graph", &coalesceDropFact);
    LO myDofsPerNode = fineLevel.Get<LO>("DofsPerNode", &coalesceDropFact);
    TEST_EQUALITY(Teuchos::as<int>(myDofsPerNode) == 1, true);

    const RCP<const Map> myImportMap = graph->GetImportMap(); // < note that the ImportMap is built from the column map of the matrix A WITHOUT dropping!
    const RCP<const Map> myDomainMap = graph->GetDomainMap();

    TEST_EQUALITY(myImportMap->getMaxAllGlobalIndex(), 35);
    TEST_EQUALITY(myImportMap->getMinAllGlobalIndex(), 0);
    TEST_EQUALITY(myImportMap->getMinLocalIndex(),0);
    TEST_EQUALITY(myImportMap->getGlobalNumElements(),Teuchos::as<size_t>(36 + (comm->getSize()-1)*2));

    TEST_EQUALITY(myDomainMap->getMaxAllGlobalIndex(), 35);
    TEST_EQUALITY(myDomainMap->getMinAllGlobalIndex(), 0);
    TEST_EQUALITY(myDomainMap->getMinLocalIndex(),0);
    TEST_EQUALITY(myDomainMap->getGlobalNumElements(),36);

    TEST_EQUALITY(graph->GetGlobalNumEdges(),105);

  } // DistanceLaplacianCut

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  TEUCHOS_UNIT_TEST_TEMPLATE_4_DECL(CoalesceDropFactory, DistanceLaplacianCutSym, Scalar, LocalOrdinal, GlobalOrdinal, Node)
#else
  TEUCHOS_UNIT_TEST_TEMPLATE_2_DECL(CoalesceDropFactory, DistanceLaplacianCutSym, Scalar, Node)
#endif
  {
#   include <MueLu_UseShortNames.hpp>
#ifndef TPETRA_ENABLE_TEMPLATE_ORDINALS
    using LocalOrdinal = typename Tpetra::Map<>::local_ordinal_type;
    using GlobalOrdinal = typename Tpetra::Map<>::global_ordinal_type;
#endif
    typedef Teuchos::ScalarTraits<SC> STS;
    typedef typename STS::magnitudeType real_type;
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    typedef Xpetra::MultiVector<real_type,LO,GO,NO> RealValuedMultiVector;
#else
    typedef Xpetra::MultiVector<real_type,NO> RealValuedMultiVector;
#endif

    MUELU_TESTING_SET_OSTREAM;
    MUELU_TESTING_LIMIT_SCOPE(Scalar,GlobalOrdinal,Node);
    out << "version: " << MueLu::Version() << std::endl;

    RCP<const Teuchos::Comm<int> > comm = Parameters::getDefaultComm();

    Level fineLevel;
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    TestHelpers::TestFactory<SC,LO,GO,NO>::createSingleLevelHierarchy(fineLevel);
#else
    TestHelpers::TestFactory<SC,NO>::createSingleLevelHierarchy(fineLevel);
#endif

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    RCP<Matrix> A = TestHelpers::TestFactory<SC,LO,GO,NO>::Build1DPoisson(36);
#else
    RCP<Matrix> A = TestHelpers::TestFactory<SC,NO>::Build1DPoisson(36);
#endif
    fineLevel.Set("A", A);

    Teuchos::ParameterList galeriList;
    galeriList.set("nx", Teuchos::as<GlobalOrdinal>(36));
    RCP<RealValuedMultiVector> coordinates = Galeri::Xpetra::Utils::CreateCartesianCoordinates<SC,LO,GO,Map,RealValuedMultiVector>("1D", A->getRowMap(), galeriList);

    // Now we doctor the coordinates so that the off-diagonal pair row 0 will want to keep (0,1) and row 1 will want to drop (1,0)
    if(!comm->getRank()) {
      auto vals = coordinates->getDataNonConst(0);
      vals[0] = vals[0] - 2000*36;
    }

    fineLevel.Set("Coordinates", coordinates);

    CoalesceDropFactory coalesceDropFact;
    // We're dropping all the interior off-diagonal entries.
    // dx = 1/36
    // L_ij = -36
    // L_ii = 72
    // criterion for dropping is |L_ij|^2 <= tol^2 * |L_ii*L_jj|
    coalesceDropFact.SetParameter("aggregation: drop tol",Teuchos::ParameterEntry(8.0));
    coalesceDropFact.SetParameter("aggregation: drop scheme",Teuchos::ParameterEntry(std::string("distance laplacian")));
    coalesceDropFact.SetParameter("aggregation: distance laplacian algo",Teuchos::ParameterEntry(std::string("scaled cut symmetric")));
    fineLevel.Request("Graph",&coalesceDropFact);
    fineLevel.Request("DofsPerNode", &coalesceDropFact);

    coalesceDropFact.Build(fineLevel);

    RCP<GraphBase> graph = fineLevel.Get<RCP<GraphBase> >("Graph", &coalesceDropFact);
    LO myDofsPerNode = fineLevel.Get<LO>("DofsPerNode", &coalesceDropFact);
    TEST_EQUALITY(Teuchos::as<int>(myDofsPerNode) == 1, true);

    const RCP<const Map> myImportMap = graph->GetImportMap(); // < note that the ImportMap is built from the column map of the matrix A WITHOUT dropping!
    const RCP<const Map> myDomainMap = graph->GetDomainMap();

    TEST_EQUALITY(myImportMap->getMaxAllGlobalIndex(), 35);
    TEST_EQUALITY(myImportMap->getMinAllGlobalIndex(), 0);
    TEST_EQUALITY(myImportMap->getMinLocalIndex(),0);
    TEST_EQUALITY(myImportMap->getGlobalNumElements(),Teuchos::as<size_t>(36 + (comm->getSize()-1)*2));

    TEST_EQUALITY(myDomainMap->getMaxAllGlobalIndex(), 35);
    TEST_EQUALITY(myDomainMap->getMinAllGlobalIndex(), 0);
    TEST_EQUALITY(myDomainMap->getMinLocalIndex(),0);
    TEST_EQUALITY(myDomainMap->getGlobalNumElements(),36);

    TEST_EQUALITY(graph->GetGlobalNumEdges(),106);

  } // DistanceLaplacianCutScaled



#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
#define MUELU_ETI_GROUP(SC,LO,GO,Node) \
      TEUCHOS_UNIT_TEST_TEMPLATE_4_INSTANT(CoalesceDropFactory,Constructor,SC,LO,GO,Node) \
      TEUCHOS_UNIT_TEST_TEMPLATE_4_INSTANT(CoalesceDropFactory,Build,SC,LO,GO,Node) \
      TEUCHOS_UNIT_TEST_TEMPLATE_4_INSTANT(CoalesceDropFactory,PreDrop,SC,LO,GO,Node) \
      TEUCHOS_UNIT_TEST_TEMPLATE_4_INSTANT(CoalesceDropFactory,AmalgamationBasic,SC,LO,GO,Node) \
      TEUCHOS_UNIT_TEST_TEMPLATE_4_INSTANT(CoalesceDropFactory,AmalgamationStrided,SC,LO,GO,Node) \
      TEUCHOS_UNIT_TEST_TEMPLATE_4_INSTANT(CoalesceDropFactory,AmalgamationStrided2,SC,LO,GO,Node) \
      TEUCHOS_UNIT_TEST_TEMPLATE_4_INSTANT(CoalesceDropFactory,AmalgamationStridedOffset,SC,LO,GO,Node) \
      TEUCHOS_UNIT_TEST_TEMPLATE_4_INSTANT(CoalesceDropFactory,AmalgamationLightweight,SC,LO,GO,Node) \
      TEUCHOS_UNIT_TEST_TEMPLATE_4_INSTANT(CoalesceDropFactory,AmalgamationLightweightDrop,SC,LO,GO,Node) \
      TEUCHOS_UNIT_TEST_TEMPLATE_4_INSTANT(CoalesceDropFactory,AmalgamationStridedLW,SC,LO,GO,Node) \
      TEUCHOS_UNIT_TEST_TEMPLATE_4_INSTANT(CoalesceDropFactory,AmalgamationStrided2LW,SC,LO,GO,Node) \
      TEUCHOS_UNIT_TEST_TEMPLATE_4_INSTANT(CoalesceDropFactory,AmalgamationStridedOffsetLW,SC,LO,GO,Node) \
      TEUCHOS_UNIT_TEST_TEMPLATE_4_INSTANT(CoalesceDropFactory,AmalgamationDroppingLW,SC,LO,GO,Node) \
      TEUCHOS_UNIT_TEST_TEMPLATE_4_INSTANT(CoalesceDropFactory,AmalgamationStridedOffsetDropping2LW,SC,LO,GO,Node) \
      TEUCHOS_UNIT_TEST_TEMPLATE_4_INSTANT(CoalesceDropFactory,DistanceLaplacian,SC,LO,GO,Node) \
      TEUCHOS_UNIT_TEST_TEMPLATE_4_INSTANT(CoalesceDropFactory,DistanceLaplacianCut,SC,LO,GO,Node) \
      TEUCHOS_UNIT_TEST_TEMPLATE_4_INSTANT(CoalesceDropFactory,DistanceLaplacianCutSym,SC,LO,GO,Node)
#else
#define MUELU_ETI_GROUP(SC,Node) \
      TEUCHOS_UNIT_TEST_TEMPLATE_2_INSTANT(CoalesceDropFactory,Constructor,SC,Node) \
      TEUCHOS_UNIT_TEST_TEMPLATE_2_INSTANT(CoalesceDropFactory,Build,SC,Node) \
      TEUCHOS_UNIT_TEST_TEMPLATE_2_INSTANT(CoalesceDropFactory,PreDrop,SC,Node) \
      TEUCHOS_UNIT_TEST_TEMPLATE_2_INSTANT(CoalesceDropFactory,AmalgamationBasic,SC,Node) \
      TEUCHOS_UNIT_TEST_TEMPLATE_2_INSTANT(CoalesceDropFactory,AmalgamationStrided,SC,Node) \
      TEUCHOS_UNIT_TEST_TEMPLATE_2_INSTANT(CoalesceDropFactory,AmalgamationStrided2,SC,Node) \
      TEUCHOS_UNIT_TEST_TEMPLATE_2_INSTANT(CoalesceDropFactory,AmalgamationStridedOffset,SC,Node) \
      TEUCHOS_UNIT_TEST_TEMPLATE_2_INSTANT(CoalesceDropFactory,AmalgamationLightweight,SC,Node) \
      TEUCHOS_UNIT_TEST_TEMPLATE_2_INSTANT(CoalesceDropFactory,AmalgamationLightweightDrop,SC,Node) \
      TEUCHOS_UNIT_TEST_TEMPLATE_2_INSTANT(CoalesceDropFactory,AmalgamationStridedLW,SC,Node) \
      TEUCHOS_UNIT_TEST_TEMPLATE_2_INSTANT(CoalesceDropFactory,AmalgamationStrided2LW,SC,Node) \
      TEUCHOS_UNIT_TEST_TEMPLATE_2_INSTANT(CoalesceDropFactory,AmalgamationStridedOffsetLW,SC,Node) \
      TEUCHOS_UNIT_TEST_TEMPLATE_2_INSTANT(CoalesceDropFactory,AmalgamationDroppingLW,SC,Node) \
      TEUCHOS_UNIT_TEST_TEMPLATE_2_INSTANT(CoalesceDropFactory,AmalgamationStridedOffsetDropping2LW,SC,Node) \
      TEUCHOS_UNIT_TEST_TEMPLATE_2_INSTANT(CoalesceDropFactory,DistanceLaplacian,SC,Node) \
      TEUCHOS_UNIT_TEST_TEMPLATE_2_INSTANT(CoalesceDropFactory,DistanceLaplacianCut,SC,Node) \
      TEUCHOS_UNIT_TEST_TEMPLATE_2_INSTANT(CoalesceDropFactory,DistanceLaplacianCutSym,SC,Node)
#endif

#include <MueLu_ETI_4arg.hpp>

} // namespace MueLuTests


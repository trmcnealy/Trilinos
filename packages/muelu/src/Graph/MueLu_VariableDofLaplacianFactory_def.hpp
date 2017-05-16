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
//                    Tobias Wiesner    (tawiesn@sandia.gov)
//                    Ray Tuminaro      (rstumin@sandia.gov)
//
// ***********************************************************************
//
// @HEADER
#ifndef PACKAGES_MUELU_SRC_GRAPH_MUELU_VARIABLEDOFLAPLACIANFACTORY_DEF_HPP_
#define PACKAGES_MUELU_SRC_GRAPH_MUELU_VARIABLEDOFLAPLACIANFACTORY_DEF_HPP_


#include "MueLu_Monitor.hpp"

#include "MueLu_VariableDofLaplacianFactory_decl.hpp"

namespace MueLu {

  template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node>
  RCP<const ParameterList> VariableDofLaplacianFactory<Scalar, LocalOrdinal, GlobalOrdinal, Node>::GetValidParameterList() const {
    RCP<ParameterList> validParamList = rcp(new ParameterList());
/*
#define SET_VALID_ENTRY(name) validParamList->setEntry(name, MasterList::getEntry(name))
    SET_VALID_ENTRY("aggregation: drop tol");
    SET_VALID_ENTRY("aggregation: Dirichlet threshold");
    SET_VALID_ENTRY("aggregation: drop scheme");
    {
      typedef Teuchos::StringToIntegralParameterEntryValidator<int> validatorType;
      validParamList->getEntry("aggregation: drop scheme").setValidator(
        rcp(new validatorType(Teuchos::tuple<std::string>("classical", "distance laplacian"), "aggregation: drop scheme")));
    }
#undef  SET_VALID_ENTRY
    validParamList->set< bool >                  ("lightweight wrap",           true, "Experimental option for lightweight graph access");
*/

    validParamList->set< int >                   ("maxDofPerNode", 1, "Maximum number of DOFs per node");

    validParamList->set< RCP<const FactoryBase> >("A",                  Teuchos::null, "Generating factory of the matrix A");
    validParamList->set< RCP<const FactoryBase> >("Coordinates",        Teuchos::null, "Generating factory for Coordinates");

    return validParamList;
  }

  template <class Scalar,class LocalOrdinal, class GlobalOrdinal, class Node>
  VariableDofLaplacianFactory<Scalar, LocalOrdinal, GlobalOrdinal, Node>::VariableDofLaplacianFactory() { }

  template <class Scalar,class LocalOrdinal, class GlobalOrdinal, class Node>
  void VariableDofLaplacianFactory<Scalar, LocalOrdinal, GlobalOrdinal, Node>::DeclareInput(Level &currentLevel) const {
    //const ParameterList& pL = GetParameterList();
    Input(currentLevel, "A");
    Input(currentLevel, "Coordinates");

    //if (currentLevel.GetLevelID() == 0) // TODO check for finest level (special treatment)
    if (currentLevel.IsAvailable("DofPresent", NoFactory::get())) {
      currentLevel.DeclareInput("DofPresent", NoFactory::get(), this);
    }
  }

  template <class Scalar,class LocalOrdinal, class GlobalOrdinal, class Node>
  void VariableDofLaplacianFactory<Scalar, LocalOrdinal, GlobalOrdinal, Node>::Build(Level &currentLevel) const {
    FactoryMonitor m(*this, "Build", currentLevel);
    typedef Teuchos::ScalarTraits<SC> STS;

    const ParameterList  & pL = GetParameterList();

    RCP<Matrix> A = Get< RCP<Matrix> >(currentLevel, "A");

    Teuchos::RCP< const Teuchos::Comm< int > > comm = A->getRowMap()->getComm();
    Xpetra::UnderlyingLib lib = A->getRowMap()->lib();

    typedef Xpetra::MultiVector<double,LO,GO,NO> dxMV;
    RCP<dxMV> Coords = Get< RCP<Xpetra::MultiVector<double,LO,GO,NO> > >(currentLevel, "Coordinates");

    int maxDofPerNode = pL.get<int>("maxDofPerNode");
    Scalar dirDropTol = 1e-5; // TODO parameter from parameter list ("ML advnaced Dirichlet: threshold"), should be magnitude type?
    Scalar amalgDropTol = 1.8e-9; // TODO parameter from parameter list ("variable DOF amalgamation: threshold")

    bool bHasZeroDiagonal = false;
    Teuchos::ArrayRCP<const bool> dirOrNot = MueLu::Utilities<Scalar, LocalOrdinal, GlobalOrdinal, Node>::DetectDirichletRowsExt(*A,bHasZeroDiagonal,STS::magnitude(dirDropTol));

    // TODO check availability + content (length)
    Teuchos::ArrayRCP<const bool> dofPresent = currentLevel.Get< Teuchos::ArrayRCP<const bool> >("DofPresent", NoFactory::get());

    // map[k] indicates that the kth dof in the variable dof matrix A would
    // correspond to the map[k]th dof in the padded system. If, i.e., it is
    // map[35] = 39 then dof no 35 in the variable dof matrix A corresponds to
    // row map id 39 in an imaginary padded matrix Apadded.
    // The padded system is never built but would be the associated matrix if
    // every node had maxDofPerNode dofs.
    std::vector<LocalOrdinal> map(A->getNodeNumRows());
    this->buildPaddedMap(dofPresent, map, A->getNodeNumRows());

    //GetOStream(Parameters0) << "lightweight wrap = " << doExperimentalWrap << std::endl;

    // map of size of number of DOFs containing local node id (dof id -> node id, inclusive ghosted dofs/nodes)
    std::vector<LocalOrdinal> myLocalNodeIds(A->getColMap()->getNodeNumElements()); // possible maximum (we need the ghost nodes, too)

    // assign the local node ids for the ghosted nodes
    size_t nLocalNodes, nLocalPlusGhostNodes;
    this->assignGhostLocalNodeIds(A->getRowMap(), A->getColMap(), myLocalNodeIds, map, maxDofPerNode, nLocalNodes, nLocalPlusGhostNodes, comm);

    TEUCHOS_TEST_FOR_EXCEPTION(Teuchos::as<size_t>(dofPresent.size()) != Teuchos::as<size_t>(nLocalNodes * maxDofPerNode),MueLu::Exceptions::RuntimeError,"VariableDofLaplacianFactory: size of provided DofPresent array is " << dofPresent.size() << " but should be " << nLocalNodes * maxDofPerNode << " on the current processor.");

    // put content of assignGhostLocalNodeIds here...

    // fill nodal maps

    Teuchos::ArrayView< const GlobalOrdinal > myGids = A->getColMap()->getNodeElementList();

    // vector containing row/col gids of amalgamated matrix (with holes)

    size_t nLocalDofs = A->getRowMap()->getNodeNumElements();
    size_t nLocalPlusGhostDofs = A->getColMap()->getNodeNumElements();

    // myLocalNodeIds (dof -> node)

    Teuchos::Array<GlobalOrdinal> amalgRowMapGIDs(nLocalNodes);
    Teuchos::Array<GlobalOrdinal> amalgColMapGIDs(nLocalPlusGhostNodes);

    // initialize
    size_t count = 0;
    if (nLocalDofs > 0) {
      amalgRowMapGIDs[count] = myGids[0];
      amalgColMapGIDs[count] = myGids[0];
      count++;
    }

    // dofs belonging to the same node must be consecutively only for the local part of myLocalNodeIds[]
    // fill amalgRowMapGIDs + local part of amalgColMapGIDs
    for(size_t i = 1; i < nLocalDofs; i++) {
      if(myLocalNodeIds[i] != myLocalNodeIds[i-1]) {
        amalgRowMapGIDs[count] = myGids[i];
        count++;
      }
    }

    size_t count2 = 1;
    for(size_t i = 1; i < myLocalNodeIds.size(); i++) {
      if(myLocalNodeIds[i] != myLocalNodeIds[i-1]) {
        amalgColMapGIDs[count2] = myGids[i];
        count2++;
      }
    }

    Teuchos::RCP<Map> amalgRowMap = MapFactory::Build(lib,
               Teuchos::OrdinalTraits<GlobalOrdinal>::invalid(),
               amalgRowMapGIDs(), //View,
               A->getRowMap()->getIndexBase(),
               comm);

    Teuchos::RCP<Map> amalgColMap = MapFactory::Build(lib,
               Teuchos::OrdinalTraits<GlobalOrdinal>::invalid(),
               amalgColMapGIDs(), //View,
               A->getRangeMap()->getIndexBase(),
               comm);

    //RCP<Teuchos::FancyOStream> fancy = Teuchos::fancyOStream(Teuchos::rcpFromRef(std::cout));
    // end fill nodal maps


    // start variable dof amalgamation

    Teuchos::RCP<CrsMatrixWrap> Awrap = Teuchos::rcp_dynamic_cast<CrsMatrixWrap>(A);
    Teuchos::RCP<CrsMatrix> Acrs = Awrap->getCrsMatrix();
    //Acrs->describe(*fancy, Teuchos::VERB_EXTREME);

    Teuchos::ArrayRCP<const size_t> rowptr(Acrs->getNodeNumRows());
    Teuchos::ArrayRCP<const LocalOrdinal> colind(Acrs->getNodeNumEntries());
    Teuchos::ArrayRCP<const Scalar> values(Acrs->getNodeNumEntries());
    Acrs->getAllValues(rowptr, colind, values);


    // create arrays for amalgamated matrix
    Teuchos::ArrayRCP<size_t> amalgRowPtr(nLocalNodes+1);
    Teuchos::ArrayRCP<LocalOrdinal> amalgCols(rowptr[rowptr.size()-1]);
    //Teuchos::ArrayRCP<const Scalar> values(Acrs->getNodeNumEntries());

    size_t nNonZeros = 0;
    std::vector<bool> isNonZero(nLocalPlusGhostDofs,false);
    std::vector<size_t> nonZeroList(nLocalPlusGhostDofs);  // ???

    // also used in DetectDirichletExt
    Teuchos::RCP<Vector> diagVecUnique = VectorFactory::Build(A->getRowMap());
    Teuchos::RCP<Vector> diagVec       = VectorFactory::Build(A->getColMap());
    A->getLocalDiagCopy(*diagVecUnique);
    Teuchos::RCP<Import> dofImporter = ImportFactory::Build(diagVecUnique->getMap(), diagVec->getMap());
    diagVec->doImport(*diagVecUnique, *dofImporter, Xpetra::INSERT);
    Teuchos::ArrayRCP< const Scalar > diagVecData = diagVec->getData(0);

    LocalOrdinal oldBlockRow = 0;
    LocalOrdinal blockRow, blockColumn;
    size_t newNzs = 0;
    amalgRowPtr[0] = newNzs;

    bool doNotDrop = false;
    if (amalgDropTol == Teuchos::ScalarTraits<Scalar>::zero()) doNotDrop = true;
    if (values.size() == 0) doNotDrop = true;

    for(decltype(rowptr.size()) i = 0; i < rowptr.size()-1; i++) {
      blockRow = std::floor<LocalOrdinal>( map[i] / maxDofPerNode);
      if (blockRow != oldBlockRow) {
        // zero out info recording nonzeros in oldBlockRow
        for(size_t j = 0; j < nNonZeros; j++) isNonZero[nonZeroList[j]] = false;
        nNonZeros = 0;
        amalgRowPtr[blockRow] = newNzs; // record start of next row
      }
      for (size_t j = rowptr[i]; j < rowptr[i+1]; j++) {
        if(doNotDrop == true ||
            ( STS::magnitude(values[j] / sqrt(STS::magnitude(diagVecData[i]) * STS::magnitude(diagVecData[colind[j]]))   ) >= STS::magnitude(amalgDropTol) )) {
          blockColumn = myLocalNodeIds[colind[j]];
          if(isNonZero[blockColumn] == false) {
            isNonZero[blockColumn] = true;
            nonZeroList[nNonZeros++] = blockColumn;
            amalgCols[newNzs++] = blockColumn;
          }
        }
      }
      oldBlockRow = blockRow;
    }
    amalgRowPtr[blockRow+1] = newNzs;

    TEUCHOS_TEST_FOR_EXCEPTION((blockRow+1 != Teuchos::as<LO>(nLocalNodes)) && (nLocalNodes !=0), MueLu::Exceptions::RuntimeError, "VariableDofsPerNodeAmalgamation: error, computed # block rows (" << blockRow+1 <<") != nLocalNodes (" << nLocalNodes <<")");

    amalgCols.resize(amalgRowPtr[nLocalNodes]);

    // end variableDofAmalg


    // begin rm differentDofsCrossings

    // Remove matrix entries (i,j) where the ith node and the jth node have
    // different dofs that are 'present'
    // Specifically, on input:
    //    dofPresent[i*maxDofPerNode+k] indicates whether or not the kth
    //                                  dof at the ith node is present in the
    //                                  variable dof matrix (e.g., the ith node
    //                                  has an air pressure dof). true means
    //                                  the dof is present while false means it
    //                                  is not.
    // We create a unique id for the ith node (i.e. uniqueId[i]) via
    //    sum_{k=0 to maxDofPerNode-1} dofPresent[i*maxDofPerNode+k]*2^k
    // and use this unique idea to remove entries (i,j) when uniqueId[i]!=uniqueId[j]

    Teuchos::ArrayRCP<LocalOrdinal> uniqueId(nLocalPlusGhostNodes);       // unique id associated with DOF
    std::vector<bool> keep(amalgRowPtr[amalgRowPtr.size()-1],true); // keep connection associated with node

    size_t ii = 0; // iteration index for present dofs
    for(decltype(amalgRowPtr.size()) i = 0; i < amalgRowPtr.size()-1; i++) {
      LocalOrdinal temp = 1; // basis for dof-id
      uniqueId[i] = 0;
      for (decltype(maxDofPerNode) j = 0; j < maxDofPerNode; j++) {
        if (dofPresent[ii++]) uniqueId[i] += temp; // encode dof to be present
        temp = temp * 2; // check next dof
      }
    }

    Teuchos::RCP<Import> nodeImporter = ImportFactory::Build(amalgRowMap, amalgColMap);

    RCP<Vector> nodeIdSrc    = VectorFactory::Build(amalgRowMap,true);
    RCP<Vector> nodeIdTarget = VectorFactory::Build(amalgColMap,true);

    Teuchos::ArrayRCP< Scalar > nodeIdSrcData = nodeIdSrc->getDataNonConst(0);
    for(decltype(amalgRowPtr.size()) i = 0; i < amalgRowPtr.size()-1; i++) {
      nodeIdSrcData[i] = uniqueId[i];
    }

    nodeIdTarget->doImport(*nodeIdSrc, *nodeImporter, Xpetra::INSERT);

    Teuchos::ArrayRCP< const Scalar > nodeIdTargetData = nodeIdTarget->getData(0);
    for(decltype(uniqueId.size()) i = 0; i < uniqueId.size(); i++) {
      uniqueId[i] = nodeIdTargetData[i];
    }

    // nodal comm uniqueId, myLocalNodeIds

    // uniqueId now should contain ghosted data

    for(decltype(amalgRowPtr.size()) i = 0; i < amalgRowPtr.size()-1; i++) {
      for(size_t j = amalgRowPtr[i]; j < amalgRowPtr[i+1]; j++) {
        if (uniqueId[i] != uniqueId[amalgCols[j]]) keep [j] = false;
      }
    }

    // squeeze out hard-coded zeros from CSR arrays
    Teuchos::ArrayRCP<Scalar> amalgVals; //(/*rowptr[rowptr.size()-1]*/); // empty array!
    this->squeezeOutNnzs(amalgRowPtr,amalgCols,amalgVals,keep);


    typedef Xpetra::MultiVectorFactory<double,LO,GO,NO> dxMVf;
    RCP<dxMV> ghostedCoords = dxMVf::Build(amalgColMap,Coords->getNumVectors());

    TEUCHOS_TEST_FOR_EXCEPTION(amalgRowMap->getNodeNumElements() != Coords->getMap()->getNodeNumElements(), MueLu::Exceptions::RuntimeError, "MueLu::VariableDofLaplacianFactory: the number of Coordinates and amalgamated nodes is inconsistent.");

    // Coords might live on a special nodeMap with consecutive ids (the natural numbering)
    // The amalgRowMap might have the same number of entries, but with holes in the ids.
    // e.g. 0,3,6,9,... as GIDs.
    // We need the ghosted Coordinates in the buildLaplacian routine. But we access the data
    // through getData only, i.e., the global ids are not interesting as long as we do not change
    // the ordering of the entries
    Coords->replaceMap(amalgRowMap);
    ghostedCoords->doImport(*Coords, *nodeImporter, Xpetra::INSERT);

    Teuchos::ArrayRCP<Scalar> lapVals(amalgRowPtr[nLocalNodes]);
    this->buildLaplacian(amalgRowPtr, amalgCols, lapVals, Coords->getNumVectors(), ghostedCoords);

    // sort column GIDs
    for(decltype(amalgRowPtr.size()) i = 0; i < amalgRowPtr.size()-1; i++) {
      size_t j = amalgRowPtr[i];
      this->MueLu_az_sort<LocalOrdinal>(&(amalgCols[j]), amalgRowPtr[i+1] - j, NULL, &(lapVals[j]));
    }

    // TODO status array?

    Teuchos::RCP<CrsMatrix> lapCrsMat = CrsMatrixFactory::Build(amalgRowMap, amalgColMap, 10); // TODO better approx for max nnz per row

    for (size_t i = 0; i < nLocalNodes; i++) {
      lapCrsMat->insertLocalValues(i, amalgCols.view(amalgRowPtr[i],amalgRowPtr[i+1]-amalgRowPtr[i]),
          lapVals.view(amalgRowPtr[i],amalgRowPtr[i+1]-amalgRowPtr[i]));
    }
    lapCrsMat->fillComplete(amalgRowMap,amalgRowMap);

    Teuchos::RCP<Matrix> lapMat = Teuchos::rcp(new CrsMatrixWrap(lapCrsMat));
    Set(currentLevel,"A",lapMat);
  }

  template <class Scalar,class LocalOrdinal, class GlobalOrdinal, class Node>
  void VariableDofLaplacianFactory<Scalar, LocalOrdinal, GlobalOrdinal, Node>::buildLaplacian(const Teuchos::ArrayRCP<size_t>& rowPtr, const Teuchos::ArrayRCP<LocalOrdinal>& cols, Teuchos::ArrayRCP<Scalar>& vals,const size_t& numdim, const RCP<Xpetra::MultiVector<double,LocalOrdinal,GlobalOrdinal,Node> > & ghostedCoords) const {
    TEUCHOS_TEST_FOR_EXCEPTION(numdim != 2 && numdim !=3, MueLu::Exceptions::RuntimeError,"buildLaplacian only works for 2d or 3d examples. numdim = " << numdim);

    if(numdim == 2) { // 2d
      Teuchos::ArrayRCP< const double > x = ghostedCoords->getData(0);
      Teuchos::ArrayRCP< const double > y = ghostedCoords->getData(1);

      for(decltype(rowPtr.size()) i = 0; i < rowPtr.size() - 1; i++) {
        Scalar sum = Teuchos::ScalarTraits<Scalar>::zero();
        LocalOrdinal diag = -1;
        for(size_t j = rowPtr[i]; j < rowPtr[i+1]; j++) {
          if(cols[j] != Teuchos::as<LO>(i)){
            vals[j] = std::sqrt( (x[i]-x[cols[j]]) * (x[i]-x[cols[j]]) +
                                 (y[i]-y[cols[j]]) * (y[i]-y[cols[j]]) );
            TEUCHOS_TEST_FOR_EXCEPTION(vals[j] == Teuchos::ScalarTraits<Scalar>::zero(), MueLu::Exceptions::RuntimeError, "buildLaplacian: error, " << i << " and " << cols[j] << " have same coordinates: " << x[i] << " and " << y[i]);
            vals[j] = -1./vals[j];
            sum = sum - vals[j];
          }
          else diag = j;
        }
        if(sum == Teuchos::ScalarTraits<Scalar>::zero()) sum = Teuchos::ScalarTraits<Scalar>::one();
        TEUCHOS_TEST_FOR_EXCEPTION(diag == -1, MueLu::Exceptions::RuntimeError, "buildLaplacian: error, row " << i << " has zero diagonal!");

        vals[diag] = sum;
      }
    } else { // 3d
      Teuchos::ArrayRCP< const double > x = ghostedCoords->getData(0);
      Teuchos::ArrayRCP< const double > y = ghostedCoords->getData(1);
      Teuchos::ArrayRCP< const double > z = ghostedCoords->getData(2);

      for(decltype(rowPtr.size()) i = 0; i < rowPtr.size() - 1; i++) {
        Scalar sum = Teuchos::ScalarTraits<Scalar>::zero();
        LocalOrdinal diag = -1;
        for(size_t j = rowPtr[i]; j < rowPtr[i+1]; j++) {
          if(cols[j] != Teuchos::as<LO>(i)){
            vals[j] = std::sqrt( (x[i]-x[cols[j]]) * (x[i]-x[cols[j]]) +
                                 (y[i]-y[cols[j]]) * (y[i]-y[cols[j]]) +
                                 (z[i]-z[cols[j]]) * (z[i]-z[cols[j]]) );

            TEUCHOS_TEST_FOR_EXCEPTION(vals[j] == Teuchos::ScalarTraits<Scalar>::zero(), MueLu::Exceptions::RuntimeError, "buildLaplacian: error, " << i << " and " << cols[j] << " have same coordinates: " << x[i] << " and " << y[i] << " and " << z[i]);

            vals[j] = -1./vals[j];
            sum = sum - vals[j];
          }
          else diag = j;
        }
        if(sum == Teuchos::ScalarTraits<Scalar>::zero()) sum = Teuchos::ScalarTraits<Scalar>::one();
        TEUCHOS_TEST_FOR_EXCEPTION(diag == -1, MueLu::Exceptions::RuntimeError, "buildLaplacian: error, row " << i << " has zero diagonal!");

        vals[diag] = sum;
      }
    }
  }

  template <class Scalar,class LocalOrdinal, class GlobalOrdinal, class Node>
  void VariableDofLaplacianFactory<Scalar, LocalOrdinal, GlobalOrdinal, Node>::squeezeOutNnzs(Teuchos::ArrayRCP<size_t> & rowPtr, Teuchos::ArrayRCP<LocalOrdinal> & cols, Teuchos::ArrayRCP<Scalar> & vals, const std::vector<bool>& keep) const {
    // get rid of nonzero entries that have 0's in them and properly change
    // the row ptr array to reflect this removal (either vals == NULL or vals != NULL)
    // Note, the arrays are squeezed. No memory is freed.

    size_t count = 0;

    size_t nRows = rowPtr.size()-1;
    if(vals.size() > 0) {
      for(size_t i = 0; i < nRows; i++) {
        size_t newStart = count;
        for(size_t j = rowPtr[i]; j < rowPtr[i+1]; j++) {
          if(vals[j] != Teuchos::ScalarTraits<Scalar>::zero()) {
            cols[count  ] = cols[j];
            vals[count++] = vals[j];
          }
        }
        rowPtr[i] = newStart;
      }
    } else {
      for (size_t i = 0; i < nRows; i++) {
        size_t newStart = count;
        for(size_t j = rowPtr[i]; j < rowPtr[i+1]; j++) {
          if (keep[j] == true) {
            cols[count++] = cols[j];
          }
        }
        rowPtr[i] = newStart;
      }
    }
    rowPtr[nRows] = count;
  }

  template <class Scalar,class LocalOrdinal, class GlobalOrdinal, class Node>
  void VariableDofLaplacianFactory<Scalar, LocalOrdinal, GlobalOrdinal, Node>::buildPaddedMap(const Teuchos::ArrayRCP<const bool> & dofPresent, std::vector<LocalOrdinal> & map, size_t nDofs) const {
    size_t count = 0;
    for (decltype(dofPresent.size()) i = 0; i < dofPresent.size(); i++)
      if(dofPresent[i] == true) map[count++] = Teuchos::as<LocalOrdinal>(i);
    TEUCHOS_TEST_FOR_EXCEPTION(nDofs != count, MueLu::Exceptions::RuntimeError, "VariableDofLaplacianFactory::buildPaddedMap: #dofs in dofPresent does not match the expected value (number of rows of A): " << nDofs << " vs. " << count);
  }

  template <class Scalar,class LocalOrdinal, class GlobalOrdinal, class Node>
  void VariableDofLaplacianFactory<Scalar, LocalOrdinal, GlobalOrdinal, Node>::assignGhostLocalNodeIds(const Teuchos::RCP<const Map> & rowDofMap, const Teuchos::RCP<const Map> & colDofMap, std::vector<LocalOrdinal> & myLocalNodeIds, const std::vector<LocalOrdinal> & dofMap, size_t maxDofPerNode, size_t& nLocalNodes, size_t& nLocalPlusGhostNodes, Teuchos::RCP< const Teuchos::Comm< int > > comm) const {

    size_t nLocalDofs = rowDofMap->getNodeNumElements();
    size_t nLocalPlusGhostDofs = colDofMap->getNodeNumElements(); // TODO remove parameters

    // create importer for dof-based information
    Teuchos::RCP<Import> importer = ImportFactory::Build(rowDofMap, colDofMap);

    // create a vector living on column map of A (dof based)
    Teuchos::RCP<Vector> localNodeIdsTemp = Xpetra::VectorFactory<Scalar,LocalOrdinal,GlobalOrdinal,Node>::Build(rowDofMap,true);
    Teuchos::RCP<Vector> localNodeIds = Xpetra::VectorFactory<Scalar,LocalOrdinal,GlobalOrdinal,Node>::Build(colDofMap,true);

    // fill local dofs (padded local ids)
    Teuchos::ArrayRCP< Scalar > localNodeIdsTempData = localNodeIdsTemp->getDataNonConst(0);
    for(size_t i = 0; i < localNodeIdsTemp->getLocalLength(); i++)
      localNodeIdsTempData[i] = std::floor<LocalOrdinal>( dofMap[i] / maxDofPerNode );

    localNodeIds->doImport(*localNodeIdsTemp, *importer, Xpetra::INSERT);
    Teuchos::ArrayRCP< const Scalar > localNodeIdsData = localNodeIds->getData(0);

    // Note: localNodeIds contains local ids for the padded version as vector values


    // we use Scalar instead of int as type
    Teuchos::RCP<Vector> myProcTemp = Xpetra::VectorFactory<Scalar,LocalOrdinal,GlobalOrdinal,Node>::Build(rowDofMap,true);
    Teuchos::RCP<Vector> myProc = Xpetra::VectorFactory<Scalar,LocalOrdinal,GlobalOrdinal,Node>::Build(colDofMap,true);

    // fill local dofs (padded local ids)
    Teuchos::ArrayRCP< Scalar > myProcTempData = myProcTemp->getDataNonConst(0);
    for(size_t i = 0; i < myProcTemp->getLocalLength(); i++)
      myProcTempData[i] = Teuchos::as<Scalar>(comm->getRank());
    myProc->doImport(*myProcTemp, *importer, Xpetra::INSERT);
    Teuchos::ArrayRCP<Scalar > myProcData = myProc->getDataNonConst(0); // we have to modify the data (therefore the non-const version)

    // At this point, the ghost part of localNodeIds corresponds to the local ids
    // associated with the current owning processor. We want to convert these to
    // local ids associated with the processor on which these are ghosts.
    // Thus we have to re-number them. In doing this re-numbering we must make sure
    // that we find all ghosts with the same id & proc and assign a unique local
    // id to this group (id&proc). To do this find, we sort all ghost entries in
    // localNodeIds that are owned by the same processor. Then we can look for
    // duplicates (i.e., several ghost entries corresponding to dofs with the same
    // node id) easily and make sure these are all assigned to the same local id.
    // To do the sorting we'll make a temporary copy of the ghosts via tempId and
    // tempProc and sort this multiple times for each group owned by the same proc.


    std::vector<size_t> location(nLocalPlusGhostDofs - nLocalDofs + 1);
    std::vector<size_t> tempId  (nLocalPlusGhostDofs - nLocalDofs + 1);
    std::vector<size_t> tempProc(nLocalPlusGhostDofs - nLocalDofs + 1);

    size_t notProcessed = nLocalDofs; // iteration index over all ghosted dofs
    size_t tempIndex = 0;
    size_t first = tempIndex;
    int neighbor;

    while (notProcessed < nLocalPlusGhostDofs) {
      neighbor = Teuchos::as<int>(myProcData[notProcessed]); // get processor id of not-processed element
      first    = tempIndex;
      location[tempIndex] = notProcessed;
      tempId[tempIndex++] = localNodeIdsData[notProcessed];
      myProcData[notProcessed] = Teuchos::as<Scalar>(-1 - neighbor);

      for(size_t i = notProcessed + 1; i < nLocalPlusGhostDofs; i++) {
        if(myProcData[i] == neighbor) {
          location[tempIndex] = i;
          tempId[tempIndex++] = localNodeIdsData[i];
          myProcData[i] = -1; // mark as visited
        }
      }
      this->MueLu_az_sort<size_t>(&(tempId[first]), tempIndex - first, &(location[first]), NULL);
      for(size_t i = first; i < tempIndex; i++) tempProc[i] = neighbor;

      // increment index. Find next notProcessed dof index corresponding to first non-visited element
      notProcessed++;
      while ( (notProcessed < nLocalPlusGhostDofs) && (myProcData[notProcessed] < 0))
        notProcessed++;
    }
    TEUCHOS_TEST_FOR_EXCEPTION(tempIndex != nLocalPlusGhostDofs-nLocalDofs, MueLu::Exceptions::RuntimeError,"Number of nonzero ghosts is inconsistent.");

    // Now assign ids to all ghost nodes (giving the same id to those with the
    // same myProc[] and the same local id on the proc that actually owns the
    // variable associated with the ghost

    nLocalNodes = 0; // initialize return value
    if(nLocalDofs > 0) nLocalNodes = localNodeIdsData[nLocalDofs-1] + 1;

    nLocalPlusGhostNodes = nLocalNodes; // initialize return value
    if(nLocalDofs < nLocalPlusGhostDofs) nLocalPlusGhostNodes++; // 1st ghost node is unique (not accounted for). number will be increased later, if there are more ghost nodes

    // check if two adjacent ghost dofs correspond to different nodes. To do this,
    // check if they are from different processors or whether they have different
    // local node ids

    // loop over all (remaining) ghost dofs
    size_t lagged = -1;
    for (size_t i = nLocalDofs+1; i < nLocalPlusGhostDofs; i++) {
      lagged = nLocalPlusGhostNodes-1;

      // i is a new unique ghost node (not already accounted for)
      if ((tempId[i-nLocalDofs] != tempId[i-1-nLocalDofs]) ||
          (tempProc[i-nLocalDofs] != tempProc[i-1-nLocalDofs]))
        nLocalPlusGhostNodes++; // update number of ghost nodes
      tempId[i-1-nLocalDofs] = lagged;
    }
    if (nLocalPlusGhostDofs > nLocalDofs)
      tempId[nLocalPlusGhostDofs-1-nLocalDofs] = nLocalPlusGhostNodes - 1;

    // fill myLocalNodeIds array. Start with local part (not ghosted)
    for(size_t i = 0; i < nLocalDofs; i++)
      myLocalNodeIds[i] = std::floor<LocalOrdinal>( dofMap[i] / maxDofPerNode );

    // copy ghosted nodal ids into myLocalNodeIds
    for(size_t i = nLocalDofs; i < nLocalPlusGhostDofs; i++)
      myLocalNodeIds[location[i-nLocalDofs]] = tempId[i-nLocalDofs];

  }

} /* MueLu */


#endif /* PACKAGES_MUELU_SRC_GRAPH_MUELU_VARIABLEDOFLAPLACIANFACTORY_DEF_HPP_ */

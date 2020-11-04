// @HEADER
//
// ***********************************************************************
//
//    MueLu: A package for multigrid based preconditioning
//          Copyright 2012 Sandia Corporation
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
//          Jonathan Hu     (jhu@sandia.gov)
//          Ray Tuminaro    (rstumin@sandia.gov)
//
// ***********************************************************************
//
// @HEADER
#ifndef MUELU_INTERFACEMAPPINGTRANSFERFACTORY_DEF_HPP_
#define MUELU_INTERFACEMAPPINGTRANSFERFACTORY_DEF_HPP_

#include "MueLu_InterfaceMappingTransferFactory_decl.hpp"

namespace MueLu
{

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
template <class LocalOrdinal, class GlobalOrdinal, class Node>
RCP<const ParameterList> InterfaceMappingTransferFactory<LocalOrdinal, GlobalOrdinal, Node>::GetValidParameterList() const
#else
template <class Node>
RCP<const ParameterList> InterfaceMappingTransferFactory<Node>::GetValidParameterList() const
#endif
{
  RCP<ParameterList> validParamList = rcp(new ParameterList());
  validParamList->set<RCP<const FactoryBase>>("CoarseDualNodeID2PrimalNodeID", Teuchos::null, "Generating factory of the CoarseDualNodeID2PrimalNodeID map");
  return validParamList;
}

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
template <class LocalOrdinal, class GlobalOrdinal, class Node>
void InterfaceMappingTransferFactory<LocalOrdinal, GlobalOrdinal, Node>::DeclareInput(Level &fineLevel, Level &coarseLevel) const
#else
template <class Node>
void InterfaceMappingTransferFactory<Node>::DeclareInput(Level &fineLevel, Level &coarseLevel) const
#endif
{
  Input(fineLevel, "CoarseDualNodeID2PrimalNodeID");
}

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
template <class LocalOrdinal, class GlobalOrdinal, class Node>
void InterfaceMappingTransferFactory<LocalOrdinal, GlobalOrdinal, Node>::Build(Level &fineLevel, Level &coarseLevel) const
#else
template <class Node>
void InterfaceMappingTransferFactory<Node>::Build(Level &fineLevel, Level &coarseLevel) const
#endif
{
  Monitor m(*this, "Interface Mapping transfer factory");

  RCP<std::map<LocalOrdinal, LocalOrdinal>> coarseLagr2Dof = Get<RCP<std::map<LocalOrdinal, LocalOrdinal>>>(fineLevel, "CoarseDualNodeID2PrimalNodeID");
  Set(coarseLevel, "DualNodeID2PrimalNodeID", coarseLagr2Dof);
}

} // namespace MueLu

#endif /* MUELU_INTERFACEMAPPINGTRANSFERFACTORY_DEF_HPP_ */

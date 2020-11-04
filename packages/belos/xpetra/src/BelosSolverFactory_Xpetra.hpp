//@HEADER
// ************************************************************************
//
//                 Belos: Block Linear Solvers Package
//                  Copyright 2004 Sandia Corporation
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
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ************************************************************************
//@HEADER

#ifndef BELOSSOLVERFACTORY_XPETRA_HPP
#define BELOSSOLVERFACTORY_XPETRA_HPP

#include "Belos_Details_Xpetra_registerSolverFactory.hpp"
#include "BelosSolverFactory.hpp"
#include "BelosXpetraAdapterMultiVector.hpp"
#include "BelosOperatorT.hpp"

namespace Belos {

template<class Scalar, class MV, class OP>
class XpetraSolverFactory : public Impl::SolverFactoryParent<Scalar, MV, OP>
{
  public:
    XpetraSolverFactory() {
      Details::Xpetra::registerSolverFactory();
    };
};

namespace Impl {

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
template<class SC, class LO, class GO, class NT>
class SolverFactorySelector<SC,::Xpetra::MultiVector<SC, LO, GO, NT>,::Belos::OperatorT<Xpetra::MultiVector<SC, LO, GO, NT>>> {
#else
template<class SC, class NT>
class SolverFactorySelector<SC,::Xpetra::MultiVector<SC, NT>,::Belos::OperatorT<Xpetra::MultiVector<SC, NT>>> {
#endif
  public:
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    typedef XpetraSolverFactory<SC,::Xpetra::MultiVector<SC, LO, GO, NT>,::Belos::OperatorT<Xpetra::MultiVector<SC, LO, GO, NT>>> type;
#else
    using LO = typename Tpetra::Map<>::local_ordinal_type;
    using GO = typename Tpetra::Map<>::global_ordinal_type;
    typedef XpetraSolverFactory<SC,::Xpetra::MultiVector<SC, NT>,::Belos::OperatorT<Xpetra::MultiVector<SC, NT>>> type;
#endif
};

} // namespace Impl
} // namespace Belos

#endif // BELOSSOLVERFACTORY_XPETRA_HPP

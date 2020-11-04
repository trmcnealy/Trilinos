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
#ifndef MUELU_BELOSSMOOTHER_DEF_HPP
#define MUELU_BELOSSMOOTHER_DEF_HPP

#include "MueLu_ConfigDefs.hpp"

#if defined(HAVE_MUELU_BELOS)

#include <Teuchos_ParameterList.hpp>

#include <Xpetra_CrsMatrix.hpp>
#include <Xpetra_Matrix.hpp>
#include <Xpetra_MultiVectorFactory.hpp>
#ifdef HAVE_XPETRA_TPETRA
#include <Xpetra_TpetraMultiVector.hpp>
#endif

#include "MueLu_BelosSmoother_decl.hpp"
#include "MueLu_Level.hpp"
#include "MueLu_FactoryManagerBase.hpp"
#include "MueLu_Utilities.hpp"
#include "MueLu_Monitor.hpp"

#include <BelosLinearProblem.hpp>
#include <BelosSolverFactory.hpp>



namespace MueLu {

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  template <class Scalar,class LocalOrdinal, class GlobalOrdinal, class Node>
  BelosSmoother<Scalar, LocalOrdinal, GlobalOrdinal, Node>::BelosSmoother(const std::string type, const Teuchos::ParameterList& paramList)
#else
  template <class Scalar, class Node>
  BelosSmoother<Scalar, Node>::BelosSmoother(const std::string type, const Teuchos::ParameterList& paramList)
#endif
  : type_(type)
  {
    bool solverSupported = false;
#ifdef HAVE_MUELU_TPETRA
    Belos::SolverFactory<Scalar,tMV,tOP> tFactory;
    solverSupported = solverSupported || tFactory.isSupported(type);
#endif
    // if (!solverSupported) {
    //   Teuchos::Array<std::string> supportedSolverNames = factory.supportedSolverNames();

    //   std::ostringstream outString;
    //   outString << "[";
    //   for (auto iter = supportedSolverNames.begin(); iter != supportedSolverNames.end(); ++iter) {
    //     outString << "\"" << *iter << "\"";
    //     if (iter + 1 != supportedSolverNames.end()) {
    //       outString << ", ";
    //     }
    //   }
    //   outString << "]";

    //   TEUCHOS_TEST_FOR_EXCEPTION(true, Exceptions::RuntimeError, "Belos does not provide the solver '" << type_ << "'.\nSupported Solvers: " << outString.str());
    // }
    this->declareConstructionOutcome(!solverSupported, "Belos does not provide the smoother '" + type_ + "'.");
    if (solverSupported)
      SetParameterList(paramList);
  }

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  template <class Scalar,class LocalOrdinal, class GlobalOrdinal, class Node>
  void BelosSmoother<Scalar, LocalOrdinal, GlobalOrdinal, Node>::SetParameterList(const Teuchos::ParameterList& paramList) {
#else
  template <class Scalar, class Node>
  void BelosSmoother<Scalar, Node>::SetParameterList(const Teuchos::ParameterList& paramList) {
#endif
    Factory::SetParameterList(paramList);
  }

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  template <class Scalar,class LocalOrdinal, class GlobalOrdinal, class Node>
  void BelosSmoother<Scalar, LocalOrdinal, GlobalOrdinal, Node>::DeclareInput(Level& currentLevel) const {
#else
  template <class Scalar, class Node>
  void BelosSmoother<Scalar, Node>::DeclareInput(Level& currentLevel) const {
#endif

    this->Input(currentLevel, "A");

  }

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  template <class Scalar,class LocalOrdinal, class GlobalOrdinal, class Node>
  void BelosSmoother<Scalar, LocalOrdinal, GlobalOrdinal, Node>::Setup(Level& currentLevel) {
#else
  template <class Scalar, class Node>
  void BelosSmoother<Scalar, Node>::Setup(Level& currentLevel) {
#endif
    FactoryMonitor m(*this, "Setup Smoother", currentLevel);

    A_ = Factory::Get< RCP<Matrix> >(currentLevel, "A");
    SetupBelos(currentLevel);
    SmootherPrototype::IsSetup(true);
    this->GetOStream(Statistics1) << description() << std::endl;
  }


#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  template <class Scalar,class LocalOrdinal, class GlobalOrdinal, class Node>
  void BelosSmoother<Scalar, LocalOrdinal, GlobalOrdinal, Node>::SetupBelos(Level& currentLevel) {
#else
  template <class Scalar, class Node>
  void BelosSmoother<Scalar, Node>::SetupBelos(Level& currentLevel) {
#endif

    bool useTpetra = A_->getRowMap()->lib() == Xpetra::UseTpetra;

    if (useTpetra) {
#ifdef HAVE_MUELU_TPETRA
      tBelosProblem_ = rcp(new Belos::LinearProblem<Scalar, tMV, tOP>());
      RCP<tOP> tA = Utilities::Op2NonConstTpetraCrs(A_);
      tBelosProblem_->setOperator(tA);

      Belos::SolverFactory<SC, tMV, tOP> solverFactory;
      tSolver_ = solverFactory.create(type_, rcpFromRef(const_cast<ParameterList&>(this->GetParameterList())));
      tSolver_->setProblem(tBelosProblem_);
#endif
    } else {
      TEUCHOS_ASSERT(false);
    }
  }

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  template <class Scalar,class LocalOrdinal, class GlobalOrdinal, class Node>
  void BelosSmoother<Scalar, LocalOrdinal, GlobalOrdinal, Node>::Apply(MultiVector& X, const MultiVector& B, bool InitialGuessIsZero) const {
#else
  template <class Scalar, class Node>
  void BelosSmoother<Scalar, Node>::Apply(MultiVector& X, const MultiVector& B, bool InitialGuessIsZero) const {
#endif
    TEUCHOS_TEST_FOR_EXCEPTION(SmootherPrototype::IsSetup() == false, Exceptions::RuntimeError, "MueLu::BelosSmoother::Apply(): Setup() has not been called");

    if (A_->getRowMap()->lib() == Xpetra::UseTpetra) {
#ifdef HAVE_MUELU_TPETRA
      if (InitialGuessIsZero) {
        X.putScalar(0.0);

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
        RCP<Tpetra::MultiVector<SC,LO,GO,NO> >       tpX = rcpFromRef(Utilities::MV2NonConstTpetraMV(X));
        RCP<const Tpetra::MultiVector<SC,LO,GO,NO> > tpB = rcpFromRef(Utilities::MV2TpetraMV(B));
#else
        RCP<Tpetra::MultiVector<SC,NO> >       tpX = rcpFromRef(Utilities::MV2NonConstTpetraMV(X));
        RCP<const Tpetra::MultiVector<SC,NO> > tpB = rcpFromRef(Utilities::MV2TpetraMV(B));
#endif

        tBelosProblem_->setInitResVec(tpB);
        tBelosProblem_->setProblem(tpX, tpB);
        tSolver_->solve();

      } else {
        typedef Teuchos::ScalarTraits<Scalar> TST;
        RCP<MultiVector> Residual   = Utilities::Residual(*A_, X, B);
        RCP<MultiVector> Correction = MultiVectorFactory::Build(A_->getDomainMap(), X.getNumVectors());

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
        RCP<Tpetra::MultiVector<SC,LO,GO,NO> >       tpX = rcpFromRef(Utilities::MV2NonConstTpetraMV(*Correction));
        RCP<const Tpetra::MultiVector<SC,LO,GO,NO> > tpB = rcpFromRef(Utilities::MV2TpetraMV(*Residual));
#else
        RCP<Tpetra::MultiVector<SC,NO> >       tpX = rcpFromRef(Utilities::MV2NonConstTpetraMV(*Correction));
        RCP<const Tpetra::MultiVector<SC,NO> > tpB = rcpFromRef(Utilities::MV2TpetraMV(*Residual));
#endif

        tBelosProblem_->setInitResVec(tpB);
        tBelosProblem_->setProblem(tpX, tpB);
        tSolver_->solve();

        X.update(TST::one(), *Correction, TST::one());
      }
#endif
    } else {
      TEUCHOS_ASSERT(false);
    }

  }

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  template <class Scalar,class LocalOrdinal, class GlobalOrdinal, class Node>
  RCP<MueLu::SmootherPrototype<Scalar, LocalOrdinal, GlobalOrdinal, Node> > BelosSmoother<Scalar, LocalOrdinal, GlobalOrdinal, Node>::Copy() const {
#else
  template <class Scalar, class Node>
  RCP<MueLu::SmootherPrototype<Scalar, Node> > BelosSmoother<Scalar, Node>::Copy() const {
#endif
    RCP<BelosSmoother> smoother = rcp(new BelosSmoother(*this) );
    smoother->SetParameterList(this->GetParameterList());
    return smoother;
  }

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  template <class Scalar,class LocalOrdinal, class GlobalOrdinal, class Node>
  std::string BelosSmoother<Scalar, LocalOrdinal, GlobalOrdinal, Node>::description() const {
#else
  template <class Scalar, class Node>
  std::string BelosSmoother<Scalar, Node>::description() const {
#endif
    std::ostringstream out;
    if (SmootherPrototype::IsSetup()) {
      if (A_->getRowMap()->lib() == Xpetra::UseTpetra) {
#ifdef MUELU_HAVE_TPETRA
        out << tSolver_->description();
#endif
      }
    } else {
      out << "BELOS {type = " << type_ << "}";
    }
    return out.str();
  }

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  template <class Scalar,class LocalOrdinal, class GlobalOrdinal, class Node>
  void BelosSmoother<Scalar, LocalOrdinal, GlobalOrdinal, Node>::print(Teuchos::FancyOStream &out, const VerbLevel verbLevel) const {
#else
  template <class Scalar, class Node>
  void BelosSmoother<Scalar, Node>::print(Teuchos::FancyOStream &out, const VerbLevel verbLevel) const {
#endif
    MUELU_DESCRIBE;

    if (verbLevel & Parameters1) {
      out0 << "Parameter list: " << std::endl;
      Teuchos::OSTab tab2(out);
      out << this->GetParameterList();
    }

    if (verbLevel & External) {
#ifdef MUELU_HAVE_TPETRA
      if (tSolver_ != Teuchos::null) {
        Teuchos::OSTab tab2(out);
        out << *tSolver_ << std::endl;
      }
#endif
    }

    if (verbLevel & Debug) {
      if (A_->getRowMap()->lib() == Xpetra::UseTpetra) {
#ifdef MUELU_HAVE_TPETRA
        out0 << "IsSetup: " << Teuchos::toString(SmootherPrototype::IsSetup()) << std::endl
             << "-" << std::endl
             << "RCP<solver_>: " << tSolver_ << std::endl;
#endif
      }
    }
  }

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  template <class Scalar,class LocalOrdinal, class GlobalOrdinal, class Node>
  size_t BelosSmoother<Scalar, LocalOrdinal, GlobalOrdinal, Node>::getNodeSmootherComplexity() const {
#else
  template <class Scalar, class Node>
  size_t BelosSmoother<Scalar, Node>::getNodeSmootherComplexity() const {
#endif
    return Teuchos::OrdinalTraits<size_t>::invalid();
  }


} // namespace MueLu

#endif // HAVE_MUELU_BELOS
#endif // MUELU_BELOSSMOOTHER_DEF_HPP

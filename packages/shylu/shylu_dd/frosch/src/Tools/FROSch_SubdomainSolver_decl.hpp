//@HEADER
// ************************************************************************
//
//               ShyLU: Hybrid preconditioner package
//                 Copyright 2012 Sandia Corporation
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
// Questions? Contact Alexander Heinlein (alexander.heinlein@uni-koeln.de)
//
// ************************************************************************
//@HEADER

#ifndef _FROSCH_SUBDOMAINSOLVER_DECL_hpp
#define _FROSCH_SUBDOMAINSOLVER_DECL_hpp

#include <ShyLU_DDFROSch_config.h>

#include <FROSch_Output.h>
#include <FROSch_Timers.h>

#include <FROSch_Tools_decl.hpp>

#ifdef HAVE_SHYLU_DDFROSCH_EPETRA
#include "Epetra_LinearProblem.h"
#endif

#ifdef HAVE_SHYLU_DDFROSCH_AMESOS
#include "Amesos_ConfigDefs.h"
#include "Amesos.h"
#include "Amesos_BaseSolver.h"
#endif

#include "Amesos2.hpp"

#ifdef HAVE_SHYLU_DDFROSCH_BELOS
#include <BelosXpetraAdapterOperator.hpp>
#include <BelosOperatorT.hpp>
#include <BelosXpetraAdapter.hpp>
#include <BelosSolverFactory.hpp>
#endif

#ifdef HAVE_SHYLU_DDFROSCH_MUELU
//#include <MueLu.hpp>
#include <MueLu_TpetraOperator.hpp>
#include <MueLu_CreateTpetraPreconditioner.hpp>
#include <MueLu_Utilities.hpp>
#endif

#ifdef HAVE_SHYLU_DDFROSCH_IFPACK2
#include <Ifpack2_Details_OneLevelFactory_decl.hpp>
#endif

#ifdef HAVE_SHYLU_DDFROSCH_THYRA
#include "Stratimikos_DefaultLinearSolverBuilder.hpp"
#ifdef HAVE_SHYLU_DDFROSCH_IFPACK2
#include "Teuchos_AbstractFactoryStd.hpp"
#include "Thyra_Ifpack2PreconditionerFactory.hpp"
#endif
#endif


namespace FROSch {

    using namespace std;
    using namespace Teuchos;
    using namespace Xpetra;

    template <class SC,
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    class LO ,
    class GO ,
#endif
    class NO >
    class OneLevelPreconditioner;

    template<class SC,
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    class LO,
    class GO,
#endif
    class NO>
    class TwoLevelPreconditioner;

    template<class SC,
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    class LO,
    class GO,
#endif
    class NO>
    class TwoLevelBlockPreconditioner;

    template <class SC = double,
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
              class LO = int,
              class GO = DefaultGlobalOrdinal,
#endif
              class NO = KokkosClassic::DefaultNode::DefaultNodeType>
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    class SubdomainSolver : public Operator<SC,LO,GO,NO> {
#else
    class SubdomainSolver : public Operator<SC,NO> {
#endif

    protected:

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
        using XMap                        = Map<LO,GO,NO>;
#else
        using LO = typename Tpetra::Map<>::local_ordinal_type;
        using GO = typename Tpetra::Map<>::global_ordinal_type;
        using XMap                        = Map<NO>;
#endif
        using XMapPtr                     = RCP<XMap>;
        using ConstXMapPtr                = RCP<const XMap>;
        using XMapPtrVecPtr               = ArrayRCP<XMapPtr>;

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
        using XMatrix                     = Matrix<SC,LO,GO,NO>;
#else
        using XMatrix                     = Matrix<SC,NO>;
#endif
        using XMatrixPtr                  = RCP<XMatrix>;
        using ConstXMatrixPtr             = RCP<const XMatrix>;

#ifdef HAVE_SHYLU_DDFROSCH_EPETRA
        using ECrsMatrix                  = Epetra_CrsMatrix;
        using ECrsMatrixPtr               = RCP<ECrsMatrix>;
        using ConstECrsMatrixPtr          = RCP<const ECrsMatrix>;
#endif
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
        using TCrsMatrix                  = Tpetra::CrsMatrix<SC,LO,GO,NO>;
#else
        using TCrsMatrix                  = Tpetra::CrsMatrix<SC,NO>;
#endif
        using TCrsMatrixPtr               = RCP<TCrsMatrix>;
        using ConstTCrsMatrixPtr          = RCP<const TCrsMatrix>;

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
        using TRowMatrix                  = Tpetra::RowMatrix<SC,LO,GO,NO>;
#else
        using TRowMatrix                  = Tpetra::RowMatrix<SC,NO>;
#endif
        using TRowMatrixPtr               = RCP<TRowMatrix>;
        using ConstTRowMatrixPtr          = RCP<const TRowMatrix>;

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
        using XMultiVector                = MultiVector<SC,LO,GO,NO>;
        using ConstXMultiVector           = const MultiVector<SC,LO,GO,NO>;
#else
        using XMultiVector                = MultiVector<SC,NO>;
        using ConstXMultiVector           = const MultiVector<SC,NO>;
#endif
        using XMultiVectorPtr             = RCP<XMultiVector>;
        using ConstXMultiVectorPtr        = RCP<const XMultiVector>;
        using ConstXMultiVectorPtrVecPtr  = ArrayRCP<ConstXMultiVectorPtr>;

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
        using TMultiVector                = Tpetra::MultiVector<SC,LO,GO,NO>;
#else
        using TMultiVector                = Tpetra::MultiVector<SC,NO>;
#endif
        using TMultiVectorPtr             = RCP<TMultiVector>;

#ifdef HAVE_SHYLU_DDFROSCH_EPETRA
        using EMultiVector                = Epetra_MultiVector;
        using EMultiVectorPtr             = RCP<EMultiVector>;
#endif

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
        using XMultiVectorFactory         = MultiVectorFactory<SC,LO,GO,NO>;
#else
        using XMultiVectorFactory         = MultiVectorFactory<SC,NO>;
#endif

        using ParameterListPtr            = RCP<ParameterList>;

#ifdef HAVE_SHYLU_DDFROSCH_EPETRA
        using ELinearProblem              = Epetra_LinearProblem;
        using ELinearProblemPtr           = RCP<Epetra_LinearProblem>;
#endif

#ifdef HAVE_SHYLU_DDFROSCH_AMESOS
        using AmesosSolverPtr             = RCP<Amesos_BaseSolver>;
#endif

#ifdef HAVE_SHYLU_DDFROSCH_EPETRA
        using Amesos2SolverEpetraPtr      = RCP<Amesos2::Solver<ECrsMatrix,EMultiVector> >;
#endif
        using Amesos2SolverTpetraPtr      = RCP<Amesos2::Solver<TCrsMatrix,TMultiVector> >;

#ifdef HAVE_SHYLU_DDFROSCH_MUELU
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
        using MueLuFactoryPtr             = RCP<MueLu::HierarchyManager<SC,LO,GO,NO> >;
        using MueLuHierarchyPtr           = RCP<MueLu::Hierarchy<SC,LO,GO,NO> >;
#else
        using MueLuFactoryPtr             = RCP<MueLu::HierarchyManager<SC,NO> >;
        using MueLuHierarchyPtr           = RCP<MueLu::Hierarchy<SC,NO> >;
#endif
#endif

        using UN                            = unsigned;
        using UNVec                         = Teuchos::Array<UN>;
        using UNVecPtr                      = Teuchos::ArrayRCP<UN>;

        using GOVecPtr                      = ArrayRCP<GO>;

    public:

        /*!
        \brief Constructor

        Create the subdomain solver object.

        A subdomain solver might require further Trilinos packages at runtime.
        Availability of required packages is tested and
        an error is thrown if required packages are not part of the build configuration.

        @param k Matrix
        @param parameterList Parameter list
        @param blockCoarseSize
        */
        SubdomainSolver(ConstXMatrixPtr k,
                        ParameterListPtr parameterList,
                        string description = "undefined",
                        GOVecPtr blockCoarseSize=null);

        //! Destructor
        virtual ~SubdomainSolver();

        //! Initialize member variables
        virtual int initialize();

        /*!
        \brief Compute/setup this operator/solver

        \pre Routine initialize() has been called and #isInitialized_ is set to \c true.

        @return Integer error code
        */
        virtual int compute();

        /*! \brief Apply subdomain solver to input \c x
         *
         * y = alpha * A^mode * X + beta * Y
         *
         * \param[in] x Input vector
         * \param[out] y result vector
         * \param[in] mode
         */

        /*!
        \brief Computes the operator-multivector application.

        Loosely, performs \f$Y = \alpha \cdot A^{\textrm{mode}} \cdot X + \beta \cdot Y\f$. However, the details of operation
        vary according to the values of \c alpha and \c beta. Specifically
        - if <tt>beta == 0</tt>, apply() <b>must</b> overwrite \c Y, so that any values in \c Y (including NaNs) are ignored.
        - if <tt>alpha == 0</tt>, apply() <b>may</b> short-circuit the operator, so that any values in \c X (including NaNs) are ignored.
        */
        virtual void apply(const XMultiVector &x,
                           XMultiVector &y,
                           ETransp mode=NO_TRANS,
                           SC alpha=ScalarTraits<SC>::one(),
                           SC beta=ScalarTraits<SC>::zero()) const;

        //! Get domain map
        virtual ConstXMapPtr getDomainMap() const;

        //! Get range map
        virtual ConstXMapPtr getRangeMap() const;

        /*!
        \brief Print description of this object to given output stream

        \param out Output stream to be used
        \param Verbosity level used for printing
        */
        virtual void describe(FancyOStream &out,
                              const EVerbosityLevel verbLevel=Describable::verbLevel_default) const;

        /*!
        \brief Get description of this operator

        \return String describing this operator
        */
        virtual string description() const;

        //! @name Access to class members
        //!@{

        //! Get #IsInitialized_
        bool isInitialized() const;

        //! Get #IsComputed_
        bool isComputed() const;

        int resetMatrix(ConstXMatrixPtr k,
                        bool reuseInitialize);

        /*!
        \brief Computes a residual using the operator
        */
        virtual void residual(const XMultiVector & X,
                              const XMultiVector & B,
                              XMultiVector& R) const;

        //!@}

    protected:

        //! Matrix
        ConstXMatrixPtr K_;

        //! Paremter list
        ParameterListPtr ParameterList_;

        //! Description of the solver
        string Description_;

        mutable XMultiVectorPtr YTmp_;

#ifdef HAVE_SHYLU_DDFROSCH_EPETRA
        ELinearProblemPtr EpetraLinearProblem_;
#endif

#ifdef HAVE_SHYLU_DDFROSCH_AMESOS
        AmesosSolverPtr AmesosSolver_;
#endif

#ifdef HAVE_SHYLU_DDFROSCH_EPETRA
        Amesos2SolverEpetraPtr Amesos2SolverEpetra_;
#endif
        Amesos2SolverTpetraPtr Amesos2SolverTpetra_;

#ifdef HAVE_SHYLU_DDFROSCH_MUELU
        //! Factory to create a MueLu hierarchy
        MueLuFactoryPtr MueLuFactory_;

        //! MueLu hierarchy object
        MueLuHierarchyPtr MueLuHierarchy_;
#endif

#ifdef HAVE_SHYLU_DDFROSCH_BELOS
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
        RCP<Belos::LinearProblem<SC,MultiVector<SC,LO,GO,NO>,Belos::OperatorT<MultiVector<SC,LO,GO,NO> > > > BelosLinearProblem_;
        RCP<Belos::SolverManager<SC,MultiVector<SC,LO,GO,NO>,Belos::OperatorT<MultiVector<SC,LO,GO,NO> > > > BelosSolverManager_;
#else
        RCP<Belos::LinearProblem<SC,MultiVector<SC,NO>,Belos::OperatorT<MultiVector<SC,NO> > > > BelosLinearProblem_;
        RCP<Belos::SolverManager<SC,MultiVector<SC,NO>,Belos::OperatorT<MultiVector<SC,NO> > > > BelosSolverManager_;
#endif
#endif

#ifdef HAVE_SHYLU_DDFROSCH_IFPACK2
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
        RCP<Ifpack2::Preconditioner<SC,LO,GO,NO> > Ifpack2Preconditioner_;
#else
        RCP<Ifpack2::Preconditioner<SC,NO> > Ifpack2Preconditioner_;
#endif
#endif

#ifdef HAVE_SHYLU_DDFROSCH_THYRA
        mutable RCP<Thyra::MultiVectorBase<SC> > ThyraYTmp_;
        RCP<Thyra::LinearOpWithSolveBase<SC> > LOWS_;
#endif

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
        RCP<TwoLevelBlockPreconditioner<SC,LO,GO,NO> > TLBP;
        RCP<TwoLevelPreconditioner<SC,LO,GO,NO> > TLP;
#else
        RCP<TwoLevelBlockPreconditioner<SC,NO> > TLBP;
        RCP<TwoLevelPreconditioner<SC,NO> > TLP;
#endif

        bool IsInitialized_ = false;

        //! Flag to indicated whether this subdomain solver has been setup/computed
        bool IsComputed_ = false;
    };

}

#endif

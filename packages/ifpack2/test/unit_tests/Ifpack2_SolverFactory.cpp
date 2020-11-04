#include "Teuchos_UnitTestHarness.hpp"
#include "Tpetra_Core.hpp"
#include "Tpetra_CrsMatrix.hpp"
#include "Tpetra_MultiVector.hpp"
#include "Ifpack2_Factory.hpp"
#include "Trilinos_Details_LinearSolver.hpp"
#include "Trilinos_Details_LinearSolverFactory.hpp"
// Define typedefs and macros for testing over all template parameter
// combinations.
#include "Ifpack2_ETIHelperMacros.h"

// FIXME (mfh 21 Aug 2015) Temporary work-around for Bug 6392.
#if ! defined(HAVE_TEUCHOS_DYNAMIC_LIBS)
namespace Ifpack2 {
namespace Details {
  // FIXME (mfh 21 Aug 2015) NONE of the commented-out things work.
  //
  // extern void __attribute__((weak)) registerLinearSolverFactory ();
  // void __attribute__((weak)) registerLinearSolverFactory ();
  // #pragma weak registerLinearSolverLibrary

  extern void registerLinearSolverFactory ();

} // namespace Details
} // namespace Ifpack2
#endif // ! defined(HAVE_TEUCHOS_DYNAMIC_LIBS)

namespace {
// FIXME (mfh 21 Aug 2015) Temporary work-around for Bug 6392.
#if ! defined(HAVE_TEUCHOS_DYNAMIC_LIBS)
  TEUCHOS_STATIC_SETUP()
  {
    // if (Ifpack2::Details::registerLinearSolverFactory == NULL) {
    //   std::cout << "-- Ifpack2::Details::registerLinearSolverFactory is NULL" << std::endl;
    // } else {
    //   Ifpack2::Details::registerLinearSolverFactory ();
    // }

    Ifpack2::Details::registerLinearSolverFactory ();
  }
#endif // ! defined(HAVE_TEUCHOS_DYNAMIC_LIBS)

  // Create a very simple square test matrix.  We use the identity
  // matrix here.  The point of this test is NOT to exercise the
  // preconditioner; it's just to check that its LinearSolverFactory
  // can create working preconditioners.  Ifpack2 has more rigorous
  // tests for each of its preconditioners.
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  template<class SC, class LO, class GO, class NT>
  Teuchos::RCP<Tpetra::CrsMatrix<SC, LO, GO, NT> >
#else
  template<class SC, class NT>
  Teuchos::RCP<Tpetra::CrsMatrix<SC, NT> >
#endif
  createTestMatrix (Teuchos::FancyOStream& out,
                    const Teuchos::RCP<const Teuchos::Comm<int> >& comm,
                    const Tpetra::global_size_t gblNumRows)
  {
    using Teuchos::Comm;
    using Teuchos::RCP;
    using Teuchos::rcp;
    using std::endl;
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    typedef Tpetra::CrsMatrix<SC,LO,GO,NT> MAT;
    typedef Tpetra::Map<LO,GO,NT> map_type;
#else
    typedef Tpetra::CrsMatrix<SC,NT> MAT;
    typedef Tpetra::Map<NT> map_type;
#endif
    typedef Teuchos::ScalarTraits<SC> STS;

    Teuchos::OSTab tab0 (out);
    out << "Create test matrix with " << gblNumRows << " row(s)" << endl;
    Teuchos::OSTab tab1 (out);

    TEUCHOS_TEST_FOR_EXCEPTION
      ( gblNumRows == 0, std::invalid_argument, "gblNumRows = 0");

    const GO indexBase = 0;
    RCP<const map_type> rowMap (new map_type (gblNumRows, indexBase, comm));
    // For this particular row matrix, the row Map and the column Map
    // are the same.  Giving a column Map to CrsMatrix's constructor
    // lets us use local indices.
    RCP<const map_type> colMap = rowMap;
    const size_t maxNumEntPerRow = 1;
    RCP<MAT> A (new MAT (rowMap, colMap, maxNumEntPerRow, Tpetra::StaticProfile));

    if (rowMap->getNodeNumElements () != 0) {
      Teuchos::Array<SC> vals (1);
      Teuchos::Array<LO> inds (1);
      for (LO lclRow = rowMap->getMinLocalIndex ();
           lclRow <= rowMap->getMaxLocalIndex (); ++lclRow) {
        inds[0] = lclRow;
        vals[0] = STS::one ();
        A->insertLocalValues (lclRow, inds (), vals ());
      }
    }

    RCP<const map_type> domMap = rowMap;
    RCP<const map_type> ranMap = rowMap;
    A->fillComplete (domMap, ranMap);
    return A;
  }

  // Create a very simple square test linear system (matrix,
  // right-hand side(s), and exact solution(s).  We use the identity
  // matrix here.  The point of this test is NOT to exercise the
  // preconditioner; it's just to check that its LinearSolverFactory
  // can create working preconditioners.  Ifpack2 has more rigorous
  // tests for each of its preconditioners.
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  template<class SC, class LO, class GO, class NT>
#else
  template<class SC, class NT>
#endif
  void
  createTestProblem (Teuchos::FancyOStream& out,
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
                     Teuchos::RCP<Tpetra::MultiVector<SC, LO, GO, NT> >& X,
                     Teuchos::RCP<Tpetra::CrsMatrix<SC, LO, GO, NT> >& A,
                     Teuchos::RCP<Tpetra::MultiVector<SC, LO, GO, NT> >& B,
#else
                     Teuchos::RCP<Tpetra::MultiVector<SC, NT> >& X,
                     Teuchos::RCP<Tpetra::CrsMatrix<SC, NT> >& A,
                     Teuchos::RCP<Tpetra::MultiVector<SC, NT> >& B,
#endif
                     const Teuchos::RCP<const Teuchos::Comm<int> >& comm,
                     const Tpetra::global_size_t gblNumRows,
                     const size_t numVecs)
  {
    using Teuchos::Comm;
    using Teuchos::RCP;
    using Teuchos::rcp;
    using std::endl;
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    typedef Tpetra::MultiVector<SC,LO,GO,NT> MV;
#else
    typedef Tpetra::MultiVector<SC,NT> MV;
#endif
    typedef Teuchos::ScalarTraits<SC> STS;

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    A = createTestMatrix<SC, LO, GO, NT> (out, comm, gblNumRows);
#else
    A = createTestMatrix<SC, NT> (out, comm, gblNumRows);
#endif
    X = rcp (new MV (A->getDomainMap (), numVecs));
    B = rcp (new MV (A->getRangeMap (), numVecs));

    X->putScalar (STS::one ());
    A->apply (*X, *B);
  }

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  template<class SC, class LO, class GO, class NT>
#else
  template<class SC, class NT>
#endif
  void
  testSolver (Teuchos::FancyOStream& out,
              bool& success,
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
              Tpetra::MultiVector<SC, LO, GO, NT>& X,
              const Teuchos::RCP<const Tpetra::CrsMatrix<SC, LO, GO, NT> >& A,
              const Tpetra::MultiVector<SC, LO, GO, NT>& B,
              const Tpetra::MultiVector<SC, LO, GO, NT>& X_exact,
#else
              Tpetra::MultiVector<SC, NT>& X,
              const Teuchos::RCP<const Tpetra::CrsMatrix<SC, NT> >& A,
              const Tpetra::MultiVector<SC, NT>& B,
              const Tpetra::MultiVector<SC, NT>& X_exact,
#endif
              const std::string& solverName)
  {
    using Teuchos::Comm;
    using Teuchos::RCP;
    using Teuchos::rcp;
    using std::endl;
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    typedef Tpetra::Operator<SC,LO,GO,NT> OP;
    typedef Tpetra::MultiVector<SC,LO,GO,NT> MV;
#else
    typedef Tpetra::Operator<SC,NT> OP;
    typedef Tpetra::MultiVector<SC,NT> MV;
#endif
    typedef Teuchos::ScalarTraits<SC> STS;
    typedef typename MV::mag_type mag_type;

    Teuchos::OSTab tab0 (out);
    out << "Test solver \"" << solverName << "\" from Ifpack2 package" << endl;
    Teuchos::OSTab tab1 (out);

    RCP<Trilinos::Details::LinearSolver<MV, OP, mag_type> > solver;
    try {
      solver = Trilinos::Details::getLinearSolver<MV, OP, mag_type> ("Ifpack2", solverName);
    } catch (std::exception& e) {
      out << "*** FAILED: getLinearSolver threw an exception: " << e.what () << endl;
      success = false;
      return;
    }
    TEST_EQUALITY_CONST( solver.is_null (), false );
    if (solver.is_null ()) {
      out << "*** FAILED to create solver \"" << solverName
          << "\" from Ifpack2 package" << endl;
      success = false;
      return;
    }

    out << "Set matrix" << endl;
    solver->setMatrix (A);

    out << "Compute symbolic and numeric factorization" << endl;
    solver->symbolic ();
    solver->numeric ();

    out << "Apply solver to \"solve\" AX=B for X.  Ifpack2 only promises "
      "preconditioning, so we don't expect to get X right.  Mainly we check "
      "that solve() doesn't throw." << endl;
    X.putScalar (STS::zero ());
    solver->solve (X, B);
  }

  //
  // The actual unit test.
  //
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  TEUCHOS_UNIT_TEST_TEMPLATE_4_DECL( SolverFactory, Solve, SC, LO, GO, NT )
#else
  TEUCHOS_UNIT_TEST_TEMPLATE_2_DECL( SolverFactory, Solve, SC, NT )
#endif
  {
#ifndef TPETRA_ENABLE_TEMPLATE_ORDINALS
    using LO = typename Tpetra::Map<>::local_ordinal_type;
    using GO = typename Tpetra::Map<>::global_ordinal_type;
#endif
    using Teuchos::Comm;
    using Teuchos::RCP;
    using Teuchos::rcp;
    using Teuchos::TypeNameTraits;
    using std::endl;
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    typedef Tpetra::CrsMatrix<SC,LO,GO,NT> MAT;
    typedef Tpetra::MultiVector<SC,LO,GO,NT> MV;
    typedef Tpetra::RowMatrix<SC,LO,GO,NT> row_matrix_type;
#else
    typedef Tpetra::CrsMatrix<SC,NT> MAT;
    typedef Tpetra::MultiVector<SC,NT> MV;
    typedef Tpetra::RowMatrix<SC,NT> row_matrix_type;
#endif

#if ! defined(TRILINOS_HAVE_LINEAR_SOLVER_FACTORY_REGISTRATION)
    out << "LinearSolverFactory run-time registration disabled; "
      "not running test" << endl;
    return;
#endif // NOT TRILINOS_HAVE_LINEAR_SOLVER_FACTORY_REGISTRATION

    RCP<const Comm<int> > comm = Tpetra::getDefaultComm ();
    const Tpetra::global_size_t gblNumRows = comm->getSize () * 10;
    const size_t numVecs = 3;

    out << "Create test problem" << endl;
    RCP<MV> X_exact, B;
    RCP<MAT> A;
    createTestProblem (out, X_exact, A, B, comm, gblNumRows, numVecs);

    RCP<MV> X = rcp (new MV (X_exact->getMap (), numVecs));

    // FIXME (mfh 26 Jul 2015) Need to test more solvers.  In
    // particular, it's important to test AdditiveSchwarz.
    const int numSolvers = 5;
    const char* solverNames[5] = {"DIAGONAL", "RELAXATION", "CHEBYSHEV",
                                  "ILUT", "RILUK"};
    int numSolversTested = 0;
    for (int k = 0; k < numSolvers; ++k) {
      const std::string solverName (solverNames[k]);

      // Use Ifpack2's factory to tell us whether the factory supports
      // the given combination of template parameters.  If create()
      // throws, the combination is not supported.
      bool skip = false;
      try {
        (void) Ifpack2::Factory::create<row_matrix_type> (solverName, A);
      }
      catch (...) {
        skip = true;
      }
      if (! skip) {
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
        testSolver<SC, LO, GO, NT> (out, success, *X, A, *B, *X_exact, solverName);
#else
        testSolver<SC, NT> (out, success, *X, A, *B, *X_exact, solverName);
#endif
        ++numSolversTested;
      }
    }

    out << "Tested " << numSolversTested << " solver(s) of " << numSolvers
        << endl;
    if (numSolversTested == 0) {
      out << "*** ERROR: Tested no solvers for template parameters"
          << "SC = " << TypeNameTraits<SC>::name ()
          << ", LO = " << TypeNameTraits<LO>::name ()
          << ", GO = " << TypeNameTraits<GO>::name ()
          << ", NT = " << TypeNameTraits<NT>::name () << endl;
      success = false;
    }
  }

  // Define typedefs that make the Tpetra macros work.
  IFPACK2_ETI_MANGLING_TYPEDEFS()

// Macro that instantiates the unit test
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
#define LCLINST( SC, LO, GO, NT ) \
  TEUCHOS_UNIT_TEST_TEMPLATE_4_INSTANT( SolverFactory, Solve, SC, LO, GO, NT )
#else
#define LCLINST( SC, NT ) \
  TEUCHOS_UNIT_TEST_TEMPLATE_2_INSTANT( SolverFactory, Solve, SC, NT )
#endif

// Ifpack2's ETI will instantiate the unit test for all enabled type
// combinations.
IFPACK2_INSTANTIATE_SLGN( LCLINST )

} // namespace (anonymous)

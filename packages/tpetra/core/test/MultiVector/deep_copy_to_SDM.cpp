/*
// @HEADER
// ***********************************************************************
//
//          Tpetra: Templated Linear Algebra Services Package
//                 Copyright (2008) Sandia Corporation
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
// @HEADER
*/

#include "Tpetra_TestingUtilities.hpp"
#include "Tpetra_Map.hpp"
#include "Tpetra_MultiVector.hpp"
#include "Tpetra_Vector.hpp"
#include "Kokkos_ArithTraits.hpp"
#include "Kokkos_Core.hpp"
#include "Teuchos_ScalarTraits.hpp"
#include "Teuchos_SerialDenseMatrix.hpp"

namespace { // (anonymous)

  template<class ViewType1, class ViewType2>
  bool
  view1dSame (ViewType1 x, ViewType2 y)
  {
    using execution_space = typename ViewType1::execution_space;
    using range_type = Kokkos::RangePolicy<execution_space, size_t>;

    if (x.extent (0) != y.extent (0)) {
      return false;
    }
    const size_t size = static_cast<size_t> (x.extent (0));
    int allSame = 0;
    Kokkos::parallel_reduce
      ("view1dSame", range_type (0, size),
       KOKKOS_LAMBDA (const size_t k, int& curResult) {
        const int equal = (x(k) == y(k)) ? 1 : 0;
        curResult = curResult && equal;
      }, Kokkos::LAnd<int> (allSame));
    return allSame == 1;
  }

  template<class ViewType1, class ViewType2>
  bool
  view2dSame (ViewType1 x, ViewType2 y)
  {
    using execution_space = typename ViewType1::execution_space;
    using range_type = Kokkos::RangePolicy<execution_space, size_t>;

    if (x.extent (0) != y.extent (0)) {
      return false;
    }
    if (x.extent (1) != y.extent (1)) {
      return false;
    }

    const size_t nrow = static_cast<size_t> (x.extent (0));
    const size_t ncol = static_cast<size_t> (x.extent (1));
    int allSame = 0;
    Kokkos::LAnd<int> reducer (allSame);
    Kokkos::parallel_reduce
      ("view2dSame", range_type (0, nrow),
       KOKKOS_LAMBDA (const size_t row, int& curResult) {
        int rowEqual = 1;
        for (size_t col = 0; col < ncol; ++col) {
          if (x(row,col) != y(row,col)) {
            rowEqual = 0;
            break;
          }
        }
        curResult = curResult && rowEqual;
      }, Kokkos::LAnd<int> (allSame));
    return allSame == 1;
  }

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  template<class ST, class LO, class GO, class NT>
#else
  template<class ST, class NT>
#endif
  bool
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  serialDenseMatrix_multiVector_same (const Tpetra::MultiVector<ST, LO, GO, NT>& X,
#else
  serialDenseMatrix_multiVector_same (const Tpetra::MultiVector<ST, NT>& X,
#endif
                                      const Teuchos::SerialDenseMatrix<int, ST>& Y)
  {
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    using MV = Tpetra::MultiVector<ST, LO, GO, NT>;
#else
    using MV = Tpetra::MultiVector<ST, NT>;
#endif
    using IST = typename MV::impl_scalar_type;
    using sdm_view_type = Kokkos::View<const IST**, Kokkos::LayoutLeft,
      Kokkos::HostSpace, Kokkos::MemoryUnmanaged>;
    using pair_type = std::pair<int, int>;

    sdm_view_type Y_orig (Y.values (), Y.stride (), Y.numCols ());
    auto Y_view = Kokkos::subview (Y_orig, pair_type (0, Y.numRows ()),
                                   pair_type (0, Y.numCols ()));
    if (X.need_sync_host ()) { // X was changed on device
      if (X.isConstantStride ()) {
        // Don't actually sync X; we don't want to change its state here.
        auto X_lcl_d = X.getLocalViewDevice ();
        auto X_lcl_h = Kokkos::create_mirror_view (X_lcl_d);
        Kokkos::deep_copy (X_lcl_h, X_lcl_d);
        return view2dSame (X_lcl_h, Y_view);
      }
      else {
        for (size_t col = 0; col < X.getNumVectors (); ++col) {
          auto X_col = X.getVector (col);
          auto X_col_lcl_d_2d = X_col->getLocalViewDevice ();
          auto X_col_lcl_d = Kokkos::subview (X_col_lcl_d_2d, Kokkos::ALL (), 0);
          // Don't actually sync X; we don't want to change its state here.
          auto X_col_lcl_h = Kokkos::create_mirror_view (X_col_lcl_d);
          Kokkos::deep_copy (X_col_lcl_h, X_col_lcl_d);
          auto Y_col = Kokkos::subview (Y_view, Kokkos::ALL (), col);
          if (! view1dSame (X_col_lcl_h, Y_col)) {
            return false;
          }
        }
        return true;
      }
    }
    else { // X is current on host
      if (X.isConstantStride ()) {
        auto X_lcl_h = X.getLocalViewHost ();
        return view2dSame (X_lcl_h, Y_view);
      }
      else {
        for (size_t col = 0; col < X.getNumVectors (); ++col) {
          auto X_col = X.getVector (col);
          auto X_col_lcl_h_2d = X_col->getLocalViewHost ();
          auto X_col_lcl_h = Kokkos::subview (X_col_lcl_h_2d, Kokkos::ALL (), 0);
          auto Y_col = Kokkos::subview (Y_view, Kokkos::ALL (), col);
          if (! view1dSame (X_col_lcl_h, Y_col)) {
            return false;
          }
        }
        return true;
      }
    }
  }

  template<class ValueType>
  KOKKOS_INLINE_FUNCTION ValueType toValue (const size_t k)
  {
    using mag_type = typename Kokkos::ArithTraits<ValueType>::mag_type;
    return static_cast<ValueType> (static_cast<mag_type> (k));
  }

  template<class ValueType, class LayoutType, class DeviceType>
  void
  view1dIota (const Kokkos::View<ValueType*, LayoutType, DeviceType>& x,
              const ValueType startValue)
  {
    using exec_space = typename DeviceType::execution_space;
    using range_type = Kokkos::RangePolicy<exec_space, ptrdiff_t>;

    Kokkos::parallel_for
      ("view1dIota", range_type (0, x.extent (0)),
       KOKKOS_LAMBDA (const ptrdiff_t k) {
         x(k) = startValue + toValue<ValueType> (k);
       });
  }

  template<class ViewType>
  void
  view2dIota (const ViewType& x,
              const typename ViewType::non_const_value_type& startValue)
  {
    using value_type = typename ViewType::non_const_value_type;
    using exec_space = typename ViewType::execution_space;
    using range_type = Kokkos::RangePolicy<exec_space, ptrdiff_t>;

    const ptrdiff_t nrow = static_cast<ptrdiff_t> (x.extent (0));
    const ptrdiff_t ncol = static_cast<ptrdiff_t> (x.extent (1));

    Kokkos::parallel_for
      ("view2dIota", range_type (0, nrow),
       KOKKOS_LAMBDA (const ptrdiff_t row) {
         for (ptrdiff_t col = 0; col < ncol; ++col) {
           x(row,col) = startValue +
             toValue<value_type> (ncol) * toValue<value_type> (row) +
             toValue<value_type> (row);
         }
       });
  }

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  template<class ST, class LO, class GO, class NT>
#else
  template<class ST, class NT>
#endif
  void
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  multiVectorIota (Tpetra::MultiVector<ST, LO, GO, NT>& X,
#else
  multiVectorIota (Tpetra::MultiVector<ST, NT>& X,
#endif
                   const ST& startValue)
  {
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    using MV = Tpetra::MultiVector<ST, LO, GO, NT>;
#else
    using MV = Tpetra::MultiVector<ST, NT>;
#endif
    using IST = typename MV::impl_scalar_type;

    const IST startValueIST = static_cast<IST> (startValue);
    const IST lclNumRowsIST = toValueHost<IST> (X.getLocalLength ());

    if (X.need_sync_device ()) { // modify on host
      X.modify_host ();
      if (X.isConstantStride ()) {
        view2dIota (X.getLocalViewHost (), startValueIST);
      }
      else {
        const size_t numCols = X.getNumVectors ();
        for (size_t col = 0; col < numCols; ++col) {
          auto X_col = X.getVectorNonConst (col);
          auto X_col_lcl_h_2d = X_col->getLocalViewHost ();
          auto X_col_lcl_h =
            Kokkos::subview (X_col_lcl_h_2d, Kokkos::ALL (), 0);
          view1dIota (X_col_lcl_h, startValueIST +
                      toValueHost<IST> (col) * lclNumRowsIST);
        }
      }
    }
    else { // modify on device
      X.modify_device ();
      if (X.isConstantStride ()) {
        view2dIota (X.getLocalViewDevice (), startValueIST);
      }
      else {
        const size_t numCols = X.getNumVectors ();
        for (size_t col = 0; col < numCols; ++col) {
          auto X_col = X.getVectorNonConst (col);
          auto X_col_lcl_d_2d = X_col->getLocalViewDevice ();
          auto X_col_lcl_d =
            Kokkos::subview (X_col_lcl_d_2d, Kokkos::ALL (), 0);
          view1dIota (X_col_lcl_d, startValueIST +
                      toValueHost<IST> (col) * lclNumRowsIST);
        }
      }
    }
  }

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  TEUCHOS_UNIT_TEST_TEMPLATE_4_DECL( MultiVector, deep_copy_to_SDM, ST, LO, GO, NT )
#else
  TEUCHOS_UNIT_TEST_TEMPLATE_2_DECL( MultiVector, deep_copy_to_SDM, ST, NT )
#endif
  {
#ifndef TPETRA_ENABLE_TEMPLATE_ORDINALS
    using LO = typename Tpetra::Map<>::local_ordinal_type;
    using GO = typename Tpetra::Map<>::global_ordinal_type;
#endif
    using Teuchos::RCP;
    using Teuchos::rcp;
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    using map_type = Tpetra::Map<LO, GO, NT>;
    using MV = Tpetra::MultiVector<ST, LO, GO, NT>;
#else
    using map_type = Tpetra::Map<NT>;
    using MV = Tpetra::MultiVector<ST, NT>;
#endif
    using IST = typename MV::impl_scalar_type;

    out << "Test Tpetra::deep_copy from Tpetra::MultiVector "
      "to Teuchos::SerialDenseMatrix" << std::endl;
    Teuchos::OSTab tab1 (out);

    const IST flagValue = Kokkos::ArithTraits<IST>::one ();
    const IST startValue = Teuchos::ScalarTraits<ST>::one () +
      Teuchos::ScalarTraits<ST>::one ();
    const GO indexBase = 0;

    auto comm = Tpetra::TestingUtilities::getDefaultComm ();

    {
      out << "Test MultiVectors with constant stride" << std::endl;
      Teuchos::OSTab tab2 (out);

      const LO lclNumRowsVals[] = {7, 3, 0, 1};
      const LO numColsVals[] = {3, 2, 5, 0, 1};
      for (LO lclNumRows : lclNumRowsVals) {
        for (LO numCols : numColsVals) {
          for (bool modify_MV_on_host : {false, true}) {
            Teuchos::SerialDenseMatrix<int, ST> Y (lclNumRows, numCols);
            Y.putScalar (flagValue);

            const GO gblNumRows = static_cast<GO> (comm->getSize ()) *
              static_cast<GO> (lclNumRows);
            RCP<const map_type> map =
              rcp (new map_type (gblNumRows, lclNumRows, indexBase, comm));
            MV X (map, numCols);

            multiVectorIota (X, startValue, modify_MV_on_host);

            Tpetra::deep_copy (Y, X);
            TEST_ASSERT( serialDenseMatrix_multiVector_same (X, Y) );
          }
        }
      }
    }

    {
      out << "Test MultiVector with nonconstant stride" << std::endl;
      Teuchos::OSTab tab2 (out);

      const LO lclNumRows = 15;
      const LO origNumCols = 7;
      const std::vector<size_t> colsToSelect {{ 1, 3, 5, 6 }};
      const LO numCols = static_cast<LO> (colsToSelect.size ());

      const GO gblNumRows = static_cast<GO> (comm->getSize ()) *
        static_cast<GO> (lclNumRows);
      RCP<const map_type> map =
        rcp (new map_type (gblNumRows, lclNumRows, indexBase, comm));
      MV X (map, origNumCols);
      auto X_sub = X.subViewNonConst (colsToSelect);
      TEST_ASSERT( static_cast<LO> (X_sub->getNumVectors ()) == numCols );

      for (bool modify_MV_on_host : {false, true}) {
        Teuchos::SerialDenseMatrix<int, ST> Y (lclNumRows, numCols);
        Y.putScalar (flagValue);

        if (modify_MV_on_host) {
          X.sync_host ();
          Kokkos::deep_copy (X.getLocalViewHost (), flagValue);
        }
        else {
          Kokkos::deep_copy (X.getLocalViewDevice (), flagValue);
        }

        multiVectorIota (*X_sub, startValue, modify_MV_on_host);

        Tpetra::deep_copy (Y, *X_sub);
        TEST_ASSERT( serialDenseMatrix_multiVector_same (*X_sub, Y) );
      }
    }
  }

  //
  // INSTANTIATIONS
  //

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
#define UNIT_TEST_GROUP( ST, LO, GO, NT ) \
  TEUCHOS_UNIT_TEST_TEMPLATE_4_INSTANT( MultiVector, deep_copy_to_SDM, ST, LO, GO, NT )
#else
#define UNIT_TEST_GROUP( ST, NT ) \
  TEUCHOS_UNIT_TEST_TEMPLATE_2_INSTANT( MultiVector, deep_copy_to_SDM, ST, NT )
#endif

  TPETRA_ETI_MANGLING_TYPEDEFS()

  TPETRA_INSTANTIATE_TESTMV( UNIT_TEST_GROUP )

} // namespace (anonymous)

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
// ************************************************************************
// @HEADER

#ifndef TPETRA_BLOCKCRSMATRIX_HELPERS_DECL_HPP
#define TPETRA_BLOCKCRSMATRIX_HELPERS_DECL_HPP

/// \file Tpetra_BlockCrsMatrix_Helpers_decl.hpp

#include "Tpetra_BlockCrsMatrix_fwd.hpp"
#include "Tpetra_CrsMatrix_fwd.hpp"
#include "Tpetra_Map_fwd.hpp"
#include "Teuchos_RCP.hpp"
#include <string>
#include <ostream>

#ifndef DOXYGEN_SHOULD_SKIP_THIS
namespace Teuchos {
  class ParameterList; // forward declaration
} // namespace Teuchos
#endif // DOXYGEN_SHOULD_SKIP_THIS

namespace Tpetra {

  /// \brief Helper function to write a BlockCrsMatrix.  Calls the 3-argument version.
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  template<class Scalar, class LO, class GO, class Node>
  void blockCrsMatrixWriter(BlockCrsMatrix<Scalar,LO,GO,Node> const &A, std::string const &fileName);
#else
  template<class Scalar, class Node>
  void blockCrsMatrixWriter(BlockCrsMatrix<Scalar,Node> const &A, std::string const &fileName);
#endif

  /// \brief Helper function to write a BlockCrsMatrix.  Calls the 3-argument version.
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  template<class Scalar, class LO, class GO, class Node>
  void blockCrsMatrixWriter(BlockCrsMatrix<Scalar,LO,GO,Node> const &A, std::string const &fileName, Teuchos::ParameterList const &params);
#else
  template<class Scalar, class Node>
  void blockCrsMatrixWriter(BlockCrsMatrix<Scalar,Node> const &A, std::string const &fileName, Teuchos::ParameterList const &params);
#endif

  /// \brief Helper function to write a BlockCrsMatrix.  Calls the 3-argument version.
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  template<class Scalar, class LO, class GO, class Node>
  void blockCrsMatrixWriter(BlockCrsMatrix<Scalar,LO,GO,Node> const &A, std::ostream &os);
#else
  template<class Scalar, class Node>
  void blockCrsMatrixWriter(BlockCrsMatrix<Scalar,Node> const &A, std::ostream &os);
#endif

  /*! \brief Helper function to write a BlockCrsMatrix.

    Writes the block matrix to the specified ostream in point form.  The following parameter list options are available:

    - "always use parallel algorithm" : on one process, this forces the use of the parallel strip-mining algorithm (default=false)
    - "print MatrixMarket header"     : if false, don't print the MatrixMarket header (default=true)
    - "precision"                     : precision to be used in printing matrix entries (default=C++ default)
    - "zero-based indexing"           : if true, print the matrix with 0-based indexing (default=false)
  */
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  template<class Scalar, class LO, class GO, class Node>
  void blockCrsMatrixWriter(BlockCrsMatrix<Scalar,LO,GO,Node> const &A, std::ostream &os, Teuchos::ParameterList const &params);
#else
  template<class Scalar, class Node>
  void blockCrsMatrixWriter(BlockCrsMatrix<Scalar,Node> const &A, std::ostream &os, Teuchos::ParameterList const &params);
#endif

  /*! @brief Helper function called by blockCrsMatrixWriter.

  This function should not be called directly.
  */
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  template<class Scalar, class LO, class GO, class Node>
  void writeMatrixStrip(BlockCrsMatrix<Scalar,LO,GO,Node> const &A, std::ostream &os, Teuchos::ParameterList const &params);
#else
  template<class Scalar, class Node>
  void writeMatrixStrip(BlockCrsMatrix<Scalar,Node> const &A, std::ostream &os, Teuchos::ParameterList const &params);
#endif

  /// \brief Non-member constructor that creates a BlockCrsMatrix from an existing point CrsMatrix.
  ///
  /// This function accepts an already constructed point version of the block matrix.
  /// Assumptions:
  ///   - All point entries in a logical block must be stored in the CrsMatrix, even
  ///     if the values are zero.
  ///   - Point rows corresponding to a particular mesh node must be stored consecutively.
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  template<class Scalar, class LO, class GO, class Node>
  Teuchos::RCP<BlockCrsMatrix<Scalar, LO, GO, Node>>
  convertToBlockCrsMatrix(const Tpetra::CrsMatrix<Scalar, LO, GO, Node>& pointMatrix, const LO &blockSize);
#else
  template<class Scalar, class Node>
  Teuchos::RCP<BlockCrsMatrix<Scalar, Node>>
  convertToBlockCrsMatrix(const Tpetra::CrsMatrix<Scalar, Node>& pointMatrix, const LO &blockSize);
#endif

  /// @brief Helper function to generate a mesh map from a point map.
  /// Important! It's assumed that point GIDs associated with a single mesh GID appear consecutively in pointMap.
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  template<class LO, class GO, class Node>
  Teuchos::RCP<const Tpetra::Map<LO,GO,Node>>
  createMeshMap(LO const &blockSize, const Tpetra::Map<LO,GO,Node> &pointMap);
#else
  template<class Node>
  Teuchos::RCP<const Tpetra::Map<Node>>
  createMeshMap(LO const &blockSize, const Tpetra::Map<Node> &pointMap);
#endif

} // namespace Tpetra

#endif // TPETRA_EXPERIMENTAL_BLOCKCRSMATRIX_HELPERS_DECL_HPP

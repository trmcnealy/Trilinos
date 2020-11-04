// @HEADER
//
// ***********************************************************************
//
//             Xpetra: A linear algebra interface package
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
#ifndef XPETRA_TPETRAIMPORT_DECL_HPP
#define XPETRA_TPETRAIMPORT_DECL_HPP

/* this file is automatically generated - do not edit (see script/tpetra.py) */

#include "Xpetra_TpetraConfigDefs.hpp"

#include "Xpetra_Import.hpp"
#include "Xpetra_Exceptions.hpp"

#include "Xpetra_TpetraMap_decl.hpp"
#include "Tpetra_Import.hpp"

namespace Xpetra {

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  template <class LocalOrdinal,
            class GlobalOrdinal,
            class Node = KokkosClassic::DefaultNode::DefaultNodeType>
#else
  template <class Node = KokkosClassic::DefaultNode::DefaultNodeType>
#endif
  class TpetraImport
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    : public Import<LocalOrdinal, GlobalOrdinal, Node>
#else
    : public Import<Node>
#endif
  {

  public:

#ifndef TPETRA_ENABLE_TEMPLATE_ORDINALS
    using LocalOrdinal = typename Tpetra::Map<>::local_ordinal_type;
    using GlobalOrdinal = typename Tpetra::Map<>::global_ordinal_type;
#endif
    //! The specialization of Map used by this class.
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    typedef Map<LocalOrdinal,GlobalOrdinal,Node> map_type;
#else
    typedef Map<Node> map_type;
#endif

    //! @name Constructor/Destructor Methods
    //@{

    //! Construct an Import from the source and target Maps.
    TpetraImport(const Teuchos::RCP< const map_type > &source, const Teuchos::RCP< const map_type > &target);

    //! Constructor (with list of parameters).
    TpetraImport(const Teuchos::RCP< const map_type > &source, const Teuchos::RCP< const map_type > &target, const Teuchos::RCP< Teuchos::ParameterList > &plist);

    //! Copy constructor.
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    TpetraImport(const Import< LocalOrdinal, GlobalOrdinal, Node > &import);
#else
    TpetraImport(const Import<Node > &import);
#endif

    //! Destructor.
    ~TpetraImport();

    //@}

    //! @name Import Attribute Methods
    //@{

    //! Number of initial identical IDs.
    size_t getNumSameIDs() const;

    //! Number of IDs to permute but not to communicate.
    size_t getNumPermuteIDs() const;

    //! List of local IDs in the source Map that are permuted.
    ArrayView< const LocalOrdinal > getPermuteFromLIDs() const;

    //! List of local IDs in the target Map that are permuted.
    ArrayView< const LocalOrdinal > getPermuteToLIDs() const;

    //! Number of entries not on the calling process.
    size_t getNumRemoteIDs() const;

    //! List of entries in the target Map to receive from other processes.
    ArrayView< const LocalOrdinal > getRemoteLIDs() const;

    //! Set parameters on distributor
    void setDistributorParameters(const Teuchos::RCP<Teuchos::ParameterList> params) const;

    //! Number of entries that must be sent by the calling process to other processes.
    size_t getNumExportIDs() const;

    //! List of entries in the source Map that will be sent to other processes.
    ArrayView< const LocalOrdinal > getExportLIDs() const;

    //! List of processes to which entries will be sent.
    ArrayView< const int > getExportPIDs() const;

    //! The Source Map used to construct this Import object.
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    Teuchos::RCP< const Map< LocalOrdinal, GlobalOrdinal, Node > > getSourceMap() const;
#else
    Teuchos::RCP< const Map<Node > > getSourceMap() const;
#endif

    //! The Target Map used to construct this Import object.
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    Teuchos::RCP< const Map< LocalOrdinal, GlobalOrdinal, Node > > getTargetMap() const;
#else
    Teuchos::RCP< const Map<Node > > getTargetMap() const;
#endif

    //@}

    //! @name I/O Methods
    //@{

    //! Print the Import's data to the given output stream.
    void print(std::ostream &os) const;

    //@}

    //! @name Xpetra specific
    //@{

    //! TpetraImport constructor to wrap a Tpetra::Import object
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    TpetraImport(const RCP<const Tpetra::Import< LocalOrdinal, GlobalOrdinal, Node > > &import);
#else
    TpetraImport(const RCP<const Tpetra::Import<Node > > &import);
#endif

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    RCP< const Tpetra::Import< LocalOrdinal, GlobalOrdinal, Node > > getTpetra_Import() const;
#else
    RCP< const Tpetra::Import<Node > > getTpetra_Import() const;
#endif

    //@}

  private:

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    RCP<const Tpetra::Import< LocalOrdinal, GlobalOrdinal, Node > > import_;
#else
    RCP<const Tpetra::Import<Node > > import_;
#endif

  }; // TpetraImport class


  // TODO: move that elsewhere
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  template <class LocalOrdinal, class GlobalOrdinal, class Node>
  const Tpetra::Import<LocalOrdinal,GlobalOrdinal,Node> & toTpetra(const Import<LocalOrdinal,GlobalOrdinal,Node> &import) {
#else
  template <class Node>
  const Tpetra::Import<Node> & toTpetra(const Import<Node> &import) {
#endif
    // TODO: throw exception
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
    const TpetraImport<LocalOrdinal,GlobalOrdinal,Node> & tpetraImport = dynamic_cast<const TpetraImport<LocalOrdinal,GlobalOrdinal,Node> &>(import);
#else
    const TpetraImport<Node> & tpetraImport = dynamic_cast<const TpetraImport<Node> &>(import);
#endif
    return *tpetraImport.getTpetra_Import();
  }

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
  template <class LocalOrdinal, class GlobalOrdinal, class Node>
  RCP<const Import<LocalOrdinal,GlobalOrdinal,Node> > toXpetra(const RCP< const Tpetra::Import<LocalOrdinal,GlobalOrdinal,Node > >& import) {
#else
  template <class Node>
  RCP<const Import<Node> > toXpetra(const RCP< const Tpetra::Import<Node > >& import) {
#endif
    if (!import.is_null())
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
      return rcp(new TpetraImport<LocalOrdinal,GlobalOrdinal,Node>(import));
#else
      return rcp(new TpetraImport<Node>(import));
#endif

    return Teuchos::null;
  }

} // Xpetra namespace

#endif // XPETRA_TPETRAIMPORT_HPP


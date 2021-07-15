// @HEADER
// ***********************************************************************
//
//                   KLU2: A Direct Linear Solver package
//                    Copyright 2011 Sandia Corporation
//
// Under terms of Contract DE-AC04-94AL85000, with Sandia Corporation, the 
// U.S. Government retains certain rights in this software.
//
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along int with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact Mike A. Heroux (maherou@sandia.gov)
//
// KLU2 is derived work from KLU, licensed under LGPL, and copyrighted by
// University of Florida. The Authors of KLU are Timothy A. Davis and
// Eka Palamadai. See Doc/KLU_README.txt for the licensing and copyright
// information for KLU.
//
// ***********************************************************************
// @HEADER

#ifndef KLU2_ORDINALTRAITS_H
#define KLU2_ORDINALTRAITS_H

#include <trilinos_colamd.h>

template <typename T>
struct KLU_OrdinalTraits
{
    /* None of this methods should technically return anything.
     * If the OrdinalType is no supported and there is no
     * specialization available, we will error out. These 
     * returns are just to keep the compiler happy and stop
     * spewing warnings.
     * */
    static inline T btf_order (T n, T *Ap, T *Ai, double maxwork,
        double *work, T *P, T *Q, T *R, T *nmatch, T *Work)
    {
        return n;
    }

    static inline T btf_strongcomp (T n, T *Ap, T *Ai, T *Q, T *P, 
        T *R, T *Work) 
    {
        return n;
    }
    
    static inline T amd_order (T n, T *Ap, T *Ai, T *P, double *Control,
        double *Info)
    {
        return n;
    }

    static inline T colamd (T n_row, T n_col, T Alen, T *A, T *p,
        double *knobs, T *stats)
    {
        return n_row;
    }

    static inline T colamd_recommended (T nnz, T n_row, T n_col)
    {
        return nnz;
    }
};

template<>
struct KLU_OrdinalTraits<int>
{
    static inline int btf_order (int n, int *Ap, int *Ai,
        double maxwork, double *work, int *P, int *Q, int *R, int *nmatch,
        int *Work)
    {
        return (trilinos_btf_order (n, Ap, Ai, maxwork, work, P, Q, R, nmatch,
                    Work)) ;
    }

    static inline int btf_strongcomp (int n, int *Ap, int *Ai, int *Q, int *P,
        int *R, int *Work)
    {
        return (trilinos_btf_strongcomp (n, Ap, Ai, Q, P, R, Work)) ;
    }

    static inline int amd_order (int n, int *Ap, int *Ai, int *P,
        double *Control, double *Info)
    {
        return (trilinos_amd_order(n, Ap, Ai, P, Control, Info)) ;
    }

    static inline int colamd (int n_row, int n_col, int Alen, int *A, int *p,
        double *knobs, int *stats)
    {
        return(trilinos_colamd (n_row, n_col, Alen, A, p, knobs, stats));
    }

    // TODO : return size_t
    static inline int colamd_recommended (int nnz, int n_row, int n_col)
    {
        return(trilinos_colamd_recommended(nnz, n_row, n_col));
    }
};

template<>
struct KLU_OrdinalTraits<UF_long>
{
    static inline UF_long btf_order (UF_long n, UF_long *Ap, UF_long *Ai,
        double maxwork, double *work, UF_long *P, UF_long *Q, UF_long *R, UF_long *nmatch,
        UF_long *Work)
    {
        return (trilinos_btf_l_order (n, Ap, Ai, maxwork, work, P, Q, R, nmatch,
                    Work));
    }

    static inline UF_long btf_strongcomp (UF_long n, UF_long *Ap, UF_long *Ai, UF_long *Q,
        UF_long *P, UF_long *R, UF_long *Work)
    {
        return(trilinos_btf_l_strongcomp (n, Ap, Ai, Q, P, R, Work)) ;
    }

    static inline UF_long amd_order (UF_long n, UF_long *Ap, UF_long *Ai, UF_long *P,
        double *Control, double *Info)
    {
        return (trilinos_amd_l_order(n, Ap, Ai, P, Control, Info)) ;
    }

    static inline UF_long colamd (UF_long n_row, UF_long n_col, UF_long Alen, UF_long *A,
        UF_long *p, double *knobs, UF_long *stats)
    {
        return(trilinos_colamd_l (n_row, n_col, Alen, A, p, knobs, stats));
    }

    static inline UF_long colamd_recommended (UF_long nnz, UF_long n_row, UF_long n_col)
    {
        return(trilinos_colamd_l_recommended(nnz, n_row, n_col));
    }
};

#endif

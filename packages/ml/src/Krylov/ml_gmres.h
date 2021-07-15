/* ******************************************************************** */
/* See the file COPYRIGHT for a complete copyright notice, contact      */
/* person and disclaimer.                                               */
/* ******************************************************************** */

/* ******************************************************************** */
/* Functions for the GMRES Krylov solver                                */
/* ******************************************************************** */
/* Author        : Charles Tong (LLNL)                                  */
/* Date          : December, 1999                                       */
/* ******************************************************************** */

#ifndef __MLGMRES__
#define __MLGMRES__

#include "ml_common.h"
#include "ml_krylov.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int ML_GMRES_Solve(ML_Krylov *,int length,double *rhs,double *sol);

#ifdef __cplusplus
}
#endif
#endif


dnl @synopsis TAC_ARG_CONFIG_MPI
dnl
dnl Test a variety of MPI options:
dnl --enable-mpi       - Turns MPI compiling mode on
dnl --with-mpi         - specify root directory of MPI
dnl --with-mpi-compilers - Turns on MPI compiling mode and sets the MPI C++
dnl                       compiler = mpicxx or mpiCC (if mpicxx not available),
dnl                       the MPI C compiler = mpicc and 
dnl                       the MPI Fortran compiler = mpif77
dnl --with-mpi-incdir - specify include directory for MPI 
dnl --with-mpi-libs    - specify MPI libraries
dnl --with-mpi-libdir  - specify location of MPI libraries
dnl
dnl If any of these options are set, HAVE_MPI will be defined for both
dnl Autoconf and Automake, and HAVE_MPI will be defined in the
dnl generated config.h file
dnl
dnl
dnl @author Mike Heroux <mheroux@cs.sandia.gov>
dnl
AC_DEFUN([TAC_ARG_CONFIG_MPI],
[
AC_ARG_ENABLE(mpi,
[AC_HELP_STRING([--enable-mpi],[MPI support])],
[HAVE_PKG_MPI=$enableval],
[HAVE_PKG_MPI=no]
)

AC_ARG_WITH(mpi,
[AC_HELP_STRING([--with-mpi=MPIROOT],[use MPI root directory (enables MPI)])],
[
  HAVE_PKG_MPI=yes
  MPI_DIR=${withval}
  AC_MSG_CHECKING(MPI directory)
  AC_MSG_RESULT([${MPI_DIR}])
]
)

AC_ARG_WITH(mpi-compilers,
[AC_HELP_STRING([--with-mpi-compilers=PATH],[use MPI C++ compiler mpicxx, or mpiCC (if mpicxx not available), MPI C compiler mpicc and MPI Fortran compiler mpif77 found in PATH (PATH optional).  Enables MPI])],
[
  if test X${withval} != Xno; then
    HAVE_PKG_MPI=yes
    if test X${withval} = Xyes; then
      # Check for mpicxx, if it does not exist, use mpiCC instead.
      AC_CHECK_PROG(MPI_CXX, mpicxx, mpicxx, mpiCC)
      MPI_CC=mpicc
      MPI_F77=mpif77
    else
      MPI_TEMP_CXX=${withval}/mpicxx
      if test -f ${MPI_TEMP_CXX}; then
        MPI_CXX=${MPI_TEMP_CXX}
      else
        MPI_CXX=${withval}/mpiCC
      fi
      MPI_CC=${withval}/mpicc
      MPI_F77=${withval}/mpif77
    fi
  fi
]
)

AC_ARG_WITH(mpi-include,
[AC_HELP_STRING([--with-mpi-include],[Obsolete.  Use --with-mpi-incdir=DIR instead.  Do not prefix DIR with '-I'.])],
[AC_MSG_ERROR([--with-mpi-include is an obsolte option.   Use --with-mpi-incdir=DIR instead.  Do not prefix DIR with '-I'.  For example '--with-mpi-incdir=/usr/lam_path/include'.])]
)

AC_ARG_WITH(mpi-libs,
[AC_HELP_STRING([--with-mpi-libs="LIBS"],[MPI libraries @<:@"-lmpi"@:>@])],
[
  MPI_LIBS=${withval}
  AC_MSG_CHECKING(user-defined MPI libraries)
  AC_MSG_RESULT([${MPI_LIBS}])
]
)

AC_ARG_WITH(mpi-incdir,
[AC_HELP_STRING([--with-mpi-incdir=DIR],[MPI include directory @<:@MPIROOT/include@:>@  Do not use -I])],
[
  MPI_INC=${withval}
  AC_MSG_CHECKING(user-defined MPI includes)
  AC_MSG_RESULT([${MPI_INC}])
]
)

AC_ARG_WITH(mpi-libdir,
[AC_HELP_STRING([--with-mpi-libdir=DIR],[MPI library directory @<:@MPIROOT/lib@:>@  Do not use -L])],
[
  MPI_LIBDIR=${withval}
  AC_MSG_CHECKING(user-defined MPI library directory)
  AC_MSG_RESULT([${MPI_LIBDIR}])
]
)

AC_MSG_CHECKING(whether we are using MPI)
AC_MSG_RESULT([${HAVE_PKG_MPI}])

if test "X${HAVE_PKG_MPI}" = "Xyes"; then
   AC_DEFINE(HAVE_MPI,,[define if we want to use MPI])
fi

dnl Define Automake version of HAVE_MPI if appropriate

AM_CONDITIONAL(HAVE_MPI, [test "X${HAVE_PKG_MPI}" = "Xyes"])


dnl
dnl --------------------------------------------------------------------
dnl Check for MPI compilers (must be done *before* AC_PROG_CXX,
dnl AC_PROG_CC and AC_PROG_F77)
dnl 
dnl --------------------------------------------------------------------

if test -n "${MPI_CXX}"; then
  if test -f ${MPI_CXX}; then
    MPI_CXX_EXISTS=yes
  else
    AC_CHECK_PROG(MPI_CXX_EXISTS, ${MPI_CXX}, yes, no)
  fi

  if test "X${MPI_CXX_EXISTS}" = "Xyes"; then
    CXX=${MPI_CXX}
  else
    echo "-----"
    echo "Cannot find MPI C++ compiler ${MPI_CXX}."
    echo "Specify a path to all mpi compilers with --with-mpi-compilers=PATH"
    echo "or specify a C++ compiler using CXX="
    echo "Do not use --with-mpi-compilers if using CXX="
    echo "-----"
    AC_MSG_ERROR([MPI C++ compiler (${MPI_CXX}) not found.])
  fi
fi

if test -n "${MPI_CC}"; then
  if test -f ${MPI_CC}; then
    MPI_CC_EXISTS=yes
  else
    AC_CHECK_PROG(MPI_CC_EXISTS, ${MPI_CC}, yes, no)
  fi

  if test "X${MPI_CC_EXISTS}" = "Xyes"; then
    CC=${MPI_CC}
  else
    echo "-----"
    echo "Cannot find MPI C compiler ${MPI_CC}."
    echo "Specify a path to all mpi compilers with --with-mpi-compilers=PATH"
    echo "or specify a C compiler using CC="
    echo "Do not use --with-mpi-compilers if using CC="
    echo "-----"
    AC_MSG_ERROR([MPI C compiler (${MPI_CC}) not found.])
  fi
fi

if test -n "${MPI_F77}"; then
  if test -f ${MPI_F77}; then
    MPI_F77_EXISTS=yes
  else
    AC_CHECK_PROG(MPI_F77_EXISTS, ${MPI_F77}, yes, no)
  fi

  if test "X${MPI_F77_EXISTS}" = "Xyes"; then
    F77=${MPI_F77}
  else
    echo "-----"
    echo "Cannot find MPI Fortran compiler ${MPI_F77}."
    echo "Specify a path to all mpi compilers with --with-mpi-compilers=PATH"
    echo "or specify a Fortran 77 compiler using F77="
    echo "Do not use --with-mpi-compilers if using F77="
    echo "-----"
    AC_MSG_ERROR([MPI Fortran 77 compiler (${MPI_F77}) not found.])
  fi
fi
])

# Initialization
AC_INIT([mpi_test], [0.1], [mts@colorado.edu], [mpi_test], [httpw://www.rc.colorado.edu/])
AC_PREREQ([2.59])
AM_INIT_AUTOMAKE([1.11 foreign -Wall -Werror])

# Tell autoconf where to find additional macros
AC_CONFIG_MACRO_DIR([m4])

# We're Using C++
AC_LANG([C++])

# Required Libraries
## Need MATH's pow
AC_CHECK_LIB([m], [pow])
## Need MPI of some sort
AX_PROG_CXX_MPI([ [[ 0 ]] ], AC_MSG_NOTICE([Setting cxx to $CXX]), AC_MSG_ERROR([This applications requires an MPI.]))

AC_PROG_CXX
#AC_PROG_CC
#AC_PROG_CC_C_O

# Setup files for compilations and Make
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile])

# Finish up
AC_OUTPUT

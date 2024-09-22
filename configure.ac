
AC_INIT([SpikingC], [1.0])

AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_AUX_DIR([build-aux])

AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])

AC_ARG_ENABLE([debug],
    [AS_HELP_STRING([--enable-debug], [Compile with debugging information and sanitizers])],
    [debug=$enableval], [default=no])

AM_CONDITIONAL([DEBUG], [test x"$debug" = x"yes"])

AC_PROG_CC

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
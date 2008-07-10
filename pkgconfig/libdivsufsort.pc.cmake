prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=${prefix}
libdir=${exec_prefix}/lib
includedir=${prefix}/include

Name: @PROJECT_NAME@
Description: @PROJECT_DESCRIPTION@
Version: @PROJECT_VERSION_FULL@
URL: @PROJECT_URL@
Libs: -L${libdir} -ldivsufsort
Cflags: -I${includedir}

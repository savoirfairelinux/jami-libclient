# this one is important
SET(CMAKE_SYSTEM_NAME Windows)

# specify the cross compiler
SET(CMAKE_C_COMPILER   i686-w64-mingw32-gcc)
SET(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)
SET(CMAKE_RC_COMPILER i686-w64-mingw32-windres)
SET(CMAKE_ASM_YASM_COMPILER yasm)
SET(CMAKE_FIND_ROOT_PATH /usr/i686-w64-mingw32/)
set(LIB_FLAGS "-Wl,--output-def,libringclient.def")
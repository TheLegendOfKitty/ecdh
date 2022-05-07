cmake_minimum_required(VERSION 3.18)

project(crypt VERSION 0.1)

SET(CCFLAGS "-O2 -DNDEBUG")
SET(LDFLAGS "")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CCFLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LDFLAGS}")

find_library(CRYPTO_LIB crypto REQUIRED)
#find_library(CDK_LIB cdk REQUIRED)
#find_library(CURSES_LIB curses REQUIRED)
find_library(PANEL_LIB panel REQUIRED)
find_library(NCURSES_LIB ncurses REQUIRED)
#include_directories(src/include) #"C:\\Program Files\\OpenSSL-Win64\\include")
#replace this with something more reasonable...
#link_directories("C:\\Program Files\\OpenSSL-Win64\\lib")
#find_library(CRYPTO_LIB libcrpyto REQUIRED)

file(GLOB SOURCES "src/*.c")
file(GLOB CXXSOURCES "src/*.cpp")
add_executable(main ${CXXSOURCES} ${SOURCES})

target_link_libraries(main ${CRYPTO_LIB} ${PANEL_LIB} ${NCURSES_LIB})#"C:\\Program Files\\OpenSSL-Win64\\lib\\libcrypto.lib")
cmake_minimum_required(VERSION 3.25)
project(Revo CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_EXTENSIONS OFF)

# compiler flags
set(COMMON_CXX_FLAGS
    -freflection
    -fPIC
    -pedantic-errors
    -I${CMAKE_CURRENT_SOURCE_DIR}/source
    -include${CMAKE_CURRENT_SOURCE_DIR}/source/util/Types.hh
    -include${CMAKE_CURRENT_SOURCE_DIR}/source/util/Console.hh
    -Wall
    -Werror
    -Wextra
    -Wcast-qual
    -Wconversion
    -Wfloat-equal
    -Wformat=2
    -Wnull-dereference
    -Wold-style-cast
    -Wpessimizing-move
    -Wredundant-move
    -Wsuggest-override
    -Wundef
)

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -flto=auto -D_FORTIFY_SOURCE=3")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g -fno-omit-frame-pointer -flto=auto -D_FORTIFY_SOURCE=3")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb -fno-omit-frame-pointer")

# source files
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/source/*.cc)

set(COMMON_FILES ${SOURCE_FILES})
list(FILTER COMMON_FILES EXCLUDE REGEX ".*/main\\.cc$")
list(FILTER COMMON_FILES EXCLUDE REGEX ".*/bindings/.*\\.cc$")

set(BINDING_FILES ${SOURCE_FILES})
list(FILTER BINDING_FILES INCLUDE REGEX ".*/bindings/.*\\.cc$")

# librevo
set(LIBREVO_CXX_FLAGS
    -fno-exceptions
    -fno-rtti
)

add_library(librevo ${COMMON_FILES})
set_target_properties(librevo PROPERTIES OUTPUT_NAME revo)
target_compile_options(librevo PRIVATE ${COMMON_CXX_FLAGS} ${LIBREVO_CXX_FLAGS})

if(WIN32)
    target_link_libraries(librevo PUBLIC stdc++exp)
endif()

# revo
add_executable(revo source/main.cc)
set_target_properties(revo PROPERTIES OUTPUT_NAME Revo)
target_compile_options(revo PRIVATE ${COMMON_CXX_FLAGS})
target_link_libraries(revo PRIVATE librevo)

# bindings
option(BUILD_BINDINGS "Build Python bindings" OFF)

if(BUILD_BINDINGS)
    set(BINDINGS_CXX_FLAGS
        -fexceptions
        -frtti
    )

    find_package(Python 3.10 REQUIRED COMPONENTS Interpreter Development.Module)

    include(FetchContent)
    FetchContent_Declare(
        nanobind
        GIT_REPOSITORY https://github.com/wjakob/nanobind.git
        GIT_TAG v3.0.1
    )
    FetchContent_MakeAvailable(nanobind)

    nanobind_add_module(bindings NB_STATIC NB_SUPPRESS_WARNINGS ${BINDING_FILES})
    set_target_properties(bindings PROPERTIES OUTPUT_NAME revo)
    target_compile_options(bindings PRIVATE ${COMMON_CXX_FLAGS} ${BINDINGS_CXX_FLAGS})
    target_link_libraries(bindings PRIVATE librevo)

    if(WIN32)
        target_link_options(bindings PRIVATE -static)
    endif()
endif()
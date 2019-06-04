set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_minimum_required(VERSION 3.0.0)
project(Zelda)

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")

find_package(Boost 1.6.9 COMPONENTS system filesystem serialization regex program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

# Sets up LLVM.
if(DEFINED ENV{CLANG_VER})
	set(CLANG_VER $ENV{${CLANG_VER}})
  set(CLANG_VER 8.0.0)

    # Get the appropriate version.
    if(${CLANG_VER} VERSION_LESS 7.0.0)
          add_definitions(-DCLANG_VER_LTE)
    else()
          add_definitions(-DCLANG_VER_GT)
    endif()
else()
	message( FATAL_ERROR "The CLANG_VER environment variable is not set. Please set it before compiling Zelda." )
endif()
if(DEFINED ENV{LLVM_PATH})
	set(LLVM_PATH $ENV{LLVM_PATH})
else()
	message( FATAL_ERROR "The LLVM_PATH environment variable is not set. Please set it before compiling Zelda." )
endif()

link_directories(${LLVM_PATH}/lib)
include_directories(${LLVM_PATH}/include)

add_definitions(
        -D__STDC_LIMIT_MACROS
        -D__STDC_CONSTANT_MACROS
        -DCLANG_INCLUD_DIR="/usr/local/lib/clang/${CLANG_VER}/include"
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11")

set(SOURCE_FILES
        Graph/TAGraph.cpp
        Graph/TAGraph.h
        Graph/ZeldaNode.cpp
        Graph/ZeldaNode.h
        Graph/ZeldaEdge.cpp
        Graph/ZeldaEdge.h
        Driver/ZeldaMaster.cpp
        Driver/ZeldaHandler.cpp
        Driver/ZeldaHandler.h
        Walker/ParentWalker.cpp
        Walker/ParentWalker.h
        Walker/ExceptWalker.cpp
        Walker/ExceptWalker.h
        Walker/ZeldaWalker.cpp
        Walker/ZeldaWalker.h
        Walker/ExceptConsumer.cpp
        Walker/ExceptConsumer.h
        Walker/Classifier.cpp
        Walker/Classifier.h
        Walker/Counter.cpp
        Walker/Counter.h
    		JSON/jsoncpp.cpp
        JSON/json-forwards.h
        JSON/json.h
        Graph/MD5.cpp
        Graph/MD5.h)

add_executable(Zelda ${SOURCE_FILES})

target_link_libraries(Zelda
        clangFrontend
        clangSerialization
        clangDriver
        clangParse
        clangSema
        clangAnalysis
        clangAST
        clangASTMatchers
        clangBasic
        clangEdit
        clangLex
        clangTooling
        )

# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(llvm_libs all)

# Link against LLVM libraries
target_link_libraries(Zelda ${llvm_libs})

include(FindCurses)
target_link_libraries(Zelda
        pthread
        z
        dl
        crypto
        ${CURSES_LIBRARIES}
        )

target_link_libraries(Zelda
        ${Boost_LIBRARIES}
        )

add_custom_command(TARGET Zelda PRE_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		${LLVM_PATH}/lib/clang/${CLANG_VER}/include ${CMAKE_SOURCE_DIR}/include)
add_custom_command(TARGET Zelda POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/include $<TARGET_FILE_DIR:Zelda>/include)
add_custom_command(TARGET Zelda POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
	${CMAKE_SOURCE_DIR}/ZELDA_IGNORE.db $<TARGET_FILE_DIR:Zelda>/ZELDA_IGNORE.db)


cmake_minimum_required(VERSION 2.8)
project(CartPole)

find_package(Armadillo REQUIRED)
include_directories(${ARMADILLO_INCLUDE_DIRS})

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
	message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

set(SOURCES
	${SOURCES}
	${CMAKE_CURRENT_SOURCE_DIR}/Params.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Utility.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Layer.cpp
	#${CMAKE_CURRENT_SOURCE_DIR}/Net.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)
set(HEADERS
	${HEADERS}
	${CMAKE_CURRENT_SOURCE_DIR}/Params.h
	${CMAKE_CURRENT_SOURCE_DIR}/Utility.h
	${CMAKE_CURRENT_SOURCE_DIR}/Layer.h
	${CMAKE_CURRENT_SOURCE_DIR}/Net.h
	${CMAKE_CURRENT_SOURCE_DIR}/Agent.h
	${CMAKE_CURRENT_SOURCE_DIR}/GameManager.h
)

add_executable(CartPole ${SOURCES} ${HEADERS})

target_link_libraries(CartPole ${ARMADILLO_LIBRARIES})	

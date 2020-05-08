cmake_minimum_required(VERSION 3.13.0)
project(ocore)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_SUPPRESS_REGENERATION true)

file(GLOB_RECURSE ocoreHpp "include/*.hpp")
file(GLOB_RECURSE ocoreCpp "src/*.cpp")

if(WIN32)
	set(platform windows CACHE STRING "Platform")
else()
	set(platform linux CACHE STRING "Platform")
endif()

set(platforms windows linux osx android ios web)
set_property(CACHE platform PROPERTY STRINGS ${platforms})

message("-- Targetting platform ${platform}")

file(GLOB_RECURSE platformHpp "platform/${platform}/include/*.hpp")
file(GLOB_RECURSE platformCpp "platform/${platform}/src/*.cpp")

add_library(
	ocore
	STATIC
	${ocoreHpp}
	${ocoreCpp}
	${platformHpp}
	${platformCpp}
	CMakeLists.txt
)

target_include_directories(ocore PRIVATE include)
target_include_directories(ocore PRIVATE platform/${platform}/include)

source_group("Header Files" FILES ${ocoreHpp})
source_group("Source Files" FILES ${ocoreCpp})
source_group("Platform Header Files" FILES ${platformHpp})
source_group("Platform Source Files" FILES ${platformCpp})

if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
	set(ARM true)
else()
	add_definitions(-D__SSE__)
	add_definitions(-D__SSE2__)
	add_definitions(-D__SSE3__)
	add_definitions(-D__SSSE3__)
	add_definitions(-D__SSE4_1__)
endif()

if(MSVC)

    target_compile_options(ocore PRIVATE /W4 /WX /MD /MP /wd4201 /EHsc /GR)

	if(NOT ${ARM})
		target_compile_options(ocore PUBLIC /arch:SSE4.1)
	endif()

else()

    target_compile_options(ocore PRIVATE -Wall -Wextra -Werror -fms-extensions)

	if(NOT ${ARM})
		target_compile_options(ocore PUBLIC -msse4.1)
	endif()

endif()

# Ways to add virtual files

set_property(GLOBAL PROPERTY virtualFiles "")

include(CMakeParseArguments)

macro(add_virtual_files)

	set(_OPTIONS)
    set(_ONE_VALUE DIRECTORY NAME)
    set(_MULTI_VALUE FILES)

    cmake_parse_arguments(_VFILES "${_OPTIONS}" "${_ONE_VALUE}" "${_MULTI_VALUE}" ${ARGN})

	get_property(vfileList GLOBAL PROPERTY virtualFiles)

	if(_VFILES_FILES)
		foreach(file ${_VFILES_FILES})
			set(vfileList ${vfileList}\ \"${_VFILES_DIRECTORY}/${file}\"|\"${_VFILES_NAME}/${file}\")
		endforeach()
    else()
        message(FATAL_ERROR "FindLibs: 'NAMES' argument required.")
    endif()
	
	set_property(GLOBAL PROPERTY virtualFiles ${vfileList})

endmacro()

add_virtual_files(
	DIRECTORY
		${CMAKE_CURRENT_SOURCE_DIR}/../res
	NAME
		core2
	FILES
		shaders/test.frag
		shaders/test.vert
)

get_property(vfileList GLOBAL PROPERTY virtualFiles)
message(Testing: ${vfileList})
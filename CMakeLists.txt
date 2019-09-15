cmake_minimum_required(VERSION 3.13.0)
project(ocore)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_SUPPRESS_REGENERATION true)

file(GLOB_RECURSE ocoreHpp "include/*.hpp")
file(GLOB_RECURSE ocoreCpp "src/*.cpp")

set(platform windows CACHE STRING "Platform")
set(platforms windows linux osx android ios web)
set_property(CACHE platform PROPERTY STRINGS ${platforms})

if(WIN32)
	set(platform windows)
endif()

file(GLOB_RECURSE platformHpp "platform/${platform}/include/*.hpp")
file(GLOB_RECURSE platformCpp "platform/${platform}/src/*.cpp")

include_directories(include)
include_directories(platform/${platform}/include)

add_library(
	ocore
	STATIC
	${ocoreHpp}
	${ocoreCpp}
	${platformHpp}
	${platformCpp}
	CMakeLists.txt
)

source_group("Header Files" FILES ${ocoreHpp})
source_group("Source Files" FILES ${ocoreCpp})
source_group("Platform Header Files" FILES ${platformHpp})
source_group("Platform Source Files" FILES ${platformCpp})

if(MSVC)
    target_compile_options(ocore PRIVATE /W4 /WX /MD /MP)
else()
    target_compile_options(ocore PRIVATE -Wall -Wextra -pedantic -Werror)
endif()
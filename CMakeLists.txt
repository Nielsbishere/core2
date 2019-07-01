cmake_minimum_required(VERSION 3.13.0)
project(ocore)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_SUPPRESS_REGENERATION true)

include_directories(include)

file(GLOB_RECURSE ocoreHpp
	"include/*.hpp"
)

file(GLOB_RECURSE ocoreCpp
	"src/*.cpp"
)

if(WIN32)

	include_directories(platform/windows/include)
	
	file(GLOB_RECURSE platformHpp
		"platform/windows/include/*.hpp"
	)

	file(GLOB_RECURSE platformCpp
		"platform/windows/src/*.cpp"
	)

endif()


add_executable(
	ocore
	${ocoreHpp}
	${ocoreCpp}
	${platformHpp}
	${platformCpp}
)

source_group("Header Files" FILES ${ocoreHpp})
source_group("Source Files" FILES ${ocoreCpp})

if(WIN32)
	source_group("Platform Header Files" FILES ${platformHpp})
	source_group("Platform Source Files" FILES ${platformCpp})
endif()

if(MSVC)
    target_compile_options(ocore PRIVATE /W4 /WX /MD /MP)
else()
    target_compile_options(ocore PRIVATE -Wall -Wextra -pedantic -Werror)
endif()
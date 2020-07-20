cmake_minimum_required(VERSION 3.13.0)
project(ocore)

add_definitions(-D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

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

message("-- Targeting platform ${platform}")

# TODO: On lost focus just reset all input

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
endif()

if(MSVC)
    target_compile_options(ocore PRIVATE /W4 /WX /MD /MP /wd4201 /EHsc /GR)
else()
    target_compile_options(ocore PRIVATE -Wall -Wextra -Werror -fms-extensions)
endif()

if(NOT ${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
	if(MSVC)
	    target_compile_options(ocore PUBLIC /arch:AVX2)
	else()
	    target_compile_options(ocore PUBLIC -mavx2)
	endif()
endif()

# Ways to add virtual files

set_property(GLOBAL PROPERTY virtualFiles "")

include(CMakeParseArguments)

# Add virtual files into the executable
# Example:
# addVirtualFiles(
#	DIRECTORY
#			${CMAKE_CURRENT_SOURCE_DIR}/res/test_shaders
#		NAME
#			shaders
#		FILES
#			${shaderTestBinaries}
# )

macro(add_virtual_files)

	set(_OPTIONS)
    set(_ONE_VALUE DIRECTORY NAME)
    set(_MULTI_VALUE FILES)

    cmake_parse_arguments(_VFILES "${_OPTIONS}" "${_ONE_VALUE}" "${_MULTI_VALUE}" ${ARGN})

	string(LENGTH "${_VFILES_DIRECTORY}/" dirLen)

	get_property(vfileList GLOBAL PROPERTY virtualFileDescription)
	get_property(vfileRoot GLOBAL PROPERTY virtualFileRoot)
	get_property(fileCount GLOBAL PROPERTY virtualFileCounter)

	if("${fileCount}" STREQUAL "")
		set(fileCount 1)
	endif()

	if(_VFILES_FILES)
		foreach(file ${_VFILES_FILES})
			
			string(FIND "${file}" "${_VFILES_DIRECTORY}/" rootIndex)

			if(rootIndex EQUAL 0)
				string(SUBSTRING "${file}" ${dirLen} -1 file)
			endif()

			# For windows, add file to RES file 

			if(WIN32)

				# Split by / and make sure all subdirs exist

				set(fullName "${_VFILES_NAME}/${file}")
				set(currentIndex "~/")

				set(parent 0)

				set(currPath ${fullName})
				string(FIND ${currPath} / subPath)
				set(offset ${subPath})


				while(${subPath} GREATER -1)
				
					MATH(EXPR subPath "${subPath}+1")
					string(SUBSTRING ${currPath} ${subPath} -1 currPath)

					string(SUBSTRING ${fullName} 0 ${offset} current)

					# TODO: Hash ~/${current} for release and use it normally otherwise

					# Add folder if required
						
						set(currentNoEscape ${current})
						string(REPLACE / \\/ current "${current}" )
						set(doMakeFolder 1)

						if(NOT vfileRoot STREQUAL "")

							# Grab the parent id if it exists

							string(REGEX MATCH "[0-9]+ [0-9]+ ~\\/${current}\n" fileMatch "${vfileRoot}")

							if(NOT fileMatch STREQUAL "")

								set(doMakeFolder 0)
								string(REGEX MATCH "[0-9]+" parentMatch "${fileMatch}")
								
								set(parent ${parentMatch})

							endif()

						endif()

						if(doMakeFolder)

							# Create new file if doesn't exist

							set(vfileRoot ${vfileRoot}${fileCount}\ ${parent}\ ~/${currentNoEscape}\n)
							
							set(parent ${fileCount})
							MATH(EXPR fileCount "${fileCount}+1")

						endif()

					# Find next subpath 
					
					string(FIND ${currPath} / subPath)
					MATH(EXPR offset "${offset}+${subPath}+1")

				endwhile()

				# Add to file dir

				set(vfileList ${vfileList}_${fileCount}\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ RCDATA\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \"${_VFILES_DIRECTORY}/${file}\"\n)

				set(vfileRoot ${vfileRoot}${fileCount}\ ${parent}\ ~/${_VFILES_NAME}/${file}|\n)

				MATH(EXPR fileCount "${fileCount}+1")

			else()
				message(FATAL_ERROR "Unsupported call to add_virtual_files")
			endif()

		endforeach()
    else()
        message(FATAL_ERROR "add_virtual_files: 'FILES' argument required.")
    endif()
	
	set_property(GLOBAL PROPERTY virtualFileDescription ${vfileList})
	set_property(GLOBAL PROPERTY virtualFileRoot ${vfileRoot})
	set_property(GLOBAL PROPERTY virtualFileCounter ${fileCount})

endmacro()

# Setting the icon of the app
# Call this immediately before configure_virtual_files with the executable

function(configure_icon target icon)
	set_property(GLOBAL PROPERTY CURRENT_EXECUTABLE_ICON ${icon})
	set_property(GLOBAL PROPERTY CURRENT_EXECUTABLE_ICON_TARGET ${target})
endfunction()

# Configure virtual files for a target

function(configure_virtual_files target)

	get_property(vfileList GLOBAL PROPERTY virtualFileDescription)
	get_property(vfileRoot GLOBAL PROPERTY virtualFileRoot)
	get_property(iconTarget GLOBAL PROPERTY CURRENT_EXECUTABLE_ICON_TARGET)
	get_property(icon GLOBAL PROPERTY CURRENT_EXECUTABLE_ICON)

	if(WIN32)

		# Configure icon for this target, if igx_test's the specified target
	
		if("${iconTarget}" STREQUAL "${target}")
			set(vfileList LOGO\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ ICON\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \"${icon}\"\n${vfileList})
			target_sources(${target} PRIVATE ${icon})
		endif()

		# Prepare root file 

		file(WRITE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${target}.root" ${vfileRoot})

		set(vfileList ${vfileList}0\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ RCDATA\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${target}.root\"\r\n)

		file(WRITE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${target}.rc" ${vfileList})
		target_sources(${target} PRIVATE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${target}.rc")

	endif()
	
endfunction()
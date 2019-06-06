#!/bin/bash

# Color constants

RED="\033[0;31m"
GREEN="\033[0;32m"
BLUE="\033[0;34m"
CYAN="\033[0;36m"
NC="\033[0m"

# Information about the command

function doHelp {
	echo -e "${GREEN}Command line arguments"
	echo -e "-name=${BLUE}x ${CYAN}The name of the environment (case sensitive, underscores instead of spaces)"
	echo -e "${GREEN}-type=${BLUE}x ${CYAN}The type of the environment (${BLUE}Android, Windows, Linux, OSX, Web${CYAN})"
	echo -e "${GREEN}-arch=${BLUE}x ${CYAN}The architecture type (${BLUE}ARM32, ARM64, x86, x64, all, 64_bit${CYAN}) can use multiple unless it's ${BLUE}all${CYAN} or ${BLUE}64_bit${CYAN}. Unused for web"
	echo -e "${GREEN}-bake=${BLUE}only ${CYAN}How to use the baker (${BLUE}only, on, off${CYAN}); ${BLUE}only${CYAN}(exclude non-baked files), ${BLUE}on${CYAN}(includes both baked and non-baked files), ${BLUE}off${CYAN}(leave all resources as-is)"
	echo -e "${GREEN}-log=${BLUE}file ${CYAN}How log calls are handled; ${BLUE}console, file, off"
	echo -e "${GREEN}--debug ${CYAN}Enable debug mode; defaulting ${BLUE}bake=on${CYAN}, ${BLUE}log=console${CYAN} and optimizations to be limited for debugging purposes"
	echo -e "${GREEN}--help ${CYAN}Will show all command line arguments. If you specify the type, it will give you specific info about that environment type"
	echo -e "${GREEN}--f<x> ${CYAN}All tags starting with 'f' are passed along to CMake and can be used by the end-user"
	echo 
	echo -e "${GREEN}Building"
	# TODO:
}

# Function for creating a new environment

function newEnviroment {

	# env name

	echo -e "${GREEN}What is the environment name? (-name)${NC}"
	
	read envName
	envName="${envName/ /_}"
	envName=$(echo ${envName} | tr \' \")
	
	echo
	
	if [ -z "${envName// }" ] ; then
		echo -e "${RED}Invalid name${NC}"
		exit 1
	fi
	
	# env type
	
	echo -e "${GREEN}What type of environment is ${envName}? (-type)"
	echo -e "${CYAN}Windows"
	echo -e "Android"
	echo -e "Linux"
	echo -e "OSX"
	echo -e "Web${NC}"
	
	read envType
	envType="${envType,,}"
	
	echo
	
	if ! ( [ "${envType}" = "web" ] || [ "${envType}" = "android" ] || [ "${envType}" = "windows" ]|| [ "${envType}" = "linux" ]|| [ "${envType}" = "osx" ] ) ; then
		echo -e "${RED}Invalid type${NC}"
		exit 1
	fi
	
	# debug
	
	echo -e "${GREEN}Enable debug mode (${CYAN}y${GREEN}/${CYAN}n${GREEN})? (--debug)${NC}"
	
	read debug
	debug="${debug,,}"
	
	if ! ( [ "${debug}" = "y" ] || [ "${debug}" = "n" ] ) ; then
		debug="n"
	fi
	
	echo
	
	# arch
	
	if ! [ "${envType}" = "web" ] ; then

		echo -e "${GREEN}What architectures would you like to build for? (-arch)"
		echo -e "${CYAN}x64"
		echo -e "ARM64"
		echo -e "x86"
		echo -e "ARM32"
		echo -e "64_bit"
		echo -e "all${NC}"
		
		read arch
		arch="${arch,,}"
		arch="${arch/ /_}"
		echo
		
		if [ "${arch}" = "all" ] ; then
		
			arch="x86,x64,ARM64,ARM32"
		
		elif [ "${arch}" = "64_bit" ] ; then
		
			arch="x64,ARM64"
		
		# else 
		
			#TODO: validate
		
		fi

		if [ "${envType}" = "android" ] ; then

			# TODO: Ask android api level

		fi
		
	else
	
		arch="web"
	
	fi

	# TODO: Pass --f<x> along to CMake
	
	echo "<?xml version='1.0' encoding='UTF-8'?>" > ".env"
	echo "<env name='$envName' type='$envType' debug='$debug' arch='$arch' />" >> ".env"

}

# Picking from an existing environment



function pickEnvironment(){

	file=".env"
	index=0

	while IFS= read line
	do

		if [[ $index > 0 ]] ; then
			echo "$line"
		fi

		index=$((index+1))

	done <$file


}


# Setting up environment by prompt
if [ "$#" -eq 0 ] ; then
	
	if [ -f ".env" ] ; then
		pickEnvironment
	else
		newEnviroment
	fi
	
else

	if [[ "$*" == *"--help"* ]] ; then
		doHelp
		exit 0
	fi

# TODO: Setting up environment with arguments
	
fi

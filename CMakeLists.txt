cmake_minimum_required(VERSION 3.10)

project(PPLaserRemote)

set (CMAKE_CXX_STANDARD 14)

################ MSVC #################

if (MSVC)
    # For multithreaded building
    add_compile_options(/MP)
endif (MSVC)

#######################################

add_subdirectory(src)


############ Installation #############

# Automatically install bin/msvcp140.dll and bin/vcruntime140.dll in ${CMAKE_INSTALL_PREFIX}
include(InstallRequiredSystemLibraries)

# Configuration of Inno Setup files
if(WIN32)
find_program(ISS_COMPILER NAMES iscc ISCC HINTS "C:/Program Files (x86)/Inno Setup 5" "C:/Program Files/Inno Setup 5")
if(NOT ISS_COMPILER)
message(WARNING "Inno Setup Compiler not found. You won't be able to build setup files.")
else()
message(STATUS "Using Inno Setup Compiler from: ${ISS_COMPILER}")

# Add installer target
add_custom_target(InnoSetup COMMAND ${ISS_COMPILER} "${CMAKE_CURRENT_SOURCE_DIR}/install.iss" VERBATIM)
endif()
endif() 

#######################################
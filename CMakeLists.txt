# Force CMake to use Visual Studio with Win32
cmake_minimum_required(VERSION 3.16)
project(aot)

# Set platform before project declaration
if(WIN32)
    if(NOT CMAKE_GENERATOR MATCHES "Visual Studio")
        set(CMAKE_GENERATOR "Visual Studio 16 2019" CACHE INTERNAL "" FORCE)
    endif()
    
    # Architecture settings
    if(NOT CMAKE_GENERATOR_PLATFORM)
        set(CMAKE_GENERATOR_PLATFORM "Win32" CACHE INTERNAL "" FORCE)
    endif()
    
    set(CMAKE_VS_PLATFORM_NAME "Win32" CACHE INTERNAL "" FORCE)
    set(CMAKE_VS_PLATFORM_TOOLSET "v142" CACHE INTERNAL "" FORCE)
    set(CMAKE_SYSTEM_NAME "Windows" CACHE INTERNAL "" FORCE)
    set(CMAKE_SYSTEM_VERSION "10.0" CACHE INTERNAL "" FORCE)
endif()

# Force Win32 for external dependencies
set(CMAKE_LIBRARY_ARCHITECTURE "Win32")
set(CMAKE_SIZEOF_VOID_P 4)


# Set binary output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Add binary directory to PATH for custom commands
if(WIN32)
    set(MORPH_GEN_PATH "${CMAKE_BINARY_DIR}/bin/Debug/morph_gen.exe")
    set(STAT_DAT_BIN_PATH "${CMAKE_BINARY_DIR}/bin/Debug/StatDatBin.exe")
    set(WORD_FREQ_BIN_PATH "${CMAKE_BINARY_DIR}/bin/Debug/word_freq_bin.exe")
else()
    set(MORPH_GEN_PATH "${CMAKE_BINARY_DIR}/bin/morph_gen")
    set(STAT_DAT_BIN_PATH "${CMAKE_BINARY_DIR}/bin/StatDatBin")
    set(WORD_FREQ_BIN_PATH "${CMAKE_BINARY_DIR}/bin/word_freq_bin")
endif()

# Macro for running morph_gen
macro(run_morph_gen language)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E env "PATH=${CMAKE_BINARY_DIR}/bin/Debug;$ENV{PATH}"
        ${MORPH_GEN_PATH} --language ${language}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE result
    )
    if(result)
        message(FATAL_ERROR "morph_gen failed with exit code: ${result}")
    endif()
endmacro()

# Set environment for custom commands
set(CMAKE_COMMAND ${CMAKE_COMMAND} -E env "PATH=${CMAKE_BINARY_DIR}/bin/$<CONFIG>;$ENV{PATH}")

set(CMAKE_LEGACY_CYGWIN_WIN32 0)

if(WIN32)
    # Visual Studio specific settings
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
    
    # Enable MFC for Windows GUI
    set(CMAKE_MFC_FLAG 2)
    add_definitions(-D_AFXDLL)
    
    # Force Win32 build
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

macro (declare_cmake_min_version)
    cmake_minimum_required(VERSION 3.16)
endmacro(declare_cmake_min_version)

declare_cmake_min_version()

set(BUILD_DICTS 0)  # build dicts by default, disable it in CMakeLists.my if you want
set(BUILD_COM 0)    # do not build windows-objects by default
set(BUILD_HTTP_SERVERS 0) # do not build http-servers by default  (you need to make libevent to build it)
set(BUILD_WINDOWS_GUI 1) # Enable Windows GUI build for VisualSynan
set(DICTS_FOLDER  ${PROJECT_SOURCE_DIR}/Dicts )

if (EXISTS  ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.my)
    message ("use " ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.my)
    include (${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.my)
endif()


set (RML $ENV{RML})
#set (INSTALL_BIN_FOLDER $ENV{RML}/Bin)
#set (RML ${CMAKE_CURRENT_SOURCE_DIR})
message ("RML=" ${RML})


set (INSTALL_BIN_FOLDER /Bin) # not "Bin", but "/Bin"

if (NOT RML)
    message (FATAL_ERROR "environment variable RML is not set")
endif (NOT RML)
message ("use environment variable RML = " ${RML} )

macro (ConvertStructDict name)
    add_custom_target(
        ${name}
        SOURCES ${PROJECT_SOURCE_DIR}/units.bin
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    )

    add_custom_command(
        OUTPUT ${PROJECT_SOURCE_DIR}/units.bin ${PROJECT_SOURCE_DIR}/cortege.bin ${PROJECT_SOURCE_DIR}/comments.bin
        COMMAND StructDictLoader --action from_txt --input-file ross.txt  --output-ross-folder .
        DEPENDS StructDictLoader ross.txt fields.json domains.json
        COMMENT "Convert StructDict for ${name} to the binary format"
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        VERBATIM
    )

endmacro(ConvertStructDict)

enable_testing()

add_subdirectory (Source)

if (BUILD_DICTS)
    message ("build dicts...")
    add_subdirectory (Dicts)
    add_subdirectory (Thes)
endif()



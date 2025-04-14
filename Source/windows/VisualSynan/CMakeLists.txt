cmake_minimum_required(VERSION 3.15)

# Enable solution folders for Visual Studio
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Force Visual Studio to use CMake
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)

declare_cmake_min_version()

project(VisualSynan)

if(NOT BUILD_WINDOWS_GUI)
    return()
endif()

# Найти путь к RC компилятору
find_program(RC_COMPILER rc HINTS "${CMAKE_CXX_COMPILER}/../" "${CMAKE_CXX_COMPILER}/../../bin/x64" "${CMAKE_CXX_COMPILER}/../../bin/x86")
if(NOT RC_COMPILER)
    message(FATAL_ERROR "Resource compiler not found")
endif()

# Найти путь к ATLMFC включаемым файлам и Windows SDK
set(ATLMFC_INCLUDE_DIR "")
set(WIN_SDK_INCLUDE_DIR "")

if(MSVC)
    # Попробуем найти atlmfc/include в нескольких стандартных местах
    if(EXISTS "$ENV{VCToolsInstallDir}/atlmfc/include")
        set(ATLMFC_INCLUDE_DIR "$ENV{VCToolsInstallDir}/atlmfc/include")
    elseif(EXISTS "$ENV{VCINSTALLDIR}/atlmfc/include")
        set(ATLMFC_INCLUDE_DIR "$ENV{VCINSTALLDIR}/atlmfc/include")
    else()
        # Попробуем найти atlmfc по пути компилятора
        get_filename_component(CL_PATH ${CMAKE_CXX_COMPILER} PATH)
        get_filename_component(MSVC_PATH ${CL_PATH} PATH)
        if(EXISTS "${MSVC_PATH}/atlmfc/include")
            set(ATLMFC_INCLUDE_DIR "${MSVC_PATH}/atlmfc/include")
        endif()
    endif()

    # Найти Windows SDK include директорию
    if(EXISTS "$ENV{WindowsSdkDir}/Include/$ENV{WindowsSDKVersion}")
        set(WIN_SDK_INCLUDE_DIR "$ENV{WindowsSdkDir}/Include/$ENV{WindowsSDKVersion}")
    else()
        # Поиск во всех возможных версиях Windows SDK 10
        file(GLOB WIN_SDK_VERSIONS "C:/Program Files (x86)/Windows Kits/10/Include/10.*")
        if(WIN_SDK_VERSIONS)
            # Берем последнюю версию из найденных
            list(SORT WIN_SDK_VERSIONS)
            list(REVERSE WIN_SDK_VERSIONS)
            list(GET WIN_SDK_VERSIONS 0 WIN_SDK_INCLUDE_DIR)
            message(STATUS "Found Windows SDK by search: ${WIN_SDK_INCLUDE_DIR}")
        else()
            # Ищем в стандартных местах
            foreach(SDK_VER "10.0.22621.0" "10.0.22000.0" "10.0.20348.0" "10.0.19041.0" "10.0.18362.0" "10.0.17763.0")
                if(EXISTS "C:/Program Files (x86)/Windows Kits/10/Include/${SDK_VER}")
                    set(WIN_SDK_INCLUDE_DIR "C:/Program Files (x86)/Windows Kits/10/Include/${SDK_VER}")
                    break()
                endif()
            endforeach()
        endif()
    endif()

    if(ATLMFC_INCLUDE_DIR STREQUAL "")
        message(WARNING "ATLMFC include directory not found. Resource compilation may fail.")
    else()
        message(STATUS "Found ATLMFC include directory: ${ATLMFC_INCLUDE_DIR}")
    endif()

    if(WIN_SDK_INCLUDE_DIR STREQUAL "")
        message(WARNING "Windows SDK include directory not found. Resource compilation may fail.")
    else()
        message(STATUS "Found Windows SDK include directory: ${WIN_SDK_INCLUDE_DIR}")
    endif()
endif()

if(WIN32)
    # MFC settings
    set(CMAKE_MFC_FLAG 2)
    add_definitions(-D_AFXDLL)
    
    # Windows specific definitions
    add_definitions(
        -D_WINDOWS
        -D_UNICODE
        -DUNICODE
        -D_WIN32_WINNT=0x0A00
        -DWINVER=0x0A00
    )
endif()

# Явно перечисляем ресурсные файлы для включения в сборку
set(RESOURCE_FILES
    VisualSynan.rc
    res/VisualSynan.ico
    res/VisualSynanDoc.ico
    res/Toolbar.bmp
    res/Synt_1.bmp
    Resource.h
)

set(SOURCES
    ChildFrm.cpp
    DlgHom.cpp
    MainFrm.cpp
    ReportDoc.cpp
    SentenceLine.cpp
    Splash.cpp
    StdAfx.cpp
    SynReport.cpp
    VisualClause.cpp
    VisualGroup.cpp
    VisualGroups.cpp
    VisualSentence.cpp
    VisualSentences.cpp
    VisualSynan.cpp
    VisualSynanDoc.cpp
    VisualSynanView.cpp
    VisualWord.cpp
    WaitDlg.cpp
    WaitThread.cpp
)

set(HEADERS
    ChildFrm.h
    DlgHom.h
    MainFrm.h
    ReportDoc.h
    SentenceLine.h
    Splash.h
    StdAfx.h
    SynReport.h
    SynanWrapper.h
    VisualSentences.h
    VisualSynan.h
    VisualSynanDoc.h
    VisualSynanView.h
    WaitDlg.h
    WaitThread.h
)

# Создаем исполняемый файл, включая ресурсы напрямую
add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS} ${RESOURCE_FILES})

# Ensure proper resource inclusion for both x86 and x64
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    # x64 specific settings
    add_definitions(-D_WIN64)
endif()

# Set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8 /MP /EHsc")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd /Od /Zi /RTC1")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD /O2 /Zi")

# Set linker flags for proper entry point и использование ресурсов
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:wWinMainCRTStartup")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /DEBUG")

target_link_libraries(${PROJECT_NAME} 
    SynCommonLib
    SynanLib
)

# Set Visual Studio specific properties for the target
set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "MFCProj"
    VS_PLATFORM_TOOLSET "v142"
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$(Configuration)"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# Включаем обработку ресурсов в Visual Studio
set_target_properties(${PROJECT_NAME} PROPERTIES VS_RESOURCE_INCLUDE_DIRECTORIES
    "${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_SOURCE_DIR}/Source"
)

# Set source groups for better organization in Visual Studio
source_group("Source Files" FILES ${SOURCES})
source_group("Header Files" FILES ${HEADERS})
source_group("Resource Files" FILES ${RESOURCE_FILES})

if (BUILD_DICTS)
    add_dependencies(${PROJECT_NAME} MorphDicts)
endif()

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
    )
endif()

# Явно перечисляем ресурсные файлы, чтобы убедиться, что они включены в сборку
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

add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS} ${RESOURCE_FILES})

# Set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8 /MP /EHsc")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd /Od /Zi /RTC1")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD /O2 /Zi")

# Set linker flags
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /DEBUG")

target_link_libraries(${PROJECT_NAME} 
    SynCommonLib
    SynanLib
)

# Set Visual Studio specific properties for the target
set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
    VS_PLATFORM_TOOLSET "v142"
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$(Configuration)"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# Set source groups for better organization in Visual Studio
source_group("Source Files" FILES ${SOURCES})
source_group("Header Files" FILES ${HEADERS})
source_group("Resource Files" FILES ${RESOURCE_FILES})

if (BUILD_DICTS)
    add_dependencies(${PROJECT_NAME} MorphDicts)
endif()

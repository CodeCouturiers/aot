declare_cmake_min_version()

project(StructDictLoader)

add_executable (${PROJECT_NAME}  "Main.cpp")

# Override global GUI settings for this console application
set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE FALSE
    LINK_FLAGS "/SUBSYSTEM:CONSOLE /ENTRY:mainCRTStartup"
)

target_link_libraries(${PROJECT_NAME} 
    StructDictLib
)

add_subdirectory(tests)

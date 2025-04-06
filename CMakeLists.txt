cmake_minimum_required(VERSION 3.12)
project(CRemovePlugin)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Nuke's include directories and libraries
find_path(NUKE_INCLUDE_DIR DDImage/NoIop.h
    PATHS
    ${NUKE_PATH}/include
    /usr/local/Nuke*/include
    "C:/Program Files/Nuke*/include"
    DOC "Path to Nuke include directory"
)

# Create the plugin library
add_library(CRemove SHARED CRemove.cpp)

# Set compiler flags
if(MSVC)
    target_compile_options(CRemove PRIVATE /W4 /EHsc)
else()
    target_compile_options(CRemove PRIVATE -Wall -Wextra -fPIC)
endif()

# Set include directories
target_include_directories(CRemove PRIVATE ${NUKE_INCLUDE_DIR})

# Set linker options
if(APPLE)
    set_target_properties(CRemove PROPERTIES
        LINK_FLAGS "-undefined dynamic_lookup"
        SUFFIX ".dylib"
    )
elseif(UNIX)
    set_target_properties(CRemove PROPERTIES
        SUFFIX ".so"
    )
elseif(WIN32)
    set_target_properties(CRemove PROPERTIES
        SUFFIX ".dll"
    )
endif()

# Install the plugin
install(TARGETS CRemove
    LIBRARY DESTINATION ${NUKE_PLUGIN_PATH}
    RUNTIME DESTINATION ${NUKE_PLUGIN_PATH}
    ARCHIVE DESTINATION lib
)

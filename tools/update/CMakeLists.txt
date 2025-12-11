cmake_minimum_required(VERSION 3.20)

option(NEKO_UPDATE_STATIC_LINK "Build NekoLc update tool with static linkage" ON)

if(NEKO_UPDATE_STATIC_LINK)
    set(NEKO_FUNCTION_STATIC_LINK ON CACHE BOOL "Static link NekoFunction for updater" FORCE)
endif()

add_executable(NekoLcUpdateTool main.cpp)
target_compile_features(NekoLcUpdateTool PRIVATE cxx_std_20)
target_link_libraries(NekoLcUpdateTool PRIVATE Neko::Function Neko::Schema)

set_target_properties(NekoLcUpdateTool PROPERTIES
    LINK_SEARCH_START_STATIC TRUE
    LINK_SEARCH_END_STATIC TRUE
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tools/update"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tools/update"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tools/update"
)

if (WIN32)
    set_property(TARGET NekoLcUpdateTool PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
    target_link_options(NekoLcUpdateTool PRIVATE -static)
endif()

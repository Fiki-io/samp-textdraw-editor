cmake_minimum_required(VERSION 3.22.1)

project("textdraw_editor")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optimization and Anti-Reverse Engineering Hardening Flags
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -fvisibility=hidden -fvisibility-inlines-hidden -ffunction-sections -fdata-sections -DNDEBUG")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -Wl,--gc-sections -Wl,-s")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/engine
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
)

set(IMGUI_SOURCES
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_impl_opengl3.cpp
)

set(ENGINE_SOURCES
    engine/TextDrawManager.cpp
    engine/Viewport.cpp
    engine/AssetManager.cpp
    engine/DffRenderer.cpp
    engine/PawnExporter.cpp
    ui/EditorUI.cpp
    utils/SampColorParser.cpp
)

add_library(textdraw_editor SHARED
    native-lib.cpp
    ${ENGINE_SOURCES}
    ${IMGUI_SOURCES}
)

find_library(log-lib log)
find_library(android-lib android)
find_library(gles3-lib GLESv3)
find_library(egl-lib EGL)
find_library(z-lib z)

target_link_libraries(textdraw_editor
    ${log-lib}
    ${android-lib}
    ${gles3-lib}
    ${egl-lib}
    ${z-lib}
)

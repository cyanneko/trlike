cmake_minimum_required(VERSION 3.20)
project(TRlike VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 编译选项
if(MSVC)
    add_compile_options(/W4 /utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ============================================================
# SFML — FetchContent 自动下载 (首次构建需下载，约10-20MB)
# ============================================================
include(FetchContent)
set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.2
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(SFML)

# 源文件
set(SOURCES
    src/main.cpp
    # Core
    src/Core/Game.cpp
    # World
    src/World/Chunk.cpp
    src/World/World.cpp
    src/World/WorldGenerator.cpp
    src/World/TileRegistry.cpp
    # Graphics
    src/Graphics/Camera.cpp
    src/Graphics/TileRenderer.cpp
    src/Graphics/TextureAtlas.cpp
)

set(HEADERS
    # Core
    src/Core/Types.hpp
    src/Core/Constants.hpp
    src/Core/Game.hpp
    src/Core/Math.hpp
    # World
    src/World/Tile.hpp
    src/World/TileRegistry.hpp
    src/World/Chunk.hpp
    src/World/World.hpp
    src/World/WorldGenerator.hpp
    # Graphics
    src/Graphics/Camera.hpp
    src/Graphics/TileRenderer.hpp
    src/Graphics/TextureAtlas.hpp
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE src)

target_link_libraries(${PROJECT_NAME} PRIVATE sfml-graphics sfml-window sfml-system)

# ============================================================
# 可选的单元测试 (Phase 2)
# ============================================================
# enable_testing()
# add_subdirectory(tests)

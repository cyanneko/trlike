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

# 查找 SFML (后续渲染阶段需要)
# find_package(SFML 2.6 COMPONENTS graphics window system QUIET)

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
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE src)

# 后续链接 SFML
# target_link_libraries(${PROJECT_NAME} PRIVATE sfml-graphics sfml-window sfml-system)

# ============================================================
# 可选的单元测试 (Phase 2)
# ============================================================
# enable_testing()
# add_subdirectory(tests)

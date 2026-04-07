cmake_minimum_required(VERSION 3.20)

project(StockyBoy LANGUAGES CXX)

# C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
    # Ensure MinGW threading support for dynamic runtime
    add_compile_options(-mthreads)
endif()

# ====================
# Fetch external dependencies
# ====================
include(FetchContent)

# ImPlot
set(FETCHCONTENT_SOURCE_DIR_IMPLOT "" CACHE PATH "" FORCE)
FetchContent_Declare(
    implot
    GIT_REPOSITORY https://github.com/epezent/implot.git
    GIT_TAG master
)
FetchContent_MakeAvailable(implot)

# LexviEngine
FetchContent_Declare(
    LexviEngine
    GIT_REPOSITORY https://github.com/alx-m24/LexviEngine
    GIT_TAG old-engine
)
FetchContent_MakeAvailable(LexviEngine)

# libcurl
FetchContent_Declare(
    curl
    GIT_REPOSITORY https://github.com/curl/curl.git
    GIT_TAG curl-8_3_0
)
FetchContent_MakeAvailable(curl)

# nlohmann/json
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(nlohmann_json)

# ====================
# Internal modules
# ====================
add_subdirectory(StockScraper)
add_subdirectory(5PercentRule-Bot)
add_subdirectory(Interface)

set(IMGUI_USE_STATIC_LIBS ON CACHE BOOL "" FORCE)
set(IMPLOT_USE_STATIC_LIBS ON CACHE BOOL "" FORCE)
set(LEXVIENGINE_STATIC ON CACHE BOOL "" FORCE)

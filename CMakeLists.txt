cmake_minimum_required(VERSION 3.14)
project(simd-img LANGUAGES CXX VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra -Wpedantic)

# Enable SSE4.2 — start here, add AVX2 later once the basics are solid
add_compile_options(-msse4.2)

# --- Library ---
add_library(simd_img
    src/image.cpp
    src/filters_scalar.cpp
    src/filters_sse.cpp
)

target_include_directories(simd_img PUBLIC include)

# --- Google Test ---
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
FetchContent_MakeAvailable(googletest)

# --- Tests ---
enable_testing()
add_executable(simd_img_tests tests/test_filters.cpp)
target_link_libraries(simd_img_tests PRIVATE simd_img GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(simd_img_tests)

cmake_minimum_required(VERSION 3.15)
project(SPO_1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Принудительно указываем toolchain файл
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "C:/Users/evgen/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

# Явно указываем triplet
if(NOT DEFINED VCPKG_TARGET_TRIPLET)
    set(VCPKG_TARGET_TRIPLET "x64-mingw-static" CACHE STRING "")
endif()

# Добавляем пути вручную (как запасной вариант)
list(APPEND CMAKE_PREFIX_PATH "C:/Users/evgen/vcpkg/installed/x64-mingw-static")
list(APPEND CMAKE_PROGRAM_PATH "C:/Users/evgen/vcpkg/installed/x64-mingw-static/tools/drogon")

find_package(Drogon CONFIG REQUIRED PATHS "C:/Users/evgen/vcpkg/installed/x64-mingw-static/share/drogon")
find_package(jsoncpp CONFIG REQUIRED PATHS "C:/Users/evgen/vcpkg/installed/x64-mingw-static/share/jsoncpp")

add_executable(SPO_1 main.cpp)

target_link_libraries(SPO_1 
    Drogon::Drogon
    JsonCpp::JsonCpp
)

# Статическая компиляция
set(BUILD_SHARED_LIBS OFF)
cmake_minimum_required(VERSION 3.10)
project(rank_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Redis
find_package(PkgConfig REQUIRED)
find_package(Boost REQUIRED COMPONENTS system date_time)
find_package(OpenSSL REQUIRED)
pkg_check_modules(HIREDIS REQUIRED hiredis)

include_directories(${HIREDIS_INCLUDE_DIRS})
link_directories(${HIREDIS_LIBRARY_DIRS})

find_path(REDIS_PLUS_PLUS_INCLUDE_DIR sw/redis++/redis++.h)
find_library(REDIS_PLUS_PLUS_LIBRARY NAMES redis++)

include_directories(${REDIS_PLUS_PLUS_INCLUDE_DIR})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(/root/work/code/clib/jwt-cpp/include)
include_directories(/root/work/code/clib/dotenv-cpp/include/laserpants/dotenv)
include_directories(${OPENSSL_INCLUDE_DIR})

# Crow
include(FetchContent)
FetchContent_Declare(
  crow
  GIT_REPOSITORY git@github.com:CrowCpp/Crow.git
  GIT_TAG v1.0+1
)
FetchContent_MakeAvailable(crow)
include_directories(${crow_SOURCE_DIR}/include)

# 构建
add_executable(rank_server
    main.cpp
    RankService.cpp
    RankUpdater.cpp
)

target_link_libraries(rank_server
    ${Boost_LIBRARIES}
    ${HIREDIS_LIBRARIES}
    ${REDIS_PLUS_PLUS_LIBRARY}
    ${OPENSSL_LIBRARIES}
    pthread
)

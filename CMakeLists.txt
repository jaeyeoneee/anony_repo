cmake_minimum_required(VERSION 3.16)
project(EpochESS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_compile_options(-Wall -Wextra -Wno-unused-parameter -O2)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3 -DNDEBUG)
endif()

find_package(Threads REQUIRED)
find_library(NTL_LIB ntl REQUIRED)
find_library(GMP_LIB gmp REQUIRED)
find_library(SODIUM_LIB sodium REQUIRED)
find_path(SODIUM_INCLUDE sodium.h REQUIRED)

include_directories(${CMAKE_SOURCE_DIR} ${SODIUM_INCLUDE})

add_library(epochess_core STATIC
    core/field.cpp core/polyring.cpp core/hash_to_field.cpp
    core/ess_backend.cpp core/lagrange.cpp core/condshare.cpp core/amd_tag.cpp)
target_link_libraries(epochess_core PUBLIC ${NTL_LIB} ${GMP_LIB} ${SODIUM_LIB} Threads::Threads)

add_library(epochess_net STATIC
    net/channel.cpp net/metrics.cpp net/msg.cpp net/inproc_bus.cpp)
target_link_libraries(epochess_net PUBLIC epochess_core)

add_library(epochess_protocol STATIC
    protocol/party.cpp protocol/distjoin.cpp protocol/softrefresh.cpp
    protocol/hardrefresh.cpp protocol/a2pe.cpp protocol/epoch_mgr.cpp)
target_link_libraries(epochess_protocol PUBLIC epochess_core epochess_net)

add_library(epochess_adversary STATIC
    adversary/byzantine.cpp adversary/grinding.cpp)
target_link_libraries(epochess_adversary PUBLIC epochess_core epochess_protocol epochess_net)

enable_testing()

add_executable(test_field tests/test_field.cpp)
target_link_libraries(test_field PRIVATE epochess_core)
add_test(NAME test_field COMMAND test_field)

add_executable(test_ess tests/test_ess.cpp)
target_link_libraries(test_ess PRIVATE epochess_core)
add_test(NAME test_ess COMMAND test_ess)

add_executable(test_distjoin tests/test_distjoin.cpp)
target_link_libraries(test_distjoin PRIVATE epochess_core epochess_protocol epochess_net)
add_test(NAME test_distjoin COMMAND test_distjoin)

add_executable(test_softrefresh tests/test_softrefresh.cpp)
target_link_libraries(test_softrefresh PRIVATE epochess_core epochess_protocol epochess_net)
add_test(NAME test_softrefresh COMMAND test_softrefresh)

add_executable(test_epoch_mgr tests/test_epoch_mgr.cpp)
target_link_libraries(test_epoch_mgr PRIVATE epochess_core epochess_protocol epochess_net)
add_test(NAME test_epoch_mgr COMMAND test_epoch_mgr)

foreach(exp exp1_distjoin_scaling exp2_softrefresh_scaling exp3_admissibility
            exp4_share_size exp5_wan_latency exp6_packet_loss
            exp7_byzantine exp8_grinding_dos exp10_extension_tradeoff)
    add_executable(${exp} experiments/${exp}.cpp)
    target_link_libraries(${exp} PRIVATE epochess_core epochess_protocol epochess_net epochess_adversary)
endforeach()

message(STATUS "EpochESS build configured")

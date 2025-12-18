#
#
#  MIT License
#
#  Copyright (c) 2024 Yafiyogi
#
#  Permission is hereby granted, free of charge, to any person obtaining a copy
#  of this software and associated documentation files (the "Software"), to deal
#  in the Software without restriction, including without limitation the rights
#  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#  copies of the Software, and to permit persons to whom the Software is
#  furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included in all
#  copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#  SOFTWARE.
#
#

# PKG_CONFIG_PATH=${HOME}/code/tp/lib/pkgconfig:${HOME}/code/tp/share/pkgconfig:${PKG_CONFIG_PATH} cmake --fresh -DCMAKE_INSTALL_PREFIX=${HOME}/code/cpp/build -DYY_THIRD_PARTY_LIBRARY=${HOME}/code/tp -DCMAKE_POSITION_INDEPENDENT_CODE=TRUE -DCMAKE_BUILD_TYPE=Release .
# cmake --fresh -DCMAKE_INSTALL_PREFIX=${HOME}/code/cpp/build -DYY_THIRD_PARTY_LIBRARY=${HOME}/code/tp -DCMAKE_POSITION_INDEPENDENT_CODE=TRUE -DCMAKE_BUILD_TYPE=Release .
# LSAN_OPTIONS="detect_leaks=1" ASAN_OPTIONS="detect_leaks=1" nohup ./yy_mendel -f ../mendel.yaml >> "mendel_$(date +'%Y-%m-%d').log" 2>&1 &
# nohup build-dir/mendel -f ./mqtt_bridge.yaml >> "mendel_$(date +'%Y-%m-%d').log" 2>&1 &

cmake_minimum_required(VERSION 3.24)

project(mendel_proj LANGUAGES CXX)

set(YY_ROOT "${PROJECT_SOURCE_DIR}/..")
set(YY_CMAKE "${YY_ROOT}/yy_cmake")

include(${YY_CMAKE}/cmake_common.txt)
include(${YY_CMAKE}/cmake_clang_tidy.txt)
include(${YY_CMAKE}/cmake_boost-1.84.0.txt)

find_package(Boost 1.84.0 CONFIG REQUIRED COMPONENTS json locale program_options)
find_package(fmt REQUIRED)
find_package(re2 REQUIRED)
find_package(spdlog REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(Iconv REQUIRED)
find_package(ICU 77.1 COMPONENTS data i18n io uc REQUIRED)
find_package(LibreSSL REQUIRED)
find_package(yy_cpp REQUIRED)
find_package(yy_json REQUIRED)
find_package(yy_maths REQUIRED)
find_package(yy_mqtt REQUIRED)
find_package(yy_values REQUIRED)
find_package(ZLIB REQUIRED)

pkg_check_modules(Mosquitto_c IMPORTED_TARGET libmosquitto_static REQUIRED)
pkg_check_modules(Mosquitto_cpp IMPORTED_TARGET libmosquittopp_static REQUIRED)

add_executable(mendel
  actions_handler.cpp
  action_kalman.cpp
  actions_store.cpp
  cache_handler.cpp
  configure_actions.cpp
  configure_logging.cpp
  configure_mqtt.cpp
  configure_mqtt_client.cpp
  configure_mqtt_handlers.cpp
  configure_mqtt_topics.cpp
  logger.cpp
  mqtt_client.cpp
  mqtt_handler.cpp
  mqtt_handler_json.cpp
  mqtt_handler_value.cpp
  mqtt_publisher.cpp
  values_store.cpp
  mendel.cpp )

target_compile_options(mendel
  PRIVATE
  "-DSPDLOG_COMPILED_LIB"
  "-DSPDLOG_FMT_EXTERNAL")

target_include_directories(mendel
  PRIVATE
    "${CMAKE_INSTALL_PREFIX}/include" )

target_include_directories(mendel
   SYSTEM PRIVATE
    "${YY_THIRD_PARTY_LIBRARY}/include")

target_link_directories(mendel
  PUBLIC
    "${CMAKE_INSTALL_PREFIX}/lib"
    "${YY_THIRD_PARTY_LIBRARY}/libressl/lib"
    "${YY_THIRD_PARTY_LIBRARY}/lib" )

if("${yy_system_name}" STREQUAL "darwin")
 target_link_directories(mendel
   PUBLIC
      "/opt/local/lib")
endif()

target_link_directories(mendel BEFORE PRIVATE ${YY_THIRD_PARTY_LIBRARY}/lib)

target_link_libraries(mendel
  yy_values::yy_values
  yy_mqtt::yy_mqtt
  yy_maths::yy_maths
  yy_json::yy_json
  yy_cpp::yy_cpp
  Boost::json
  Boost::locale
  Boost::program_options
  fmt::fmt
  re2::re2
  spdlog::spdlog
  yaml-cpp::yaml-cpp
  PkgConfig::Mosquitto_cpp
  PkgConfig::Mosquitto_c
  LibreSSL::Crypto
  LibreSSL::SSL
  ICU::i18n
  ICU::io
  ICU::uc
  ICU::data
  Iconv::Iconv
  ZLIB::ZLIB )

add_yy_tidy_all(mendel)

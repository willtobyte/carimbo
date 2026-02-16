cmake_minimum_required(VERSION 3.15)
project(carimbo LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)

set(SDL3_DIR "${CMAKE_BINARY_DIR}")

file(GLOB SOURCE_FILES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB HEADER_FILES CONFIGURE_DEPENDS "src/*.hpp")

add_compile_definitions(
  NOMINMAX
  WIN32_LEAN_AND_MEAN
)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE
  ${SOURCE_FILES}
  ${HEADER_FILES}
  ${TEMPLATE_FILES}
)

execute_process(
  COMMAND git describe --tags --match "v*.*.*" --abbrev=0
  OUTPUT_VARIABLE latest_tag
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
  COMMAND git rev-parse --short=6 HEAD
  OUTPUT_VARIABLE commit_short
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
add_compile_definitions(GIT_VERSION="${latest_tag}+${commit_short}")

if(WIN32)
  set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE $<NOT:$<CONFIG:Debug>>
  )
endif()

set(IS_EMSCRIPTEN FALSE)
if (CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
  set(IS_EMSCRIPTEN TRUE)
endif()

find_package(Boost CONFIG REQUIRED)
find_package(box2d CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(PhysFS CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(SPNG CONFIG REQUIRED)
find_package(yyjson CONFIG REQUIRED)
find_package(sol2 CONFIG REQUIRED)
find_package(miniaudio CONFIG REQUIRED)
find_package(opusfile CONFIG REQUIRED)

if(NOT IS_EMSCRIPTEN)
  find_package(OpenSSL CONFIG REQUIRED)
  find_package(sentry CONFIG QUIET)
  if(HAS_LUAJIT)
    find_package(luajit CONFIG REQUIRED)
  endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(DEBUG TRUE)
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE
  $<$<BOOL:${DEVELOPMENT}>:DEVELOPMENT>
  $<$<BOOL:${DEBUG}>:DEBUG>
)

target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<BOOL:${IS_EMSCRIPTEN}>:-pthread>
  $<$<AND:$<NOT:$<BOOL:${DEBUG}>>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-ffast-math>
  $<$<AND:$<NOT:$<BOOL:${DEBUG}>>,$<CXX_COMPILER_ID:MSVC>>:/fp:fast>
)

target_link_options(${PROJECT_NAME} PRIVATE
  $<$<BOOL:${IS_EMSCRIPTEN}>:
    -sENVIRONMENT=web
    -sINVOKE_RUN=0
    -sEXIT_RUNTIME=0
    -sINITIAL_MEMORY=256MB
    -sALLOW_MEMORY_GROWTH=1
    -sSTACK_SIZE=8MB

    -sFILESYSTEM=1
    -sUSE_SDL=3

    -sWASM_BIGINT=1

    -sDISABLE_EXCEPTION_CATCHING=0
    -sASSERTIONS=1

    -sEXPORTED_FUNCTIONS=['_main']
    -sEXPORTED_RUNTIME_METHODS=['callMain']

    -sAUDIO_WORKLET=1
    -sWASM_WORKERS=1
    -sASYNCIFY
  >
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  boost::boost
  box2d::box2d
  EnTT::EnTT
  physfs-static
  SDL3::SDL3-static
  yyjson::yyjson
  spng::spng_static
  sol2::sol2
  miniaudio::miniaudio
  opusfile::opusfile
  $<$<NOT:$<BOOL:${IS_EMSCRIPTEN}>>:openssl::openssl>
  $<$<BOOL:${luajit_FOUND}>:luajit::luajit>
  $<$<BOOL:${sentry_FOUND}>:sentry-native::sentry-native>
)

target_precompile_headers(${PROJECT_NAME} PRIVATE ${HEADER_FILES})

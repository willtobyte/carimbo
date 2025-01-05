cmake_minimum_required(VERSION 3.20)

project(carimbo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SDL2_DIR "${CMAKE_BINARY_DIR}")

if(NOT CMAKE_SYSTEM_NAME MATCHES "Emscripten")
  find_package(Boost CONFIG REQUIRED)
  find_package(OpenSSL CONFIG REQUIRED)
endif()

find_package(Ogg CONFIG REQUIRED)
find_package(PhysFS CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(SPNG CONFIG REQUIRED)
find_package(sol2 CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenAL CONFIG REQUIRED)
find_package(Vorbis CONFIG REQUIRED)

file(GLOB HEADER_FILES "src/*.hpp")
file(GLOB SOURCE_FILES "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_precompile_headers(${PROJECT_NAME} PRIVATE ${HEADER_FILES})

if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Werror -pedantic)
endif()

if (CMAKE_BUILD_TYPE MATCHES "Debug")
  if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /Od /Zi)
  else()
    target_compile_options(${PROJECT_NAME} PRIVATE -O0 -g)
  endif()
elseif (CMAKE_BUILD_TYPE MATCHES "Release")
  if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /O2 /GL)
  else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Os -flto)
  endif()
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES Emscripten)
  set(EMSCRIPTEN_OPTIONS
    -sEVAL_CTORS=1
    -sWASM_BIGINT=1
    -sALLOW_MEMORY_GROWTH=1
    -sINITIAL_MEMORY=128MB
    -lwebsocket.js
    -sUSE_SDL=2
    -sWEBSOCKET_SUBPROTOCOL=text
    -sEXPORTED_RUNTIME_METHODS=['callMain']
  )

  if (ENABLE_PROFILING)
    list(APPEND EMSCRIPTEN_OPTIONS
      -O0
      -sASSERTIONS=1
      -sSAFE_HEAP=1
      -sSTACK_OVERFLOW_CHECK=2
      -sNO_DISABLE_EXCEPTION_CATCHING=1
    )
  else()
    list(APPEND EMSCRIPTEN_OPTIONS
      -O2
    )
  endif()

  target_link_options(${PROJECT_NAME} PRIVATE ${EMSCRIPTEN_OPTIONS})
endif()

option(LOCAL "Use local environment" OFF)
if(LOCAL)
  target_compile_definitions(${PROJECT_NAME} PRIVATE LOCAL)
endif()

option(SANDBOX "Use sandbox" OFF)
if(SANDBOX)
  target_compile_definitions(${PROJECT_NAME} PRIVATE SANDBOX)
endif()

option(HITBOX "Show hitboxes" OFF)
if(LOCAL)
  target_compile_definitions(${PROJECT_NAME} PRIVATE HITBOX)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
  physfs-static
  SDL2::SDL2-static
  SDL2::SDL2main
  spng::spng_static
  sol2::sol2
  nlohmann_json::nlohmann_json
  Ogg::ogg
  vorbis::vorbis
)

if (NOT CMAKE_SYSTEM_NAME MATCHES "Emscripten")
  target_link_libraries(${PROJECT_NAME} PRIVATE Boost::boost OpenAL::OpenAL openssl::openssl)
endif()

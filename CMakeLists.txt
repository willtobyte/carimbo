cmake_minimum_required(VERSION 3.20)
project(carimbo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SDL2_DIR "${CMAKE_BINARY_DIR}")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type (Debug or Release)" FORCE)
endif()

if(NOT CMAKE_SYSTEM_NAME MATCHES "Emscripten")
  find_package(Boost CONFIG REQUIRED)
  find_package(OpenAL CONFIG REQUIRED)
  find_package(OpenSSL CONFIG REQUIRED)
endif()
find_package(nlohmann_json CONFIG REQUIRED)
find_package(Ogg CONFIG REQUIRED)
find_package(PhysFS CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(sol2 CONFIG REQUIRED)
find_package(SPNG CONFIG REQUIRED)
find_package(Vorbis CONFIG REQUIRED)

file(GLOB SOURCE_FILES "src/*.cpp")
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

file(GLOB HEADER_FILES "src/*.hpp")
target_precompile_headers(${PROJECT_NAME} PRIVATE ${HEADER_FILES})

if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Werror -pedantic)
endif()

option(HITBOX "Enable HITBOX flag" OFF)
if(HITBOX)
  target_compile_definitions(${PROJECT_NAME} PRIVATE HITBOX)
endif()

option(LOCAL "Enable LOCAL flag" OFF)
if(LOCAL)
  target_compile_definitions(${PROJECT_NAME} PRIVATE LOCAL)
endif()

option(SANDBOX "Enable SANDBOX flag" OFF)
if(SANDBOX)
  target_compile_definitions(${PROJECT_NAME} PRIVATE SANDBOX)
endif()

if(EMSCRIPTEN)
  set(EMSDK_COMPILE_DEBUG_FLAGS
    -O0
    -g
    -gsource-map
  )

  set(EMSDK_LINK_DEBUG_FLAGS
    -sASSERTIONS=1
    -sSAFE_HEAP=1
    -sSTACK_OVERFLOW_CHECK=2
    -sDISABLE_EXCEPTION_CATCHING=0
  )

  set(EMSDK_RELEASE_FLAGS -Os)

  target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:${EMSDK_COMPILE_DEBUG_FLAGS}>
    $<$<CONFIG:Release>:${EMSDK_RELEASE_FLAGS}>
  )

  target_link_options(${PROJECT_NAME} PRIVATE
    -sEVAL_CTORS=1
    -sWASM_BIGINT=1
    -sALLOW_MEMORY_GROWTH=1
    -sINITIAL_MEMORY=128MB
    -lwebsocket.js
    -sUSE_SDL=2
    -sWEBSOCKET_SUBPROTOCOL=text
    -sEXPORTED_RUNTIME_METHODS=['callMain']
    $<$<CONFIG:Debug>:${EMSDK_LINK_DEBUG_FLAGS}>
    $<$<CONFIG:Release>:${EMSDK_RELEASE_FLAGS}>
  )
else()
  if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
      $<$<CONFIG:Debug>:/Od;/Zi>
      $<$<CONFIG:Release>:/O2>
    )
  else()
    target_compile_options(${PROJECT_NAME} PRIVATE
      $<$<CONFIG:Debug>:-O0;-g>
      $<$<CONFIG:Release>:-O2;-flto>
    )
    target_link_options(${PROJECT_NAME} PRIVATE
      $<$<CONFIG:Release>:-flto>
    )
  endif()
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE
  $<$<CONFIG:Debug>:DEBUG>
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  nlohmann_json::nlohmann_json
  Ogg::ogg
  physfs-static
  SDL2::SDL2-static
  SDL2::SDL2main
  sol2::sol2
  spng::spng_static
  vorbis::vorbis
)

if(NOT CMAKE_SYSTEM_NAME MATCHES "Emscripten")
  target_link_libraries(${PROJECT_NAME} PRIVATE
    Boost::boost
    OpenAL::OpenAL
    openssl::openssl
  )
endif()

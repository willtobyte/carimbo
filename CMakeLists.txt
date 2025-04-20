cmake_minimum_required(VERSION 3.15)
project(carimbo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SDL3_DIR "${CMAKE_BINARY_DIR}")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type (Debug or Release)" FORCE)
endif()

if(NOT CMAKE_SYSTEM_NAME MATCHES "Emscripten")
  find_package(Boost CONFIG REQUIRED)
  find_package(luajit CONFIG REQUIRED)
  find_package(OpenAL CONFIG REQUIRED)
  find_package(OpenSSL CONFIG REQUIRED)
endif()
find_package(fmt CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(Ogg CONFIG REQUIRED)
find_package(PhysFS CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(sol2 CONFIG REQUIRED)
find_package(SPNG CONFIG REQUIRED)
find_package(Vorbis CONFIG REQUIRED)

file(GLOB SOURCE_FILES "src/*.cpp")
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

file(GLOB HEADER_FILES "src/*.hpp")
target_precompile_headers(${PROJECT_NAME} PRIVATE ${HEADER_FILES})

file(GLOB TEMPLATE_FILES "src/*.tpp")
target_sources(${PROJECT_NAME} PRIVATE ${TEMPLATE_FILES})

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
    -g3
    -gsource-map
    -fexperimental-library
  )

  set(EMSDK_LINK_DEBUG_FLAGS
    -g3
    -gsource-map
    -sASSERTIONS=2
    -sSAFE_HEAP=1
    -sSTACK_OVERFLOW_CHECK=2
    -sDISABLE_EXCEPTION_CATCHING=0
  )

  set(EMSDK_COMPILE_RELEASE_FLAGS
    -Oz
    -g0
    -flto
    -fno-exceptions
    -fexperimental-library
  )

  set(EMSDK_LINK_RELEASE_FLAGS
    -Oz
    -g0
    -flto
    --strip-all
    -sASSERTIONS=0
    -sDISABLE_EXCEPTION_CATCHING=1
  )

  target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:${EMSDK_COMPILE_DEBUG_FLAGS}>
    $<$<CONFIG:Release>:${EMSDK_COMPILE_RELEASE_FLAGS}>
  )

  target_link_options(${PROJECT_NAME} PRIVATE
    -sUSE_SDL=3
    -sEVAL_CTORS=1
    -sWASM_BIGINT=1
    -sFILESYSTEM=1
    -sINVOKE_RUN=0
    -sMINIMAL_RUNTIME=2
    -sEXIT_RUNTIME=0
    -sINITIAL_MEMORY=128MB
    -sALLOW_MEMORY_GROWTH=1
    -sEXPORTED_RUNTIME_METHODS=['callMain']
    -sWEBSOCKET_SUBPROTOCOL=text
    -lwebsocket.js
    $<$<CONFIG:Debug>:${EMSDK_LINK_DEBUG_FLAGS}>
    $<$<CONFIG:Release>:${EMSDK_LINK_RELEASE_FLAGS}>
  )
else()
  set(SANITIZER_FLAGS -fsanitize=address,undefined)

  if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
      $<$<CONFIG:Debug>:/Od;/Zi>
      $<$<CONFIG:Release>:/O3>
    )
  else()
    target_compile_options(${PROJECT_NAME} PRIVATE
      $<$<CONFIG:Debug>:-O0;-g;${SANITIZER_FLAGS}>
      $<$<CONFIG:Release>:-O3;-flto>
    )
    target_link_options(${PROJECT_NAME} PRIVATE
      $<$<CONFIG:Debug>:${SANITIZER_FLAGS}>
      $<$<CONFIG:Release>:-flto>
    )

    target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:NDEBUG>)
  endif()
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE
  $<$<CONFIG:Debug>:DEBUG>
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  fmt::fmt
  nlohmann_json::nlohmann_json
  Ogg::ogg
  physfs-static
  SDL3::SDL3-static
  sol2::sol2
  spng::spng_static
  vorbis::vorbis
)

if(NOT CMAKE_SYSTEM_NAME MATCHES "Emscripten")
  target_link_libraries(${PROJECT_NAME} PRIVATE
    Boost::boost
    luajit::luajit
    OpenAL::OpenAL
    openssl::openssl
  )
endif()

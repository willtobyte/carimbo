cmake_minimum_required(VERSION 3.15)
project(carimbo VERSION 1.0 LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_CXX_COMPILER_ID STREQUAL Emscripten)
  set(EMSCRIPTEN TRUE)
else()
  set(EMSCRIPTEN FALSE)
endif()

file(GLOB_RECURSE SRC_FILES
  src/*.cpp
  src/*.hpp
  src/*.tpp
)

add_executable(${PROJECT_NAME}
  ${SRC_FILES}
)

target_precompile_headers(${PROJECT_NAME}
  PRIVATE
    src/*.hpp
    src/*.tpp
)

target_include_directories(${PROJECT_NAME} PRIVATE src)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

set(WARNINGS
  -Wall
  -Wextra
  -Wpedantic
)

if(EMSCRIPTEN)
  set(DEBUG_FLAGS
    -O0
    -g3
    -gsource-map
  )

  set(RELEASE_FLAGS
    -Oz
    -g0
    -flto
    -fno-plt
    -fstrict-aliasing
    -ffast-math
    -fomit-frame-pointer
    -DNDEBUG
    ${WARNINGS}
    -Werror
  )

  target_compile_options(${PROJECT_NAME}
    PRIVATE
      $<$<CONFIG:Debug>:${DEBUG_FLAGS}>
      $<$<CONFIG:Release>:${RELEASE_FLAGS}>
  )

  set(LINK_DEBUG
    -g3
    -gsource-map
    -sASSERTIONS=2
    -sDISABLE_EXCEPTION_CATCHING=0
  )

  set(LINK_RELEASE
    -Oz
    -g0
    -flto
    -fno-plt
    --strip-all
    --ignore-external-input
    -sASSERTIONS=0
    -sDISABLE_EXCEPTION_CATCHING=1
  )

  target_link_options(${PROJECT_NAME}
    PRIVATE
      -sUSE_SDL=3
      -sEVAL_CTORS=1
      -sWASM_BIGINT=1
      -sINVOKE_RUN=0
      -sEXIT_RUNTIME=0
      -sINITIAL_MEMORY=128MB
      -sALLOW_MEMORY_GROWTH=1
      -sEXPORTED_RUNTIME_METHODS=['callMain']
      -sWEBSOCKET_SUBPROTOCOL=text
      -lwebsocket.js
      $<$<CONFIG:Debug>:${LINK_DEBUG}>
      $<$<CONFIG:Release>:${LINK_RELEASE}>
  )
else()
  if(MSVC)
    set(DEBUG_FLAGS
      /Od
      /Zi
      /W4
      /bigobj
    )

    set(RELEASE_FLAGS
      /O2
      /GL
      /DNDEBUG
      /W4
    )

    set(RELEASE_LINK_FLAGS
      /LTCG
    )
  else()
    set(DEBUG_FLAGS
      -O0
      -g
      ${WARNINGS}
    )

    set(RELEASE_FLAGS
      -O3
      -flto
      -fstrict-aliasing
      -ffast-math
      -funroll-loops
      -fomit-frame-pointer
      -DNDEBUG
      ${WARNINGS}
    )

    set(RELEASE_LINK_FLAGS
      -flto
      -fno-plt
    )
  endif()

  target_compile_options(${PROJECT_NAME}
    PRIVATE
      $<$<CONFIG:Debug>:${DEBUG_FLAGS}>
      $<$<CONFIG:Release>:${RELEASE_FLAGS}>
  )

  target_link_options(${PROJECT_NAME}
    PRIVATE
      $<$<CONFIG:Release>:${RELEASE_LINK_FLAGS}>
  )

  target_compile_definitions(${PROJECT_NAME}
    PRIVATE
      $<$<CONFIG:Release>:NDEBUG>
  )
endif()

target_compile_definitions(${PROJECT_NAME}
  PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
)

foreach(flag IN ITEMS
  HITBOX
  LOCAL
  SANDBOX
)
  option(${flag} "" OFF)
  if(${flag})
    target_compile_definitions(${PROJECT_NAME} PRIVATE ${flag})
  endif()
endforeach()

find_package(fmt CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(Ogg CONFIG REQUIRED)
find_package(PhysFS CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(sol2 CONFIG REQUIRED)
find_package(SPNG CONFIG REQUIRED)
find_package(Vorbis CONFIG REQUIRED)

if(NOT EMSCRIPTEN)
  find_package(Boost CONFIG REQUIRED)
  find_package(luajit CONFIG REQUIRED)
  find_package(OpenAL CONFIG REQUIRED)
  find_package(OpenSSL CONFIG REQUIRED)
endif()

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    fmt::fmt
    nlohmann_json::nlohmann_json
    Ogg::ogg
    physfs-static
    SDL3::SDL3-static
    sol2::sol2
    spng::spng_static
    vorbis::vorbis
    $<$<NOT:EMSCRIPTEN>:Boost::boost>
    $<$<NOT:EMSCRIPTEN>:luajit::luajit>
    $<$<NOT:EMSCRIPTEN>:OpenAL::OpenAL>
    $<$<NOT:EMSCRIPTEN>:openssl::openssl>
)

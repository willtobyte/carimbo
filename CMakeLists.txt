cmake_minimum_required(VERSION 3.15)
project(carimbo LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SDL3_DIR "${CMAKE_BINARY_DIR}")

file(GLOB SOURCE_FILES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB HEADER_FILES CONFIGURE_DEPENDS "src/*.hpp")
file(GLOB TEMPLATE_FILES CONFIGURE_DEPENDS "src/*.tpp")

if(CMAKE_SYSTEM_NAME STREQUAL "Android")
  add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${TEMPLATE_FILES})
elseif(CMAKE_SYSTEM_NAME STREQUAL "iOS")
  add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES} ${TEMPLATE_FILES})
else()
  add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${TEMPLATE_FILES})
endif()

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

add_compile_definitions(GIT_TAG="${latest_tag}+${commit_short}")

foreach(flag SANDBOX)
  if(${flag})
    target_compile_definitions(${PROJECT_NAME} PRIVATE ${flag})
  endif()
endforeach()

find_package(nlohmann_json CONFIG REQUIRED)
find_package(Ogg CONFIG REQUIRED)
find_package(PhysFS CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(sol2 CONFIG REQUIRED)
find_package(SPNG CONFIG REQUIRED)
find_package(Vorbis CONFIG REQUIRED)

if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
  set(EMSDK_COMPILE_DEBUG_FLAGS
    -O0
    -g3
    -gsource-map
    -fno-rtti
    -fno-omit-frame-pointer
  )

  set(EMSDK_LINK_DEBUG_FLAGS
    -g3
    -gsource-map
    -fno-rtti
    -sASSERTIONS=2
    -sDISABLE_EXCEPTION_CATCHING=0
  )

  set(EMSDK_COMPILE_RELEASE_FLAGS
    -Oz
    -g0
    -flto
    -fno-rtti
    -fstrict-aliasing
    -fomit-frame-pointer
    -fdata-sections
    -ffunction-sections
    -DNDEBUG
    -Wall
    -Werror
    -Wextra
    -Wpedantic
  )

  set(EMSDK_LINK_RELEASE_FLAGS
    -Oz
    -g0
    -flto
    -fno-rtti
    -s
    -sASSERTIONS=1
    -sDISABLE_EXCEPTION_CATCHING=0
  )

  target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:${EMSDK_COMPILE_DEBUG_FLAGS}>
    $<$<CONFIG:Release>:${EMSDK_COMPILE_RELEASE_FLAGS}>
  )

  target_link_options(${PROJECT_NAME} PRIVATE
    -sENVIRONMENT=web

    -sINVOKE_RUN=0
    -sEXIT_RUNTIME=0
    -sEVAL_CTORS=1

    -sINITIAL_MEMORY=256MB
    -sALLOW_MEMORY_GROWTH=1
    -sSTACK_SIZE=8MB

    -sFILESYSTEM=1

    -sEXPORTED_FUNCTIONS=['_main']
    -sEXPORTED_RUNTIME_METHODS=['callMain']

    -sUSE_SDL=3
    -sWASM_BIGINT=1
    -sWEBSOCKET_SUBPROTOCOL=text
    -lwebsocket.js

    $<$<CONFIG:Debug>:${EMSDK_LINK_DEBUG_FLAGS}>
    $<$<CONFIG:Release>:${EMSDK_LINK_RELEASE_FLAGS}>
  )
endif()

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
  find_package(Boost CONFIG REQUIRED)
  find_package(OpenAL CONFIG REQUIRED)
  find_package(OpenSSL CONFIG REQUIRED)

  find_package(sentry CONFIG QUIET)

  if(SOL_LUAJIT)
    find_package(luajit CONFIG REQUIRED)
  endif()

  target_link_libraries(${PROJECT_NAME} PRIVATE
    Boost::boost
    OpenAL::OpenAL
    openssl::openssl
  )

  if(SOL_LUAJIT)
    target_link_libraries(${PROJECT_NAME} PRIVATE luajit::luajit)
  endif()

  if(sentry_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE sentry-native::sentry-native)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_SENTRY=1)
  endif()

  if(MSVC)
    set(DEBUG_FLAGS
      /Od
      /Zi
      /RTC1
      /GS
      /fp:precise
      /Zc:inline
      /Zc:__cplusplus
      /GR-
      /EHsc
      /DDEBUG
      /W4
      /wd4100
      /bigobj
    )

    set(RELEASE_FLAGS
      /O2
      /GL
      /Gw
      /Gy
      /GR-
      /EHsc
      /DNDEBUG
      /W4
      /permissive-
      /Zc:inline
      /Zc:__cplusplus
    )

    set(DEBUG_LINK_FLAGS
      /DEBUG
      /INCREMENTAL
      /OPT:NOREF
      /OPT:NOICF
      /SAFESEH
      /NXCOMPAT
      /DYNAMICBASE
    )

    set(RELEASE_LINK_FLAGS
      /LTCG
      /OPT:REF
      /OPT:ICF
      /INCREMENTAL:NO
      /RELEASE
      /NXCOMPAT
      /DYNAMICBASE
      /NOCOFFGRPINFO
      /DEBUG:NONE
    )

    target_compile_options(${PROJECT_NAME} PRIVATE
      $<$<CONFIG:Debug>:${DEBUG_FLAGS}>
      $<$<CONFIG:Release>:${RELEASE_FLAGS}>
    )

    target_link_options(${PROJECT_NAME} PRIVATE
      $<$<CONFIG:Debug>:${DEBUG_LINK_FLAGS}>
      $<$<CONFIG:Release>:${RELEASE_LINK_FLAGS}>
    )

    target_compile_definitions(${PROJECT_NAME} PRIVATE
      NOMINMAX
      WIN32_LEAN_AND_MEAN
      _WIN32_WINNT=0x0601
    )
  else()
    set(GENERIC_WARNINGS
      -Wall
      -Werror
      -Wextra
      -Wpedantic
      -Wconversion
      -Wsign-conversion
      -Wnarrowing
      -Wdouble-promotion
      -Wfloat-equal
    )

    set(DEBUG_FLAGS
      -O0
      -g3
      -fno-inline
      -fstandalone-debug
      -fno-rtti
      -fno-omit-frame-pointer
      -fstack-protector-strong
      -fno-elide-constructors
      -fsanitize=address,undefined
      -DDEBUG
      -Wall
      -Wextra
      -Wpedantic
      -Wno-unused-parameter
      ${GENERIC_WARNINGS}
    )

    set(RELEASE_FLAGS
      -O3
      -g0
      -flto
      -fstrict-aliasing
      -funroll-loops
      -ffunction-sections
      -fdata-sections
      -fvisibility=hidden
      -fvisibility-inlines-hidden
      -fomit-frame-pointer
      -fno-rtti
      -DNDEBUG
      ${GENERIC_WARNINGS}
    )

    set(RELEASE_LINKER_FLAGS
      -flto
      -s
      -Wl,-x
    )

    target_compile_options(${PROJECT_NAME} PRIVATE
      $<$<CONFIG:Debug>:${DEBUG_FLAGS}>
      $<$<CONFIG:Release>:${RELEASE_FLAGS}>
    )

    target_link_options(${PROJECT_NAME} PRIVATE
      $<$<CONFIG:Debug>:${DEBUG_FLAGS}>
      $<$<CONFIG:Release>:${RELEASE_LINKER_FLAGS}>
    )
  endif()
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:DEBUG>)

target_link_libraries(${PROJECT_NAME} PRIVATE
  nlohmann_json::nlohmann_json
  Ogg::ogg
  physfs-static
  SDL3::SDL3-static
  sol2::sol2
  spng::spng_static
  vorbis::vorbis
)

target_precompile_headers(${PROJECT_NAME} PRIVATE ${HEADER_FILES})

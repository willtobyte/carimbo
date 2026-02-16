#define MA_NO_DECODING
#define MA_NO_ENCODING
#define MA_NO_WAV
#define MA_NO_FLAC
#define MA_NO_MP3
#define MA_NO_RESOURCE_MANAGER
#define MA_NO_GENERATION

#ifdef EMSCRIPTEN
#define MA_ENABLE_AUDIO_WORKLETS
#endif

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

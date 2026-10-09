/*
 * Minimal SDL2 audio declarations for the CardputerZero BSP, which ships
 * runtime libraries without SDL development headers. Desktop builds use
 * official SDL headers. The embedded declarations match SDL2's stable C ABI.
 */
#pragma once

#if USE_DESKTOP
#include <SDL.h>
#else
#include <cstdint>

extern "C" {
typedef struct SDL_RWops SDL_RWops;
typedef uint32_t SDL_AudioDeviceID;
typedef struct SDL_AudioSpec {
    int freq;
    uint16_t format;
    uint8_t channels;
    uint8_t silence;
    uint16_t samples;
    uint16_t padding;
    uint32_t size;
    void (*callback)(void*, uint8_t*, int);
    void* userdata;
} SDL_AudioSpec;

int SDL_InitSubSystem(uint32_t flags);
void SDL_QuitSubSystem(uint32_t flags);
const char* SDL_GetError(void);
SDL_RWops* SDL_RWFromFile(const char* file, const char* mode);
SDL_AudioSpec* SDL_LoadWAV_RW(SDL_RWops* src, int freesrc, SDL_AudioSpec* spec,
                             uint8_t** audio_buf, uint32_t* audio_len);
void SDL_FreeWAV(uint8_t* audio_buf);
SDL_AudioDeviceID SDL_OpenAudioDevice(const char* device, int iscapture,
                                     const SDL_AudioSpec* desired,
                                     SDL_AudioSpec* obtained, int allowed_changes);
void SDL_CloseAudioDevice(SDL_AudioDeviceID device);
void SDL_PauseAudioDevice(SDL_AudioDeviceID device, int pause_on);
int SDL_QueueAudio(SDL_AudioDeviceID device, const void* data, uint32_t length);
void SDL_ClearQueuedAudio(SDL_AudioDeviceID device);
}
#endif

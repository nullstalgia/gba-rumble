#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EZODE_MIN_RUMBLE   0xF0
#define EZODE_MED_RUMBLE   0xF1
#define EZODE_MAX_RUMBLE   0xF2
#define EZ3IN1_STOP_RUMBLE 0x8

// Initializes ROM GPIO Pin 3 Direction as Output.
void gba_rumble_rio_init();

// Set RIO Rumble active state.
//
// Define `GBA_RUMBLE_RIO_SHARE` before inclusion to bitwise OR IO3 rather than just set the whole register's value.
// (Useful if you intend to share the ROM IO pins with RTC or some other custom setup.)
void gba_rumble_rio_update(bool rumble);

// Initializes DS Rumble Pak's actuator to Idle state.
void gba_rumble_ds_init();

// Flick DS Rumble Pak's actuator to specified direction, flick several times per second for continuous rumble.
void gba_rumble_ds_update(bool direction);

// Initalize EZ-Flash Omega Definitive Edition's Rumble, sets strength to max and sets state to idle.
void gba_rumble_ezflash_init();

// Set EZODE/3-in-1 Rumble active state.
void gba_rumble_ezflash_update(bool rumble);

// On an EZ-Flash Omega Definitive Edition and 3-in-1, this supposedly sets the strength of the rumble, but doesn't itself activate it.
//
// https://github.com/VeaNika/GBARunner3/commit/84681c16918eea1c122474c8d7e0caa0771bdba8#diff-5064eb4a236b30f048ece54356370b3b2c56fd92057e35fab84a0947623c15e8
//
// https://gbatemp.net/threads/gamecube-gba-rumble-games-and-rom-hacked-rumble.589863/page-2#post-10236322 <3
//
// https://github.com/Dartz150/EZFlash3in1/blob/602ceadfb5d4af28ad6910ff91757b4a851c0321/dsCard.cpp#L655
//
// https://wiki.asie.pl/doku.php?id=notes:flashcart:ezflash
void gba_rumble_ezflash_write_rumble(uint16_t rumble);

#ifdef __cplusplus
}
#endif

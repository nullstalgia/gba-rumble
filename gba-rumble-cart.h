#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EZODE_MAX_RUMBLE   0xF1
#define EZ3IN1_MAX_RUMBLE  0x7
#define EZ3IN1_STOP_RUMBLE 0x8

// Initializes ROM GPIO Pin 3 Direction as Output,
void gba_rumble_rio_init();

// Set RIO Rumble active state.
//
// Define `GBA_RUMBLE_RIO_SHARE` before inclusion to bitwise OR IO3 rather than just set the whole register's value.
// (Useful if you intend to share the ROM IO pins with RTC or some other custom setup.)
void gba_rumble_rio_update(bool rumble);

// Initalize EZ-Flash Omega Definitive Edition's Rumble, sets strength to max and sets state to idle.
void gba_rumble_ezode_init();

// Sets strength to be used when EZODE Rumble is triggered.
void gba_rumble_ezode_strength(uint16_t rumble_strength);

// Set EZODE Rumble active state.
void gba_rumble_ezode_update(bool rumble);

// Initalize EZ-Flash 3-in-1's Rumble, sets strength to max and sets state to idle.
void gba_rumble_ez3in1_init();

// Sets strength to be used when EZ3in-1 Rumble is triggered.
void gba_rumble_ez3in1_strength(uint16_t rumble_strength);

// Set EZ3in-1 Rumble active state.
void gba_rumble_ez3in1_update(bool rumble);

// On an EZ-Flash Omega Definitive Edition, this supposedly sets the strength of the rumble, but doesn't itself activate it.
//
// Documented ranges and personal testing conflict, but the consistent factor is that `0xF1` doesn't _not_ work.
//
// On an EZ-Flash 3-in-1, this sets the active Rumble state directly. This pseudo-register's
// behavior isn't yet fully documented either, but the best guess is:
// 0 - 0b0000_0000 - off(?)
// 1 - 0b0000_0001 - low
// 3 - 0b0000_0011 - medium
// 7 - 0b0000_0111 - high0
// 8 - 0b0000_1xxx - stop(?)
//
// ?: Apparently writing 8 is more reliable than 0 for halting active rumble, according to the GBATemp post.
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

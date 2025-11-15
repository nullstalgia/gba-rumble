#include "gba-rumble-cart.h"

#define RIO_PORT_DATA        (*(volatile uint16_t *)0x80000C4)
#define RIO_PORT_DIRECTION   (*(volatile uint16_t *)0x80000C6)

static uint16_t s_EZ3in1_strength = EZ3IN1_MAX_RUMBLE;  // Defaults to max strength of EZ3-in-1.

void gba_rumble_rio_init() {
    // Set ROM GPIO Pin 3 to Output
    #ifdef GBA_RUMBLE_RIO_SHARE
        RIO_PORT_DIRECTION |= 1 << 3;
    #else
        RIO_PORT_DIRECTION = 1 << 3;
    #endif
}

void gba_rumble_rio_update(bool rumble_on) {
    #ifdef GBA_RUMBLE_RIO_SHARE
    if (rumble_on) {
        RIO_PORT_DATA |= (1 << 3);
    } else {
        RIO_PORT_DATA &= ~(1 << 3);
    }
    #else
        RIO_PORT_DATA = (rumble_on << 3);
    #endif
}

void gba_rumble_ezode_init() {
    gba_rumble_ezode_strength(EZODE_MAX_RUMBLE);
}

void gba_rumble_ezode_strength(uint16_t rumble_strength) {
    gba_rumble_ezflash_write_rumble(rumble_strength);
}

void gba_rumble_ezode_update(bool rumble_on) {
    // This also seems to control the ChisFlash v1.2 Rumble, in addition to the more "standard" ROM GPIO control.
    (*(volatile uint16_t *)0x8001000) = (rumble_on << 1);
}

void gba_rumble_ez3in1_init() {
    s_EZ3in1_strength = EZ3IN1_MAX_RUMBLE;
}

void gba_rumble_ez3in1_strength(uint16_t rumble_strength) {
    s_EZ3in1_strength = rumble_strength;
}

void gba_rumble_ez3in1_update(bool rumble_on) {
    if (rumble_on) {
        gba_rumble_ezflash_write_rumble(s_EZ3in1_strength);
    } else {
        gba_rumble_ezflash_write_rumble(EZ3IN1_STOP_RUMBLE);
    }
}

void gba_rumble_ezflash_write_rumble(uint16_t rumble_strength) {
    (*(volatile uint16_t *)0x9fe0000) = 0xd200; // Magic handshake shared between _all_ EZ-Flash "internal" calls, it seems.
	(*(volatile uint16_t *)0x8000000) = 0x1500;
	(*(volatile uint16_t *)0x8020000) = 0xd200;
	(*(volatile uint16_t *)0x8040000) = 0x1500;

	(*(volatile uint16_t *)0x9E20000) = rumble_strength; // Set pseudo-register inside EZ-Flash, in this case Rumble Strength

	(*(volatile uint16_t *)0x9fc0000) = 0x1500; // Finish transaction
}

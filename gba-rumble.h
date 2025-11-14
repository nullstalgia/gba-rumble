#pragma once
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif


struct GBARumbleGBPConfig {
    void (*serial_irq_setup_)(void (*rumble_isr)(void));
};


enum GBARumbleState {
    gba_rumble_start     = 0x40000026,
    gba_rumble_stop      = 0x40000004,
    gba_rumble_hard_stop = 0x40000015,
};

enum GBARumbleCartType : uint8_t {
    gba_rumble_cart_uninitialized,
    gba_rumble_cart_gpio,
    gba_rumble_cart_ezode,
    gba_rumble_cart_ez3in1,
};

void gba_rumble_init_gbp(struct GBARumbleGBPConfig gbp_config);

void gba_rumble_init_cart(enum GBARumbleCartType cart_type);

// should be called once per frame
void gba_rumble_loop();

// set new active rumble state
void gba_rumble_update(enum GBARumbleState state);

#ifdef __cplusplus
}
#endif

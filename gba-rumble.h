#pragma once
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif


struct GBARumbleGBPConfig {
    void (*serial_irq_setup_)(void (*rumble_isr)(void));
};


enum GBARumbleCartType : uint8_t {
    gba_rumble_cart_uninitialized,
    gba_rumble_cart_rio,
    gba_rumble_cart_ds,
    gba_rumble_cart_ezode,
};

void gba_rumble_init_gbp(struct GBARumbleGBPConfig gbp_config);

void gba_rumble_init_cart(enum GBARumbleCartType cart_type);

// should be called once per frame
void gba_rumble_loop();

// set new active rumble state

void gba_rumble_start();
void gba_rumble_stop();
// GBP-only
void gba_rumble_hard_stop();

#ifdef __cplusplus
}
#endif

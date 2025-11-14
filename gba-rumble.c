#include <stdbool.h>
#include "gba-rumble.h"
#include "gba-rumble-cart.h"


/*
  NOTE:

  Here is a brief explanation, from GBATEK:

  After detecting/unlocking the Gameboy Player, init RCNT and SIOCNT to 32bit
  normal mode, external clock, SO=high, with IRQ enabled, and set the transfer
  start bit. You should then receive the following sequence (about once per
  frame), and your serial IRQ handler should send responses accordingly:

   Receive  Response
   0000494E 494EB6B1
   xxxx494E 494EB6B1
   B6B1494E 544EB6B1
   B6B1544E 544EABB1
   ABB1544E 4E45ABB1
   ABB14E45 4E45B1BA
   B1BA4E45 4F44B1BA
   B1BA4F44 4F44B0BB
   B0BB4F44 8000B0BB
   B0BB8002 10000010
   10000010 20000013
   20000013 40000004
   30000003 40000004
   30000003 40000004
   30000003 40000004
   30000003 400000yy
   30000003 40000004

   The first part of the transfer just contains the string “NINTENDO” split into
   16bit fragments, and bitwise inversions thereof (eg. 494Eh=”NI”, and
   B6B1h=NOT 494Eh). In the second part, <yy> should be 04h=RumbleOff, or
   26h=RumbleOn.
*/


typedef unsigned int u32;
typedef unsigned short u16;


#define REG_BASE 0x04000000

#define REG_RCNT              *(volatile u16*)(REG_BASE + 0x134)
#define REG_SIOCNT            *(volatile u16*)(REG_BASE + 0x128)
#define REG_SIODATA32         *(volatile u32*)(REG_BASE + 0x120)

#define SIO_SO_HIGH (1<<3)
#define SIO_START (1<<7)
#define SIO_32BIT 0x1000
#define SIO_IRQ	0x4000

#define R_NORMAL 0x0000

static enum GBARumbleCartType rumble_cart_type = gba_rumble_cart_uninitialized;
static enum GBARumbleState rumble_state = gba_rumble_stop;
static bool gbp_configured = false;

static void (*cart_rumble_update_func)(bool rumble_on) = 0;

enum GBPCommsStage {
    gbp_comms_nintendo_handshake,
    gbp_comms_check_magic1,
    gbp_comms_check_magic2,
    gbp_comms_rumble,
    gbp_comms_finalize
};


static struct GBPComms {
    enum GBPCommsStage stage_;
    u32 serial_in_;
    u16 index_;
    u16 out_0_;
    u16 out_1_;
} gbp_comms;


static void gbp_serial_start()
{

    if (gbp_comms.serial_in_ == 0x30000003) {
        /* We're currently in the middle of the GBP comms rumble stage. */
        return;
    }

    REG_SIOCNT &= ~1;
    REG_SIOCNT |= SIO_START;
}


static void gbp_serial_isr()
{
    gbp_comms.serial_in_ = REG_SIODATA32;

    u32 result = 0;

    switch (gbp_comms.stage_) {
    case gbp_comms_rumble:
        if (gbp_comms.serial_in_ == 0x30000003) {
            result = rumble_state;
        } else {
            gbp_comms.stage_ = gbp_comms_finalize;
        }
        break;

    case gbp_comms_finalize: {
        struct GBPComms reset = {0};
        gbp_comms = reset;
        return;
    }

    case gbp_comms_nintendo_handshake: {
        const u16 in_lower = gbp_comms.serial_in_;

        if (in_lower == 0x8002) {
            result = 0x10000010;
            gbp_comms.stage_ = gbp_comms_check_magic1;
            break;
        }

        if ((gbp_comms.serial_in_ >> 16) != gbp_comms.out_1_) {
            gbp_comms.index_ = 0;
        }

        if (gbp_comms.index_ > 3) {
            gbp_comms.out_0_ = 0x8000;
        } else {
            if (gbp_comms.serial_in_ ==
                (u32) ~(gbp_comms.out_1_ | (gbp_comms.out_0_ << 16))) {
                gbp_comms.index_ += 1;
            }

            static char const comms_handshake_data[] = {"NINTENDO"};

            gbp_comms.out_0_ =
                ((const u16*)comms_handshake_data)[gbp_comms.index_];
        }

        gbp_comms.out_1_ = ~in_lower;
        result = gbp_comms.out_1_;
        result |= gbp_comms.out_0_ << 16;
        break;
    }

    case gbp_comms_check_magic1:
        /* The GBATEK reference says to check for these integer constants in
           this order... but why? Who knows. Anyway, it seems to work. */
        if (gbp_comms.serial_in_ == 0x10000010) {
            result = 0x20000013;
            gbp_comms.stage_ = gbp_comms_check_magic2;
        } else {
            gbp_comms.stage_ = gbp_comms_finalize;
        }
        break;

    case gbp_comms_check_magic2:
        if (gbp_comms.serial_in_ == 0x20000013) {
            result = 0x40000004;
            gbp_comms.stage_ = gbp_comms_rumble;
        } else {
            gbp_comms.stage_ = gbp_comms_finalize;
        }
        break;
    }

    REG_SIODATA32 = result;
    REG_SIOCNT |= SIO_START;
}


void gba_rumble_init_gbp(struct GBARumbleGBPConfig config) {
    rumble_state = gba_rumble_stop;

    config.serial_irq_setup_(gbp_serial_isr);
    REG_RCNT = R_NORMAL;
    REG_SIOCNT = SIO_32BIT | SIO_SO_HIGH;
    REG_SIOCNT |= SIO_IRQ;
    gbp_configured = true;
    gbp_comms.serial_in_ = 0;
    gbp_comms.stage_ = gbp_comms_nintendo_handshake;
    gbp_comms.index_ = 0;
    gbp_comms.out_0_ = 0;
    gbp_comms.out_1_ = 0;
}

void gba_rumble_init_cart(enum GBARumbleCartType cart_type) {
    rumble_state = gba_rumble_stop;

    switch (cart_type) {
        case gba_rumble_cart_gpio:
            gba_rumble_gpio_init();
            cart_rumble_update_func = gba_rumble_gpio_update;
            break;
        case gba_rumble_cart_ezode:
            gba_rumble_ezode_init();
            cart_rumble_update_func = gba_rumble_ezode_update;
            break;
        case gba_rumble_cart_ez3in1:
            gba_rumble_ez3in1_init();
            cart_rumble_update_func = gba_rumble_ez3in1_update;
            break;

        default:
            return;
    }
    rumble_cart_type = cart_type;

    gba_rumble_update(gba_rumble_stop);
}

void gba_rumble_loop()
{
    if (gbp_configured) {
        gbp_serial_start();
    }
}

void gba_rumble_update(enum GBARumbleState state)
{
    rumble_state = state;

    if (!gbp_configured && rumble_cart_type != gba_rumble_cart_uninitialized) {
        if (!cart_rumble_update_func)
            return;

        const bool rumble_on = state == gba_rumble_start;

        cart_rumble_update_func(rumble_on);
    }
}

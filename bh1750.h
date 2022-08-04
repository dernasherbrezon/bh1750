#include <stdint.h>

typedef struct bh1750_t bh1750;

#define BH1750_ADDRESS_DEFAULT 0x23
#define BH1750_ADDRESS_ALT     0x5C

#define BH1750_MIN_MEASUREMENT_TIME     31
#define BH1750_DEFAULT_MEASUREMENT_TIME 69
#define BH1750_MAX_MEASUREMENT_TIME    254

#define BH1750_POWER_DOWN		0x00
#define BH1750_POWER_UP 		0x01
#define BH1750_RESET		    0x07

#define BH1750_CONTINUOUSLY_L_RES_MODE     0x13 // Start measurement at 4lx resolution. Measurement Time is typically 16ms.
#define BH1750_CONTINUOUSLY_H_RES_MODE     0x10 // Start measurement at 1lx resolution. Measurement Time is typically 120ms.
#define BH1750_CONTINUOUSLY_H_RES_MODE2    0x11 // Start measurement at 0.5lx resolution. Measurement Time is typically 120ms.

#define BH1750_ONE_TIME_L_RES_MODE     0x23 // Start measurement at 4lx resolution. Measurement Time is typically 16ms.
#define BH1750_ONE_TIME_H_RES_MODE     0x20 // Start measurement at 1lx resolution. Measurement Time is typically 120ms.
#define BH1750_ONE_TIME_H_RES_MODE2    0x21 // Start measurement at 0.5lx resolution. Measurement Time is typically 120ms.

int bh1750_create(const char *i2c_dev, int address, uint8_t mode, bh1750 **dev);

int bh1750_read(bh1750 *dev, float *value);

// Reset Data register value. Reset command is not acceptable in Power Down mode.
int bh1750_reset(bh1750 *dev);

int bh1750_set_measurement_time(bh1750 *dev, uint8_t value);

int bh1750_destroy(bh1750 *dev);
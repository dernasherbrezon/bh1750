#include "bh1750.h"
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

struct bh1750_t {
    int file;
    int address;

    uint8_t mode;
    uint8_t measurement_time;
    bool configured;
    struct timespec ts;
    float factor;
};

int bh1750_create(const char *i2c_dev, int address, uint8_t mode, bh1750 **dev) {
    switch (mode) {
        case BH1750_CONTINUOUSLY_L_RES_MODE:
        case BH1750_CONTINUOUSLY_H_RES_MODE:
        case BH1750_CONTINUOUSLY_H_RES_MODE2:
        case BH1750_ONE_TIME_L_RES_MODE:
        case BH1750_ONE_TIME_H_RES_MODE:
        case BH1750_ONE_TIME_H_RES_MODE2:
            break;
        default:
            errno = EINVAL;
            return -1;
    }
    struct bh1750_t *result = malloc(sizeof(struct bh1750_t));
    if (result == NULL) {
        return -ENOMEM;
    }
    // init all fields with 0 so that destroy_* method would work
    *result = (struct bh1750_t) {0};
    result->file = -1;
    result->mode = mode;
    result->address = address;
    result->measurement_time = BH1750_DEFAULT_MEASUREMENT_TIME;
    result->file = open(i2c_dev, O_RDWR);
    if (result->file < 0) {
        bh1750_destroy(result);
        return -1;
    }
    if (ioctl(result->file, I2C_SLAVE, address) < 0) {
        bh1750_destroy(result);
        return -1;
    }
    unsigned long funcs = 0;
    ioctl(result->file, I2C_FUNCS, &funcs);
    unsigned long expected = (I2C_FUNC_I2C | I2C_FUNC_SMBUS_WRITE_BYTE);
    if ((funcs & expected) != expected) {
        bh1750_destroy(result);
        return -1;
    }
    int code = bh1750_set_measurement_time(result, BH1750_DEFAULT_MEASUREMENT_TIME);
    if (code < 0) {
        bh1750_destroy(result);
        return code;
    }
    *dev = result;
    return 0;
}

int bh1750_read(bh1750 *dev, float *value) {
    // if continuous mode then setup only once
    // if one-time mode then setup every time
    if ((dev->mode & 0x20) || dev->configured == false) {
        __s32 code = i2c_smbus_write_byte(dev->file, dev->mode);
        if (code < 0) {
            return code;
        }
        // continuous modes
        if (dev->mode & 0x10) {
            dev->configured = true;
        }
    }

    if (nanosleep(&dev->ts, &dev->ts) != 0) {
        return -1;
    }

    __s32 ret = i2c_smbus_read_word_data(dev->file, dev->address);
    if (ret < 0) {
        return ret;
    }
    *value = (float) (ret >> 8 & 0xff | (ret << 8) & 0xff00) * dev->factor;
    return 0;
}

int bh1750_reset(bh1750 *dev) {
    // make sure mode is "power up"
    __s32 code = i2c_smbus_write_byte(dev->file, BH1750_POWER_UP);
    if (code < 0) {
        return code;
    }
    code = i2c_smbus_write_byte(dev->file, BH1750_RESET);
    if (code < 0) {
        return code;
    }
    return 0;
}

int bh1750_set_mtreg(bh1750 *dev, uint8_t value) {
    __s32 code = i2c_smbus_write_byte(dev->file, BH1750_POWER_DOWN);
    if (code < 0) {
        return code;
    }

    code = i2c_smbus_write_byte(dev->file, 0x40 | ((value & 0xE0) >> 5));
    if (code < 0) {
        return code;
    }

    code = i2c_smbus_write_byte(dev->file, 0x60 | (value & 0x1F));
    if (code < 0) {
        return code;
    }

    dev->measurement_time = value;
    return 0;
}

int bh1750_set_measurement_time(bh1750 *dev, uint8_t value) {
    if (value < BH1750_MIN_MEASUREMENT_TIME || value > BH1750_MAX_MEASUREMENT_TIME) {
        errno = EINVAL;
        return -1;
    }
    long maxMeasurementTime;
    if ((dev->mode & 0x3) == 0x3) {
        dev->factor = 1 / 1.2f * (BH1750_DEFAULT_MEASUREMENT_TIME / (float) value);
        maxMeasurementTime = (long) ((24.0f / BH1750_DEFAULT_MEASUREMENT_TIME) * (float) value);
    } else {
        dev->factor = 1 / 1.2f * (BH1750_DEFAULT_MEASUREMENT_TIME / (float) value);
        if (dev->mode & 0x01) {
            dev->factor = dev->factor / 2;
        }
        maxMeasurementTime = (long) ((180.0f / BH1750_DEFAULT_MEASUREMENT_TIME) * (float) value);
    }
    if (dev->measurement_time != value) {
        int code = bh1750_set_mtreg(dev, value);
        if (code != 0) {
            return code;
        }
    }
    dev->ts.tv_sec = 0;
    dev->ts.tv_nsec = maxMeasurementTime * 1000000;
    return 0;
}

int bh1750_destroy(bh1750 *dev) {
    if (dev->file >= 0) {
        if (dev->configured) {
            i2c_smbus_write_byte(dev->file, BH1750_POWER_DOWN);
        }

        if (dev->measurement_time != BH1750_DEFAULT_MEASUREMENT_TIME) {
            bh1750_set_mtreg(dev, BH1750_DEFAULT_MEASUREMENT_TIME);
        }

        int code = close(dev->file);
        if (code != 0) {
            return code;
        }
    }
    free(dev);
    return 0;
}
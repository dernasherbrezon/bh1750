#include "bh1750.h"
#include <stdlib.h>
#include <stdio.h>

void run(const char *message, uint8_t mode, uint8_t measurement_time) {
    bh1750 *dev = NULL;
    int code = bh1750_create("/dev/i2c-1", BH1750_ADDRESS_DEFAULT, mode, &dev);
    if (code < 0) {
        perror("unable to create sensor");
        return;
    }

    code = bh1750_set_measurement_time(dev, measurement_time);
    if (code < 0) {
        perror("unable to configure measurement time");
        return;
    }

    printf("%s\n", message);
    for (int i = 0; i < 10; i++) {
        float value = 0.0f;
        code = bh1750_read(dev, &value);
        if (code < 0) {
            perror("unable to read sensor");
            break;
        }
        printf("\t%f\n", value);
    }
    bh1750_destroy(dev);
}

int main(int argc, char **argv) {

    run("one time L  31", BH1750_ONE_TIME_L_RES_MODE, 31);
    run("one time H  31", BH1750_ONE_TIME_H_RES_MODE, 31);
    run("one time H2 31", BH1750_ONE_TIME_H_RES_MODE2, 31);

    run("one time L  69", BH1750_ONE_TIME_L_RES_MODE, 69);
    run("one time H  69", BH1750_ONE_TIME_H_RES_MODE, 69);
    run("one time H2 69", BH1750_ONE_TIME_H_RES_MODE2, 69);

    run("one time L  254", BH1750_ONE_TIME_L_RES_MODE, 254);
    run("one time H  254", BH1750_ONE_TIME_H_RES_MODE, 254);
    run("one time H2 254", BH1750_ONE_TIME_H_RES_MODE2, 254);

    return EXIT_SUCCESS;
}
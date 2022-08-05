## About

Linux userspace program to read sensor BH1750.

## Features

See [official documentation](docs/bh1750fvi-e-186247.pdf) for more details.

* "one time" and "continuous" modes
* set custom measurement time. This can decrease precision - suitable for sunny days. And increase precision up to 0.11 lux - for twilight.
* reset data register

See main.c for usage.

## Build

```bash
mkdir build
cd build
cmake ..
make
```

## Run

The main files are bh1750.c and bh1750.h. Copy/paste them into your project to use.

There is also main.c with some tests. It can be executed after the build:

```
./bh1750
```

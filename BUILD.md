# How to build the firmwares

## Prerequisites

You have two options to get a working compilation environment for HydraUSB3 : the Docker way or the native way.


### The Docker way (Linux/Windows)

You first need to install Docker : https://docs.docker.com/engine/install/.

Then simply go to the `tools/Docker` folder and launch :

on Linux

```shell
docker build -t hydradancer_compile --build-arg UID=$(id -u) .
```

on Windows
```shell
docker build -t hydradancer_compile --build-arg UID=1000 .
```

from the command line. Then go back to the top folder and execute

```shell
docker run --mount type=bind,source=.,target=/home/user/mounted -it hydradancer_compile
```

The working directory will be the current folder, so you have to be in the top folder.

from the folder you want to be mounted inside your Docker instance (for example, the top folder of wch-ch56x-lib). The two scripts `tools/Docker/build.sh` and `tools/Docker/run.sh` respectively will run the same commands on Linux.

### Native way (Linux only)

You will need to install the following tools on your system first : `git` `wget` `tar` `make` `cmake`.

#### GCC RISC-V 12.2 toolchain

Get the latest release from https://github.com/hydrausb3/riscv-none-elf-gcc-xpack/releases , e.g.

```shell
wget https://github.com/hydrausb3/riscv-none-elf-gcc-xpack/releases/download/12.2.0-1/xpack-riscv-none-elf-gcc-12.2.0-2-linux-x64.tar.gz
tar xf xpack-riscv-none-elf-gcc-12.2.0-2-linux-x64.tar.gz
```
To use it, add it to your path.
```shell
export PATH="$PATH:$(pwd)/xpack-riscv-none-elf-gcc-12.2.0-1/bin"
```

## Test firmwares compilation

1. Clone the project with all its submodules

```shell
git clone --recurse-submodules https://github.com/hydrausb3/wch-ch56x-lib
```

2. From the top folder, execute

```shell
cmake --toolchain ../cmake/wch-ch56x-toolchain.cmake -DCMAKE_BUILD_TYPE=release -DBUILD_TESTS=1 -DBUILD_TOOLS=1 -B build .
```

Then

```shell
cmake --build build/
```

And finally

```shell
cmake --install build/
```

3. You should find the compilation artefacts in `out/`

# Configuring the HydraUSB3 boards and flashing the firmware

## Physical configuration

Currently, you need two HydraUSB3 boards connected together via HSPI and SerDes.

- Optional : Add a jumper on PB24 on the top board (might be used by the firmware to determine if it is top or bottom board)

## Permissions

To be able to access the HydraUSB3 boards and flash them, root privileges may be required, or you can provide them to your regular user, e.g. with the creation of a file `/etc/udev/rules.d/99-hydrausb3.rules` with
```
# UDEV Rules for HydraUSB3 boards, https://github.com/hydrausb3
# WinChipHead CH569W Bootloader
SUBSYSTEMS=="usb", ATTRS{idVendor}=="4348", ATTRS{idProduct}=="55e0", MODE="0664", GROUP="plugdev"
```
and having your user as member of the group `plugdev`.


## Firmware Flashing

### Prerequisites

Clone https://github.com/hydrausb3/wch-ch56x-isp and follow the instructions.

### Flashing

Set top board in Flash Mode.
With a jumper on P3:

* Put a jumper on the top board (both boards are connected to the same jumper)
* Press & release reset button of the top board

Or with a button or momentary short on P3:

* Press Flash mode button (common to both boards)
* Press & release reset button of the top board
* Release Flash mode button

Warning! You have 10 seconds to flash the board! Note: root privileges may be required, see above.

```shell
wch-ch56x-isp -vf flash out/x/board1.bin
```

Press & release reset button of the top board again to leave Flash Mode. If you used a jumper, don't forget to remove it before resetting the board!

Repeat the same procedure with the bottom board but press the bottom board reset button and flash the proper firmware.

```shell
wch-ch56x-isp -vf flash out/x/board2.bin
```


# wch-ch56x-lib

This library was built alongside [Hydradancer](https://github.com/HydraDancer/hydradancer_fw), a new backend for the Facedancer USB emulation library based on the WCH569 chip.

Reliable drivers were needed for this project and many features were missing from the WCH examples or untested.

This library provides USB3, USB2, HSPI and SerDes drivers that have been tested using the benchmark/integrity tests in `tests/`. It is based on `wch-ch56x-bsp` but its goal is to provide a higher level library. As Hydradancer was using several peripherals at a time (USB3/HSPI, USB2/HSPI), an interrupt queue was implemented to avoid missing interrupts for use with `HSPIDeviceScheduled` along with a static memory pool. While the tests in this repository are simple enough to do the processing inside the interrupt handlers, Hydradancer was missing interrupts and deferring interrupts to user mode was required.

# Using this library in your project

In a CMake-based project, simply use `add_subdirectory(path_to_wch-ch56x-lib)` and link to the `wch-ch56x-lib` target. `wch-ch56x-lib` is currently an INTERFACE target, meaning the source/header files will be compiled with your target, not independently.

Available options

* LOG_LEVEL
* LOG_FILTER_IDS
* LOG_TYPE_PRINTF, LOG_TYPE_BUFFER, LOG_TYPE_SERDES
* POOL_BLOCK_SIZE, POOL_BLOCK_NUM
* INTERRUPT_QUEUE_SIZE

# Building the tests and compilation details

To build and flash the firmware, see [the build tutorial](BUILD.md).

More information on compilation options can also be found there.

# More details on compilation

This project uses one or two HydraUSB3 boards, which are based on the WCH569 RISCV MCU.

# Structure of the project

```
wch-ch56x-lib/
|   ├─  tests/ # test firmwares to test various parts of the library
|   ├─  src/
|   |   ├─wch-ch56x-lib # the source/header files of the lib
|   ├─  tools/
|   ├─  docs/ # additional documentation
```

# Tests

More information about the different tests and how to use them can be found in [docs/Testing.md](docs/Testing.md)

# How to contribute

If you encounter bugs or want to suggest new features, please check the existing issues and create a new issue if necessary.

To contribute code, it's recommended to check existing issues and inform the other contributors that you are working on something by commenting. This way you won't waste your time working on a feature that was not needed or was already in the works.

After that, please create a pull-request and link the related issue.

Your code should follow the [coding style](CODING_STYLE.md) guidelines.

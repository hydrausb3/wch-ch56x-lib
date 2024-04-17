# wch-ch56x-lib

This library was built alongside [Hydradancer](https://github.com/HydraDancer/hydradancer_fw), a new backend for the Facedancer USB emulation library based on the WCH569 chip.

Reliable drivers were needed for this project and many features were missing from the WCH examples, or untested.

This library provides USB3, USB2, HSPI and SerDes drivers that have been tested using the benchmark/integrity tests in `tests/`. It is based on `wch-ch56x-bsp` but its goal is to provide a higher level library. As Hydradancer was using several peripherals at a time (USB3/HSPI, USB2/HSPI), an interrupt queue was implemented to avoid missing interrupts for use with `HSPIDeviceScheduled` along with a static memory pool. While the tests in this repository are simple enough to do the processing inside the interrupt handlers, Hydradancer was missing interrupts and deferring interrupts to user mode was required.

# Using this library in your project

In a CMake-based project, simply use `add_subdirectory(path_to_wch-ch56x-lib)` and link to the `wch-ch56x-lib` target. `wch-ch56x-lib` is currently an INTERFACE target, meaning the source/header files will be compiled with your target, not independently.

Available options

* LOG_LEVEL
* LOG_FILTER_IDS
* LOG_TYPE_PRINTF, LOG_TYPE_BUFFER, LOG_TYPE_SERDES
* POOL_BLOCK_SIZE, POOL_BLOCK_NUM
* INTERRUPT_QUEUE_SIZE

# Building the tests

To build and flash the firmware, see [the build tutorial](BUILD.md).

# More details on compilation

This project uses one or two HydraUSB3 boards, which are based on the WCH569 RISCV MCU.

## Build options

Those options can be set the following way

```shell
cmake --toolchain ../cmake/wch-ch56x-toolchain.cmake -DCMAKE_BUILD_TYPE=release -DOPTION_1=1 -DOPTION_2=1 -B build .
```

- `-DCMAKE_BUILD_TYPE=Debug` use debug optimization
- `-DBUILD_TESTS=1` build the tests

Most warnings will be considered as errors.

## More options

You can set different options to activate more flags, static analysis or the logging system.

Those flags can either be set as build options (but they will apply to all projects) by passing a `-DOPTION=value` to CMake, or by adding a `set(option_name value)` in the project `CMakeLists.txt`.

- `STATIC_ANALYSIS` : activate GCC's built-in static analysers
- `EXTRACFLAGS` : activate -Wconversion and -Wsign-compare

## Logging options

Several logging options can get you infos on different parts of the library/firmwares. By default, no logs are activated so there is no impact on performances.

- Log methods
    - `LOG_OUTPUT=printf`. Logs are written directly to the UART
    - `LOG_OUTPUT=buffer`. Logs are stored in a buffer, and flushed to the UART when calling `LOG_DUMP()`
    - `LOG_OUTPUT=serdes`. Logs are directly sent using Serdes. Might be used to share logs from one board to the other.
- Log levels
    - `LOG_OUTPUT=x LOG_LEVEL=y`. With y=1(LOG_LEVEL_CRITICAL), y=2(LOG_LEVEL_ERROR), y=3(LOG_LEVEL_WARNING), y=4(LOG_LEVEL_INFO), y=5(LOG_LEVEL_DEBUG). All levels <=y will be displayed.
- Log filters
    - `LOG_OUTPUT=x LOG_FILTER_IDS=a,b,c, ...` You can set any number of filters from the following list 1(LOG_ID_USER), 2(LOG_ID_USB2), 3(LOG_ID_USB3), 4(LOG_ID_HSPI), 5(LOG_ID_SERDES), 6(LOG_ID_INTERRUPT_QUEUE), 7(LOG_ID_RAMX_ALLOC). If `LOG_LEVEL` is also defined, the logs with IDs will only be displayed if they have the right level.

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

There are no automated tests for now, but that doesn't mean tests are not required.

For now, the tests in hydradancer/tests consist in loop-back devices, to test for integrity and benchmark the speed of the device in different scenarios.

More information about the different scenarios can be found in [docs/Testing.md](docs/Testing.md).

# How to contribute

If you encounter bugs or want to suggest new features, please check the existing issues and create a new issue if necessary.

To contribute code, it's recommended to check existing issues and inform the other contributors that you are working on something by commenting. This way you won't waste your time working on a feature that was not needed or was already in the works.

After that, please create a pull-request and link the related issue.

Your code should follow the [coding style](CODING_STYLE.md) guidelines.

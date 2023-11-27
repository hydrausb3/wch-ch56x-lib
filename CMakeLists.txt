cmake_minimum_required(VERSION 3.17)
project(hydradancer LANGUAGES C ASM)

## Adding lwrb, setting compiler options

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/submodules/lwrb)

if (${RISCV_GCC_TOOLCHAIN_PREFIX} STREQUAL "riscv-none-embed-gcc")
    message("Using riscv-none-embed-gcc")
    target_compile_options(lwrb INTERFACE -march=rv32imac)
elseif(${RISCV_GCC_TOOLCHAIN_PREFIX} STREQUAL "riscv-none-elf-gcc")
    message("Using riscv-none-elf-gcc")
    target_compile_options(lwrb INTERFACE -march=rv32imac_zicsr)
else()
    message("Toolchain not found")
endif()

## Compiling wch-ch56x-bsp

add_library(wch-ch56x-bsp STATIC)
target_sources(wch-ch56x-bsp PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/submodules/wch-ch56x-bsp/board/hydrausb3_v1.c
    ${CMAKE_CURRENT_LIST_DIR}/submodules/wch-ch56x-bsp/drv/CH56x_bsp.c
    ${CMAKE_CURRENT_LIST_DIR}/submodules/wch-ch56x-bsp/drv/CH56x_bus8.c
    ${CMAKE_CURRENT_LIST_DIR}/submodules/wch-ch56x-bsp/drv/CH56x_clk.c
    ${CMAKE_CURRENT_LIST_DIR}/submodules/wch-ch56x-bsp/drv/CH56x_debug_log.c
    ${CMAKE_CURRENT_LIST_DIR}/submodules/wch-ch56x-bsp/drv/CH56x_dvp.c
    ${CMAKE_CURRENT_LIST_DIR}/submodules/wch-ch56x-bsp/drv/CH56x_ecdc.c
    ${CMAKE_CURRENT_LIST_DIR}/submodules/wch-ch56x-bsp/drv/CH56x_flash.c
    ${CMAKE_CURRENT_LIST_DIR}/submodules/wch-ch56x-bsp/drv/CH56x_gpio.c
    ${CMAKE_CURRENT_LIST_DIR}/submodules/wch-ch56x-bsp/drv/CH56x_hspi.c
    ${CMAKE_CURRENT_LIST_DIR}/submodules/wch-ch56x-bsp/drv/CH56x_pwm.c
    ${CMAKE_CURRENT_LIST_DIR}/submodules/wch-ch56x-bsp/drv/CH56x_pwr.c
    ${CMAKE_CURRENT_LIST_DIR}/submodules/wch-ch56x-bsp/drv/CH56x_serdes.c
    ${CMAKE_CURRENT_LIST_DIR}/submodules/wch-ch56x-bsp/drv/CH56x_spi.c
    ${CMAKE_CURRENT_LIST_DIR}/submodules/wch-ch56x-bsp/drv/CH56x_sys.c
    ${CMAKE_CURRENT_LIST_DIR}/submodules/wch-ch56x-bsp/drv/CH56x_timer.c
    ${CMAKE_CURRENT_LIST_DIR}/submodules/wch-ch56x-bsp/drv/CH56x_uart.c
    ${CMAKE_CURRENT_LIST_DIR}/submodules/wch-ch56x-bsp/rvmsis/core_riscv.c
    ${CMAKE_CURRENT_LIST_DIR}/submodules/wch-ch56x-bsp/startup/startup_CH56x.S
)

target_include_directories(wch-ch56x-bsp PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/submodules/wch-ch56x-bsp/board
    ${CMAKE_CURRENT_LIST_DIR}/submodules/wch-ch56x-bsp/drv
    ${CMAKE_CURRENT_LIST_DIR}/submodules/wch-ch56x-bsp/rvmsis
)

target_compile_options(wch-ch56x-bsp PRIVATE -std=gnu99 -MMD -MP -mabi=ilp32  -msmall-data-limit=8 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections)
target_compile_options(wch-ch56x-bsp PRIVATE $<$<CONFIG:Debug>:-Og> $<$<CONFIG:Release>:-Oz>)
target_link_options(wch-ch56x-bsp PRIVATE -T ".ld" -nostartfiles -Xlinker --gc-sections -Xlinker --print-memory-usage -Wl,-Map,"${PROJECT_NAME}.map" --specs=nano.specs --specs=nosys.specs)

if (${RISCV_GCC_TOOLCHAIN_PREFIX} STREQUAL "riscv-none-embed-gcc")
    message("Using riscv-none-embed-gcc")
    target_compile_options(wch-ch56x-bsp PRIVATE -march=rv32imac)
elseif(${RISCV_GCC_TOOLCHAIN_PREFIX} STREQUAL "riscv-none-elf-gcc")
    message("Using riscv-none-elf-gcc")
    target_compile_options(wch-ch56x-bsp PRIVATE -march=rv32imac_zicsr)
else()
    message("Toolchain not found")
endif()

## Adding nanoprintf as interface as it's header-only

add_library(nanoprintf INTERFACE)
target_include_directories(nanoprintf INTERFACE ${CMAKE_CURRENT_LIST_DIR}/submodules/nanoprintf)

## Adding hydradancer

add_subdirectory(src)

## Adding the tests

if (DEFINED BUILD_TESTS)
add_subdirectory(tests)
endif()

## Adding tools

if (DEFINED BUILD_TOOLS)
add_subdirectory(tools/firmware_debug_board)
endif()


/********************************** (C) COPYRIGHT *******************************
Copyright (c) 2022 Benjamin VERNOUX
Copyright (c) 2023 Quarkslab

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*******************************************************************************/

#ifndef LOG_SERDES_H
#define LOG_SERDES_H
// Disable warnings in bsp arising from -pedantic -Wall -Wconversion
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wvariadic-macros"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#include "CH56x_common.h"
#include "CH56xSFR.h"
#pragma GCC diagnostic pop
#pragma GCC diagnostic pop
#pragma GCC diagnostic pop

#include "wch-ch56x-lib/logging/logging_definitions.h"

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
* -> on transmitting end, use
        serdes_init(SERDES_TYPE_HOST, 4096);
        log_serdes_init(&log_buf_serdes);
* -> on receiving end, use
    serdes_init(SERDES_TYPE_DEVICE, 4096);

    and send the received logs, for example on usb endpoint 1

    void serdes_rx_callback(uint8_t* buffer, uint16_t size, uint16_t
custom_register)
    {
        endp1_tx_set_new_buffer(buffer, size);
    }
*/
void log_serdes_init(debug_log_buf_t* buf);

void vlog_serdes(const char* fmt, va_list args);

void log_serdes(const char* fmt, ...);

void _write_to_serdes(char* buffer, size_t size);

#ifdef __cplusplus
}
#endif

#endif

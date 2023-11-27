add_library(wch-ch56x-lib INTERFACE)

# With HSPIDeviceScheduled
target_sources(wch-ch56x-lib INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/HSPIDevice/hspi.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/interrupt_queue/interrupt_queue.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/logging/log_printf.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/logging/log_serdes.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/logging/log_to_buffer.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/logging/logging.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/logging/nanoprintf_impl.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/memory/alloc.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/memory/fifo.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/memory/ramx_alloc.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/SerDesDevice/serdes.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/USBDevice/usb_device.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/USBDevice/usb_endpoints.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/USBDevice/usb20.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/USBDevice/usb30.c
    )

target_include_directories(wch-ch56x-lib INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(wch-ch56x-lib INTERFACE wch-ch56x-bsp lwrb nanoprintf)

# Without HSPIDeviceScheduled

add_library(wch-ch56x-lib-scheduled INTERFACE)

target_sources(wch-ch56x-lib-scheduled INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/HSPIDeviceScheduled/hspi_scheduled.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/interrupt_queue/interrupt_queue.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/logging/log_printf.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/logging/log_serdes.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/logging/log_to_buffer.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/logging/logging.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/logging/nanoprintf_impl.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/memory/alloc.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/memory/fifo.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/memory/ramx_alloc.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/SerDesDevice/serdes.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/USBDevice/usb_device.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/USBDevice/usb_endpoints.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/USBDevice/usb20.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/USBDevice/usb30.c
    )

target_include_directories(wch-ch56x-lib-scheduled INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(wch-ch56x-lib-scheduled INTERFACE wch-ch56x-bsp lwrb nanoprintf)

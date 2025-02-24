add_library(wch-ch56x-lib INTERFACE)

# With hspi_scheduled
target_sources(wch-ch56x-lib INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/utils/critical_section.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/hspi/hspi.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/interrupt_queue/interrupt_queue.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/logging/log_printf.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/logging/log_serdes.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/logging/log_to_buffer.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/logging/logging.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/logging/nanoprintf_impl.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/memory/alloc.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/memory/ramx_alloc.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/serdes/serdes.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/usb/usb_device.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/usb/usb_endpoints.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/usb/usb20.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/usb/usb30.c
    )

target_include_directories(wch-ch56x-lib INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(wch-ch56x-lib INTERFACE wch-ch56x-bsp lwrb nanoprintf)

# Without hspi_scheduled

add_library(wch-ch56x-lib-scheduled INTERFACE)

target_sources(wch-ch56x-lib-scheduled INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/utils/critical_section.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/hspi_scheduled/hspi_scheduled.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/interrupt_queue/interrupt_queue.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/logging/log_printf.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/logging/log_serdes.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/logging/log_to_buffer.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/logging/logging.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/logging/nanoprintf_impl.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/memory/alloc.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/memory/ramx_alloc.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/serdes/serdes.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/usb/usb_device.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/usb/usb_endpoints.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/usb/usb20.c
    ${CMAKE_CURRENT_LIST_DIR}/wch-ch56x-lib/usb/usb30.c
    )

target_include_directories(wch-ch56x-lib-scheduled INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(wch-ch56x-lib-scheduled INTERFACE wch-ch56x-bsp lwrb nanoprintf)

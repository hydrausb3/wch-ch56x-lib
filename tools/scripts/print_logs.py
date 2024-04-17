#!/usr/bin/python3
# Copyright 2023 Quarkslab

"""
Read the first IN endpoint we find, in a loop, and decoding it as UTF-8.
This can be used together with firmware_debug_board and log_to_serdes to export logs from one board over USB.
"""
import sys
import usb.core
import usb.util

ENDP_BURST_SIZE = 4
TOTAL_TIME_NS = 0

if __name__ == "__main__":
    # find our device
    dev = usb.core.find(idVendor=0x16c0, idProduct=0x27d9)

    if dev is None:
        raise ValueError('Device not found')

    if dev.speed == usb.util.SPEED_HIGH:
        print("USB20 High-speed")
        ENDP_BURST_SIZE = 1
    elif dev.speed == usb.util.SPEED_SUPER:
        ENDP_BURST_SIZE = 4
        print(f"USB30 Superspeed burst {ENDP_BURST_SIZE}")

    print("Configuration of the device :")

    for cfg in dev:
        sys.stdout.write(str(cfg) + '\n')

    # get an endpoint instance
    cfg = dev.get_active_configuration()
    intf = cfg[(0, 0)]

    ep_in = list(usb.util.find_descriptor(
        intf,
        find_all=True,
        # match the first OUT endpoint
        custom_match=lambda e: \
        usb.util.endpoint_direction(e.bEndpointAddress) == \
        usb.util.ENDPOINT_IN))

    ep_out = list(usb.util.find_descriptor(
        intf,
        # match the first OUT endpoint
        find_all=True,
        custom_match=lambda e: \
        usb.util.endpoint_direction(e.bEndpointAddress) == \
        usb.util.ENDPOINT_OUT))

    assert ep_in is not None
    assert ep_out is not None

    print("Reading ...")

    ROUNDS = 4
    SUCCESS = True

    while 1:
        try:
            print(bytes(ep_in[0].read(ENDP_BURST_SIZE *
                  ep_in[0].wMaxPacketSize)).decode("utf-8"))
        except usb.USBError as e:
            if (e.errno == 19):
                print("device disconnected")
            pass

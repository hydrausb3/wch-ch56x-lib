#!/usr/bin/python3
# Copyright 2023 Quarkslab

"""
Test the integrity of data sent by a device on its EP OUT, sent back to us with its EP IN.
This time, randomize the size of individual packets, to check if the device can handle packet sizes different from its max.
"""

import sys
import time
import random
import usb.core
import usb.util
import array

ENDP_BURST_SIZE = 4
TOTAL_TIME_NS = 0
BUFFER_SIZE = int(100 * 1e3)


def check(byte_array, packet_size, reference_array):
    """
    Check the received buffers against what has been sent, to check for integrity
    """
    for i in range(len(byte_array) // packet_size):
        packet_in = byte_array[i * packet_size:i*packet_size + packet_size]
        packet_ref = reference_array[i *
                                     packet_size:i*packet_size + packet_size]
        if packet_in != packet_ref:
            print(f"Error at {i} \r\n")
            print(packet_in)
            print(packet_ref)
            return False
            break
    return True


def send_next_packet(head, ep_in, ep_out, endp_max_packet_size, buffer_in, buffer_out):
    """
    Send next packet from buffer_out at head, receive it back in buffer_in.
    Saves the time diff in TOTAL_TIME_NS, to try to remove python processing from the measurement.
    """
    global TOTAL_TIME_NS
    start = time.time_ns()
    packet_size = random.randint(
        0, min(endp_max_packet_size, len(buffer_out) - head))
    ep_out.write(buffer_out[head:head + packet_size])
    buf = ep_in.read(packet_size)
    stop = time.time_ns()
    TOTAL_TIME_NS += (stop - start)

    num_read = len(buf)
    if num_read != packet_size:
        print("retrying packet ! \r\n")
        return send_next_packet(head, ep_in, ep_out, endp_max_packet_size, buffer_in, buffer_out)
    buffer_in[head:head + packet_size] = buf
    head += num_read
    return head


if __name__ == "__main__":
    # find our device
    dev = usb.core.find(idVendor=0x16c0, idProduct=0x27d8)

    if dev is None:
        raise ValueError('Device not found')

    if dev.speed == usb.util.SPEED_SUPER:
        ENDP_BURST_SIZE = 4
        print(f"USB30 Superspeed burst {ENDP_BURST_SIZE}")
    else:
        print("USB20")
        ENDP_BURST_SIZE = 1

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
        custom_match=lambda e:
        usb.util.endpoint_direction(e.bEndpointAddress) ==
        usb.util.ENDPOINT_IN))

    ep_out = list(usb.util.find_descriptor(
        intf,
        # match the first OUT endpoint
        find_all=True,
        custom_match=lambda e:
        usb.util.endpoint_direction(e.bEndpointAddress) ==
        usb.util.ENDPOINT_OUT))

    assert ep_in is not None
    assert ep_out is not None
    # assert ep_out.wMaxPacketSize == ep_in.wMaxPacketSize

    print("Reading ...")

    ROUNDS = 4
    SUCCESS = True

    fails = []
    for i in range(len(ep_in)):
        TOTAL_TIME_NS = 0
        print(f"EP {i}")
        try:
            endp_max_packet_size = ENDP_BURST_SIZE * ep_out[i].wMaxPacketSize

            buffer_out = array.array(
                'B', [int(random.random() * 255) for i in range(BUFFER_SIZE)])
            buffer_in = array.array('B', [0 for i in range(BUFFER_SIZE)])

            START = time.time_ns()
            head = 0
            while head < len(buffer_out):
                try:
                    head = send_next_packet(
                        head, ep_in[i], ep_out[i], endp_max_packet_size, buffer_in, buffer_out)
                except usb.core.USBTimeoutError:
                    head = send_next_packet(
                        head, ep_in[i], ep_out[i], endp_max_packet_size, buffer_in, buffer_out)

            STOP = time.time_ns()

            if check(buffer_in, endp_max_packet_size,  buffer_out):
                print(
                    f"Success ! Transfer rate with python processing {len(buffer_in) / ((STOP - START) * 1e-9) * 1e-6} MB/s")
                print(
                    f"Success ! Transfer rate with only transfer {len(buffer_in) / ((TOTAL_TIME_NS) * 1e-9) * 1e-6} MB/s")
            else:
                print("Error")
                fails.append(i)
        except Exception as e:
            print(e)
            fails.append(i)

print(
    f"There have been {len(fails)} fails. Endpoints {[ep + 1 for ep in fails]} failed \r\n")

if len(fails) == 0:
    print("Test successful ! \r\n")

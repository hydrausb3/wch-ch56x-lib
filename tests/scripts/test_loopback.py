#!/usr/bin/python3
# Copyright 2023 Quarkslab

"""
Test the integrity of data sent by a device on its EP OUT, sent back to us with its EP IN.
"""
import sys
import time
import random
import array
import usb.core
import usb.util
import argparse

ENDP_BURST_SIZE = 4
TOTAL_TIME_NS = 0
BUFFER_SIZE = int(100*1e3)


def check(byte_array, packet_size, reference_array):
    """
    Check the received buffers against what has been sent, to check for integrity
    """
    for j in range(len(byte_array) // packet_size):
        packet_in = byte_array[j * packet_size:j*packet_size + packet_size]
        packet_ref = reference_array[j *
                                     packet_size:j*packet_size + packet_size]
        if packet_in != packet_ref:
            print(f"Error at {j} ")
            print(packet_in)
            print(packet_ref)
            return False
    return True


def send_next_packet(head, ep_in, ep_out, endp_max_packet_size, buffer_in, buffer_out):
    global TOTAL_TIME_NS
    start = time.time_ns()
    packet_size = min(endp_max_packet_size, len(buffer_out) - head)
    # print(f"sending {packet_size} bytes on {ep_out}")
    ep_out.write(buffer_out[head:head + packet_size])
    # print(f"reading on {ep_in}")
    buf = ep_in.read(packet_size)
    stop = time.time_ns()
    TOTAL_TIME_NS += (stop - start)

    num_read = len(buf)
    if num_read != packet_size:
        print("retrying packet ! ")
        return send_next_packet(head, ep_in, ep_out, endp_max_packet_size, buffer_in, buffer_out)
    buffer_in[head:head + packet_size] = buf
    head += num_read
    return head


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--device_num", default=0,
                        help="In case multiple devices are found, select the index of the device that will be used.", type=int)
    parser.add_argument(
        "--zlp", help="Only send packets of size 0", action='store_true')
    args = parser.parse_args()

    if args.zlp:
        BUFFER_SIZE = 0

    # find our device
    devs = usb.core.find(idVendor=0x16c0, idProduct=0x27d8, find_all=True)

    if devs is None:
        raise ValueError('Device not found')

    dev = list(devs)[args.device_num]

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
    for i, (current_ep_in, current_ep_out) in enumerate(zip(ep_in, ep_out)):
        TOTAL_TIME_NS = 0
        print(f"EP {current_ep_in.bEndpointAddress & 0x7f}")
        try:
            endp_max_packet_size = ENDP_BURST_SIZE * ep_out[i].wMaxPacketSize
            buffer_out = array.array(
                'B', [int(random.random() * 255) for i in range(BUFFER_SIZE)])
            buffer_in = array.array('B', [0 for i in range(BUFFER_SIZE)])

            START = time.time_ns()
            head = 0
            num_sent = 0
            total_to_send = len(buffer_out)

            if BUFFER_SIZE == 0:
                send_next_packet(
                    head, current_ep_in, current_ep_out, endp_max_packet_size, buffer_in, buffer_out)
            else:
                while head < total_to_send:
                    try:
                        head = send_next_packet(
                            head, current_ep_in, current_ep_out, endp_max_packet_size, buffer_in, buffer_out)
                        sys.stdout.write(
                            f"\r{100. * head/total_to_send} % sent")
                    except usb.core.USBTimeoutError:  # HydraUSB3 tends to timeout when handling USB3
                        print("error timeout, retrying ! ")

            STOP = time.time_ns()
            sys.stdout.write("\r")
            if check(buffer_in, endp_max_packet_size,  buffer_out):
                print(
                    f"Success ! Transfer rate with python processing {len(buffer_in) / ((STOP - START) * 1e-9) * 1e-6} MB/s")
                print(
                    f"Success ! Transfer rate with only transfer {len(buffer_in) / ((TOTAL_TIME_NS) * 1e-9) * 1e-6} MB/s")
            else:
                print("Error")
                fails.append(current_ep_in.bEndpointAddress & 0x7f)
        except:
            fails.append(current_ep_in.bEndpointAddress & 0x7f)

print(
    f"There have been {len(fails)} fails. Endpoints {fails} failed ")

if len(fails) == 0:
    print("Test successful ! ")

#
# This file is based on Facedancer.
#
# BSD-3-Clause license
# Copyright (c) 2024 Quarkslab
# Copyright (c) 2019 Katherine J. Temkin <k@ktemkin.com>
# Copyright (c) 2018 Dominic Spill <dominicgs@gmail.com>
# Copyright (c) 2018 Travis Goodspeed <travis@radiantmachines.com>

# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:

# 1. Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.

# 2. Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation and/or
# other materials provided with the distribution.

# 3. Neither the name of the copyright holder nor the names of its contributors
# may be used to endorse or promote products derived from this software without
# specific prior written permission.

# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

import logging
import random
import unittest
import usb1

# How many iterations to run for stress test
ITERATIONS = 20000

logging.basicConfig(level=logging.DEBUG)

# - helpers -------------------------------------------------------------------


def generate_data(length):
    return bytes([(byte % 256) for byte in range(length)])

# Transfer length for tests


def test_transfer_length():
    return random.randrange(1, MAX_TRANSFER_LENGTH)


VENDOR_ID = 0x1209
PRODUCT_ID = 0x0001

OUT_ENDPOINT = 0x01
IN_ENDPOINT = 0x82

# This is constrained by pygreat::comms_backends::usb1::LIBGREAT_MAX_COMMAND_SIZE
# and is board dependent.
MAX_TRANSFER_LENGTH = 768


class FacedancerTestCase(unittest.TestCase):

    # - life-cycle ------------------------------------------------------------

    @classmethod
    def setUpClass(cls):
        logging.basicConfig(level=logging.INFO)
        cls.context = usb1.USBContext().open()
        cls.device = cls.context.getByVendorIDAndProductID(
            VENDOR_ID, PRODUCT_ID)
        cls.device_handle = cls.device.open()
        if cls.device_handle is None:
            raise Exception("device not found")
        cls.device_handle.claimInterface(0)
        cls.device_speed = cls.device.getDeviceSpeed()

    @classmethod
    def tearDownClass(cls):
        cls.context.close()

    # - transfers -------------------------------------------------------------

    def bulk_out_transfer(self, data):
        logging.debug("Testing bulk OUT endpoint")
        response = self.device_handle.bulkWrite(
            endpoint=0x01,
            data=data,
            timeout=1000,
        )
        logging.debug("sent %d bytes\n", response)
        return response

    def bulk_in_transfer(self, length):
        logging.debug("Testing bulk IN endpoint")
        response = self.device_handle.bulkRead(
            endpoint=0x82,
            length=length,
            timeout=1000,
        )
        logging.debug(
            "[host] received %d bytes from bulk endpoint", len(response))
        return response

    def interrupt_out_transfer(self, data):
        logging.debug("Testing interrupt OUT endpoint")
        response = self.device_handle.interruptWrite(
            endpoint=0x01,
            data=data,
            timeout=0 if self.device_speed == usb1.SPEED_LOW else 1000,
        )
        logging.debug("sent %d bytes\n", response)
        return response

    def interrupt_in_transfer(self, length):
        logging.debug("Testing interrupt IN endpoint")
        response = self.device_handle.interruptRead(
            endpoint=0x82,
            length=length,
            timeout=0 if self.device_speed == usb1.SPEED_LOW else 1000,
        )
        logging.debug(
            "[host] received %d bytes from interrupt endpoint", len(response))
        return response

    def control_out_transfer(self, data):
        logging.debug("Testing OUT control transfer")
        hi, lo = len(data).to_bytes(2, byteorder="big")
        response = self.device_handle.controlWrite(
            request_type=usb1.TYPE_VENDOR | usb1.RECIPIENT_DEVICE,
            request=10,
            index=hi,
            value=lo,
            data=data,
            timeout=0 if self.device_speed == usb1.SPEED_LOW else 1000,
        )
        logging.debug("sent %d bytes\n", response)
        return response

    def control_in_transfer(self, length):
        logging.debug("Testing IN control transfer")
        hi, lo = length.to_bytes(2, byteorder="big")
        response = self.device_handle.controlRead(
            request_type=usb1.TYPE_VENDOR | usb1.RECIPIENT_DEVICE,
            request=20,
            index=hi,
            value=lo,
            length=length,
            timeout=0 if self.device_speed == usb1.SPEED_LOW else 1000,
        )
        logging.debug(
            "[host] received %d bytes from control endpoint", len(response))
        return response

    # - device control ------------------------------------------------------------

    def set_in_transfer_length(self, length):
        hi, lo = length.to_bytes(2, byteorder="big")
        logging.debug("Setting transfer length to %d bytes", length)
        response = self.device_handle.controlWrite(
            request_type=usb1.TYPE_VENDOR | usb1.RECIPIENT_DEVICE,
            request=1,
            index=hi,
            value=lo,
            data=[],
            timeout=0 if self.device_speed == usb1.SPEED_LOW else 1000,
        )
        return response

    def get_last_out_transfer_data(self):
        logging.debug("Getting last OUT transfer data")
        response = self.device_handle.controlRead(
            request_type=usb1.TYPE_VENDOR | usb1.RECIPIENT_DEVICE,
            request=2,
            index=0,
            value=0,
            length=MAX_TRANSFER_LENGTH,
            timeout=0 if self.device_speed == usb1.SPEED_LOW else 1000,
        )
        logging.debug(
            "[host] sent %d bytes with last out transfer", len(response))
        return response

    def reset_device_state(self):
        logging.debug(f"Resetting stress test device state")
        response = self.device_handle.controlWrite(
            request_type=usb1.TYPE_VENDOR | usb1.RECIPIENT_DEVICE,
            request=3,
            index=0,
            value=0,
            data=[],
            timeout=0 if self.device_speed == usb1.SPEED_LOW else 1000,
        )
        return response


# Run tests in random order
#
# Note: if you can't reproduce a failed run check the order of the
#       tests in the failed run!
unittest.TestLoader.sortTestMethodsUsing = lambda self, a, b: random.choice([
                                                                            1, 0, -1])


class TestTransfers(FacedancerTestCase):
    """Transfer tests for test device"""

    # - life-cycle ------------------------------------------------------------

    def setUp(self):
        # reset test device state between tests
        self.reset_device_state()

    # - transfer checks -------------------------------------------------------

    def check_out_transfer(self, length, sent_data, bytes_sent):
        # request a copy of the received data to compare against
        received_data = self.get_last_out_transfer_data()

        # did we send the right amount of data?
        self.assertEqual(bytes_sent, length)

        # does the length of the sent data match the length of the received data?
        self.assertEqual(len(sent_data), len(received_data))

        # does the content of the sent data match the content of the received data?
        self.assertEqual(sent_data, received_data)

    def check_in_transfer(self, length, received_data):
        # generate a set of data to compare against
        compare_data = generate_data(length)

        # did we receive the right amount of data?
        self.assertEqual(len(received_data), length)

        # does the content of the received data match the content of our comparison data?
        self.assertEqual(received_data, compare_data)

    # - tests -----------------------------------------------------------------

    def test_bulk_out_transfer(self):
        # generate test data
        length = test_transfer_length()
        data = generate_data(length)

        # perform Bulk OUT transfer
        bytes_sent = self.bulk_out_transfer(data)

        # check transfer
        self.check_out_transfer(length, data, bytes_sent)

    def test_bulk_in_transfer(self):
        # set desired IN transfer length
        length = test_transfer_length()
        self.set_in_transfer_length(length)

        # perform Bulk IN transfer
        received_data = self.bulk_in_transfer(length)

        # check transfer
        self.check_in_transfer(length, received_data)

    def test_interrupt_out_transfer(self):
        # generate test data
        length = test_transfer_length()
        data = generate_data(length)

        # perform Interrupt OUT transfer
        bytes_sent = self.interrupt_out_transfer(data)

        # check transfer
        self.check_out_transfer(length, data, bytes_sent)

    def test_interrupt_in_transfer(self):
        # set desired IN transfer length
        length = test_transfer_length()
        self.set_in_transfer_length(length)

        # perform Bulk IN transfer
        received_data = self.interrupt_in_transfer(length)

        # check transfer
        self.check_in_transfer(length, received_data)

    def test_control_out_transfer(self):
        # generate test data
        length = test_transfer_length()
        data = generate_data(length)

        # perform Control OUT transfer
        bytes_sent = self.control_out_transfer(data)

        # check transfer
        self.check_out_transfer(length, data, bytes_sent)

    def test_control_in_transfer(self):
        # set desired IN transfer length
        length = test_transfer_length()
        self.set_in_transfer_length(length)

        # perform Bulk IN transfer
        received_data = self.control_in_transfer(length)

        # check transfer
        self.check_in_transfer(length, received_data)


def highly_stressed_edition():
    available_tests = []
    with usb1.USBContext() as context:
        device = context.getByVendorIDAndProductID(VENDOR_ID, PRODUCT_ID)
        # No BULK endpoint in LS
        if device.getDeviceSpeed() == usb1.SPEED_LOW:
            available_tests = [
                "test_interrupt_out_transfer",
                "test_interrupt_in_transfer",
                "test_control_out_transfer",
                "test_control_in_transfer",
            ]
        else:
            available_tests = [
                "test_bulk_out_transfer",
                "test_bulk_in_transfer",
                "test_control_out_transfer",
                "test_control_in_transfer",
            ]

    tests = [random.choice(available_tests) for _ in range(ITERATIONS)]

    suite = unittest.TestSuite()
    for test in tests:
        suite.addTest(TestTransfers(test))

    runner = unittest.TextTestRunner(failfast=True)
    runner.run(suite)


if __name__ == "__main__":
    highly_stressed_edition()

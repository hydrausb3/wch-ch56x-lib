# Testing

## Running the scripts

First, you need to create a virtual env or reuse one.

Go to the `tests/scripts` folder.

```shell
sudo apt install python3 python3-venv
python3 -m venv venv
```
Activate the virtual environment.

```shell
source venv/bin/activate
```

Install the dependencies.

```shell
pip install requirements.txt
```

Run the scripts.

```shell
python3 script.py
```

or if you need root privileges (if the script does not work, try as follows)

```shell
sudo ./venv/bin/python3 script.py
```
## Tests description

During the development of this project, several tests have been created to check if everything was working as intended.

Different firmwares have been created, along with Python scripts to execute the tests. All that can be found in `tests`.

Below are the different test cases and how to test them:

### Unittests

* Compile : compile the tests with `-DBUILD_TESTS=1`
* Run : flash to one board, and read its UART.

The firmware will simply execute a list of functions returning either `true` or `false`, to check if the test has passed.

The UART will indicate if the test has passed, along with some informative logs from the tests.

Those "unittests" are a way to get some certainty about edge cases of some parts of the code (like the interrupt_queue or the memory pool).

### HSPI

* Compile : compile the tests with `-DBUILD_TESTS=1`
* Run : flash to both boards, the jumper is used to differentiate the boards. Run `test_hspi_serdes.py`.

The test simply sends data from one board to the other using HSPI, then exports it to the host to check integrity.

### SerDes

* Compile : compile the tests with `-DBUILD_TESTS=1`
* Run : flash to both boards, the jumper is used to differentiate the boards. Run `test_hspi_serdes.py`.

The test simply sends data from one board to the other using SerDes, then exports it to the host to check integrity.

### Loopback

* Compile : compile the tests with `-DBUILD_TESTS=1`
* Run : flash to both boards, the jumper is used to differentiate the boards. Run `test_loopback.py` or `test_loopback_randomize.py`.

`test_loopback.py` will send data to the first board via USB, which will transmit it to the second board using HSPI, then receive it back with SerDes, and back to the host via USB. The script will then check for integrity.

Run `test_loopback_randomize.py` to send packets with variable sizes, to check if the boards can handle various packet sizes.

The goal here is to check that all parts of the data loop works, although it does not use HSPI as half-duplex and does not use the interrupt_queue.

### USB loopack

* Compile : compile the tests with `-DBUILD_TESTS=1`
* Run : flash to one board. Run `test_loopback.py` or `test_loopback_randomize.py`.

`test_loopback.py` will send data to the board via USB, which will send it back. The test will then check the integrity of the transmission. The test is repeated for all 7 IN/OUT endpoints pairs.

Run `test_loopback_randomize.py` to send packets with variable sizes, to check if the board can handle various packet sizes.

The goal is to test if the USB3 and USB2 peripherals are working correctly.

By default, the USB3 peripheral is active but you can switch to USB2 by maintaining the user button while resetting the board.

### USB speedtest

* Compile : compile the tests with `-DBUILD_TESTS=1`
* Run : flash to one board. Run `test_speedtest.py` or `test_speedtest_one_by_one.py`.

`test_speedtest.py` will first send data to the board in one go (one call to libusb), then read data from the board in one go (one call to libusb).

The goal is to check the capabilities of the USB3 and USB2 peripherals.

Note that this test only works because packets of the maximum size are exchanged, not short packets. A short packet would interrupt the transfer immediately.

Thus, `test_speedtest_one_by_one.py` sends and reads data one packet (with burst) at a time, to test the case where individual transfers are used. In that case, the test firmware could be modified to send short packets (packets of size less than the maximum packet size).

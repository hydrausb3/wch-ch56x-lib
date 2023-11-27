# Note on IN burst transfers in USB3

Here are some extract from the USB 3.2 Revision 1.1, concerning IN transactions


- NumP field for ERDY TP: For an IN endpoint this field is set by the endpoint to the number of packets
it can transmit when the host resumes transactions to it. This field shall not
have a value greater than the maximum burst size supported by the endpoint
as indicated by the value in the bMaxBurst field in the Endpoint Companion
Descriptor. Note that the value reported in this field may be treated by the
host as informative only.


-Short packets: When the host or a device receives a DP with the Data Length field shorter than the
maximum packet size for that endpoint it shall deem that transfer is complete.
In the case of an IN transfer, a device shall stop sending DPs after sending a short DP. The
host shall respond to the short DP with an ACK TP with the NumP field set to zero unless it
has another transfer for the same endpoint in which case it may set the NumP field as
mentioned in Section 8.10.2. The host shall schedule transactions to the endpoint on the
device when another transfer is initiated for that endpoint.

- End of burst sign : If the host asks for multiple DPs from a device and the device does not have that number of
DPs available to send at the time, the device shall send the last DP with the End Of Burst
flag in the DPH set to one. Note that it is not necessary to set the End Of Burst flag if the DP
sent to the host has a payload that is less than the MaxPacketSize for that endpoint.

- When a transfer ends : A transfer is complete when a device sends all the data that is expected by the host or it
sends a DP with a payload that is less than the MaxPacketSize.

To summarize :

In a IN transaction, here are the respective capacities of each end:
-> host : can set NumP in the ACK TP, defining the number of packets it can receive
-> device : send NRDY when not ready, ERDY when ready, set lpf (end of burst sign) or send short packet (size less than max packet size) to signify it has no packets to send anymore.

The host does not know how much it will receive, it can only set the max number of packets it can receive. That's why it should always be a multiple of the max packet size, to avoid overflows.

When the device sends a short packet, the transfer (if you use the synchronous API of libusb, this means the call to your function) is considered to be finished. This means that if you expect libusb to read 5000 bytes in one go, and your device sends packets of size 500 bytes, you will only receive a short packet (500 bytes).

The end of burst, however, does not indicate the end of a transfer, but simply the end of the current burst. This makes the host enter the "flow control state", which means the host will only resume the transfer after receiving an ERDY.

Helpful links :
- https://billauer.co.il/blog/2019/12/usb-bulk-overflow-halt-reset/
- https://xillybus.com/tutorials/usb-superspeed-transfers-bursts-short-packets
- https://www.usb.org/document-library/usb-32-revision-11-june-2022

#!/usr/bin/env python3
# Copyright 2023 Quarkslab

from datetime import timedelta
import string
import numpy as np
import pandas as pd
from matplotlib import pyplot as plt
import matplotlib

FILE = "usb2_irq_handler.dump"
START_NAME = "USBHS_IRQHandler-start"
END_NAME = "USBHS_IRQHandler-end"

start_data = []
end_data = []

with open(FILE, "r") as f:
    matplotlib.use('QtAgg')
    for line in f.readlines():
        # remove non printable caracters
        line = "".join(filter(lambda x: x in string.printable, line))
        line_infos = line.removesuffix("\n").split(" ")
        s, ms, us = int(line_infos[0].removesuffix("s")), int(
            line_infos[1].removesuffix("ms")), int(line_infos[2].removesuffix("us"))
        name = line_infos[-1]
        time_info = timedelta(seconds=s, milliseconds=ms, microseconds=us)
        if name == START_NAME:
            start_data.append(time_info)
        elif name == END_NAME:
            end_data.append(time_info)

    result = pd.Series(np.array(end_data) -
                       np.array(start_data)).astype('timedelta64[us]')
    result.plot()
    plt.show()

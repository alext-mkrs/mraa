/*
 * Author: Brendan Le Foll <brendan.le.foll@intel.com>
 * Copyright (c) 2015 Intel Corporation.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "iio.h"
#include "mraa_internal.h"

mraa_iio_context
mraa_iio_init(int device)
{
    if (device > plat->iio_device_count) {
        return NULL;
    }
    return &plat->iio_devices[device];
}

int
mraa_iio_get_channel_count(mraa_iio_context dev)
{
    return 1;
}

mraa_result_t
mraa_iio_read(mraa_iio_context dev, int channel, const char* attribute, float* data)
{
    char buf[64];
    snprintf(buf, 64, "/sys/bus/iio/devices/iio:device%d/in_voltage%d_%s", dev->num, channel, attribute);
    int fd = open(buf, O_RDONLY);
    if (fd != -1) {
        int len = read(fd, &buf, 64);
        *data = strtol(buf, NULL, 10);
        return MRAA_SUCCESS;
    }
    return MRAA_ERROR_UNSPECIFIED;
}

mraa_result_t
mraa_iio_write(mraa_iio_context dev, int channel, const char* attribute)
{
    return MRAA_ERROR_FEATURE_NOT_IMPLEMENTED;
}

mraa_result_t
mraa_iio_stop(mraa_iio_context dev)
{
    free(dev);
    return MRAA_SUCCESS;
}


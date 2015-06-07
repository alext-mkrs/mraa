/*
 * Author: Nandkishor Sonar
 * Copyright (c) 2014 Intel Corporation.
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

#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

#include "aio.h"
#include "mraa_internal.h"

#define DEFAULT_BITS 10

static int raw_bits;

static mraa_result_t
aio_get_valid_fp(mraa_aio_context dev)
{
    if (advance_func->aio_get_valid_fp != NULL)
        return advance_func->aio_get_valid_fp(dev);

    char file_path[64] = "";

    // Open file Analog device input channel raw voltage file for reading.
    snprintf(file_path, 64, "/sys/bus/iio/devices/iio:device0/in_voltage%d_raw", dev->channel);

    dev->adc_in_fp = open(file_path, O_RDONLY);
    if (dev->adc_in_fp == -1) {
        syslog(LOG_ERR, "aio: Failed to open input raw file %s for reading!", file_path);
        return MRAA_ERROR_INVALID_RESOURCE;
    }

    return MRAA_SUCCESS;
}

mraa_aio_context
mraa_aio_init(unsigned int aio)
{
    if (plat == NULL) {
        syslog(LOG_ERR, "aio: Platform not initialised");
        return NULL;
    }
    if (advance_func->aio_init_pre != NULL) {
        mraa_result_t pre_ret = (advance_func->aio_init_pre(aio));
        if (pre_ret != MRAA_SUCCESS)
            return NULL;
    }
    if (aio > plat->aio_count) {
        syslog(LOG_ERR, "aio: requested channel out of range");
        return NULL;
    }

    int pin = aio + plat->gpio_count;

    if (plat->pins[pin].capabilites.aio != 1) {
        syslog(LOG_ERR, "aio: pin uncapable of aio");
        return NULL;
    }

    if (plat->pins[pin].aio.mux_total > 0) {
        if (mraa_setup_mux_mapped(plat->pins[pin].aio) != MRAA_SUCCESS) {
            syslog(LOG_ERR, "aio: unable to setup multiplexers for pin");
            return NULL;
        }
    }

    // Create ADC device connected to specified channel
    mraa_aio_context dev = malloc(sizeof(struct _aio));
    if (dev == NULL) {
        syslog(LOG_ERR, "aio: Insufficient memory for specified input channel "
                        "%d\n",
               aio);
        return NULL;
    }
    dev->channel = plat->pins[pin].aio.pinmap;
    dev->value_bit = DEFAULT_BITS;

    // Open valid  analog input file and get the pointer.
    if (MRAA_SUCCESS != aio_get_valid_fp(dev)) {
        free(dev);
        return NULL;
    }
    raw_bits = mraa_adc_raw_bits();

    if (advance_func->aio_init_post != NULL) {
        mraa_result_t ret = advance_func->aio_init_post(dev);
        if (ret != MRAA_SUCCESS) {
            free(dev);
            return NULL;
        }
    }

    return dev;
}

unsigned int
mraa_aio_read(mraa_aio_context dev)
{
    char buffer[17];
    unsigned int shifter_value = 0;

    if (dev->adc_in_fp == -1) {
        if (aio_get_valid_fp(dev) != MRAA_SUCCESS) {
            syslog(LOG_ERR, "aio: Failed to get to the device");
            return 0;
        }
    }

    lseek(dev->adc_in_fp, 0, SEEK_SET);
    if (read(dev->adc_in_fp, buffer, sizeof(buffer)) < 1) {
        syslog(LOG_ERR, "aio: Failed to read a sensible value");
    }
    // force NULL termination of string
    buffer[16] = '\0';
    lseek(dev->adc_in_fp, 0, SEEK_SET);

    errno = 0;
    char* end;
    unsigned int analog_value = (unsigned int) strtoul(buffer, &end, 10);
    if (end == &buffer[0]) {
        syslog(LOG_ERR, "aio: Value is not a decimal number");
    } else if (errno != 0) {
        syslog(LOG_ERR, "aio: Errno was set");
    }

    if (dev->value_bit != raw_bits) {
        /* Adjust the raw analog input reading to supported resolution value*/
        if (raw_bits > dev->value_bit) {
            shifter_value = raw_bits - dev->value_bit;
            analog_value = analog_value >> shifter_value;
        } else {
            shifter_value = dev->value_bit - raw_bits;
            analog_value = analog_value << shifter_value;
        }
    }

    return analog_value;
}

float
mraa_aio_read_float(mraa_aio_context dev)
{
    if (dev == NULL) {
        syslog(LOG_ERR, "aio: Device not valid");
        return 0.0;
    }

    float max_analog_value = (1 << dev->value_bit) - 1;
    unsigned int analog_value_int = mraa_aio_read(dev);

    return analog_value_int / max_analog_value;
}

mraa_result_t
mraa_aio_close(mraa_aio_context dev)
{
    if (NULL != dev) {
        if (dev->adc_in_fp != -1)
            close(dev->adc_in_fp);
        free(dev);
    }

    return (MRAA_SUCCESS);
}

mraa_result_t
mraa_aio_set_bit(mraa_aio_context dev, int bits)
{
    if (dev == NULL || bits < 1) {
        syslog(LOG_ERR, "aio: Device not valid");
        return MRAA_ERROR_INVALID_RESOURCE;
    }
    dev->value_bit = bits;
    return MRAA_SUCCESS;
}

int
mraa_aio_get_bit(mraa_aio_context dev)
{
    if (dev == NULL) {
        syslog(LOG_ERR, "aio: Device not valid");
        return 0;
    }
    return dev->value_bit;
}

/* Experimental buffered AIO stuff - work in progress */
mraa_result_t
mraa_write_str_to_file(char *file_path, char *data)
{
    if (file_path == NULL) {
        syslog(LOG_ERR, "aio: File path provided is NULL, cannot proceed");
        return MRAA_ERROR_INVALID_PARAMETER;
    }

    if (data == NULL) {
        syslog(LOG_ERR, "aio: Data string provided is NULL, cannot proceed");
        return MRAA_ERROR_INVALID_PARAMETER;
    }

    char buf[64] = "\0";
    int length, err, fp;

    length = snprintf(buf, sizeof(buf), "%s", data);
    if (length < 0) {
        syslog(LOG_ERR, "aio: Failed to copy data string '%s' to buffer", data);
        return MRAA_ERROR_UNSPECIFIED;
    } else if (length >= sizeof(buf)) {
        syslog(LOG_WARNING, "aio: Data string '%s' truncated to '%s' while copying to the buffer", data, buf);
    }

    fp = open(file_path, O_WRONLY);
    if (fp == -1) {
        err = errno;
        syslog(LOG_ERR, "aio: Failed to open file %s for writing, error: %s (%d)", file_path, strerror(err), err);
        return MRAA_ERROR_INVALID_RESOURCE;
    }

    lseek(fp, 0, SEEK_SET);

    ssize_t res = write(fp, buf, length*sizeof(char));
    if (res == -1) {
        err = errno;
        syslog(LOG_ERR, "aio: Failed to write string '%s' into file '%s', error: %s (%d)", buf, file_path, strerror(err), err);
        close(fp);
        return MRAA_ERROR_INVALID_RESOURCE;
    }

    close(fp);
    return MRAA_SUCCESS;
}

mraa_result_t
mraa_write_int_to_file(char *file_path, int data)
{
    char buf[64] = "\0";
    int length, err;

    length = snprintf(buf, sizeof(buf), "%d", data);
    if (length > 0) {
        syslog(LOG_NOTICE, "aio: Converted int %d to string '%s' with length %d", data, buf, length);
    } else {
        syslog(LOG_ERR, "aio: Failed to convert int %d to string", data);
        return MRAA_ERROR_UNSPECIFIED;
    }

    return mraa_write_str_to_file(file_path, buf);
}

ssize_t
mraa_read_from_file(char *file_path, unsigned int num_reads, unsigned int scan_size, char *buf)
{
    if (file_path == NULL) {
        syslog(LOG_ERR, "aio: File path provided is NULL, cannot proceed");
        return 0;
    }

    int fp, err;
    ssize_t read_bytes;

    fp = open(file_path, O_RDONLY);
    if (fp == -1) {
        err = errno;
        syslog(LOG_ERR, "aio: Failed to open file %s for writing, error: %s (%d)", file_path, strerror(err), err);
        return 0;
    }

    read_bytes = read(fp,
                      buf,
                      scan_size*num_reads);

    if (read_bytes != -1) {
        syslog(LOG_NOTICE, "aio: Read %d bytes from ADC buffer file", read_bytes);
    } else {
        err = errno;
        syslog(LOG_ERR, "aio: Error reading ADC buffer file: %s (%d)", strerror(err), err);
        close(fp);
        return 0;
    }

    close(fp);
    return read_bytes;
}

// Reads num_reads datums from ADC into buf buffer. Returns actual number of datums read
ssize_t
mraa_aio_read_buffered(mraa_aio_context dev, unsigned int num_reads, char *buf)
{
    if (dev == NULL) {
        syslog(LOG_ERR, "aio: Device not valid");
        return 0;
    }

    if (buf == NULL) {
        syslog(LOG_ERR, "aio: Supplied data buffer is not valid");
        return 0;
    }

    /* setup trigger */
    // FIXME: This path will only exist if iio-trig-sysfs module is loaded
    // or if the device supports buffered mode right away.
    char add_trigger_file_path[64] = "/sys/bus/iio/devices/iio_sysfs_trigger/add_trigger";
    mraa_result_t res = mraa_write_int_to_file(add_trigger_file_path, 0);
    if (res == MRAA_SUCCESS) {
        syslog(LOG_NOTICE, "Successfully added trigger");
    } else {
        syslog(LOG_ERR, "Could not add trigger, error: %d", res);
    }
    // FIXME: This path is Edison-only
    char curr_trigger_file_path[64] = "/sys/bus/iio/devices/iio:device1/trigger/current_trigger";
    res = mraa_write_str_to_file(curr_trigger_file_path, "sysfstrig0");
    if (res == MRAA_SUCCESS) {
        syslog(LOG_NOTICE, "Successfully connected trigger to ADC device");
    } else {
        syslog(LOG_ERR, "Could not connect trigger to ADC device, error: %d", res);
    }

    /* enable channel */
    // FIXME: This path is Edison-only
    char adc_channel_0_file_path[64] = "/sys/bus/iio/devices/iio:device1/scan_elements/in_voltage0_en";
    res = mraa_write_int_to_file(adc_channel_0_file_path, 1);
    if (res == MRAA_SUCCESS) {
        syslog(LOG_NOTICE, "Successfully enabled ADC channel 0");
    } else {
        syslog(LOG_ERR, "Could not enable ADC channel 0, error: %d", res);
    }

    /* setup buffer */
    char buf_len_file_path[64] = "/sys/bus/iio/devices/iio:device1/buffer/length";
    res = mraa_write_int_to_file(buf_len_file_path, num_reads);
    if (res == MRAA_SUCCESS) {
        syslog(LOG_NOTICE, "Successfully set ADC buffer length to %d", num_reads);
    } else {
        syslog(LOG_ERR, "Could not set ADC buffer length to %d, error: %d", num_reads, res);
    }
    // FIXME: This path is Edison-only
    char buf_en_file_path[64] = "/sys/bus/iio/devices/iio:device1/buffer/enable";
    res = mraa_write_int_to_file(buf_en_file_path, 1);
    if (res == MRAA_SUCCESS) {
        syslog(LOG_NOTICE, "Successfully enabled ADC buffer");
    } else {
        syslog(LOG_ERR, "Could not enable ADC buffer, error: %d", res);
    }

    /* trigger reading */
    char run_trigger_file_path[64] = "/sys/bus/iio/devices/trigger0/trigger_now";
    int i;
    for (i=0; i<num_reads; i++) {
        res = mraa_write_int_to_file(run_trigger_file_path, 1);
        if (res == MRAA_SUCCESS) {
            syslog(LOG_NOTICE, "Successfully triggered ADC read, run %d of %d", i, num_reads);
        } else {
            syslog(LOG_ERR, "Could not trigger ADC read, run %d of %d, error: %d", i, num_reads, res);
        }
    }

    /* read & convert readings */
    // FIXME: This scan_size is Edison-only, should be autodetected
    unsigned int scan_size = 2;
    // FIXME: This is Edison-only
    char adc_char_dev[64] = "/dev/iio:device1";
    ssize_t bytes_read = mraa_read_from_file(adc_char_dev, num_reads, scan_size, buf);

    /* disable buffer */
    res = mraa_write_int_to_file(buf_en_file_path, 0);
    if (res == MRAA_SUCCESS) {
        syslog(LOG_NOTICE, "Successfully disabled ADC buffer");
    } else {
        syslog(LOG_ERR, "Could not disable ADC buffer, error: %d", res);
    }

    /* disable channel */
    res = mraa_write_int_to_file(adc_channel_0_file_path, 0);
    if (res == MRAA_SUCCESS) {
        syslog(LOG_NOTICE, "Successfully disabled ADC channel 0");
    } else {
        syslog(LOG_ERR, "Could not disable ADC channel 0, error: %d", res);
    }

    /* remove trigger */
    // FIXME: This path depends on iio-trig-sysfs module being loaded
    char remove_trigger_file_path[64] = "/sys/bus/iio/devices/iio_sysfs_trigger/remove_trigger";
    res = mraa_write_int_to_file(remove_trigger_file_path, 0);
    if (res == MRAA_SUCCESS) {
        syslog(LOG_NOTICE, "Successfully removed trigger");
    } else {
        syslog(LOG_ERR, "Could not remove trigger, error: %d", res);
    }
    // FIXME: This path is Edison-only
    res = mraa_write_str_to_file(curr_trigger_file_path, "");
    if (res == MRAA_SUCCESS) {
        syslog(LOG_NOTICE, "Successfully disconnected trigger from ADC device");
    } else {
        syslog(LOG_ERR, "Could not disconnect trigger from ADC device, error: %d", res);
    }

    return bytes_read;
}

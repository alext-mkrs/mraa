#!/usr/bin/env python

# Author: Alex Tereschenko <alext.mkrs@gmail.com>
# Copyright (c) 2016 Alex Tereschenko <alext.mkrs@gmail.com>
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

import mraa as m
import unittest as u

MRAA_TEST_PIN = 0

class GpioChecks(u.TestCase):
  def setUp(self):
    self.pin = m.Gpio(MRAA_TEST_PIN)

  def tearDown(self):
    del self.pin

  def test_get_pin_num(self):
    self.pin_no = self.pin.getPin()
    self.assertEqual(self.pin_no, MRAA_TEST_PIN, "Returned GPIO pin number is incorrect")

  def test_set_GPIO_as_output(self):
    direction = m.DIR_OUT
    res = self.pin.dir(direction)
    self.assertEqual(res, m.SUCCESS, "Setting GPIO to output failed")
    self.assertEqual(self.pin.readDir(), direction, "GPIO has incorrect direction after dir()")

  def test_set_GPIO_as_output_HIGH(self):
    direction = m.DIR_OUT_HIGH;
    res = self.pin.dir(direction)
    self.assertEqual(res, m.SUCCESS, "Setting GPIO to output HIGH failed")
    self.assertEqual(self.pin.readDir(), direction, "GPIO has incorrect direction after dir()")

  def test_set_GPIO_as_output_LOW(self):
    direction = m.DIR_OUT_LOW
    res = self.pin.dir(direction)
    self.assertEqual(res, m.SUCCESS, "Setting GPIO to output LOW failed")
    self.assertEqual(self.pin.readDir(), direction, "GPIO has incorrect direction after dir()")

  def test_set_GPIO_as_input(self):
    direction = m.DIR_IN
    res = self.pin.dir(direction)
    self.assertEqual(res, m.SUCCESS, "Setting GPIO to input failed")
    self.assertEqual(self.pin.readDir(), direction, "GPIO has incorrect direction after dir()")

  def test_GPIO_as_output_write_HIGH(self):
    res = self.pin.dir(m.DIR_OUT)
    self.assertEqual(res, m.SUCCESS, "Setting GPIO to output failed")
    res = self.pin.write(1)
    self.assertEqual(res, m.SUCCESS, "Setting GPIO to HIGH failed")

  def test_GPIO_as_output_write_LOW(self):
    res = self.pin.dir(m.DIR_OUT)
    self.assertEqual(res, m.SUCCESS, "Setting GPIO to output failed")
    res = self.pin.write(0)
    self.assertEqual(res, m.SUCCESS, "Setting GPIO to LOW failed")

  def test_GPIO_as_input_write_HIGH(self):
    self.pin.dir(m.DIR_IN)
    res = self.pin.write(1)
    self.assertNotEqual(res, m.SUCCESS, "Setting GPIO in INPUT to HIGH should have failed")

  def test_GPIO_as_input_write_LOW(self):
    self.pin.dir(m.DIR_IN)
    res = self.pin.write(0)
    self.assertNotEqual(res, m.SUCCESS, "Setting GPIO in INPUT to LOW should have failed")

if __name__ == '__main__':
  u.main()

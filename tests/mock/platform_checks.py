#!/usr/bin/env python

# Author: Costin Constantin <costin.c.constantin@intel.com>
# Copyright (c) 2015 Intel Corporation.
#
# Contributors: Alex Tereschenko <alext.mkrs@gmail.com>
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

class PlatformChecks(u.TestCase):
  def test_platform_num_of_pins(self):
    pinCount = m.getPinCount()
    self.assertEqual(pinCount, 1, "Wrong number of pins reported by platform")

  def test_platform_ADC_max_resolution(self):
    ADC_max_res = m.adcRawBits()
    print("Platform ADC max. resolution is: " + str(ADC_max_res) + " bits")
    self.assertEqual(ADC_max_mres, 12, "Wrong ADC max. resolution")

  def test_platform_ADC_std_resolution(self):
    ADC_std_res = m.adcSupportedBits()
    print("Platform ADC standard resolution is: " + str(ADC_std_res) + " bits")
    self.assertEqual(ADC_std_res, 10, "Wrong ADC standard resolution")

if __name__ == "__main__":
  u.main()

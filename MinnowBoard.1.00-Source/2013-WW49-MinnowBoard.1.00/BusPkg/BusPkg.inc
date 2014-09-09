## @file  BusPkg.inc
#
# The .DSC include file containing the platform independent bus support.
#
# Copyright (c) 2012-2013, Intel Corporation. All rights reserved.<BR>
#
# This program and the accompanying materials are licensed and made available under
# the terms and conditions of the BSD License which accompanies this distribution.
# The full text of the license may be found at
# http://opensource.org/licenses/bsd-license.php
#
# THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
# WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
##

[LibraryClasses]
  AsciiDump|BusPkg/Library/AsciiDump/AsciiDump.inf
  DriverLib|BusPkg/Library/DriverLib/DriverLib.inf

[Components]
  BusPkg/I2cBusDxe/I2cBusDxe.inf
  BusPkg/I2cHostDxe/I2cHostDxe.inf


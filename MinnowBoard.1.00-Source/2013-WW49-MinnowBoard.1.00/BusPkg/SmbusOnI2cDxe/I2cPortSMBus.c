/** @file
  Implement the I2C port driver
  
  Copyright (c) 2012, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Library/I2cPortDxe.h>

///
/// Set the FIFO size
///
#define FIFO_SIZE_IN_BYTES    256

///
/// Set the CID value for the I2C driver
/// Use mCidStr if mCid value is zero
///
CONST UINT32 mCid = 0;

///
/// Set the CID value for the I2C driver
/// Set to NULL when mCid is non-zero
///
CONST CHAR8 *CONST mCidStr = "INTC33B1";

///
/// Controller name string table
///
CONST EFI_UNICODE_STRING_TABLE mControllerNameStringTable[] = {
  { "eng", L"SMBus Controller" },
  { NULL , NULL }
};

///
/// Driver name string table
///
CONST EFI_UNICODE_STRING_TABLE mDriverNameStringTable[] = {
  { "eng", L"SMBus Port Driver" },
  { NULL , NULL }
};

///
/// The maximum number of bytes the I2C host controller
/// is able to receive from the I2C bus.
///
CONST UINT32 mMaximumReceiveBytes = 33;

///
/// The maximum number of bytes the I2C host controller
/// is able to send on the I2C bus.
///
CONST UINT32 mMaximumTransmitBytes = 33;

///
/// The maximum number of bytes in the I2C bus transaction.
///
CONST UINT32 mMaximumTotalBytes = 66;

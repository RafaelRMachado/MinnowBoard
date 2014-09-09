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

#include <SmbusOnI2c.h>


/**
  Compute the packet error code value for SMBus packets

  @param PreviousCrc      Previous CRC value
  @param Data             Data byte to combine into the CRC

  @returns Returns the updated packet error code value

**/
UINT8
Crc8 ( 
  IN UINT8 PreviousCrc,
  IN UINT8 Data
  )
{
  UINT8 Crc;

  //
  //  http://en.wikipedia.org/wiki/Header_error_correction
  //  http://en.wikipedia.org/wiki/System_Management_Bus
  //

  //
  //  Not implemented
  //
  Print ( L"ERROR - Crc8 is not implemented\r\n" );
  ASSERT ( FALSE );
  Crc = Data;

  //
  //  Return the CRC value
  //
  return Crc;
}

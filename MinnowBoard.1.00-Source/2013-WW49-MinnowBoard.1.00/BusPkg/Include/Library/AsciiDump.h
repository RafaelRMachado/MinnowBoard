/** @file
  Declare the ASCII dump routine

Copyright (c) 2006 - 2010, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials are licensed and made available under 
the terms and conditions of the BSD License that accompanies this distribution.  
The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php.                                          
    
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

**/

#ifndef _ASCII_DUMP_H
#define _ASCII_DUMP_H

#define DUMP_BYTES_PER_LINE         16    ///< Number of bytes to display per line

/**
  Display the buffer contents

  @param [in] pDisplayOffset    Display address
  @param [in] pBuffer           Data buffer address
  @param [in] LengthInBytes     Length of data in buffer

**/
VOID
EFIAPI
AsciiDump (
  IN CONST UINT8 * pDisplayOffset,
  IN CONST UINT8 * pBuffer,
  IN INTN LengthInBytes
  );

#endif  //  _ASCII_DUMP_H

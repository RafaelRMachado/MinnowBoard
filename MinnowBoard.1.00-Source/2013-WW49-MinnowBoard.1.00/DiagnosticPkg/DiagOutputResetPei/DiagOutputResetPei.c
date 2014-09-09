/** @file
  This PEIM counts down from nine (9) and then resets the system.
  
  Copyright (c) 2013, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Library/SerialPortLib.h>

/**
  This is the entrypoint of PEIM
  
  @param  FileHandle  Handle of the file being invoked.
  @param  PeiServices Describes the list of possible PEI Services.

  @retval EFI_SUCCESS if it completed successfully.  
**/
EFI_STATUS
EFIAPI
DiagOutputReset (
  IN       EFI_PEI_FILE_HANDLE  FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
  )
{
  UINTN BytesWritten;
  CHAR8 Digit;
  INTN Index;

  //
  //  Count down to reset
  //
  BytesWritten = 0;
  for ( Index = 9; 0 <= Index; Index-- )
  {
    BytesWritten = SerialPortWrite ( (UINT8 *)"Reset in ", 9 );
    Digit = (CHAR8)( Index | '0' );
    BytesWritten = SerialPortWrite ( (UINT8*)&Digit, 1 );
    BytesWritten = SerialPortWrite ( (UINT8 *)"\r\n", 2 );
  }

  //
  //  Reset the system
  //
  (*PeiServices)->ResetSystem ( PeiServices );

  //
  //  Return the diagnostic status
  //
  return EFI_SUCCESS;
}

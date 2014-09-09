/** @file
  Application wrapper for the E6XX GPIO ports.
  
  Copyright (c) 2012-2013, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "GpioE6xx.h"


INTN
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{
  EFI_STATUS Status;

  //
  // initialize the shell lib (we must be in non-auto-init...)
  //
  Status = ShellInitialize ( );
  if ( !EFI_ERROR ( Status )) {
    //
    //  Run the application
    //
    Status = GpioE6xxMain ( Argc, Argv );
  }

  //
  //  Return the command status
  //
  return (INTN)Status;
}

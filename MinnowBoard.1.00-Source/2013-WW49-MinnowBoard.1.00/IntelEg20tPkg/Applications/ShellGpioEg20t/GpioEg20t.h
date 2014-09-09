/** @file
  Header file for the EG20T GPIO application.
  
  Copyright (c) 2012-2013, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/


#ifndef _GPIO_EG20T_H_
#define _GPIO_EG20T_H_

#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/Eg20tGpioLib.h>
#include <Library/HiiLib.h>
#include <Library/MinnowShellLib.h>   // For ShellPrintHelp missing from UDK2010.SR1.UP1.P1 
#include <Library/ShellCommandLib.h>
#include <Library/ShellCEntryLib.h>
#include <Library/ShellLib.h>
#include <Library/UefiLib.h>

#include <Protocol/EfiShellParameters.h>

#define DIM(x)      ( sizeof ( x ) / sizeof ( x [ 0 ]))

typedef struct {
  UINTN RequiredParameters;
  UINTN MaxParameters;
  CONST CHAR16 * Command;
  CONST CHAR16 * HelpText;
} GPIO_COMMAND;


EFI_STATUS
EFIAPI
GpioShellCommand (
  IN INTN Argc,
  IN CHAR16 ** Argv
  );

extern EFI_HANDLE gGpioEg20tHiiHandle;

VOID
GpioEg20tGetHiiHandle (
  VOID
  );

EFI_STATUS
GpioEg20tMain (
  IN UINTN Argc,
  IN CHAR16 ** Argv
  );

#endif  //  _GPIO_EG20T_H_

/** @file
  Shell library to turn on the LED blinking.
  
  Copyright (c) 2013, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/HiiLib.h>
#include <Library/LedLib.h>
#include <Library/PcdLib.h>
#include <Library/ShellCommandLib.h>
#include <Library/ShellCEntryLib.h>
#include <Library/ShellLib.h>
#include <Library/UefiLib.h>


// {6855B745-4059-406c-B013-1E483256CB90}
STATIC CONST GUID mBlinkOnGuid =
{ 0x6855b745, 0x4059, 0x406c, { 0xb0, 0x13, 0x1e, 0x48, 0x32, 0x56, 0xcb, 0x90 } };

STATIC CONST CHAR16 mBlinkOnFileName[] = L"ShellCommands";
EFI_HANDLE gBlinkOnHiiHandle = NULL;


SHELL_STATUS
EFIAPI
BlinkOn (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  //
  //  Disable blinking by the LED timer routine
  //
  PcdSet32 ( PcdDiagBootPhasesLedBlinkRate, 100 );

  //
  //  Turn on the LED
  //
  Led2On ( );

  //
  //  Return the operation status
  //
  return EFI_SUCCESS;
}


CONST CHAR16 *
BlinkOnManFile (
  VOID
  )
{
  return mBlinkOnFileName;
}

EFI_STATUS
EFIAPI
BlinkOnInitialize (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  gBlinkOnHiiHandle = HiiAddPackages ( &mBlinkOnGuid,
                                       gImageHandle,
                                       STRING_ARRAY_NAME,
                                       NULL );
  ASSERT ( NULL != gBlinkOnHiiHandle );
  ShellCommandRegisterCommandName ( L"blinkon",
                                    BlinkOn,
                                    BlinkOnManFile,
                                    1,
                                    L"blinkon",
                                    TRUE,
                                    gBlinkOnHiiHandle,
                                    STRING_TOKEN(STR_GET_HELP_BLINK_ON) );
  return EFI_SUCCESS;
}

/**
  Destructor for the library.  free any resources.
**/
EFI_STATUS
EFIAPI
BlinkOnDestructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  if (gBlinkOnHiiHandle != NULL) {
    HiiRemovePackages (gBlinkOnHiiHandle);
  }
  return EFI_SUCCESS;
}


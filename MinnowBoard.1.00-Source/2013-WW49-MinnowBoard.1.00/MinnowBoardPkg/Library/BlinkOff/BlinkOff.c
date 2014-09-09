/** @file
  Shell library to turn off the LED blinking.
  
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


// {025AC475-65EA-4c1a-B062-8A98A33F7950}
STATIC CONST GUID mBlinkOffGuid = 
{ 0x025ac475, 0x65ea, 0x4c1a, { 0xb0, 0x62, 0x8a, 0x98, 0xa3, 0x3f, 0x79, 0x50 } };

STATIC CONST CHAR16 mBlinkOffFileName[] = L"ShellCommands";
EFI_HANDLE gBlinkOffHiiHandle = NULL;


SHELL_STATUS
EFIAPI
BlinkOff (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  //
  //  Disable blinking by the timer routine
  //
  PcdSet32 ( PcdDiagBootPhasesLedBlinkRate, 0 );

  //
  //  Turn off the LED
  //
  Led2Off ( );

  //
  //  Return the operation status
  //
  return EFI_SUCCESS;
}


CONST CHAR16 *
BlinkOffManFile (
  VOID
  )
{
  return mBlinkOffFileName;
}

EFI_STATUS
EFIAPI
BlinkOffInitialize (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  gBlinkOffHiiHandle = HiiAddPackages ( &mBlinkOffGuid,
                                        gImageHandle,
                                        STRING_ARRAY_NAME,
                                        NULL );
  ASSERT ( NULL != gBlinkOffHiiHandle );
  ShellCommandRegisterCommandName ( L"blinkoff",
                                    BlinkOff,
                                    BlinkOffManFile,
                                    1,
                                    L"blinkoff",
                                    TRUE,
                                    gBlinkOffHiiHandle,
                                    STRING_TOKEN(STR_GET_HELP_BLINK_OFF) );
  return EFI_SUCCESS;
}

/**
  Destructor for the library.  free any resources.
**/
EFI_STATUS
EFIAPI
BlinkOffDestructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  if (gBlinkOffHiiHandle != NULL) {
    HiiRemovePackages (gBlinkOffHiiHandle);
  }
  return EFI_SUCCESS;
}


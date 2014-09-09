/** @file
  Shell library wrapper for the E6xx GPIO application.
  
  Copyright (c) 2012-2013, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "GpioE6xx.h"

STATIC CONST CHAR16 mGpioE6xxFileName[] = L"ShellCommands";


SHELL_STATUS
EFIAPI
ShellCommandRunGpioE6xx (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_SHELL_PARAMETERS_PROTOCOL * ParametersProtocol;
  EFI_STATUS Status;

  //
  // initialize the shell lib (we must be in non-auto-init...)
  //
  Status = ShellInitialize ( );
  if ( !EFI_ERROR ( Status )) {
    Status = CommandInit ( );
    if ( !EFI_ERROR ( Status )) {
      Status = gBS->LocateProtocol ( &gEfiShellParametersProtocolGuid,
                                     NULL,
                                     (VOID**)&ParametersProtocol );
      if ( !EFI_ERROR ( Status )) {
        Status = GpioE6xxMain ( ParametersProtocol->Argc,
                                ParametersProtocol->Argv );
      }
      else {
        DEBUG (( DEBUG_ERROR,
                  "ERROR - Unable to locate gEfiShellParametersProtocol, Status: %r\r\n",
                  Status ));
      }
    }
    else {
      DEBUG (( DEBUG_ERROR,
                "ERROR - Failed call to CommandInit, Status: %r\r\n",
                Status ));
    }
  }
  else {
    DEBUG (( DEBUG_ERROR,
              "ERROR - Failed call to ShellInitialize, Status: %r\r\n",
              Status ));
  }

  //
  //  Return the operation status
  //
  return Status;
}


CONST CHAR16 *
GpioE6xxManFile (
  VOID
  )
{
  return mGpioE6xxFileName;
}

EFI_STATUS
EFIAPI
ShellGpioE6xxInitialize (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  GpioE6xxGetHiiHandle ( );
  ASSERT ( NULL != gGpioE6xxHiiHandle );
  ShellCommandRegisterCommandName ( L"gpioe6xx",
                                    ShellCommandRunGpioE6xx,
                                    GpioE6xxManFile,
                                    1,
                                    L"gpioe6xx",
                                    TRUE,
                                    gGpioE6xxHiiHandle,
                                    STRING_TOKEN(STR_GET_HELP_GPIO_E6XX) );
  return EFI_SUCCESS;
}

/**
  Destructor for the library.  free any resources.
**/
EFI_STATUS
EFIAPI
ShellGpioE6xxDestructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  if (gGpioE6xxHiiHandle != NULL) {
    HiiRemovePackages (gGpioE6xxHiiHandle);
  }
  return EFI_SUCCESS;
}


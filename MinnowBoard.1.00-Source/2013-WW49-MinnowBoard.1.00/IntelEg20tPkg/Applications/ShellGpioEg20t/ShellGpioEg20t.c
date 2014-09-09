/** @file
  Shell library wrapper for the EG20T GPIO application.
  
  Copyright (c) 2012-2013, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "GpioEg20t.h"

STATIC CONST CHAR16 mGpioEg20tFileName[] = L"ShellCommands";


SHELL_STATUS
EFIAPI
ShellCommandRunGpioEg20t (
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
        Status = GpioEg20tMain ( ParametersProtocol->Argc,
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
GpioEg20tManFile (
  VOID
  )
{
  return mGpioEg20tFileName;
}

EFI_STATUS
EFIAPI
ShellGpioEg20tInitialize (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  GpioEg20tGetHiiHandle ( );
  ASSERT ( NULL != gGpioEg20tHiiHandle );
  ShellCommandRegisterCommandName ( L"gpioeg20t",
                                    ShellCommandRunGpioEg20t,
                                    GpioEg20tManFile,
                                    1,
                                    L"gpioeg20t",
                                    TRUE,
                                    gGpioEg20tHiiHandle,
                                    STRING_TOKEN(STR_GET_HELP_GPIO_EG20T) );
  return EFI_SUCCESS;
}

/**
  Destructor for the library.  free any resources.
**/
EFI_STATUS
EFIAPI
ShellGpioEg20tDestructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  if (gGpioEg20tHiiHandle != NULL) {
    HiiRemovePackages (gGpioEg20tHiiHandle);
  }
  return EFI_SUCCESS;
}


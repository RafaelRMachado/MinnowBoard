/** @file
  The implementation for Shell application IfConfig6.

  Copyright (c) 2009 - 2013, Intel Corporation. All rights reserved.<BR>

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/HiiLib.h>
#include <Library/ShellCommandLib.h>
#include <Library/ShellLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Protocol/Ip6Config.h>

#include "IfConfig6.h"

STATIC CONST CHAR16 mFileName[] = L"ShellCommands";

EFI_HANDLE gIfConfig6HiiHandle = NULL;


STATIC
CONST CHAR16 *
GetFileFile (
  VOID
  )
{
  return mFileName;
}


EFI_STATUS
EFIAPI
IfConfig6Constructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  gIfConfig6HiiHandle = HiiAddPackages ( gImageHandle,
                               NULL,
                               STRING_ARRAY_NAME,
                               NULL );
  ASSERT ( NULL != gIfConfig6HiiHandle );
  ShellCommandRegisterCommandName ( L"ifconfig6",
                                    (SHELL_RUN_COMMAND)IfConfig6Initialize,
                                    GetFileFile,
                                    1,
                                    L"ifconfig6",
                                    TRUE,
                                    gIfConfig6HiiHandle,
                                    STRING_TOKEN(STR_IFCONFIG6_CMD_HELP));
  return EFI_SUCCESS;
}

/**
  Destructor for the library.  free any resources.
**/
EFI_STATUS
EFIAPI
IfConfig6Destructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  if (gIfConfig6HiiHandle != NULL) {
    HiiRemovePackages (gIfConfig6HiiHandle);
  }
  return EFI_SUCCESS;
}


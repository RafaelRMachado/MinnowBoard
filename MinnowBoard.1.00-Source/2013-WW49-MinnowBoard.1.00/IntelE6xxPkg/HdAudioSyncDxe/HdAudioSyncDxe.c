/** @file
  DXE module to synchronize the platform specific HD audio codec
  descriptions with the HD audio driver.
  
  Copyright (c) 2013, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/PlatformHdAudioConnection.h>

/**
  Indicate that all the HD codec descriptions are available

  @retval  EFI_SUCCESS  Tag GUID installed successfully

**/

EFI_STATUS
EFIAPI
HdAudioSync (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS  Status;

  //
  //  Install the tag GUID
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &gImageHandle,
                  &gHdAudioSyncGuid,
                  NULL,
                  NULL
                  );
  return Status;
}

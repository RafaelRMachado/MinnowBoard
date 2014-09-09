/** @file
  Common driver entry point implementation.
  
  Copyright (c) 2011 - 2013, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Library/DriverLib.h>


/**
Common driver entry point

@param [in] ImageHandle       Handle for the image
@param [in] SystemTable       Address of the system table.

@retval EFI_SUCCESS           Image successfully loaded.

**/
EFI_STATUS
EFIAPI
DlEntryPoint (
  IN EFI_HANDLE ImageHandle,
  IN EFI_SYSTEM_TABLE * SystemTable
  )
{
  EFI_LOADED_IMAGE_PROTOCOL * LoadedImage;
  EFI_STATUS Status;

  //
  //  Enable unload support
  //
  Status = gBS->HandleProtocol (
                  gImageHandle,
                  &gEfiLoadedImageProtocolGuid,
                  (VOID *)&LoadedImage
                  );
  if (!EFI_ERROR (Status)) {
    LoadedImage->Unload = mDriverLib.Unload;

    //
    //  Add the driver to the list of drivers
    //
    Status = EfiLibInstallDriverBindingComponentName2 (
               ImageHandle,
               SystemTable,
               mDriverLib.DriverBindingProtocol,
               ImageHandle,
               FeaturePcdGet ( PcdComponentNameDisable )
                 ? NULL : mDriverLib.ComponentNameProtocol,
               FeaturePcdGet ( PcdComponentName2Disable )
                 ? NULL : mDriverLib.ComponentName2Protocol
               );
    if ( !EFI_ERROR ( Status )) {
      DEBUG (( DEBUG_POOL | DEBUG_INIT | DEBUG_INFO,
                "Installed: gEfiDriverBindingProtocolGuid on   0x%016lx\r\n",
                (UINT64)((UINTN)ImageHandle )));
      if (( !FeaturePcdGet ( PcdComponentNameDisable ))
        && ( NULL != mDriverLib.ComponentNameProtocol )) {
        DEBUG (( DEBUG_POOL | DEBUG_INIT | DEBUG_INFO,
                  "Installed: gEfiComponentNameProtocolGuid on   0x%016lx\r\n",
                  (UINT64)((UINTN)ImageHandle )));
      }
      if (( !FeaturePcdGet ( PcdComponentName2Disable ))
        && ( NULL != mDriverLib.ComponentName2Protocol )) {
        DEBUG (( DEBUG_POOL | DEBUG_INIT | DEBUG_INFO,
                  "Installed: gEfiComponentName2ProtocolGuid on   0x%016lx\r\n",
                  (UINT64)((UINTN)ImageHandle )));
      }
    }
  }

  //
  //  Return the image loaded status
  //
  return Status;
}

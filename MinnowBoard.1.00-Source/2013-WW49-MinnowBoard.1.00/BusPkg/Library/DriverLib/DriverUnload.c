/** @file
  Common driver unload implementation.
  
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
  Driver unload routine

  @param [in] ImageHandle       Handle for the image.

  @retval EFI_SUCCESS           Image may be unloaded

**/
EFI_STATUS
EFIAPI
DlDriverUnload (
  IN EFI_HANDLE ImageHandle
  )
{
  UINTN Index;
  UINTN LengthInBytes;
  UINTN Max;
  EFI_HANDLE * Controller;
  EFI_STATUS Status;

  //
  //  Assume success
  //
  Status = EFI_SUCCESS;

  //
  //  Use break instead of goto
  //
  Controller = NULL;
  for ( ; ; ) {
    //
    //  Determine the buffer size
    //
    LengthInBytes = 0;
    Status = gBS->LocateHandle ( ByProtocol,
                                 mDriverProtocol,
                                 NULL,
                                 &LengthInBytes,
                                 NULL );
    if ( EFI_BUFFER_TOO_SMALL != Status ) {
      DEBUG (( DEBUG_INIT | DEBUG_INFO,
                "No controllers found, Status: %r\r\n",
                Status ));
      Status = EFI_SUCCESS;
      break;
    }

    //
    //  One or more devices are present
    //
    Controller = AllocateZeroPool ( LengthInBytes );
    if ( NULL == Controller ) {
      Status = EFI_OUT_OF_RESOURCES;
      DEBUG (( DEBUG_ERROR | DEBUG_INIT | DEBUG_INFO,
                "Insufficient memory, failed handle buffer allocation\r\n" ));
      break;
    }

    //
    //  Get the lists of controllers
    //
    Status = gBS->LocateHandle ( ByProtocol,
                                 mDriverProtocol,
                                 NULL,
                                 &LengthInBytes,
                                 Controller );
    if ( EFI_ERROR ( Status )) {
      //
      //  Error getting handles
      //
      DEBUG (( DEBUG_ERROR | DEBUG_INIT | DEBUG_INFO,
              "Failure getting controller handles, Status: %r\r\n",
              Status ));
      break;
    }
      
    //
    //  Remove each of the driver instances
    //
    Max = LengthInBytes / sizeof ( Controller[ 0 ]);
    for ( Index = 0; Max > Index; Index++ ) {
      Status = mDriverLib.DriverBindingProtocol->Stop ( mDriverLib.DriverBindingProtocol,
                                                        Controller[ Index ],
                                                        0,
                                                        NULL );
      if ( EFI_ERROR ( Status )) {
        DEBUG (( DEBUG_ERROR | DEBUG_INIT | DEBUG_INFO,
                  "ERROR - Failed to stop the driver on handle %08x\r\n",
                  Controller[ Index ]));
        break;
      }
    }
    break;
  }

  //
  // Free the handle array
  //
  if ( NULL != Controller ) {
    FreePool ( Controller );
  }

  //
  //  Remove the protocols installed by the EntryPoint routine.
  //
  if (( !EFI_ERROR ( Status ))
    && ( !FeaturePcdGet ( PcdComponentName2Disable ))
    && ( NULL != mDriverLib.ComponentName2Protocol )) {
    Status = gBS->UninstallMultipleProtocolInterfaces (
                ImageHandle,
                &gEfiComponentName2ProtocolGuid,
                mDriverLib.ComponentName2Protocol,
                NULL
                );
    if ( !EFI_ERROR ( Status )) {
      DEBUG (( DEBUG_POOL | DEBUG_INIT | DEBUG_INFO,
              "Removed:   gEfiComponentName2ProtocolGuid from 0x%016lx\r\n",
              (UINT64)((UINTN)ImageHandle )));
    }
    else {
      DEBUG (( DEBUG_ERROR | DEBUG_POOL | DEBUG_INIT,
                  "ERROR - Failed to remove gEfiComponentName2ProtocolGuid from 0x%016lx, Status: %r\r\n",
                  (UINT64)((UINTN)ImageHandle ),
                  Status ));
    }
  }

  if (( !EFI_ERROR ( Status ))
    && ( !FeaturePcdGet ( PcdComponentNameDisable ))
    && ( NULL != mDriverLib.ComponentNameProtocol )) {
    Status = gBS->UninstallMultipleProtocolInterfaces (
                ImageHandle,
                &gEfiComponentNameProtocolGuid,
                mDriverLib.ComponentNameProtocol,
                NULL
                );
    if ( !EFI_ERROR ( Status )) {
      DEBUG (( DEBUG_POOL | DEBUG_INIT | DEBUG_INFO,
                "Removed:   gEfiComponentNameProtocolGuid from 0x%016lx\r\n",
                (UINT64)((UINTN)ImageHandle )));
    }
    else {
      DEBUG (( DEBUG_ERROR | DEBUG_POOL | DEBUG_INIT,
                  "ERROR - Failed to remove gEfiComponentNameProtocolGuid from 0x%016lx, Status: %r\r\n",
                  (UINT64)((UINTN)ImageHandle ),
                  Status ));
    }
  }

  if ( !EFI_ERROR ( Status )) {
    Status = gBS->UninstallMultipleProtocolInterfaces (
                ImageHandle,
                &gEfiDriverBindingProtocolGuid,
                mDriverLib.DriverBindingProtocol,
                NULL
                );
    if ( !EFI_ERROR ( Status )) {
      DEBUG (( DEBUG_POOL | DEBUG_INIT | DEBUG_INFO,
                "Removed:   gEfiDriverBindingProtocolGuid from 0x%016lx\r\n",
                (UINT64)((UINTN)ImageHandle )));
    }
    else {
      DEBUG (( DEBUG_ERROR | DEBUG_POOL | DEBUG_INIT,
                  "ERROR - Failed to remove gEfiDriverBindingProtocolGuid from 0x%016lx, Status: %r\r\n",
                  (UINT64)((UINTN)ImageHandle ),
                  Status ));
    }
  }

  //
  //  Return the unload status
  //
  return Status;
}

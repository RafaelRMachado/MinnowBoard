/** @file
  Implement the device path extension

  Copyright (c) 2011 - 2012, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "I2cBus.h"
#include <Library/DevicePathLib.h>

/**
  Driver path template
**/

typedef struct {
  VENDOR_DEVICE_PATH Vendor;
  CONTROLLER_DEVICE_PATH Controller;
  EFI_DEVICE_PATH_PROTOCOL End;
} VENDOR_PATH;

CONST STATIC VENDOR_PATH mVendorPath = {
  {
    HARDWARE_DEVICE_PATH,
    HW_VENDOR_DP,
    {
      sizeof ( VENDOR_DEVICE_PATH ),
      0
    },
    {
      0x00000000,
      0x0000,
      0x0000,
      { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
    }
  },
  {
    HARDWARE_DEVICE_PATH,
    HW_CONTROLLER_DP,
    {
      sizeof ( CONTROLLER_DEVICE_PATH ),
      0
    },
    0
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      END_DEVICE_PATH_LENGTH,
      0
    }
  }
};


/**
  Create a path for the I2C device

  Append the I2C slave path to the I2C master controller path.

  @param [in] I2cDevice     Address of an I2C_DEVICE_CONTEXT structure.
  @param[in] Controller     Handle to the controller

**/
EFI_STATUS
I2cBusDevicePathAppend (
  IN I2C_DEVICE_CONTEXT *I2cDevice,
  IN EFI_HANDLE Controller
  )
{
  EFI_DEVICE_PATH_PROTOCOL *DevPath;
  VENDOR_PATH *Path;
  EFI_STATUS Status;

  //
  //  Display entry
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cBusDevicePathAppend entered\r\n" ));

  //
  //  Locate the existing device path
  //
  Status = gBS->HandleProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  (VOID**) &DevPath
                  );
  if ( !EFI_ERROR (Status)) {
    //
    //  Allocate a buffer
    //
    Path = (VENDOR_PATH *)AllocateZeroPool ( sizeof ( mVendorPath ));
    if ( NULL == Path ) {
      DEBUG (( DEBUG_ERROR,
                "ERROR - Failed to allocate device path buffer!\r\n" ));
      Status = EFI_OUT_OF_RESOURCES;
    }
    else {
      //
      //  Build the new device path
      //
      CopyMem ( Path,
                &mVendorPath,
                sizeof ( mVendorPath ));
      CopyMem ( &Path->Vendor.Guid,
                I2cDevice->IoApi.I2cDevice->DeviceGuid,
                sizeof ( Path->Vendor.Guid ));
      Path->Controller.ControllerNumber = I2cDevice->IoApi.I2cDevice->DeviceIndex;
      I2cDevice->DevPath = AppendDevicePath ( DevPath,
                                              &Path->Vendor.Header );
      if ( NULL == I2cDevice->DevPath ) {
        Status = EFI_OUT_OF_RESOURCES;
      }

      //
      //  Free the buffer
      //
      FreePool ( Path );
    }
  }

  //
  //  Display exit
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cBusDevicePathAppend exiting, Status: %r\r\n", Status ));
  return Status;
}

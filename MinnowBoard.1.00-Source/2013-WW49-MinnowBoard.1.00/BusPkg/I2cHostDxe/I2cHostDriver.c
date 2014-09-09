/** @file
  Implement the driver binding protocol.
  
  Copyright (c) 2012, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "I2cHost.h"


/**
  Verify the controller type

  This routine determines if an I2C controller is available.

  This routine is called by the UEFI driver framework during connect
  processing.

  @param [in] DriverBinding        Protocol instance pointer.
  @param [in] Controller           Handle of device to test.
  @param [in] RemainingDevicePath  Not used.

  @retval EFI_SUCCESS          This driver supports this device.
  @retval other                This driver does not support this device.

**/
EFI_STATUS
EFIAPI
I2cHostDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL *DriverBinding,
  IN EFI_HANDLE Controller,
  IN EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath
  )
{
  CONST VOID *Protocol;
  EFI_STATUS Status;

  //
  //  Display entry
  //
  DEBUG (( DEBUG_LOAD, "I2cHostDriverSupported entered\r\n" ));

  //
  //  Locate an I2C master (controller)
  //  Verify that the device is not reserved for SMBus
  //  Determine if the I2C protocol is already running
  //
  Status = gBS->OpenProtocol ( Controller,
                               &gEfiI2cBusConfigurationManagementProtocolGuid,
                               (VOID **)&Protocol,
                               DriverBinding->DriverBindingHandle,
                               Controller,
                               EFI_OPEN_PROTOCOL_BY_DRIVER );
  if ( !EFI_ERROR ( Status )) {
    //
    //  Verify that the I2C master protocol is available
    //
    Status = gBS->OpenProtocol ( Controller,
                                 &gEfiI2cMasterProtocolGuid,
                                 (VOID **)&Protocol,
                                 DriverBinding->DriverBindingHandle,
                                 Controller,
                                 EFI_OPEN_PROTOCOL_GET_PROTOCOL );
    if ( !EFI_ERROR ( Status )) {
      //
      //  Found an idle I2C controller
      //
      DEBUG (( DEBUG_LOAD, "I2C controller found\r\n" ));
    }

    //
    //  Release the I2C bus management protocol
    //
    gBS->CloseProtocol ( Controller,
                         &gEfiI2cBusConfigurationManagementProtocolGuid,
                         DriverBinding->DriverBindingHandle,
                         Controller );
  }

  //
  //  Display exit
  //
  DEBUG (( DEBUG_LOAD, "I2cHostDriverSupported exiting, Status: %r\r\n", Status ));

  //
  //  Return success only when an available I2C controller is detected.
  //
  return Status;
}


/**
  Connect to the I2C controller

  This routine initializes an instance of the I2C host driver
  for this I2C controller.

  This routine is called by the UEFI driver framework during connect
  processing if the controller passes the tests in I2cHostDriverSupported.

  @param [in] DriverBinding        Protocol instance pointer.
  @param [in] Controller           Handle of device to work with.
  @param [in] RemainingDevicePath  Not used, always produce all possible children.

  @retval EFI_SUCCESS          This driver is added to Controller.
  @retval other                This driver does not support this device.

**/
EFI_STATUS
EFIAPI
I2cHostDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL *DriverBinding,
  IN EFI_HANDLE Controller,
  IN EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath
  )
{
  CONST EFI_I2C_BUS_CONFIGURATION_MANAGEMENT_PROTOCOL *I2cBusConfigurationManagement;
  I2C_HOST_CONTEXT *I2cHost;
  CONST EFI_I2C_MASTER_PROTOCOL *I2cMaster;
  EFI_STATUS Status;

  //
  //  Display entry
  //
  DEBUG (( DEBUG_LOAD, "I2cHostDriverStart entered\r\n" ));

  //
  //  Get the I2C bus management protocol
  //
  Status = gBS->OpenProtocol ( Controller,
                               &gEfiI2cBusConfigurationManagementProtocolGuid,
                               (VOID **)&I2cBusConfigurationManagement,
                               DriverBinding->DriverBindingHandle,
                               Controller,
                               EFI_OPEN_PROTOCOL_BY_DRIVER );
  if ( !EFI_ERROR ( Status )) {
    //
    //  Get the I2C master (controller) protocol
    //
    Status = gBS->OpenProtocol ( Controller,
                                 &gEfiI2cMasterProtocolGuid,
                                 (VOID **)&I2cMaster,
                                 DriverBinding->DriverBindingHandle,
                                 Controller,
                                 EFI_OPEN_PROTOCOL_GET_PROTOCOL );
    if ( !EFI_ERROR ( Status )) {
      //
      //  Found the I2C master
      //  Allocate the I2C context structure
      //
      I2cHost = AllocateZeroPool ( sizeof ( *I2cHost ));
      if ( NULL == I2cHost ) {
        DEBUG (( DEBUG_ERROR, "ERROR - No memory for I2C host driver\r\n" ));
        Status = EFI_OUT_OF_RESOURCES;
      }
      else {
        DEBUG (( DEBUG_POOL | DEBUG_INFO,
                  "0x%016lx: I2cHost allocated\r\n",
                  (UINT64)((UINTN)I2cHost )));

        //
        //  Initialize the context structure
        //
        I2cHost->Signature = I2C_HOST_SIGNATURE;
        I2cHost->I2cMaster = I2cMaster;
        I2cHost->I2cBusConfigurationManagement = I2cBusConfigurationManagement;

        //
        //  Start the driver
        //
        Status = I2cHostApiStart ( I2cHost );
        if ( !EFI_ERROR ( Status )) {
          //
          //  Install the driver protocol
          //
          Status = gBS->InstallMultipleProtocolInterfaces (
                          &Controller,
                          mDriverProtocol,
                          &I2cHost->HostApi,
                          NULL
                          );
          if ( !EFI_ERROR ( Status )) {
            DEBUG (( DEBUG_INIT,
                      "0x%016lx: I2cHost started\r\n",
                      (UINT64)(UINTN)I2cHost ));
          }
          else {
            //
            //  Release the API resources upon failure
            //
            I2cHostApiStop ( I2cHost );
          }
        }
        
        //
        //  Release the context structure upon failure
        //
        if ( EFI_ERROR ( Status )) {
          DEBUG (( DEBUG_WARN, "WARNING - Failed to start I2C host, Status: %r\r\n",
                   Status ));
          DEBUG (( DEBUG_POOL | DEBUG_INFO,
                    "0x%016lx: I2cHost released\r\n",
                    (UINT64)(UINTN)I2cHost ));
          FreePool ( I2cHost );
        }
      }
    }

    //
    //  Release the I2C bus management protocol upon failure
    //
    if (( EFI_ERROR ( Status ))
      && ( NULL != I2cBusConfigurationManagement )) {
      gBS->CloseProtocol ( Controller,
                           &gEfiI2cBusConfigurationManagementProtocolGuid,
                           DriverBinding->DriverBindingHandle,
                           Controller );
    }
  }

  //
  //  Display exit
  //
  DEBUG (( DEBUG_LOAD, "I2cHostDriverStart exiting, Status: %r\r\n", Status ));

  //
  //  Return the operation status.
  //
  return Status;
}


/**
  Disconnect from the I2C host controller.

  This routine disconnects from the I2C controller.

  This routine is called by DriverUnload when the I2C host driver
  is being unloaded.
  
  @param [in] DriverBinding        Protocol instance pointer.
  @param [in] Controller           Handle of device to stop driver on.
  @param [in] NumberOfChildren     How many children need to be stopped.
  @param [in] ChildHandleBuffer    Not used.

  @retval EFI_SUCCESS          This driver is removed Controller.
  @retval EFI_DEVICE_ERROR     The device could not be stopped due to a device error.
  @retval other                This driver was not removed from this device.

**/
EFI_STATUS
EFIAPI
I2cHostDriverStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL *DriverBinding,
  IN  EFI_HANDLE Controller,
  IN  UINTN NumberOfChildren,
  IN  EFI_HANDLE *ChildHandleBuffer
  )
{
  I2C_HOST_CONTEXT *I2cHost;
  CONST EFI_I2C_HOST_PROTOCOL *I2cHostProtocol;
  EFI_STATUS Status;

  //
  //  Display entry
  //
  DEBUG (( DEBUG_LOAD, "I2cHostDriverStop entered\r\n" ));

  //
  //  Disconnect any connected drivers and locate the context
  //  structure
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  mDriverProtocol,
                  (VOID**)&I2cHostProtocol,
                  DriverBinding->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER | EFI_OPEN_PROTOCOL_EXCLUSIVE );
  if ( !EFI_ERROR ( Status )) {
    I2cHost = I2C_HOST_CONTEXT_FROM_PROTOCOL ( I2cHostProtocol );

    //
    //  Done with the I2C host protocol
    //
    Status = gBS->CloseProtocol ( Controller,
                                  mDriverProtocol,
                                  DriverBinding->DriverBindingHandle,
                                  Controller );
    if ( !EFI_ERROR ( Status )) {
      //
      //  Remove the I2C host protocol
      //
      Status = gBS->UninstallMultipleProtocolInterfaces (
                    Controller,
                    mDriverProtocol,
                    I2cHostProtocol,
                    NULL );
      if ( !EFI_ERROR ( Status )) {
        //
        //  Stop the driver
        //
        DEBUG (( DEBUG_INIT,
                  "0x%016lx: I2cHost stopped\r\n",
                  (UINT64)(UINTN)I2cHost ));
        I2cHostApiStop ( I2cHost );

        //
        //  Release the I2C controller
        //
        gBS->CloseProtocol ( Controller,
                             &gEfiI2cBusConfigurationManagementProtocolGuid,
                             DriverBinding->DriverBindingHandle,
                             Controller );

        //
        //  Release the context
        //
        DEBUG (( DEBUG_POOL | DEBUG_INFO,
                  "0x%016lx: I2cHost released\r\n",
                  (UINT64)(UINTN)I2cHost ));
        FreePool ( I2cHost );
      }
      else {
        DEBUG (( DEBUG_ERROR,
                  "ERROR - Failed to uninstall I2C host protocol, Status: %r\r\n",
                  Status ));
      }
    }
    else {
      DEBUG (( DEBUG_ERROR,
                "ERROR - Failed to close I2C host protocol, Status: %r\r\n",
                Status ));
    }
  }

  //
  //  Display exit
  //
  DEBUG (( DEBUG_LOAD, "I2cHostDriverStop exiting, Status: %r\r\n", Status ));

  //
  //  Return the stop status
  //
  return Status;
}


/**
  Driver binding protocol support
**/
EFI_DRIVER_BINDING_PROTOCOL mI2cHostDriverBinding = {
  I2cHostDriverSupported,
  I2cHostDriverStart,
  I2cHostDriverStop,
  0x10,
  NULL,
  NULL
};

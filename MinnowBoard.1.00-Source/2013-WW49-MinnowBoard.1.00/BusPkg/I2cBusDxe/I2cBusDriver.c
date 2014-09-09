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

#include "I2cBus.h"


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
I2cBusDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL *DriverBinding,
  IN EFI_HANDLE Controller,
  IN EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath
  )
{
  VOID *Protocol;
  EFI_STATUS Status;

  //
  //  Display entry
  //
  DEBUG (( DEBUG_LOAD, "I2cBusDriverSupported entered\r\n" ));

  //
  //  Determine if the I2C controller is available
  //
  Status = gBS->OpenProtocol ( Controller,
                               &gEfiI2cHostProtocolGuid,
                               &Protocol,
                               DriverBinding->DriverBindingHandle,
                               Controller,
                               EFI_OPEN_PROTOCOL_BY_DRIVER );
  if ( !EFI_ERROR ( Status )) {
    DEBUG (( DEBUG_LOAD, "I2C host found\r\n" ));

    //
    //  Determine if the platform data is available
    //
    Status = gBS->OpenProtocol ( Controller,
                                 &gEfiI2cEnumerateProtocolGuid,
                                 &Protocol,
                                 DriverBinding->DriverBindingHandle,
                                 Controller,
                                 EFI_OPEN_PROTOCOL_BY_DRIVER );
    if ( !EFI_ERROR ( Status )) {
      DEBUG (( DEBUG_LOAD, "I2C bus enumeration data found\r\n" ));

      //
      //  The platform can enumerate the I2C bus
      //
      gBS->CloseProtocol ( Controller,
                           &gEfiI2cEnumerateProtocolGuid,
                           DriverBinding->DriverBindingHandle,
                           Controller );
    }

    //
    //  The I2C controller is available
    //
    gBS->CloseProtocol ( Controller,
                         &gEfiI2cHostProtocolGuid,
                         DriverBinding->DriverBindingHandle,
                         Controller );
  }

  //
  //  Display exit
  //
  DEBUG (( DEBUG_LOAD, "I2cBusDriverSupported exiting, Status: %r\r\n", Status ));

  //
  //  Return success only when an available I2C controller is detected.
  //
  return Status;
}


/**
  Connect to the I2C controller

  This routine initializes an instance of the I2C driver for this
  controller.

  This routine is called by the UEFI driver framework during connect
  processing if the controller passes the tests in I2cBusDriverSupported.

  @param [in] DriverBinding        Protocol instance pointer.
  @param [in] Controller           Handle of device to work with.
  @param [in] RemainingDevicePath  Not used, always produce all possible children.

  @retval EFI_SUCCESS          This driver is added to Controller.
  @retval other                This driver does not support this device.

**/
EFI_STATUS
EFIAPI
I2cBusDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL *DriverBinding,
  IN EFI_HANDLE Controller,
  IN EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath
  )
{
  EFI_I2C_ENUMERATE_PROTOCOL *I2cEnumerate;
  EFI_I2C_HOST_PROTOCOL *I2cHost;
  I2C_BUS_CONTEXT *I2cBus;
  EFI_STATUS Status;

  //
  //  Display entry
  //
  DEBUG (( DEBUG_LOAD, "I2cBusDriverStart entered\r\n" ));

  //
  //  Allocate the I2C context structure
  //
  I2cBus = AllocateZeroPool ( sizeof ( *I2cBus ));
  if ( NULL == I2cBus ) {
    DEBUG (( DEBUG_ERROR, "ERROR - No memory for I2C bus driver\r\n" ));
    Status = EFI_OUT_OF_RESOURCES;
  }
  else {
    DEBUG (( DEBUG_POOL | DEBUG_INFO,
              "0x%016Lx: I2cBus allocated\r\n",
              (UINT64)(UINTN)I2cBus ));

    //
    //  Determine if the I2C controller is available
    //
    Status = gBS->OpenProtocol ( Controller,
                                 &gEfiI2cHostProtocolGuid,
                                 (VOID**)&I2cHost,
                                 DriverBinding->DriverBindingHandle,
                                 Controller,
                                 EFI_OPEN_PROTOCOL_BY_DRIVER );
    if ( !EFI_ERROR ( Status )) {
      DEBUG (( DEBUG_INFO, "0x%016Lx: I2C host found\r\n", I2cHost ));

      //
      //  Get the I2C bus enumeration API
      //
      Status = gBS->OpenProtocol ( Controller,
                                   &gEfiI2cEnumerateProtocolGuid,
                                   (VOID**)&I2cEnumerate,
                                   DriverBinding->DriverBindingHandle,
                                   Controller,
                                   EFI_OPEN_PROTOCOL_BY_DRIVER );
      if ( !EFI_ERROR ( Status )) {
        DEBUG (( DEBUG_INFO, "0x%016Lx: I2C host found\r\n", I2cHost ));

        //
        //  Initialize the context structure
        //
        I2cBus->Signature = I2C_BUS_SIGNATURE;
        I2cBus->I2cHost = I2cHost;
        I2cBus->I2cEnumerate = I2cEnumerate;

        //
        //  Start the driver
        //
        Status = I2cBusApiStart ( I2cBus, Controller );
        if ( !EFI_ERROR ( Status )) {
          //
          //  Install the driver protocol
          //
          Status = gBS->InstallMultipleProtocolInterfaces (
                          &Controller,
                          mDriverProtocol,
                          I2cBus,
                          NULL
                          );
          if ( !EFI_ERROR ( Status )) {
            DEBUG (( DEBUG_INIT,
                      "0x%016Lx: I2cBus started\r\n",
                      (UINT64)(UINTN)I2cBus ));
          }
          else {
            //
            //  Release the API resources upon failure
            //
            I2cBusApiStop ( I2cBus );
          }
        }

        //
        //  Release the enumeration protocol upon failure
        //
        if ( EFI_ERROR ( Status )) {
          gBS->CloseProtocol ( Controller,
                               &gEfiI2cEnumerateProtocolGuid,
                               DriverBinding->DriverBindingHandle,
                               Controller );
        }
      }

      //
      //  Release the host protocol upon failure
      //
      if ( EFI_ERROR ( Status )) {
        gBS->CloseProtocol ( Controller,
                             &gEfiI2cHostProtocolGuid,
                             DriverBinding->DriverBindingHandle,
                             Controller );
      }
    }

    //
    //  Release the context structure upon failure
    //
    if ( EFI_ERROR ( Status )) {
      DEBUG (( DEBUG_ERROR, "ERROR - Failed to start I2C bus driver, Status: %r\r\n", Status ));
      DEBUG (( DEBUG_POOL | DEBUG_INFO,
                "0x%016Lx: I2cBus released\r\n",
                (UINT64)(UINTN)I2cBus ));
      FreePool ( I2cBus );
    }
  }

  //
  //  Display exit
  //
  DEBUG (( DEBUG_LOAD, "I2cBusDriverStart exiting, Status: %r\r\n", Status ));

  //
  //  Return the operation status.
  //
  return Status;
}


/**
  Disconnect from the I2C host controller.

  This routine disconnects from the I2C controller.

  This routine is called by DriverUnload when the I2C bus driver
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
I2cBusDriverStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL *DriverBinding,
  IN  EFI_HANDLE Controller,
  IN  UINTN NumberOfChildren,
  IN  EFI_HANDLE *ChildHandleBuffer
  )
{
  VOID *DriverProtocol;
  I2C_BUS_CONTEXT *I2cBus;
  EFI_STATUS Status;

  //
  //  Display entry
  //
  DEBUG (( DEBUG_LOAD, "I2cBusDriverStop entered\r\n" ));

  //
  //  Disconnect any connected drivers and locate the context
  //  structure
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  mDriverProtocol,
                  &DriverProtocol,
                  DriverBinding->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER | EFI_OPEN_PROTOCOL_EXCLUSIVE );
  if ( !EFI_ERROR ( Status )) {
    I2cBus = I2C_BUS_CONTEXT_FROM_PROTOCOL ( DriverProtocol );

    //
    //  Done with the driver protocol
    //
    Status = gBS->CloseProtocol ( Controller,
                                  mDriverProtocol,
                                  DriverBinding->DriverBindingHandle,
                                  Controller );
    if ( !EFI_ERROR ( Status )) {
      //
      //  Stop the driver
      //
      DEBUG (( DEBUG_INIT,
                "0x%016Lx: I2cBus stopped\r\n",
                (UINT64)(UINTN)I2cBus ));
      I2cBusApiStop ( I2cBus );
      if (( NULL != I2cBus->DeviceListHead )
        || ( NULL != I2cBus->DeviceListTail )) {
        //
        //  Not done yet
        //
        Status = EFI_NOT_READY;
      }
      else {
        //
        //  Remove the driver protocol
        //
        Status = gBS->UninstallMultipleProtocolInterfaces (
                      Controller,
                      mDriverProtocol,
                      DriverProtocol,
                      NULL );
        if ( !EFI_ERROR ( Status )) {
          //
          //  Release the enumeration API
          //
          Status = gBS->CloseProtocol ( Controller,
                                        &gEfiI2cEnumerateProtocolGuid,
                                        DriverBinding->DriverBindingHandle,
                                        Controller );
          ASSERT ( Status == EFI_SUCCESS );

          //
          //  Release the I2C controller
          //
          Status = gBS->CloseProtocol ( Controller,
                                        &gEfiI2cHostProtocolGuid,
                                        DriverBinding->DriverBindingHandle,
                                        Controller );
          ASSERT ( Status == EFI_SUCCESS );

          //
          //  Release the context
          //
          DEBUG (( DEBUG_POOL | DEBUG_INFO,
                    "0x%016Lx: pI2cBus released\r\n",
                    (UINT64)(UINTN)I2cBus ));
          FreePool ( I2cBus );
        }
        else {
          DEBUG (( DEBUG_ERROR,
                    "ERROR - Failed to uninstall driver protocol, Status: %r\r\n",
                    Status ));
        }
      }
    }
    else {
      DEBUG (( DEBUG_ERROR,
                "ERROR - Failed to close driver protocol, Status: %r\r\n",
                Status ));
    }
  }

  //
  //  Display exit
  //
  DEBUG (( DEBUG_LOAD, "I2cBusDriverStop exiting, Status: %r\r\n", Status ));

  //
  //  Return the stop status
  //
  return Status;
}


/**
  Driver binding protocol support
**/
EFI_DRIVER_BINDING_PROTOCOL mI2cBusDriverBinding = {
  I2cBusDriverSupported,
  I2cBusDriverStart,
  I2cBusDriverStop,
  0x10,
  NULL,
  NULL
};

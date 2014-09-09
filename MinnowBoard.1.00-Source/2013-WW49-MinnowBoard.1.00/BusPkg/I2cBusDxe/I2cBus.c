/** @file
  Implement the I2C bus protocol.
  
  Copyright (c) 2012-2013, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "I2cBus.h"


/**
  Enumerate the I2C bus

  This routine walks the platform specific data describing the
  I2C bus to create the I2C devices where driver GUIDs were
  specified.

  @param[in] I2cBus       Address of an I2C_BUS_CONTEXT structure
  @param[in] Controller   Handle to the controller

  @retval EFI_SUCCESS     The bus is successfully configured

**/
EFI_STATUS
I2cBusEnumerate (
  IN I2C_BUS_CONTEXT *I2cBus,
  IN EFI_HANDLE Controller
  )
{
  CONST EFI_I2C_DEVICE *Device;
  EFI_HANDLE Handle;
  I2C_DEVICE_CONTEXT *I2cDevice;
  I2C_DEVICE_CONTEXT *I2cDevicePrevious;
  EFI_STATUS Status;
  EFI_TPL TplPrevious;

  //
  //  Display entry
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cBusEnumerate entered\r\n" ));

  //
  //  Assume success
  //
  Status = EFI_SUCCESS;

  //
  //  Walk the list of I2C devices on this bus
  //
  Device = NULL;
  do {
    //
    //  Get the next I2C device
    //
    Status = I2cBus->I2cEnumerate->Enumerate ( I2cBus->I2cEnumerate,
                                               &Device );
    if ( EFI_ERROR ( Status )) {
      if ( EFI_NO_MAPPING != Status ) {
        break;
      }
      Device = NULL;
      Status = EFI_SUCCESS;
    }
    if ( NULL != Device ) {
      //
      //  Another device is on the I2C bus
      //  Determine if the device info is valid
      //
      if (( NULL != Device->DeviceGuid )
        && ( 0 < Device->SlaveAddressCount )
        && ( NULL != Device->SlaveAddressArray )) {
        //
        //  Allocate the I2C device context
        //
        I2cDevice = AllocateZeroPool ( sizeof ( *I2cDevice ));
        if ( NULL == I2cDevice ) {
          DEBUG (( DEBUG_ERROR, "ERROR - No memory for I2C device structure!\r\n" ));
          Status = EFI_OUT_OF_RESOURCES;
          break;
        }

        //
        //  Initialize the device context
        //
        I2cDevice->Signature = I2C_DEVICE_SIGNATURE;
        I2cDevice->I2cBus = I2cBus;

        //
        //  Build the I/O protocol
        //
        I2cDevice->IoApi.QueueRequest = &I2cBusQueueRequest;
        I2cDevice->IoApi.I2cDevice = Device;
        I2cDevice->IoApi.I2cControllerCapabilities = I2cBus->I2cHost->I2cControllerCapabilities;

        //
        //  Build the device path
        //
        Status = I2cBusDevicePathAppend ( I2cDevice, Controller );
        if ( EFI_ERROR ( Status )) {
          //
          //  Out of resources
          //
          break;
        }

        //
        //  Install the protocol
        //
        Handle = NULL;
        Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEfiI2cIoProtocolGuid,
                  &I2cDevice->IoApi,
                  &gEfiDevicePathProtocolGuid,
                  I2cDevice->DevPath,
                  NULL );
        if ( EFI_ERROR ( Status )) {
          DEBUG (( DEBUG_ERROR,
                    "ERROR - Failed to install the device protocol, Status: %r\r\n",
                    Status ));
          break;
        }
        else {
          //
          //  Synchronize with the other threads
          //
          TplPrevious = gBS->RaiseTPL ( TPL_I2C_SYNC );

          //
          //  Add this device to the device list
          //
          I2cDevicePrevious = I2cBus->DeviceListTail;
          if ( NULL == I2cDevicePrevious ) {
            I2cBus->DeviceListHead = I2cDevice;
          }
          else {
            I2cDevicePrevious->Next = I2cDevice;
          }
          I2cBus->DeviceListTail = I2cDevice;

          //
          //  Release the thread synchronization
          //
          gBS->RestoreTPL ( TplPrevious );
        }
      }
    }
  } while ( NULL != Device );

  //
  //  Display exit
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cBusEnumerate exiting, Status: %r\r\n", Status ));
  return Status;
}


/**
  Queue an I2C transaction for execution on the I2C device.

  This routine must be called at or below TPL_NOTIFY.  For synchronous
  requests this routine must be called at or below TPL_CALLBACK.

  This routine queues an I2C transaction to the I2C controller for
  execution on the I2C bus.

  When Event is NULL, QueueRequest() operates synchronously and returns
  the I2C completion status as its return value.

  When Event is not NULL, QueueRequest() synchronously returns EFI_SUCCESS
  indicating that the asynchronous I2C transaction was queued.  The values
  above are returned in the buffer pointed to by I2cStatus upon the
  completion of the I2C transaction when I2cStatus is not NULL.

  The upper layer driver writer provides the following to the platform
  vendor:
  
  1.  Vendor specific GUID for the I2C part
  2.  Guidance on proper construction of the slave address array when the
      I2C device uses more than one slave address.  The I2C bus protocol
      uses the SlaveAddressIndex to perform relative to physical address
      translation to access the blocks of hardware within the I2C device.

  @param[in] This               Pointer to an EFI_I2C_IO_PROTOCOL structure.
  @param[in] SlaveAddressIndex  Index value into an array of slave addresses
                                for the I2C device.  The values in the array
                                are specified by the board designer, with the
                                third party I2C device driver writer providing
                                the slave address order.

                                For devices that have a single slave address,
                                this value must be zero.  If the I2C device
                                uses more than one slave address then the
                                third party (upper level) I2C driver writer
                                needs to specify the order of entries in the
                                slave address array.

                                \ref ThirdPartyI2cDrivers "Third Party I2C
                                Drivers" section in I2cMaster.h.
  @param[in] Event              Event to signal for asynchronous transactions,
                                NULL for synchronous transactions
  @param[in] RequestPacket      Pointer to an EFI_I2C_REQUEST_PACKET structure
                                describing the I2C transaction
  @param[out] I2cStatus         Optional buffer to receive the I2C transaction
                                completion status

  @retval EFI_SUCCESS           The asynchronous transaction was successfully
                                queued when Event is not NULL.
  @retval EFI_SUCCESS           The transaction completed successfully when
                                Event is NULL.
  @retval EFI_ABORTED           The request did not complete because the driver
                                binding Stop() routine was called.
  @retval EFI_BAD_BUFFER_SIZE   The RequestPacket->LengthInBytes value is too
                                large.
  @retval EFI_DEVICE_ERROR      There was an I2C error (NACK) during the
                                transaction.
  @retval EFI_INVALID_PARAMETER RequestPacket is NULL
  @retval EFI_NOT_FOUND         Reserved bit set in the SlaveAddress parameter
  @retval EFI_NO_MAPPING        The EFI_I2C_HOST_PROTOCOL could not set the
                                bus configuration required to access this I2C
                                device.
  @retval EFI_NO_RESPONSE       The I2C device is not responding to the slave
                                address selected by SlaveAddressIndex.
                                EFI_DEVICE_ERROR will be returned if the
                                controller cannot distinguish when the NACK
                                occurred.
  @retval EFI_OUT_OF_RESOURCES  Insufficient memory for I2C transaction
  @retval EFI_UNSUPPORTED       The controller does not support the requested
                                transaction.

**/
EFI_STATUS
EFIAPI
I2cBusQueueRequest (
  IN CONST EFI_I2C_IO_PROTOCOL *This,
  IN UINTN SlaveAddressIndex,
  IN EFI_EVENT Event OPTIONAL,
  IN EFI_I2C_REQUEST_PACKET *RequestPacket,
  OUT EFI_STATUS *I2cStatus OPTIONAL
  )
{
  CONST EFI_I2C_DEVICE *Device;
  I2C_BUS_CONTEXT *I2cBus;
  CONST EFI_I2C_HOST_PROTOCOL *I2cHost;
  I2C_DEVICE_CONTEXT *I2cDevice;
  EFI_STATUS Status;

  //
  //  Display entry
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cBusQueueRequest entered\r\n" ));

  //
  //  Assume failure
  //
  Status = EFI_ACCESS_DENIED;

  //
  //  Validate the I2C slave index
  //
  I2cDevice = I2C_DEVICE_CONTEXT_FROM_PROTOCOL ( This );
  Device = I2cDevice->IoApi.I2cDevice;
  if ( SlaveAddressIndex < Device->SlaveAddressCount ) {
    //
    //  Locate the host protocol
    //
    I2cBus = I2cDevice->I2cBus;
    I2cHost = I2cBus->I2cHost;

    //
    //  Display the operation
    //
    DEBUG_CODE_BEGIN ( );
      UINTN Index;

      DEBUG (( DEBUG_I2C_OPERATION,
               "I2cBus\r\n  0x%03x: SlaveAddress\r\n",
               Device->I2cBusConfiguration,
               Device->SlaveAddressArray [ SlaveAddressIndex ] ));
      for ( Index = 0; RequestPacket->OperationCount > Index; Index++ ) {
        DEBUG (( DEBUG_I2C_OPERATION,
                 "  0x%016Lx: Buffer\r\n  0x%08x: LengthInBytes\r\n  0x%08x: Flags - %s%s%s%s%s%s\r\n",
                 RequestPacket->Operation [ Index ].Buffer,
                 RequestPacket->Operation [ Index ].LengthInBytes,
                 RequestPacket->Operation [ Index ].Flags,
                 ( RequestPacket->Operation [ Index ].Flags & I2C_FLAG_READ ) ? "Read" : "Write",
                 ( RequestPacket->Operation [ Index ].Flags & I2C_FLAG_SMBUS_OPERATION ) ? ", SMBus" : "",
                 ( RequestPacket->Operation [ Index ].Flags & I2C_FLAG_SMBUS_BLOCK ) ? "Block" : "",
                 ( RequestPacket->Operation [ Index ].Flags & I2C_FLAG_SMBUS_PROCESS_CALL ) ? ", Process Call" : "",
                 ( RequestPacket->Operation [ Index ].Flags & I2C_FLAG_SMBUS_PEC ) ? ", PEC" : "" ));
        if (( 0 != ( DEBUG_I2C_OPERATION & PcdGet32 ( PcdDebugPrintErrorLevel )))
          && ( 0 == ( RequestPacket->Operation [ Index ].Flags & I2C_FLAG_READ ))) {
          AsciiDump ( RequestPacket->Operation [ Index ].Buffer,
                      RequestPacket->Operation [ Index ].Buffer,
                      RequestPacket->Operation [ Index ].LengthInBytes );
        }
      }
    DEBUG_CODE_END ( );

    //
    //  Start the I2C operation
    //
    Status = I2cHost->QueueRequest ( I2cHost,
                                     Device->I2cBusConfiguration,
                                     Device->SlaveAddressArray [ SlaveAddressIndex ],
                                     Event,
                                     RequestPacket,
                                     I2cStatus );

    //
    //  Display the read data
    //
    DEBUG_CODE_BEGIN ( );
      UINTN Index;

      for ( Index = 0; RequestPacket->OperationCount > Index; Index++ ) {
        if (( 0 != ( DEBUG_I2C_OPERATION & PcdGet32 ( PcdDebugPrintErrorLevel )))
          && ( 0 != ( RequestPacket->Operation [ Index ].Flags & I2C_FLAG_READ ))) {
          AsciiDump ( RequestPacket->Operation [ Index ].Buffer,
                      RequestPacket->Operation [ Index ].Buffer,
                      RequestPacket->Operation [ Index ].LengthInBytes );
        }
      }
    DEBUG_CODE_END ( );
  }

  //
  //  Display exit
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cBusQueueRequest exiting, Status: %r\r\n", Status ));
  return Status;
}


/**
  Start the I2C driver

  This routine allocates the necessary resources for the driver.

  This routine is called by I2cBusDriverStart to complete the driver
  initialization.

  @param [in] I2cBus          Address of an I2C_BUS_CONTEXT structure
  @param[in] Controller       Handle to the controller

  @retval EFI_SUCCESS         Driver API properly initialized
  
**/
EFI_STATUS
I2cBusApiStart (
  IN I2C_BUS_CONTEXT *I2cBus,
  IN EFI_HANDLE Controller
  )
{
  CONST EFI_I2C_HOST_PROTOCOL *I2cHost;
  EFI_STATUS Status;

  //
  //  Display entry
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cBusApiStart entered\r\n" ));

  //
  //  Build the I2C bus protocol
  //
  I2cHost = I2cBus->I2cHost;

  //
  //  Enumerate the I2C bus
  //
  Status = I2cBusEnumerate ( I2cBus, Controller );

  //
  //  Display exit
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cBusApiStart exiting, Status: %r\r\n", Status ));

  //
  //  Return the startup status
  //
  return Status;
}


/**
  Stop the I2C driver

  This routine releases the resources allocated by I2cApiStart.

  This routine is called by I2cBusDriverStop to initiate the driver
  shutdown.

  @param [in] I2cBus          Address of an I2C_BUS_CONTEXT structure

**/
VOID
I2cBusApiStop (
  IN I2C_BUS_CONTEXT *I2cBus
  )
{
  EFI_HANDLE *Handle;
  EFI_HANDLE *HandleArray;
  EFI_HANDLE *HandleArrayEnd;
  UINTN HandleCount;
  I2C_DEVICE_CONTEXT *I2cDevice;
  I2C_DEVICE_CONTEXT *I2cDevicePrevious;
  EFI_I2C_IO_PROTOCOL *I2cIoProtocol;
  EFI_STATUS Status;
  EFI_TPL TplPrevious;

  //
  //  Display entry
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cBusApiStop entered\r\n" ));

  //
  //  Locate the I2C devices
  //
  Status = gBS->LocateHandleBuffer ( ByProtocol,
                                     &gEfiI2cIoProtocolGuid,
                                     NULL,
                                     &HandleCount,
                                     &HandleArray );
  if ( !EFI_ERROR ( Status ))  {
    Handle = HandleArray;
    HandleArrayEnd = &Handle [ HandleCount ];
    while ( HandleArrayEnd > Handle ) {
      //
      //  Remove the driver stack
      //
      Status = gBS->OpenProtocol ( *Handle,
                                   &gEfiI2cIoProtocolGuid,
                                   (VOID **)&I2cIoProtocol,
                                   mI2cBusDriverBinding.DriverBindingHandle,
                                   *Handle,
                                   EFI_OPEN_PROTOCOL_BY_DRIVER | EFI_OPEN_PROTOCOL_EXCLUSIVE );
      if ( EFI_ERROR ( Status )) {
        DEBUG (( DEBUG_ERROR,
                  "ERROR - Failed to remove I2C driver stack, Status: %r\r\n",
                  Status ));
      }
      else {
        //
        //  Done with the protocol
        //
        I2cDevice = I2C_DEVICE_CONTEXT_FROM_PROTOCOL ( I2cIoProtocol );
        gBS->CloseProtocol ( *Handle,
                             &gEfiI2cIoProtocolGuid,
                             mI2cBusDriverBinding.DriverBindingHandle,
                             *Handle );

        //
        //  Remove this protocol
        //
        Status = gBS->UninstallMultipleProtocolInterfaces ( *Handle,
                                                            &gEfiI2cIoProtocolGuid,
                                                            &I2cDevice->IoApi,
                                                            &gEfiDevicePathProtocolGuid,
                                                            I2cDevice->DevPath,
                                                            NULL );
        if ( EFI_ERROR ( Status )) {
          DEBUG (( DEBUG_ERROR,
                    "ERROR - Failed to uninstall gEfiI2cIoProtocol, Status: %r\r\n",
                    Status ));
        }
        else {
          //
          //  Synchronize with the other threads
          //
          TplPrevious = gBS->RaiseTPL ( TPL_I2C_SYNC );

          //
          //  Locate this device in the list
          //
          I2cDevicePrevious = I2cBus->DeviceListHead;
          if ( I2cDevice != I2cDevicePrevious ) {
            //
            //  The device is at the head of the list
            //
            I2cBus->DeviceListHead = I2cDevice->Next;
            I2cDevicePrevious = NULL;
          }
          else {
            //
            //  Locate the device in the middle of the list
            //
            while ( I2cDevice != I2cDevicePrevious->Next ) {
              I2cDevicePrevious = I2cDevicePrevious->Next;
            }

            //
            //  Remove the device form the middle of the list
            //
            I2cDevicePrevious->Next = I2cDevice->Next;
          }

          //
          //  Remove the device from the end of the list if necessary
          //
          if ( I2cBus->DeviceListTail == I2cDevice ) {
            I2cBus->DeviceListTail = I2cDevicePrevious;
          }

          //
          //  Release the thread synchronization
          //
          gBS->RestoreTPL ( TplPrevious );

          //
          //  Display the device
          //
          DEBUG (( DEBUG_I2C_DEVICE,
                    "0x%016Lx: Bus freeing I2C Device 0x%x on I2C bus configuration %d\r\n",
                    I2cDevice,
                    I2cDevice->IoApi.I2cDevice->SlaveAddressArray [ 0 ],
                    I2cDevice->IoApi.I2cDevice->I2cBusConfiguration ));

          //
          //  Free this device
          //
          FreePool ( I2cDevice );
        }
      }

      //
      //  Set the next handle
      //
      Handle += 1;
    }

    //
    //  Done with the handles
    //
    FreePool ( HandleArray );
  }

  //
  //  Display exit
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cBusApiStop exiting\r\n" ));
}

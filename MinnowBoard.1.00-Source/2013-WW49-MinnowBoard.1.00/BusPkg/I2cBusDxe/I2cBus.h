/** @file
  I2C Bus Driver Declarations

  Copyright (c) 2012, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _I2C_BUS_H
#define _I2C_BUS_H

#include <Uefi.h>
#include <Library/AsciiDump.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/DriverLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiLib.h>

#include <Protocol/DriverBinding.h>
#include <Protocol/I2cEnumerate.h>
#include <Protocol/I2cHost.h>
#include <Protocol/I2cIo.h>
#include <Protocol/LoadedImage.h>

///
/// Debug I2C operation
///
#define DEBUG_I2C_OPERATION           0x40000000

///
/// Debug routine entry and exit
///
#define DEBUG_I2C_ROUTINE_ENTRY_EXIT  0x20000000

///
/// Debug I2C data blobs
///
#define DEBUG_I2C_DATA_BLOB           0x01000000

///
/// Debug I2C bus configuration
///
#define DEBUG_I2C_DEVICE              0x08000000

///
/// "I2Cb"
///
#define I2C_BUS_SIGNATURE       0x62433249

///
/// "I2Cd"
///
#define I2C_DEVICE_SIGNATURE    0x64433249

///
/// TPL used to synchronize add/remove from list
///
#define TPL_I2C_SYNC            TPL_NOTIFY

extern EFI_DRIVER_BINDING_PROTOCOL mI2cBusDriverBinding;

typedef struct _I2C_BUS_CONTEXT I2C_BUS_CONTEXT;
typedef struct _I2C_DEVICE_CONTEXT I2C_DEVICE_CONTEXT;

///
/// I2C device context
///
/// Each I2C device instance uses an I2C_DEVICE_CONTEXT structure
/// to manage access to the I2C device
///
struct _I2C_DEVICE_CONTEXT {
  ///
  /// Structure identification
  ///
  UINTN Signature;

  ///
  /// Next device in the list
  ///
  I2C_DEVICE_CONTEXT *Next;

  ///
  /// Upper level API to support the I2C device I/O
  ///
  EFI_I2C_IO_PROTOCOL IoApi;

  ///
  /// I2C bus configuration to access the I2C device
  ///
  UINTN I2cBusConfiguration;

  ///
  /// Device path for this device
  ///
  EFI_DEVICE_PATH_PROTOCOL *DevPath;

  ///
  /// Context for the common I/O support including the
  /// lower level API to the host controller.
  ///
  I2C_BUS_CONTEXT *I2cBus;
};

#define I2C_DEVICE_CONTEXT_FROM_PROTOCOL(a) CR (a, I2C_DEVICE_CONTEXT, IoApi, I2C_DEVICE_SIGNATURE)  ///< Locate I2C_DEVICE_CONTEXT from protocol

/**
  I2C bus context

  Each I2C bus instance uses an I2C_BUS_CONTEXT structure
  to manage the I2C host.
**/
struct _I2C_BUS_CONTEXT {
  ///
  /// Structure identification
  ///
  UINTN Signature;

  ///
  /// Head of the device list
  ///
  I2C_DEVICE_CONTEXT *DeviceListHead;

  ///
  /// Tail of the device list
  ///
  I2C_DEVICE_CONTEXT *DeviceListTail;

  ///
  /// Platform API to enumerate the I2C devices
  ///
  CONST EFI_I2C_ENUMERATE_PROTOCOL *I2cEnumerate;

  ///
  /// Lower level API to the host controller
  ///
  CONST EFI_I2C_HOST_PROTOCOL *I2cHost;
};

///
/// Locate I2C_BUS_CONTEXT from protocol
///
#define I2C_BUS_CONTEXT_FROM_PROTOCOL(a)  CR (a, I2C_BUS_CONTEXT, Signature, I2C_BUS_SIGNATURE)


/**
  Start the I2C driver

  This routine allocates the necessary resources for the driver.

  This routine is called by I2cBusDriverStart to complete the driver
  initialization.

  @param[in] I2cBus           Address of an I2C_BUS_CONTEXT structure
  @param[in] Controller       Handle to the controller

  @retval EFI_SUCCESS         Driver API properly initialized
  
**/
EFI_STATUS
I2cBusApiStart (
  IN I2C_BUS_CONTEXT *I2cBus,
  IN EFI_HANDLE Controller
  );

/**
  Stop the I2C driver

  This routine releases the resources allocated by I2cApiStart.

  This routine is called by I2cBusDriverStop to initiate the driver
  shutdown.

  @param[in] I2cBus           Address of an I2C_BUS_CONTEXT structure

**/
VOID
I2cBusApiStop (
  IN I2C_BUS_CONTEXT *I2cBus
  );

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
  );

/**
  Perform an I2C operation on the device

  This routine must be called at or below TPL_NOTIFY.  For synchronous
  requests this routine must be called at or below TPL_CALLBACK.

  N.B. The typical consumers of this API are the I2C bus driver and
  on rare occasions the I2C test application.  Extreme care must be
  taken by other consumers of this API to prevent confusing the
  third party I2C drivers due to a state change at the I2C device
  which the third party I2C drivers did not initiate.  I2C platform
  drivers may use this API within these guidelines.

  This routine queues an operation to the I2C controller for execution
  on the I2C bus.

  As an upper layer driver writer, the following need to be provided
  to the platform vendor:
  
  1.  ACPI CID value or string - this is used to connect the upper layer
      driver to the device.
  2.  Slave address array guidance when the I2C device uses more than one
      slave address.  This is used to access the blocks of hardware within
      the I2C device.

  @param[in] This               Address of an EFI_I2C_IO_PROTOCOL
                                structure
  @param[in] SlaveAddressIndex  Index into an array of slave addresses for
                                the I2C device.  The values in the array are
                                specified by the board designer, with the
                                driver writer providing the slave address
                                order.  For devices that have a single 
                                slave address, this value must be zero.
                                If the I2C device uses more than one slave
                                address then the upper level driver writer
                                needs to specify the order of entries in the
                                slave address array.
  @param[in] Event              Event to set for asynchronous operations,
                                NULL for synchronous operations
  @param[in] RequestPacket      Address of an EFI_I2C_REQUEST_PACKET
                                structure describing the I2C operation
  @param[out] I2cStatus         Optional buffer to receive the I2C operation
                                completion status

  @return  When Event is NULL, QueueRequest operates synchrouously and returns the
  I2C completion status as its return value.  In this case it is recommended to use
  NULL for I2cStatus.  The values returned from QueueRequest are:

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_ABORTED           The request did not complete because the driver
                                was shutdown.
  @retval EFI_ACCESS_DENIED     Invalid SlaveAddressIndex value
  @retval EFI_BAD_BUFFER_SIZE   The WriteBytes or ReadBytes buffer size is too large.
  @retval EFI_DEVICE_ERROR      There was an I2C error (NACK) during the operation.
                                One possible cause is that the slave device is not present.
  @retval EFI_INVALID_PARAMETER RequestPacket is NULL
  @retval EFI_INVALID_PARAMETER TPL is too high
  @retval EFI_NOT_FOUND         SlaveAddress exceeds maximum address
  @retval EFI_NO_MAPPING        Invalid I2cBusConfiguration value due to invalid platform
                                data
  @retval EFI_NO_RESPONSE       The I2C device is not responding to the
                                slave address.  EFI_DEVICE_ERROR may also be
                                returned if the controller cannot distinguish
                                when the NACK occurred.
  @retval EFI_OUT_OF_RESOURCES  Insufficient memory for I2C operation
  @retval EFI_TIMEOUT           The transaction did not complete within an internally
                                specified timeout period.

  @return   When Event is not NULL, QueueRequest synchronously returns EFI_NOT_READY
  indicating that the I2C operation was started asynchronously.  The following values
  are returned upon the completion of the I2C operation when I2cStatus is not NULL:

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_ABORTED           The request did not complete because the driver
                                was shutdown.
  @retval EFI_ACCESS_DENIED     Invalid SlaveAddressIndex value
  @retval EFI_BAD_BUFFER_SIZE   The WriteBytes or ReadBytes buffer size is too large.
  @retval EFI_DEVICE_ERROR      There was an I2C error (NACK) during the operation.
                                One possible cause is that the slave device is not present.
  @retval EFI_INVALID_PARAMETER RequestPacket is NULL
  @retval EFI_INVALID_PARAMETER TPL is too high
  @retval EFI_NOT_FOUND         SlaveAddress exceeds maximum address
  @retval EFI_NO_MAPPING        Invalid I2cBusConfiguration value due to invalid platform
                                data
  @retval EFI_NO_RESPONSE       The I2C device is not responding to the
                                slave address.  EFI_DEVICE_ERROR may also be
                                returned if the controller cannot distinguish
                                when the NACK occurred.
  @retval EFI_OUT_OF_RESOURCES  Insufficient memory for I2C operation
  @retval EFI_TIMEOUT           The transaction did not complete within an internally
                                specified timeout period.

**/
EFI_STATUS
EFIAPI
I2cBusQueueRequest (
  IN CONST EFI_I2C_IO_PROTOCOL *This,
  IN UINTN SlaveAddressIndex,
  IN EFI_EVENT Event OPTIONAL,
  IN EFI_I2C_REQUEST_PACKET *RequestPacket,
  OUT EFI_STATUS *I2cStatus
  );

#endif  //  _I2C_BUS_H

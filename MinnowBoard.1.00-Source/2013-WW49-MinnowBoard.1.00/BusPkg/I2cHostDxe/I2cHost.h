/** @file
  I2C Host Driver Declarations

  Copyright (c) 2012, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _I2C_HOST_H_
#define _I2C_HOST_H_

#include <Uefi.h>

#include <IndustryStandard/Scsi.h>

#include <Library/BaseMemoryLib.h>
#include <Library/DriverLib.h>

#include <Protocol/I2cHost.h>
#include <Protocol/I2cMaster.h>
#include <Protocol/I2cBusConfigurationManagement.h>

///
/// Debug I2C operation
///
#define DEBUG_I2C_OPERATION           0x40000000

///
/// Debug routine entry and exit
///
#define DEBUG_I2C_ROUTINE_ENTRY_EXIT  0x20000000

///
/// "I2Ch"
///
#define I2C_HOST_SIGNATURE      0x68433249

///
/// Synchronize access to the list of requests
///
#define TPL_I2C_SYNC    TPL_NOTIFY


extern EFI_DRIVER_BINDING_PROTOCOL mI2cHostDriverBinding;
extern CONST EFI_I2C_HOST_PROTOCOL mI2cHostProtocol;

///
/// I2C Request
///
typedef struct _I2C_REQUEST I2C_REQUEST;
struct _I2C_REQUEST {
  ///
  /// Next request in the pending request list
  ///
  I2C_REQUEST *Next;

  ///
  /// I2C bus configuration for the operation
  ///
  UINTN I2cBusConfiguration;

  ///
  /// I2C slave address for the operation
  ///
  UINTN SlaveAddress;

  ///
  /// Event to set for asynchronous operations, NULL for
  /// synchronous operations
  ///
  EFI_EVENT Event;

  ///
  /// I2C operation description
  ///
  EFI_I2C_REQUEST_PACKET RequestPacket;

  ///
  /// Optional buffer to receive the I2C operation completion status
  ///
  EFI_STATUS *Status;
};

///
/// I2C host context
///
/// Each I2C host instance uses an I2C_HOST_CONTEXT structure
/// to maintain its context.
///
typedef struct {
  ///
  /// Structure identification
  ///
  UINTN Signature;

  ///
  /// Current I2C bus configuration
  ///
  UINTN I2cBusConfiguration;

  ///
  /// I2C bus configuration management event
  ///
  EFI_EVENT I2cBusConfigurationEvent;

  ///
  /// I2C operation completion event
  ///
  EFI_EVENT I2cEvent;

  ///
  /// I2C operation and I2C bus configuration management status
  ///
  EFI_STATUS Status;

  ///
  /// I2C bus configuration management operation pending
  ///
  volatile BOOLEAN I2cBusConfigurationManagementPending;

  ///
  /// State of the host driver, FALSE = running, TRUE = shutting down
  ///
  volatile BOOLEAN ShuttingDown;

  ///
  /// Head of the pending request list
  ///
  I2C_REQUEST * volatile RequestListHead;

  ///
  /// Tail of the pending request list
  ///
  I2C_REQUEST * volatile RequestListTail;

  ///
  /// Upper level API
  ///
  EFI_I2C_HOST_PROTOCOL HostApi;

  ///
  /// I2C bus configuration management protocol
  ///
  CONST EFI_I2C_BUS_CONFIGURATION_MANAGEMENT_PROTOCOL *I2cBusConfigurationManagement;

  ///
  /// Lower level API for I2C master (controller)
  ///
  CONST EFI_I2C_MASTER_PROTOCOL *I2cMaster;
} I2C_HOST_CONTEXT;

///
/// Locate I2C_HOST_CONTEXT from protocol
///
#define I2C_HOST_CONTEXT_FROM_PROTOCOL(a) CR (a, I2C_HOST_CONTEXT, HostApi, I2C_HOST_SIGNATURE)


/**
  Complete the current request with an error

  @param[in] I2cHost  Address of an I2C_HOST_CONTEXT structure.
  @param[in] Status   Status of the I<sub>2</sub>C operation.

  @return This routine returns the input status value.

**/
EFI_STATUS
I2cHostRequestCompleteError (
  I2C_HOST_CONTEXT *I2cHost,
  EFI_STATUS Status
  );

/**
  Start the I2C driver

  This routine allocates the necessary resources for the driver.

  This routine is called by I2cHostDriverStart to complete the driver
  initialization.

  @param[in] I2cHost          Address of an I2C_HOST_CONTEXT structure

  @retval EFI_SUCCESS         Driver API properly initialized
  
**/
EFI_STATUS
I2cHostApiStart (
  IN I2C_HOST_CONTEXT *I2cHost
  );

/**
  Stop the I2C driver

  This routine releases the resources allocated by I2cApiStart.

  This routine is called by I2cHostDriverStop to initiate the driver
  shutdown.

  @param[in] I2cHost          Address of an I2C_HOST_CONTEXT structure

**/
VOID
I2cHostApiStop (
  IN I2C_HOST_CONTEXT *I2cHost
  );

/**
  Complete the current request

  @param[in] I2cHost  Address of an I2C_HOST_CONTEXT structure.
  @param[in] Status   Status of the I<sub>2</sub>C operation.

  @return This routine returns the input status value.

**/
EFI_STATUS
I2cHostRequestComplete (
  I2C_HOST_CONTEXT *I2cHost,
  EFI_STATUS Status
  );

/**
  Enable access to the I2C bus configuration

  @param[in] I2cHost    Address of an I2C_HOST_CONTEXT structure

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_ABORTED           The request did not complete because the driver
                                was shutdown.
  @retval EFI_BAD_BUFFER_SIZE   The WriteBytes or ReadBytes buffer size is too large.
  @retval EFI_DEVICE_ERROR      There was an I2C error (NACK) during the operation.
                                This could indicate the slave device is not present.
  @retval EFI_INVALID_PARAMETER RequestPacket is NULL
  @retval EFI_NO_MAPPING        Invalid I2cBusConfiguration value
  @retval EFI_NO_RESPONSE       The I2C device is not responding to the
                                slave address.  EFI_DEVICE_ERROR may also be
                                returned if the controller can not distinguish
                                when the NACK occurred.
  @retval EFI_NOT_FOUND         I2C slave address exceeds maximum address
  @retval EFI_NOT_READY         I2C bus is busy or operation pending, wait for
                                the event and then read status.
  @retval EFI_OUT_OF_RESOURCES  Insufficient memory for I2C operation
  @retval EFI_TIMEOUT           The transaction did not complete within an internally
                                specified timeout period.

**/
EFI_STATUS
I2cHostRequestEnable (
  I2C_HOST_CONTEXT *I2cHost
  );

#endif  //  _I2C_HOST_H_

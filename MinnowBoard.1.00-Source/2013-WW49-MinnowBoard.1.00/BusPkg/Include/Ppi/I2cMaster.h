/** @file
  I2C Master PPI interface

  A driver or application uses the I2C master PPI to
  manage the master mode of an I2C controller.  Note that
  access is restricted to the current configuration of the I2C
  bus.

  Copyright (c) 2012-2013, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __I2C_MASTER_H__
#define __I2C_MASTER_H__

#include <Protocol/I2cSharedDxePei.h>

///
/// I2C master mode PEIM-to-PEIM Interface
///
/// This PPI manipulates the I2C host controller to perform transactions
/// as a master on the I2C bus using the current state of any switches or
/// multiplexers in the I2C bus.
///
typedef struct _EFI_PEI_I2C_MASTER    EFI_PEI_I2C_MASTER;

/**
  Set the frequency for the I2C clock line.

  The software and controller do a best case effort of using the specified
  frequency for the I2C bus.  If the frequency does not match exactly then
  the I2C master protocol selects the next lower frequency to avoid
  exceeding the operating conditions for any of the I2C devices on the bus.
  For example if 400 KHz was specified and the controller's divide network
  only supports 402 KHz or 398 KHz then the controller would be set to 398
  KHz.  If there are no lower frequencies available, then return
  EFI_UNSUPPORTED.

  @param[in] This             Pointer to an EFI_PEI_I2C_MASTER_PPI structure.
  @param[in] BusClockHertz    Pointer to the requested I2C bus clock frequency
                              in Hertz.  Upon return this value contains
                              the actual frequency in use by the I2C
                              controller.

  @retval EFI_SUCCESS           The bus frequency was set successfully.
  @retval EFI_INVALID_PARAMETER BusClockHertz is NULL
  @retval EFI_UNSUPPORTED       The controller does not support this frequency.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_PEI_I2C_MASTER_PPI_SET_BUS_FREQUENCY) (
  IN EFI_PEI_I2C_MASTER *This,
  IN UINTN BusClockHertz
  );

/**
  Reset the I2C controller and configure it for use.

  The I2C controller is reset.  The caller must call SetBusFrequency()
  after calling Reset().

  @param[in] This             Pointer to an EFI_PEI_I2C_MASTER_PPI
                              structure.

  @retval EFI_SUCCESS         The reset completed successfully.
  @retval EFI_DEVICE_ERROR    The reset operation failed.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_PEI_I2C_MASTER_PPI_RESET) (
  IN EFI_PEI_I2C_MASTER *This
  );

/**
  Start an I2C transaction on the host controller.

  This function initiates an I2C transaction on the controller.

  The transaction is performed by sending a start-bit and selecting
  the I2C device with the specified I2C slave address and then performing
  the specified I2C operations.  When multiple operations are requested
  they are separated with a repeated start bit and the slave address.
  The transaction is terminated with a stop bit.  When the transaction
  completes, the status value is returned.

  @param[in] This             Pointer to an EFI_PEI_I2C_MASTER_PPI structure.
  @param[in] SlaveAddress     Address of the device on the I2C bus.  Set the
                              I2C_ADDRESSING_10_BIT when using 10-bit
                              addresses, clear this bit for 7-bit addressing.
                              Bits 0-6 are used for 7-bit I2C slave addresses
                              and bits 0-9 are used for 10-bit I2C slave
                              addresses.
  @param[in] RequestPacket    Pointer to an EFI_I2C_REQUEST_PACKET structure
                              describing the I2C transaction.

  @retval EFI_SUCCESS           The transaction completed successfully.
  @retval EFI_BAD_BUFFER_SIZE   The RequestPacket->LengthInBytes value is
                                too large.
  @retval EFI_DEVICE_ERROR      There was an I2C error (NACK) during the
                                transaction.
  @retval EFI_INVALID_PARAMETER RequestPacket is NULL
  @retval EFI_NO_RESPONSE       The I2C device is not responding to the
                                slave address.  EFI_DEVICE_ERROR will be
                                returned if the controller cannot distinguish
                                when the NACK occurred.
  @retval EFI_NOT_FOUND         Reserved bit set in the SlaveAddress parameter
  @retval EFI_OUT_OF_RESOURCES  Insufficient memory for I2C transaction
  @retval EFI_UNSUPPORTED       The controller does not support the requested
                                transaction.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_PEI_I2C_MASTER_PPI_START_REQUEST) (
  IN EFI_PEI_I2C_MASTER *This,
  IN UINTN SlaveAddress,
  IN EFI_I2C_REQUEST_PACKET *RequestPacket
  );

///
/// I2C master mode PEIM-to-PEIM Interface
///
struct _EFI_PEI_I2C_MASTER {
  ///
  /// Set the clock frequency in Hertz for the I2C bus.
  ///
  EFI_PEI_I2C_MASTER_PPI_SET_BUS_FREQUENCY SetBusFrequency;

  ///
  /// Reset the I2C host controller.
  ///
  EFI_PEI_I2C_MASTER_PPI_RESET Reset;

  ///
  /// Start an I2C transaction in master mode on the host controller.
  ///
  EFI_PEI_I2C_MASTER_PPI_START_REQUEST StartRequest;

  ///
  /// Pointer to an EFI_I2C_CONTROLLER_CAPABILITIES data structure
  /// containing the capabilities of the I2C host controller.
  ///
  CONST EFI_I2C_CONTROLLER_CAPABILITIES * I2cControllerCapabilities;

  ///
  /// Identifier which uniquely identifies thisI2C controller in the system.
  ///
  EFI_GUID Identifier;
};

///
/// GUID for the EFI_PEI_I2C_MASTER
///
#define EFI_PEI_I2C_MASTER_PPI_GUID   { 0xb3bfab9b, 0x9f9c, 0x4e8b, { 0xad, 0x37, 0x7f, 0x8c, 0x51, 0xfc, 0x62, 0x80 }}

///
/// Reference to variable defined in the .DEC file
///
extern EFI_GUID gEfiPeiI2cMasterGuid;

#endif  //  __I2C_MASTER_H__

/** @file
  I2C Master interface

  A driver or application uses the I2C master protocol to
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


  \section I2cDriverStack       I2C Driver Stack
  
  The following is a representation of the I<sup>2</sup>C (I2C)
  driver stack and an I2C bus layout.

  <code><pre>
              +-----------------+
              |   Application   |
              +-----------------+
                       |
                       | Third Party or UEFI
                       |
                       V
 +--------+   +-----------------+
 | Slave  |   |   Third Party   |
 | Driver |   |   I2C Device    |
 |        |   |     Driver      |
 +--------+   +-----------------+
      |                |
      |           IO   |
      |                |
      |                V
      |       +-----------------+
      |       | I2C Bus Driver  |------------------.
      |       +-----------------+                  |
      |                |                           |
      |         HOST   |          BUS              |
      |                |          CONFIGURATION    |
SLAVE |                V          MANAGEMENT       | ENUMERATE
      |       +-----------------+                  |
      |       | I2C Host Driver |----------.       |
      |       +-----------------+          |       |
      |                |                   V       V
      |        MASTER  |               +---------------------+
      |                | <-------------| I2C Platform Driver |
      |                V               +---------------------+
      |       +-----------------+          ^    |      |
      `------>| I2C Port Driver |          |    |      |
              +-----------------+  Vendor  |    |      |
                       |      |   Specific |    |      |
            Software   |      `------------`    |      |
            --------------------------------------------------
            Hardware   |                        |      |
                       |                        |      |
                       V                        |      |
              +-----------------+               |      |
              | I2C Controller  |               |      |
              +-----------------+               |      |
                       |                        |      |
            -----------------------             |      |
            I2C Bus    |                        |      |
                       |    +------------+      |      |
                       +----| High speed |      |      |
                       |    | I2C device |      |      |
                       |    |    0x01    |      |      |
                       |    +------------+      |      |
                       |                        |      |
                  +---------+  0                |      |
                  | Switch  |<------------------`      |
                  +---------+  1                       |
                       |                               |
                       |    +------------+             |
                       +----| Fast speed |             |
                       |    | I2C device |             |
                       |    |    0x02    |             |
                       |    +------------+             |
                       |                               |
                +-------------+                        |
                | Multiplexer |<-----------------------`
                +-------------+
                 0 |       | 1
                   |       |
                   |       |
                   |       |    +-------------+
                   |       +----| Third Party |
                   |       |    | I2C Device  |
                   |       |    |  0x03, 0x04 |
                   |       |    +-------------+
                   |       |
                   |
                   |            +-------------+
                   +------------| Third Party |
                   |            | I2C Device  |
                   |            |  0x03, 0x04 |
                   |            +-------------+
                   |
  </pre></code>

  The platform hardware designer chooses the bus layout based upon
  the platform, I2C chip and software requirements.  The design uses
  switches to truncate the bus to enable higher speed operation for a
  subset of devices which are placed closer to the controller.  When the
  switch is on, the extended bus must operate at a lower speed.  The
  design uses multiplexer to create separate address spaces enabling
  the use of multiple devices which would otherwise have conflicting
  addresses. See the
  <a href="http://www.nxp.com/documents/user_manual/UM10204.pdf">I<sup>2</sup>C-bus
  specification and user manual</a> for more details.

  N.B. Some operating systems may prohibit the changing of switches
  and multiplexers in the I2C bus.  In this case the platform hardware
  and software designers must select a single I2C bus configuration
  consisting of constant input values for the switches and multiplexers.
  The platform software designer must then ensure that this I2C bus
  configuration is enabled prior to passing control to the operating
  system.

  The platform hardware designer needs to provide the platform software
  designer the following data for each I2C bus:

  1.  Which controller controls this bus

  2.  A list of logic blocks contained in one or more I2C devices:

      a.  I2C device which contains this logic block

      b.  Logic block slave address

      c.  Logic block name

  3.  For each configuration of the switches and multiplexer

      a.  What is the maximum speed of operation

      b.  What devices are accessible

  4.  The settings for the switches and multiplexers when control is
      given to the operating system.

  \section ThirdPartyI2cDrivers   Third Party I2C Drivers

  This layer is I2C chip specific but platform and host controller
  independent.

  Third party I2C driver writers, typically silicon vendors, need
  to provide:

  1.  The device path node data that is used to select their
      driver.

  2.  The order for the blocks of logic that get referenced
      by the entries in the slave address array.

  3.  The hardware version of the I2C device, this value is passed
      to the third party I2C driver to enable it to perform work
      arounds for the specific hardware version.  It is reocmmended
      that value match the value in the ACPI _HRV tag.

  The third party I2C driver uses relative addressing to abstract
  the platform specific details of the I2C device.  Using an
  example I2C device containing an accelerometer and a magnetometer
  which consumes two slave addresses, one for each logic block.  The
  third party I2C driver writer may choose to write two drivers, one
  for each block of logic, in which case each driver refers to the
  single I2C slave address using the relative value of zero (0).
  However if the third party I2C driver writer chooses to write a
  single driver which consumes multiple slave addresses then the
  third party I2C driver writer needs to convey the order of the
  I2C slave address entries in the slave address array to the
  platform software designer.  For the example:

      0: Accelerometer

      1: Magnetometer

  The platform hardware designer picks the actual slave addresses
  from the I2C device's data sheet and provides this information
  to the platform software designer.  The platform software designer
  then places the slave addresses into the slave address array in the
  order specified by the third party I2C driver writer.  The third
  party driver I2C writer then indirectly references this array by
  specifying the index value as the relative slave address.  The
  relative value always starts at zero (0) and its maximum value is
  the number of entries in slave address array minus one.

  The slave address is specified as a 32-bit integer to allow room
  for future slave address expansion.  Only the port driver knows
  the maximum slave address value.  All other drivers and
  applications must look for the EFI_NOT_FOUND status for the
  indication that the maximum slave address was exceeded.

  \section I2cBusDriver         I2C Bus Driver

  This layer is platform, host controller, and I2C chip independent.

  The I2C bus driver creates a handle for each of the I2C devices
  described within the platform driver.  The I2C controller's device
  path is extended with the device path node provided by the platform
  driver and attached to the handle.  The third party I2C device driver
  uses the device path to determine if it may connect.

  The I2C bus driver validates the relative address for the I2C device
  and then converts the relative value to an actual I2C slave address.
  The request is then passed to the I2C host driver.

  \section I2cHostDriver        I2C Host Driver

  This layer is platform, host controller, and I2C chip independent.

  N.B. For proper operation of the I2C bus, only the I2C bus driver
  and the I2C test application connect to the I2C host driver
  via the EFI_I2C_HOST_DRIVER_PROTOCOL.

  The I2C host driver may access any device on the I2C bus.  The I2C
  host driver has the following responsibilities:

  1.  Limits the number of requests to the I2C port driver to one.
      The I2C host driver holds on to additional requests until the
      I2C port driver is available to process the request.  The I2C
      requests are issued in FIFO order to the I2C port driver.

  2.  Enable the proper I2C bus configuration before starting the
      I2C request on the I2C port driver

  I2C devices are addressed as the tuple: BusConfiguration:SlaveAddress.
  I2C bus configuration zero (0) is the portion of the I2C bus that
  connects to the host controller.  The bus configuration specifies
  the control values for the switches and multiplexers in the I2C bus.
  After the switches and multiplexers are properly configured, the I2C
  controller uses the slave address to access the requested I2C device.

  Since the I2C driver stack supports asynchronous transactions this
  layer maintains a queue of I2C requests until the I2C controller
  is available them.  When a request reaches the head of the queue
  the necessary bus configuration is enabled and then the request
  is sent to the I2C port driver.

  \section I2cPortDriver        I2C Port Driver

  This layer is I2C controller specific but platform independent.

  This layer manipulates the I2C controller to perform a transaction
  on the I2C bus.  This layer does not configure the I2C bus so it
  is up to the caller to ensure that the I2C bus is in the proper
  configuration before issuing the I2C request.

  This layer typically needs the following information:

  1.  Host controller address
  2.  Controller's input clock frequency

  Depending upon the I2C controller, more data may be necessary.
  This layer may use any method to get these values: hard coded
  values, PCD values, or may choose to communicate with the platform
  layer using an undefined mechanism to get these values.

  If the I2C port driver requires data from the platform driver then
  the I2C port driver writer needs to provide the platform interface
  details to the platform software designer.

  \section I2cPlatformDriver    I2C Platform Driver

  When enabling access to I2C devices within UEFI, this driver
  installs the EFI_I2C_ENUMERATE_PROTOCOL to provide the I2C device
  descriptions to the I2C bus driver using the EFI_I2C_DEVICE
  structure.  These descriptions include the bus configuration
  number required for the I2C device, the slave address array
  and the device path.

  The EFI_I2C_BUS_CONFIGURATION_MANAGEMENT protocol is optional.
  This protocol needs to be specified under the following conditions:

  1.  The I2C bus must operate at a frequency greater than 100 KHz
  2.  The I2C bus contains switches or multiplexers.

  The EFI_I2C_BUS_CONFIGURATION_MANAGEMENT protocol enables the
  I2C host driver to call into the I2C platform driver to enable
  a specific I2C bus configuration and set its maximum clock speed.

  The platform software designer collects the data requirements
  from third party I2C driver writers, the I2C controller
  driver writer, the EFI_I2C_ENUMERATE_PROTOCOL and
  EFI_I2C_BUS_CONFIGURATION_MANAGEMENT_PROTOCOL.  The platform
  software designer gets the necessary data from the platform
  hardware designer.  The platform software designer then builds
  the data structures and implements the necessary routines to
  construct the I2C platform driver.

  \section I2cSwitches          Switches and Multiplexers

  There are some I2C switches and I2C multiplexers where the control
  is done via I2C commands.  When the control inputs come via the
  same I2C bus that is being configured then the platform driver must
  use the EFI_I2C_MASTER_PROTOCOL that is passed to the platform
  driver.  While the I2C host driver makes the call to the I2C
  platform driver to configure the bus, the host driver keeps the
  I2C port driver idle, to allow the I2C platform driver preform
  the necessary configuration transactions.

  If however the configuration control is done via and I2C device
  connected to a different I2C bus (host controller), then it is
  possible for the platform software designer may choose between
  the following:

  1.  Call into a third party I2C driver to manipulate the I2C
      bus control device.
  2.  Call into the EFI_I2C_IO_PROTOCOL if no third party I2C
      driver exists for the I2C bus control device
  3.  Call into the EFI_I2C_HOST_PROTOCOL if the platform does
      not expose the I2C bus control device.

**/

#ifndef __I2C_MASTER_H__
#define __I2C_MASTER_H__

#include <Protocol/I2cSharedDxePei.h>

/**
  Declare the forward reference
**/
typedef struct _EFI_I2C_MASTER_PROTOCOL EFI_I2C_MASTER_PROTOCOL;  ///< I2C master protocol

///
/// Protocol GUID
///
#define EFI_I2C_MASTER_PROTOCOL_GUID { 0xcd72881f, 0x45b5, 0x4feb, { 0x98, 0xc8, 0x31, 0x3d, 0xa8, 0x11, 0x74, 0x62 }}

/**
  Set the frequency for the I2C clock line.

  This routine must be called at or below TPL_NOTIFY.

  The software and controller do a best case effort of using the specified
  frequency for the I2C bus.  If the frequency does not match exactly then
  the I2C master protocol selects the next lower frequency to avoid
  exceeding the operating conditions for any of the I2C devices on the bus.
  For example if 400 KHz was specified and the controller's divide network
  only supports 402 KHz or 398 KHz then the I2C master protocol selects 398
  KHz.  If there are not lower frequencies available, then return
  EFI_UNSUPPORTED.

  @param[in] This           Pointer to an EFI_I2C_MASTER_PROTOCOL structure
  @param[in] BusClockHertz  Pointer to the requested I2C bus clock frequency
                            in Hertz.  Upon return this value contains the
                            actual frequency in use by the I2C controller.

  @retval EFI_SUCCESS           The bus frequency was set successfully.
  @retval EFI_ALREADY_STARTED   The controller is busy with another transaction.
  @retval EFI_INVALID_PARAMETER BusClockHertz is NULL
  @retval EFI_UNSUPPORTED       The controller does not support this frequency.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_I2C_MASTER_PROTOCOL_SET_BUS_FREQUENCY) (
  IN CONST EFI_I2C_MASTER_PROTOCOL *This,
  IN UINTN *BusClockHertz
  );

/**
  Reset the I2C controller and configure it for use

  This routine must be called at or below TPL_NOTIFY.

  The I2C controller is reset.  The caller must call SetBusFrequench() after
  calling Reset().

  @param[in]     This       Pointer to an EFI_I2C_MASTER_PROTOCOL structure.

  @retval EFI_SUCCESS         The reset completed successfully.
  @retval EFI_ALREADY_STARTED The controller is busy with another transaction.
  @retval EFI_DEVICE_ERROR    The reset operation failed.
  
**/
typedef
EFI_STATUS
(EFIAPI *EFI_I2C_MASTER_PROTOCOL_RESET) (
  IN CONST EFI_I2C_MASTER_PROTOCOL *This
  );

/**
  Start an I2C transaction on the host controller.

  This routine must be called at or below TPL_NOTIFY.  For synchronous
  requests this routine must be called at or below TPL_CALLBACK.

  This function initiates an I2C transaction on the controller.  To
  enable proper error handling by the I2C protocol stack, the I2C
  master protocol does not support queuing but instead only manages
  one I2C transaction at a time.  This API requires that the I2C bus
  is in the correct configuration for the I2C transaction.

  The transaction is performed by sending a start-bit and selecting the
  I2C device with the specified I2C slave address and then performing
  the specified I2C operations.  When multiple operations are requested
  they are separated with a repeated start bit and the slave address.
  The transaction is terminated with a stop bit.

  When Event is NULL, StartRequest operates synchronously and returns
  the I2C completion status as its return value.

  When Event is not NULL, StartRequest synchronously returns EFI_SUCCESS
  indicating that the I2C transaction was started asynchronously.  The
  transaction status value is returned in the buffer pointed to by
  I2cStatus upon the completion of the I2C transaction when I2cStatus
  is not NULL.  After the transaction status is returned the Event is
  signaled.

  Note: The typical consumer of this API is the I2C host protocol.
  Extreme care must be taken by other consumers of this API to prevent
  confusing the third party I2C drivers due to a state change at the
  I2C device which the third party I2C drivers did not initiate.  I2C
  platform specific code may use this API within these guidelines.

  @param[in] This           Pointer to an EFI_I2C_MASTER_PROTOCOL structure.
  @param[in] SlaveAddress   Address of the device on the I2C bus.  Set the
                            I2C_ADDRESSING_10_BIT when using 10-bit addresses,
                            clear this bit for 7-bit addressing.  Bits 0-6
                            are used for 7-bit I2C slave addresses and bits
                            0-9 are used for 10-bit I2C slave addresses.
  @param[in] RequestPacket  Pointer to an EFI_I2C_REQUEST_PACKET
                            structure describing the I2C transaction.
  @param[in] Event          Event to signal for asynchronous transactions,
                            NULL for asynchronous transactions
  @param[out] I2cStatus     Optional buffer to receive the I2C transaction
                            completion status

  @retval EFI_SUCCESS           The asynchronous transaction was successfully
                                started when Event is not NULL.
  @retval EFI_SUCCESS           The transaction completed successfully when
                                Event is NULL.
  @retval EFI_ALREADY_STARTED   The controller is busy with another transaction.
  @retval EFI_BAD_BUFFER_SIZE   The RequestPacket->LengthInBytes value is too
                                large.
  @retval EFI_DEVICE_ERROR      There was an I2C error (NACK) during the
                                transaction.
  @retval EFI_INVALID_PARAMETER RequestPacket is NULL
  @retval EFI_NOT_FOUND         Reserved bit set in the SlaveAddress parameter
  @retval EFI_NO_RESPONSE       The I2C device is not responding to the slave
                                address.  EFI_DEVICE_ERROR will be returned if
                                the controller cannot distinguish when the NACK
                                occurred.
  @retval EFI_OUT_OF_RESOURCES  Insufficient memory for I2C transaction
  @retval EFI_UNSUPPORTED       The controller does not support the requested
                                transaction.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_I2C_MASTER_PROTOCOL_START_REQUEST) (
  IN CONST EFI_I2C_MASTER_PROTOCOL *This,
  IN UINTN SlaveAddress,
  IN EFI_I2C_REQUEST_PACKET *RequestPacket,
  IN EFI_EVENT Event OPTIONAL,
  OUT EFI_STATUS *I2cStatus OPTIONAL
  );

///
/// I2C master mode protocol
///
/// This protocol manipulates the I2C host controller to perform transactions as a
/// master on the I2C bus using the current state of any switches or multiplexers
/// in the I2C bus.
///
struct _EFI_I2C_MASTER_PROTOCOL {
  ///
  /// Set the clock frequency for the I2C bus.
  ///
  EFI_I2C_MASTER_PROTOCOL_SET_BUS_FREQUENCY SetBusFrequency;

  ///
  /// Reset the I2C host controller.
  ///
  EFI_I2C_MASTER_PROTOCOL_RESET Reset;

  ///
  /// Start an I2C transaction in master mode on the host controller.
  ///
  EFI_I2C_MASTER_PROTOCOL_START_REQUEST StartRequest;

  ///
  /// Pointer to an EFI_I2C_CONTROLLER_CAPABILITIES data structure containing
  /// the capabilities of the I2C host controller.
  ///
  CONST EFI_I2C_CONTROLLER_CAPABILITIES * I2cControllerCapabilities;
};

///
/// GUID for the EFI_I2C_MASTER_PROTOCOL
///
#define EFI_I2C_MASTER_PROTOCOL_GUID  { 0xcd72881f, 0x45b5, 0x4feb, { 0x98, 0xc8, 0x31, 0x3d, 0xa8, 0x11, 0x74, 0x62 }}

///
/// Reference to variable defined in the .DEC file
///
extern EFI_GUID gEfiI2cMasterProtocolGuid;

#endif  //  __I2C_MASTER_H__

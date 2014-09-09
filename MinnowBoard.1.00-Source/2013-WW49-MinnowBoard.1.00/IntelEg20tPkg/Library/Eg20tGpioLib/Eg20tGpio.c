/** @file
  EG20T GPIO support

  Copyright (c) 2013, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Uefi.h>
#include <IndustryStandard/Pci22.h>

#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/PciLib.h>
#include <Library/UefiLib.h>

#include <Register/Eg20t.h>

#define PCI_ADDRESS(Base, Offset) ((Base & 0xFFFFF000)|((Offset) & 0xFFF))


EFI_STATUS
EFIAPI
Eg20tGpioBaseAddress (
  UINTN * RegisterAddress
  )
{
  UINT16 Command;
  UINT32 Data32;
  UINTN PciConfigurationSpace;
  UINTN PciConfigurationSpaceAddress;
  EFI_STATUS Status;

  //
  //  Read the vendor and device IDs
  //
  // BUGBUG: Assumes Bus #2.  Need to update to get bus number from parent PCI to PCI Bridges
  //
  PciConfigurationSpace = PCI_LIB_ADDRESS ( 2, EG20T_GPIO_PCI_DEVICE_NUMBER, EG20T_GPIO_PCI_FUNCTION_NUMBER, 0 );
  PciConfigurationSpaceAddress = PCI_ADDRESS ( PciConfigurationSpace, PCI_VENDOR_ID_OFFSET );
  Data32 = PciRead32 ( PciConfigurationSpaceAddress );
//Print ( L"0x%08x --> 0x%08x\r\n", PciConfigurationSpaceAddress, Data32 );
  if (( EG20T_PCI_VENDOR_ID != ( Data32 & 0xffff ))
    || ( EG20T_GPIO_PCI_DEVICE_ID != ( Data32 >> 16 ))) {
    //
    //  Wrong device
    //
    Status = EFI_UNSUPPORTED;
  }
  else {
    //
    //  Found the GPIO device
    //  Verify that the address is set
    //
    PciConfigurationSpaceAddress = PCI_ADDRESS ( PciConfigurationSpace, PCI_BASE_ADDRESSREG_OFFSET + 4 );
    Data32 = PciRead32 ( PciConfigurationSpaceAddress );
//Print ( L"0x%08x --> 0x%08x\r\n", PciConfigurationSpaceAddress, Data32 );
    if ( 0 == Data32 ) {
      //
      //  The device is not configured
      //
      Status = EFI_NO_MAPPING;
    }
    else {
      //
      //  The device is configured
      //  Determine if the device is enabled
      //
      PciConfigurationSpaceAddress = PCI_ADDRESS ( PciConfigurationSpace, PCI_COMMAND_OFFSET );
      Command = PciRead16 ( PciConfigurationSpaceAddress );
//Print ( L"0x%08x --> 0x%04x\r\n", PciConfigurationSpaceAddress, Command );
      if ( 0 == ( Command & 2 )) {
        //
        //  The device is not enabled
        //  Enable the device
        //
        Command |= EFI_PCI_COMMAND_MEMORY_SPACE
                | EFI_PCI_COMMAND_BUS_MASTER;
//Print ( L"0x%08x <-- 0x%04x\r\n", PciConfigurationSpaceAddress, Command );
        PciWrite32 ( PciConfigurationSpaceAddress, Command );
      }

      //
      //  Return the base address
      //
      *RegisterAddress = Data32;
      Status = EFI_SUCCESS;
    }
  }

  //
  //  Return the operation status
  //
  return Status;
}

STATIC
EFI_STATUS
Eg20tGpioRegisterModify (
  UINT64 RegisterOffset,
  UINT32 AndMask,
  UINT32 XorMask
  )
{
  UINT32 HexValue;
  UINTN RegisterAddress;
  EFI_STATUS Status;

  //
  //  Validate the controller number and port number
  //
  if (( 0 != ( RegisterOffset & 3 ))
    || ( EG20T_REG_GPIO_SRST < RegisterOffset )
    || (( EG20T_REG_GPIO_IM1 < RegisterOffset ) && ( EG20T_REG_GPIO_SRST > RegisterOffset ))) {
    Status = EFI_INVALID_PARAMETER;
  }
  else {
    //
    //  Compute the register address
    //
    Status = Eg20tGpioBaseAddress ( &RegisterAddress );
    if ( !EFI_ERROR ( Status )) {
      RegisterAddress += (UINTN)RegisterOffset;

      //
      //  Clear the register bit
      //
      HexValue = MmioRead32 ( RegisterAddress );
//Print ( L"0x%08x --> 0x%08x\r\n", RegisterAddress, HexValue );
      HexValue &= AndMask;
      HexValue ^= XorMask;
//Print ( L"0x%08x <-- 0x%08x\r\n", RegisterAddress, HexValue );
      MmioWrite32 ( RegisterAddress, HexValue );
    }
  }

  //
  //  Return the operation status
  //
  return Status;
}


STATIC
EFI_STATUS
Eg20tGpioRegisterRead (
  UINT64 RegisterOffset,
  UINTN RightShiftCount,
  UINTN AndMask,
  UINTN *BitValue
  )
{
  UINTN HexValue;
  UINTN RegisterAddress;
  EFI_STATUS Status;

  //
  //  Validate the controller number and port number
  //
  if (( NULL == BitValue )
    || ( 0 != ( RegisterOffset & 3 ))
    || ( EG20T_REG_GPIO_SRST < RegisterOffset )
    || (( EG20T_REG_GPIO_IM1 < RegisterOffset ) && ( EG20T_REG_GPIO_SRST > RegisterOffset ))) {
    Status = EFI_INVALID_PARAMETER;
  }
  else {
    //
    //  Compute the register address
    //
    Status = Eg20tGpioBaseAddress ( &RegisterAddress );
    if ( !EFI_ERROR ( Status )) {
      RegisterAddress += (UINTN)RegisterOffset;

      //
      //  Read the register
      //
      HexValue = MmioRead32 ( RegisterAddress );
//Print ( L"0x%08x --> 0x%08x\r\n", RegisterAddress, HexValue );

      //
      //  Return the requested bit
      //
      *BitValue = ( HexValue >> RightShiftCount ) & AndMask;
    }
  }

  //
  //  Return the operation status
  //
  return Status;
}


STATIC
EFI_STATUS
Eg20tGpioRegisterWriteBit (
  UINT64 PortNumber,
  UINT64 RegisterOffset
  )
{
  UINTN HexValue;
  UINTN RegisterAddress;
  EFI_STATUS Status;

  //
  //  Validate the controller number and port number
  //
  HexValue = 1 << PortNumber;
  if ( 0 == ( HexValue & EG20T_GPIO_VALID_PORTS )) {
    Status = EFI_INVALID_PARAMETER;
  }
  else {
    //
    //  Validate the controller number and port number
    //
    if (( 0 != ( RegisterOffset & 3 ))
      || ( EG20T_REG_GPIO_SRST < RegisterOffset )
      || (( EG20T_REG_GPIO_IM1 < RegisterOffset ) && ( EG20T_REG_GPIO_SRST > RegisterOffset ))) {
      Status = EFI_INVALID_PARAMETER;
    }
    else {
      //
      //  Compute the register address
      //
      Status = Eg20tGpioBaseAddress ( &RegisterAddress );
      if ( !EFI_ERROR ( Status )) {
        RegisterAddress += (UINTN)RegisterOffset;

        //
        //  Write the register
        //
//Print ( L"0x%08x <-- 0x%08x\r\n", RegisterAddress, HexValue );
        MmioWrite32 ( RegisterAddress, HexValue );
      }
    }
  }

  //
  //  Return the operation status
  //
  return Status;
}


STATIC
EFI_STATUS
Eg20tGpioBitClear (
  UINT64 PortNumber,
  UINT64 RegisterOffset
  )
{
  UINT32 HexValue;
  EFI_STATUS Status;

  //
  //  Validate the controller number and port number
  //
  HexValue = 1 << PortNumber;
  if ( 0 == ( HexValue & EG20T_GPIO_VALID_PORTS )) {
    Status = EFI_INVALID_PARAMETER;
  }
  else {
    //
    //  Clear the bit
    //
    Status = Eg20tGpioRegisterModify ( RegisterOffset, ~HexValue, 0 );
  }

  //
  //  Return the operation status
  //
  return Status;
}


STATIC
EFI_STATUS
Eg20tGpioBitRead (
  UINT64 PortNumber,
  UINT64 RegisterOffset,
  UINTN *BitValue
  )
{
  UINTN HexValue;
  EFI_STATUS Status;

  //
  //  Validate the port number
  //
  HexValue = 1 << PortNumber;
  if ( 0 == ( HexValue & EG20T_GPIO_VALID_PORTS )) {
    Status = EFI_INVALID_PARAMETER;
  }
  else {
    //
    //  Read the register
    //
    Status = Eg20tGpioRegisterRead ( RegisterOffset,
                                     (UINTN)PortNumber,
                                     1,
                                     BitValue );
  }

  //
  //  Return the operation status
  //
  return Status;
}


STATIC
EFI_STATUS
Eg20tGpioBitSet (
  UINT64 PortNumber,
  UINT64 RegisterOffset
  )
{
  UINT32 HexValue;
  EFI_STATUS Status;

  //
  //  Validate the controller number and port number
  //
  HexValue = 1 << PortNumber;
  if ( 0 == ( HexValue & EG20T_GPIO_VALID_PORTS )) {
    Status = EFI_INVALID_PARAMETER;
  }
  else {
    //
    //  Set the bit
    //
    Status = Eg20tGpioRegisterModify ( RegisterOffset,
                                       ~HexValue,
                                       HexValue );
  }

  //
  //  Return the operation status
  //
  return Status;
}


EFI_STATUS
EFIAPI
Eg20tGpioDirectionInput (
  UINT64 PortNumber
  )
{
  //
  //  Set the GPIO port as an input
  //
  return Eg20tGpioBitClear ( PortNumber, EG20T_REG_GPIO_PM );
}


EFI_STATUS
EFIAPI
Eg20tGpioDirectionOutput (
  UINT64 PortNumber
  )
{
  //
  //  Set the GPIO port as an output
  //
  return Eg20tGpioBitSet ( PortNumber, EG20T_REG_GPIO_PM );
}


EFI_STATUS
EFIAPI
Eg20tGpioInterruptClear (
  UINT64 PortNumber
  )
{
  //
  //  Clear the interrupt
  //
  return Eg20tGpioRegisterWriteBit ( PortNumber, EG20T_REG_GPIO_ICLR );
}


EFI_STATUS
EFIAPI
Eg20tGpioInterruptDisable (
  UINT64 PortNumber
  )
{
  //
  //  Disable the interrupt
  //
  return Eg20tGpioBitClear ( PortNumber, EG20T_REG_GPIO_IEN );
}


EFI_STATUS
EFIAPI
Eg20tGpioInterruptEnable (
  UINT64 PortNumber
  )
{
  //
  //  Enable the interrupt
  //
  return Eg20tGpioBitSet ( PortNumber, EG20T_REG_GPIO_IEN );
}


EFI_STATUS
EFIAPI
Eg20tGpioInterruptMaskClear (
  UINT64 PortNumber
  )
{
  //
  //  Clear the interrupt mask
  //
  return Eg20tGpioRegisterWriteBit ( PortNumber, EG20T_REG_GPIO_IMASKCLR );
}


EFI_STATUS
EFIAPI
Eg20tGpioInterruptMaskSet (
  UINT64 PortNumber
  )
{
  //
  //  Set the interrupt mask
  //
  return Eg20tGpioRegisterWriteBit ( PortNumber, EG20T_REG_GPIO_IMASK );
}


EFI_STATUS
EFIAPI
Eg20tGpioInterruptMode (
  UINT64 PortNumber,
  UINT64 Value
  )
{
  UINT32 HexValue;
  UINT32 Mask;
  UINT64 RegisterOffset;
  EFI_STATUS Status;

  //
  //  Validate the controller number and port number
  //
  HexValue = 1 << PortNumber;
  if (( 0 == ( HexValue & EG20T_GPIO_VALID_PORTS ))
    || ( 4 < Value )) {
    Status = EFI_INVALID_PARAMETER;
  }
  else {
    //
    //  Determine the register offset
    //
    RegisterOffset = EG20T_REG_GPIO_IM0 + (( PortNumber >> 3 ) << 2 );

    //
    //  Determine the port data position
    //
    HexValue = (UINT32)( PortNumber & 7 ) << 2;
    Mask = (UINT32)~( 7 << HexValue );
    HexValue = (UINT32)Value << HexValue;

    //
    //  Set the value
    //
    Status = Eg20tGpioRegisterModify ( RegisterOffset, Mask, HexValue );
  }

  //
  //  Return the operation status
  //
  return Status;
}


EFI_STATUS
EFIAPI
Eg20tGpioOutputClear (
  UINT64 PortNumber
  )
{
  //
  //  Clear the GPIO output
  //
  return Eg20tGpioBitClear ( PortNumber, EG20T_REG_GPIO_PO );
}


EFI_STATUS
EFIAPI
Eg20tGpioOutputSet (
  UINT64 PortNumber
  )
{
  //
  //  Set the GPIO output
  //
  return Eg20tGpioBitSet ( PortNumber, EG20T_REG_GPIO_PO );
}


EFI_STATUS
EFIAPI
Eg20tGpioPinDirection (
  UINT64 PortNumber,
  UINTN *BitValue
  )
{
  //
  //  Get the pin's direction
  //
  return Eg20tGpioBitRead ( PortNumber,
                            EG20T_REG_GPIO_PM,
                            BitValue );
}


EFI_STATUS
EFIAPI
Eg20tGpioPinInterruptEnable (
  UINT64 PortNumber,
  UINTN *BitValue
  )
{
  //
  //  Get the pin's interrupt enable/disable status
  //
  return Eg20tGpioBitRead ( PortNumber,
                            EG20T_REG_GPIO_IEN,
                            BitValue );
}


EFI_STATUS
EFIAPI
Eg20tGpioPinInterruptMask (
  UINT64 PortNumber,
  UINTN *BitValue
  )
{
  //
  //  Get the pin's interrupt mask
  //
  return Eg20tGpioBitRead ( PortNumber,
                            EG20T_REG_GPIO_IMASK,
                            BitValue );
}


EFI_STATUS
EFIAPI
Eg20tGpioPinInterruptMode (
  UINT64 PortNumber,
  UINTN *Value
  )
{
  UINT32 HexValue;
  UINT64 RegisterOffset;
  EFI_STATUS Status;

  //
  //  Validate the controller number and port number
  //
  HexValue = 1 << PortNumber;
  if ( 0 == ( HexValue & EG20T_GPIO_VALID_PORTS )) {
    Status = EFI_INVALID_PARAMETER;
  }
  else {
    //
    //  Determine the register offset
    //
    RegisterOffset = EG20T_REG_GPIO_IM0 + (( PortNumber >> 3 ) << 2 );

    //
    //  Determine the port data position
    //
    HexValue = (UINT32)( PortNumber & 7 ) << 2;

    //
    //  Get the pin's interrupt mode
    //
    Status = Eg20tGpioRegisterRead ( RegisterOffset,
                                     HexValue,
                                     7,
                                     Value );
  }

  //
  //  Return the operation status
  //
  return Status;
}


EFI_STATUS
EFIAPI
Eg20tGpioPinInterruptPending (
  UINT64 PortNumber,
  UINTN *BitValue
  )
{
  //
  //  Get the pin's interrupt source
  //
  return Eg20tGpioBitRead ( PortNumber,
                            EG20T_REG_GPIO_IDISP,
                            BitValue );
}


EFI_STATUS
EFIAPI
Eg20tGpioPinInterruptRequest (
  UINT64 PortNumber,
  UINTN *BitValue
  )
{
  //
  //  Get the pin's interrupt status
  //
  return Eg20tGpioBitRead ( PortNumber,
                            EG20T_REG_GPIO_ISTATUS,
                            BitValue );
}


EFI_STATUS
EFIAPI
Eg20tGpioPinPortInput (
  UINT64 PortNumber,
  UINTN *BitValue
  )
{
  //
  //  Get the pin's input value
  //
  return Eg20tGpioBitRead ( PortNumber,
                            EG20T_REG_GPIO_PI,
                            BitValue );
}


EFI_STATUS
EFIAPI
Eg20tGpioPinPortMode (
  UINT64 PortNumber,
  UINTN *BitValue
  )
{
  //
  //  Get the pin's direction
  //
  return Eg20tGpioBitRead ( PortNumber,
                            EG20T_REG_GPIO_PM,
                            BitValue );
}


EFI_STATUS
EFIAPI
Eg20tGpioPinPortOutput (
  UINT64 PortNumber,
  UINTN *BitValue
  )
{
  //
  //  Get the pin's output value
  //
  return Eg20tGpioBitRead ( PortNumber,
                            EG20T_REG_GPIO_PO,
                            BitValue );
}


EFI_STATUS
EFIAPI
Eg20tGpioSoftReset (
  UINT64 Value
  )
{
  EFI_STATUS Status;

  switch ( Value ) {
  default:
    Status = EFI_INVALID_PARAMETER;
    break;

  case 1:
    //
    //  Reset the controller
    //
    Status = Eg20tGpioBitSet ( 0, EG20T_REG_GPIO_SRST );
    break;

  case 0:
    //
    //  Clear the reset
    //
    Status = Eg20tGpioBitSet ( 0, EG20T_REG_GPIO_SRST );
    break;
  }

  //
  //  Return the operation status
  //
  return Status;
}


EFI_STATUS
EFIAPI
Eg20tGpioSoftResetStatus (
  UINTN *BitValue
  )
{
  //
  //  Get the software reset status
  //
  return Eg20tGpioBitRead ( 0,
                            EG20T_REG_GPIO_SRST,
                            BitValue );
}

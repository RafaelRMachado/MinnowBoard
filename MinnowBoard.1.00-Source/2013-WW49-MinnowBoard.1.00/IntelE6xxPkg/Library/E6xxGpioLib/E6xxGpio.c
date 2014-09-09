/** @file
E6xx GPIO support

Copyright (c) 2012-2013, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials are licensed and made available under 
the terms and conditions of the BSD License that accompanies this distribution.  
The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php.                                          
    
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

**/

#include <Uefi.h>
#include <Library/E6xx.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>

#define DIM(x)          ( sizeof ( x ) / sizeof ( x [ 0 ]))

CONST UINT64 E6xxValidGpioPorts [ ] = { 0x1f, 0x1ff };


EFI_STATUS
EFIAPI
E6xxGpioBaseAddress (
  UINT64 ControllerNumber,
  UINTN *GpioBaseAddress
  )
{
  UINTN RegisterAddress;

  //
  //  Get the GPIO base address
  //
  RegisterAddress = (UINTN)PcdGet16 ( PcdE6xxGpioIoPortBaseAddress );
  RegisterAddress += (UINTN)( ControllerNumber << 5 );
  *GpioBaseAddress = RegisterAddress;
  return EFI_SUCCESS;
}


STATIC
EFI_STATUS
E6xxGpioBitClear (
  UINT64 ControllerNumber,
  UINT64 PortNumber,
  UINT64 RegisterOffset
  )
{
  UINT32 HexValue;
  UINTN RegisterAddress;
  EFI_STATUS Status;

  //
  //  Validate the controller number and port number
  //
  HexValue = 1 << PortNumber;
  if (( DIM ( E6xxValidGpioPorts ) < ControllerNumber )
    || ( 0 != ( RegisterOffset & 3 ))
    || ( 0x1c < RegisterOffset )
    || ( 0 == ( HexValue & E6xxValidGpioPorts [ ControllerNumber ]))) {
    Status = EFI_INVALID_PARAMETER;
  }
  else {
    //
    //  Compute the register address
    //
    RegisterAddress = (UINTN)PcdGet16 ( PcdE6xxGpioIoPortBaseAddress );
    RegisterAddress += (UINTN)( ControllerNumber << 5 );
    RegisterAddress += (UINTN)RegisterOffset;

    //
    //  Clear the register bit
    //
    HexValue = IoRead32 ( RegisterAddress );
//Print ( L"0x%08x --> 0x%08x\r\n", RegisterAddress, HexValue );
    HexValue &= ~( 1 << PortNumber );
//Print ( L"0x%08x <-- 0x%08x\r\n", RegisterAddress, HexValue );
    IoWrite32 ( RegisterAddress, HexValue );
    Status = EFI_SUCCESS;
  }

  //
  //  Return the operation status
  //
  return Status;
}


STATIC
EFI_STATUS
E6xxGpioBitRead (
  UINT64 ControllerNumber,
  UINT64 PortNumber,
  UINT64 RegisterOffset,
  UINTN *BitValue
  )
{
  UINTN HexValue;
  UINTN RegisterAddress;
  EFI_STATUS Status;

  //
  //  Validate the controller number and port number
  //
  HexValue = 1 << PortNumber;
  if (( NULL == BitValue )
    || ( DIM ( E6xxValidGpioPorts ) < ControllerNumber )
    || ( 0 != ( RegisterOffset & 3 ))
    || ( 0x1c < RegisterOffset )
    || ( 0 == ( HexValue & E6xxValidGpioPorts [ ControllerNumber ]))) {
    Status = EFI_INVALID_PARAMETER;
  }
  else {
    //
    //  Compute the register address
    //
    RegisterAddress = (UINTN)PcdGet16 ( PcdE6xxGpioIoPortBaseAddress );
    RegisterAddress += (UINTN)( ControllerNumber << 5 );
    RegisterAddress += (UINTN)RegisterOffset;

    //
    //  Read the register
    //
    HexValue = IoRead32 ( RegisterAddress );
//Print ( L"0x%08x --> 0x%08x\r\n", RegisterAddress, HexValue );

    //
    //  Return the requested bit
    //
    *BitValue = ( HexValue >> PortNumber ) & 1;
    Status = EFI_SUCCESS;
  }

  //
  //  Return the operation status
  //
  return Status;
}


STATIC
EFI_STATUS
E6xxGpioBitSet (
  UINT64 ControllerNumber,
  UINT64 PortNumber,
  UINT64 RegisterOffset
  )
{
  UINT32 HexValue;
  UINTN RegisterAddress;
  EFI_STATUS Status;

  //
  //  Validate the controller number and port number
  //
  HexValue = 1 << PortNumber;
  if (( DIM ( E6xxValidGpioPorts ) < ControllerNumber )
    || ( 0 != ( RegisterOffset & 3 ))
    || ( 0x1c < RegisterOffset )
    || ( 0 == ( HexValue & E6xxValidGpioPorts [ ControllerNumber ]))) {
    Status = EFI_INVALID_PARAMETER;
  }
  else {
    //
    //  Compute the register address
    //
    RegisterAddress = (UINTN)PcdGet16 ( PcdE6xxGpioIoPortBaseAddress );
    RegisterAddress += (UINTN)( ControllerNumber << 5 );
    RegisterAddress += (UINTN)RegisterOffset;

    //
    //  Set the register bit
    //
    HexValue = IoRead32 ( RegisterAddress );
//Print ( L"0x%08x --> 0x%08x\r\n", RegisterAddress, HexValue );
    HexValue |= 1 << PortNumber;
//Print ( L"0x%08x <-- 0x%08x\r\n", RegisterAddress, HexValue );
    IoWrite32 ( RegisterAddress, HexValue );
    Status = EFI_SUCCESS;
  }

  //
  //  Return the operation status
  //
  return Status;
}


EFI_STATUS
EFIAPI
E6xxGpioDirectionInput (
  UINT64 ControllerNumber,
  UINT64 PortNumber
  )
{
  //
  //  Set the GPIO port as an input
  //
  return E6xxGpioBitSet ( ControllerNumber, PortNumber, GPIO_CGIO );
}


EFI_STATUS
EFIAPI
E6xxGpioDirectionOutput (
  UINT64 ControllerNumber,
  UINT64 PortNumber
  )
{
  //
  //  Set the GPIO port as an output
  //
  return E6xxGpioBitClear ( ControllerNumber, PortNumber, GPIO_CGIO );
}


EFI_STATUS
EFIAPI
E6xxGpioGpeDisable (
  UINT64 ControllerNumber,
  UINT64 PortNumber
  )
{
  //
  //  Disable the general purpose event
  //
  return E6xxGpioBitClear ( ControllerNumber, PortNumber, GPIO_CGGPE );
}


EFI_STATUS
EFIAPI
E6xxGpioGpeEnable (
  UINT64 ControllerNumber,
  UINT64 PortNumber
  )
{
  //
  //  Enable the general purpose event
  //
  return E6xxGpioBitSet ( ControllerNumber, PortNumber, GPIO_CGGPE );
}


EFI_STATUS
EFIAPI
E6xxGpioOutputClear (
  UINT64 ControllerNumber,
  UINT64 PortNumber
  )
{
  //
  //  Clear the GPIO output
  //
  return E6xxGpioBitClear ( ControllerNumber, PortNumber, GPIO_CGLV );
}


EFI_STATUS
EFIAPI
E6xxGpioOutputSet (
  UINT64 ControllerNumber,
  UINT64 PortNumber
  )
{
  //
  //  Set the GPIO output
  //
  return E6xxGpioBitSet ( ControllerNumber, PortNumber, GPIO_CGLV );
}


EFI_STATUS
EFIAPI
E6xxGpioNegativeEdgeDisable (
  UINT64 ControllerNumber,
  UINT64 PortNumber
  )
{
  //
  //  Disable the negative edge trigger
  //
  return E6xxGpioBitClear ( ControllerNumber, PortNumber, GPIO_CGTNE );
}


EFI_STATUS
EFIAPI
E6xxGpioNegativeEdgeEnable (
  UINT64 ControllerNumber,
  UINT64 PortNumber
  )
{
  //
  //  Enable the negative edge trigger
  //
  return E6xxGpioBitSet ( ControllerNumber, PortNumber, GPIO_CGTNE );
}


EFI_STATUS
EFIAPI
E6xxGpioPinDirection (
  UINT64 ControllerNumber,
  UINT64 PortNumber,
  UINTN *BitValue
  )
{
  //
  //  Get the pin's direction
  //
  return E6xxGpioBitRead ( ControllerNumber,
                           PortNumber,
                           GPIO_CGIO,
                           BitValue );
}


EFI_STATUS
EFIAPI
E6xxGpioPinEnable (
  UINT64 ControllerNumber,
  UINT64 PortNumber,
  UINTN *BitValue
  )
{
  //
  //  Get the pin's enable/disable status
  //
  return E6xxGpioBitRead ( ControllerNumber,
                           PortNumber,
                           GPIO_CGEN,
                           BitValue );
}


EFI_STATUS
EFIAPI
E6xxGpioPinGpe (
  UINT64 ControllerNumber,
  UINT64 PortNumber,
  UINTN *BitValue
  )
{
  //
  //  Get the pin's GPE state
  //
  return E6xxGpioBitRead ( ControllerNumber,
                           PortNumber,
                           GPIO_CGGPE,
                           BitValue );
}


EFI_STATUS
EFIAPI
E6xxGpioPinLevel (
  UINT64 ControllerNumber,
  UINT64 PortNumber,
  UINTN *BitValue
  )
{
  //
  //  Get the pin's level
  //
  return E6xxGpioBitRead ( ControllerNumber,
                           PortNumber,
                           GPIO_CGLV,
                           BitValue );
}


EFI_STATUS
EFIAPI
E6xxGpioPinNegativeEdge (
  UINT64 ControllerNumber,
  UINT64 PortNumber,
  UINTN *BitValue
  )
{
  //
  //  Get the pin's negative edge trigger
  //
  return E6xxGpioBitRead ( ControllerNumber,
                           PortNumber,
                           GPIO_CGTNE,
                           BitValue );
}


EFI_STATUS
EFIAPI
E6xxGpioPinPositiveEdge (
  UINT64 ControllerNumber,
  UINT64 PortNumber,
  UINTN *BitValue
  )
{
  //
  //  Get the pin's positive edge trigger
  //
  return E6xxGpioBitRead ( ControllerNumber,
                           PortNumber,
                           GPIO_CGTPE,
                           BitValue );
}


EFI_STATUS
EFIAPI
E6xxGpioPinSmi (
  UINT64 ControllerNumber,
  UINT64 PortNumber,
  UINTN *BitValue
  )
{
  //
  //  Get the pin's ACPI SMI state
  //
  return E6xxGpioBitRead ( ControllerNumber,
                           PortNumber,
                           GPIO_CGSMI,
                           BitValue );
}


EFI_STATUS
EFIAPI
E6xxGpioPinTriggerStatus (
  UINT64 ControllerNumber,
  UINT64 PortNumber,
  UINTN *BitValue
  )
{
  //
  //  Enable the ACPI SMI
  //
  return E6xxGpioBitRead ( ControllerNumber,
                           PortNumber,
                           GPIO_CGTS,
                           BitValue );
}


EFI_STATUS
EFIAPI
E6xxGpioPortDisable (
  UINT64 ControllerNumber,
  UINT64 PortNumber
  )
{
  //
  //  Disable the GPIO port
  //
  return E6xxGpioBitClear ( ControllerNumber, PortNumber, GPIO_CGEN );
}


EFI_STATUS
EFIAPI
E6xxGpioPortEnable (
  UINT64 ControllerNumber,
  UINT64 PortNumber
  )
{
  //
  //  Enable the GPIO port
  //
  return E6xxGpioBitSet ( ControllerNumber, PortNumber, GPIO_CGEN );
}


EFI_STATUS
EFIAPI
E6xxGpioPositiveEdgeDisable (
  UINT64 ControllerNumber,
  UINT64 PortNumber
  )
{
  //
  //  Disable the positive edge trigger
  //
  return E6xxGpioBitClear ( ControllerNumber, PortNumber, GPIO_CGTPE );
}


EFI_STATUS
EFIAPI
E6xxGpioPositiveEdgeEnable (
  UINT64 ControllerNumber,
  UINT64 PortNumber
  )
{
  //
  //  Enable the positive edge trigger
  //
  return E6xxGpioBitSet ( ControllerNumber, PortNumber, GPIO_CGTPE );
}


EFI_STATUS
EFIAPI
E6xxGpioSmiDisable (
  UINT64 ControllerNumber,
  UINT64 PortNumber
  )
{
  //
  //  Disable the ACPI SMI
  //
  return E6xxGpioBitClear ( ControllerNumber, PortNumber, GPIO_CGSMI );
}


EFI_STATUS
EFIAPI
E6xxGpioSmiEnable (
  UINT64 ControllerNumber,
  UINT64 PortNumber
  )
{
  //
  //  Enable the ACPI SMI
  //
  return E6xxGpioBitSet ( ControllerNumber, PortNumber, GPIO_CGSMI );
}

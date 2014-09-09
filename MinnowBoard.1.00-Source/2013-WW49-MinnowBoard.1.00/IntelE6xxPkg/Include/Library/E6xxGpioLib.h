/** @file
E6xx GPIO support library header file

Copyright (c) 2012-2013, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials are licensed and made available under 
the terms and conditions of the BSD License that accompanies this distribution.  
The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php.                                          
    
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

**/

#ifndef _E6XX_GPIO_H_
#define _E6XX_GPIO_H_

EFI_STATUS
EFIAPI
E6xxGpioBaseAddress (
  UINT64 ControllerNumber,
  UINTN *GpioBaseAddress
  );

EFI_STATUS
EFIAPI
E6xxGpioDirectionInput (
  UINT64 ControllerNumber,
  UINT64 PortNumber
  );

EFI_STATUS
EFIAPI
E6xxGpioDirectionOutput (
  UINT64 ControllerNumber,
  UINT64 PortNumber
  );

EFI_STATUS
EFIAPI
E6xxGpioGpeDisable (
  UINT64 ControllerNumber,
  UINT64 PortNumber
  );

EFI_STATUS
EFIAPI
E6xxGpioGpeEnable (
  UINT64 ControllerNumber,
  UINT64 PortNumber
  );

EFI_STATUS
EFIAPI
E6xxGpioOutputClear (
  UINT64 ControllerNumber,
  UINT64 PortNumber
  );

EFI_STATUS
EFIAPI
E6xxGpioOutputSet (
  UINT64 ControllerNumber,
  UINT64 PortNumber
  );

EFI_STATUS
EFIAPI
E6xxGpioNegativeEdgeDisable (
  UINT64 ControllerNumber,
  UINT64 PortNumber
  );

EFI_STATUS
EFIAPI
E6xxGpioNegativeEdgeEnable (
  UINT64 ControllerNumber,
  UINT64 PortNumber
  );

EFI_STATUS
EFIAPI
E6xxGpioPinDirection (
  UINT64 ControllerNumber,
  UINT64 PortNumber,
  UINTN *BitValue
  );

EFI_STATUS
EFIAPI
E6xxGpioPinEnable (
  UINT64 ControllerNumber,
  UINT64 PortNumber,
  UINTN *BitValue
  );

EFI_STATUS
EFIAPI
E6xxGpioPinGpe (
  UINT64 ControllerNumber,
  UINT64 PortNumber,
  UINTN *BitValue
  );

EFI_STATUS
EFIAPI
E6xxGpioPinLevel (
  UINT64 ControllerNumber,
  UINT64 PortNumber,
  UINTN *BitValue
  );

EFI_STATUS
EFIAPI
E6xxGpioPinNegativeEdge (
  UINT64 ControllerNumber,
  UINT64 PortNumber,
  UINTN *BitValue
  );

EFI_STATUS
EFIAPI
E6xxGpioPinPositiveEdge (
  UINT64 ControllerNumber,
  UINT64 PortNumber,
  UINTN *BitValue
  );

EFI_STATUS
EFIAPI
E6xxGpioPinSmi (
  UINT64 ControllerNumber,
  UINT64 PortNumber,
  UINTN *BitValue
  );

EFI_STATUS
EFIAPI
E6xxGpioPinTriggerStatus (
  UINT64 ControllerNumber,
  UINT64 PortNumber,
  UINTN *BitValue
  );

EFI_STATUS
EFIAPI
E6xxGpioPortDisable (
  UINT64 ControllerNumber,
  UINT64 PortNumber
  );

EFI_STATUS
EFIAPI
E6xxGpioPortEnable (
  UINT64 ControllerNumber,
  UINT64 PortNumber
  );

EFI_STATUS
EFIAPI
E6xxGpioPositiveEdgeDisable (
  UINT64 ControllerNumber,
  UINT64 PortNumber
  );

EFI_STATUS
EFIAPI
E6xxGpioPositiveEdgeEnable (
  UINT64 ControllerNumber,
  UINT64 PortNumber
  );

EFI_STATUS
EFIAPI
E6xxGpioSmiDisable (
  UINT64 ControllerNumber,
  UINT64 PortNumber
  );

EFI_STATUS
EFIAPI
E6xxGpioSmiEnable (
  UINT64 ControllerNumber,
  UINT64 PortNumber
  );

#endif  //  _E6XX_GPIO_H_

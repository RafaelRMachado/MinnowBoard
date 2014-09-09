/** @file
  EG20T GPIO support library header file

  Copyright (c) 2013, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _EG20T_GPIO_H_
#define _EG20T_GPIO_H_

EFI_STATUS
EFIAPI
Eg20tGpioBaseAddress (
  UINTN * RegisterAddress
  );

EFI_STATUS
EFIAPI
Eg20tGpioDirectionInput (
  UINT64 PortNumber
  );

EFI_STATUS
EFIAPI
Eg20tGpioDirectionOutput (
  UINT64 PortNumber
  );

EFI_STATUS
EFIAPI
Eg20tGpioInterruptClear (
  UINT64 PortNumber
  );

EFI_STATUS
EFIAPI
Eg20tGpioInterruptDisable (
  UINT64 PortNumber
  );

EFI_STATUS
EFIAPI
Eg20tGpioInterruptEnable (
  UINT64 PortNumber
  );

EFI_STATUS
EFIAPI
Eg20tGpioInterruptMaskClear (
  UINT64 PortNumber
  );

EFI_STATUS
EFIAPI
Eg20tGpioInterruptMaskSet (
  UINT64 PortNumber
  );

EFI_STATUS
EFIAPI
Eg20tGpioInterruptMode (
  UINT64 PortNumber,
  UINT64 Value
  );

EFI_STATUS
EFIAPI
Eg20tGpioOutputClear (
  UINT64 PortNumber
  );

EFI_STATUS
EFIAPI
Eg20tGpioOutputSet (
  UINT64 PortNumber
  );

EFI_STATUS
EFIAPI
Eg20tGpioPinDirection (
  UINT64 PortNumber,
  UINTN *BitValue
  );

EFI_STATUS
EFIAPI
Eg20tGpioPinInterruptEnable (
  UINT64 PortNumber,
  UINTN *BitValue
  );

EFI_STATUS
EFIAPI
Eg20tGpioPinInterruptMask (
  UINT64 PortNumber,
  UINTN *BitValue
  );

EFI_STATUS
EFIAPI
Eg20tGpioPinInterruptMode (
  UINT64 PortNumber,
  UINTN *Value
  );

EFI_STATUS
EFIAPI
Eg20tGpioPinInterruptPending (
  UINT64 PortNumber,
  UINTN *BitValue
  );

EFI_STATUS
EFIAPI
Eg20tGpioPinInterruptRequest (
  UINT64 PortNumber,
  UINTN *BitValue
  );

EFI_STATUS
EFIAPI
Eg20tGpioPinPortInput (
  UINT64 PortNumber,
  UINTN *BitValue
  );

EFI_STATUS
EFIAPI
Eg20tGpioPinPortMode (
  UINT64 PortNumber,
  UINTN *BitValue
  );

EFI_STATUS
EFIAPI
Eg20tGpioPinPortOutput (
  UINT64 PortNumber,
  UINTN *BitValue
  );

EFI_STATUS
EFIAPI
Eg20tGpioSoftReset (
  UINT64 Value
  );

EFI_STATUS
EFIAPI
Eg20tGpioSoftResetStatus (
  UINTN *BitValue
  );

#endif  //  _EG20T_GPIO_H_

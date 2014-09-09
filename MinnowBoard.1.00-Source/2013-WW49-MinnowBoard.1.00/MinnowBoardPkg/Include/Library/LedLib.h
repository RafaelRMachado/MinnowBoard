/** @file
  LED support library declarations
  
  Copyright (c) 2013, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __LED_LIB_H__
#define __LED_LIB_H__

/**
  Turn on LED1

**/
VOID
EFIAPI
Led1On (
  VOID
  );

/**
  Turn off LED1

**/
VOID
EFIAPI
Led1Off (
  VOID
  );

/**
  Toggle the on/off state of LED1

**/
VOID
EFIAPI
Led1Toggle (
  VOID
  );

/**
  Turn on LED2

**/
VOID
EFIAPI
Led2On (
  VOID
  );

/**
  Turn off LED2

**/
VOID
EFIAPI
Led2Off (
  VOID
  );

/**
  Toggle the on/off state of LED2

**/
VOID
EFIAPI
Led2Toggle (
  VOID
  );

#endif  //  __LED_LIB_H__

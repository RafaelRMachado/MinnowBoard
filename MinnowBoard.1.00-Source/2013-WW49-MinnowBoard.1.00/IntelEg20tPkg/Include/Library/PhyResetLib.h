/** @file
  Declare the Ethernet PHY library interface.
  
  Copyright (c) 2013, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _PHY_RESET_LIB_H_
#define _PHY_RESET_LIB_H_

extern CONST CHAR16 * gResetGpioChip;
extern CONST CHAR16 * gResetGpioController [ ];


/**
  Hardware reset the PHY chip

  @param[in] ResetActive      New state for the PHY reset line

**/
VOID
EthernetPhyResetChip (
  BOOLEAN ResetActive
  );

#endif  //  _PHY_RESET_LIB_H_

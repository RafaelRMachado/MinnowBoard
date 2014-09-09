/** @file
  Implement the PHY reset routine.
  
  Copyright (c) 2013, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/Eg20tGpioLib.h>
#include <Library/PcdLib.h>
#include <Library/PhyResetLib.h>

#include <Register/Eg20t.h>


CONST CHAR16 * gResetGpioChip = L"EG20T";
CONST CHAR16 * gResetGpioController [ ] = { L"_GPIO" };

/**
  Hardware reset the PHY chip

  @param[in] ResetActive      New state for the PHY reset line

**/
VOID
EthernetPhyResetChip (
  BOOLEAN ResetActive
  )
{
  UINT32 GpioReset;

  GpioReset = PcdGet32 ( PcdEthernetPhyResetGpio );
  Eg20tGpioDirectionOutput ( GpioReset );
  if ( ResetActive ) {
    //
    //  Reset the PHY
    //
    Eg20tGpioOutputClear ( GpioReset );
  }
  else {
    //
    //  Remove the PHY reset
    //
    Eg20tGpioOutputSet ( GpioReset );
  }
  MemoryFence ( );
}

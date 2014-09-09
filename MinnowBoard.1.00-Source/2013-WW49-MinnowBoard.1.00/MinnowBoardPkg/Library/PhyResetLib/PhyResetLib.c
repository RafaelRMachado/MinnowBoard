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
#include <Library/DebugLib.h>
#include <Library/E6xxGpioLib.h>
#include <Library/PcdLib.h>
#include <Library/PhyResetLib.h>


CONST CHAR16 * gResetGpioChip = L"E6XX";
CONST CHAR16 * gResetGpioController [ ] = { L"_GPIO_", L"_GPIO_SUS" };

/**
  Hardware reset the PHY chip

  @param[in] ResetActive      New state for the PHY reset line

**/
VOID
EthernetPhyResetChip (
  BOOLEAN ResetActive
  )
{
  UINT32 GpioController;
  UINT32 GpioReset;

  GpioController = PcdGet32 ( PcdEthernetPhyResetGpioController );
  GpioReset = PcdGet32 ( PcdEthernetPhyResetGpio );
  E6xxGpioDirectionOutput ( GpioController, GpioReset );
  if ( ResetActive ) {
    //
    //  Reset the PHY
    //
    E6xxGpioOutputClear ( GpioController, GpioReset );
  }
  else {
    //
    //  Remove the PHY reset
    //
    E6xxGpioOutputSet ( GpioController, GpioReset );
  }
  MemoryFence ( );
}

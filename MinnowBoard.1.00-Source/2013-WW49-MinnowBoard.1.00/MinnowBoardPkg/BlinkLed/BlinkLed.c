/** @file
  Blink LED.

  Copyright (c) 2013, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PiDxe.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/PcdLib.h>
#include <Library/LedLib.h>

/**
  Toggle the LED to indicate that the BDS is still alive

  @param  Event                 Event whose notification function is being invoked.
  @param  Context               The pointer to the notification function's context,
                                which is implementation-dependent.

**/
VOID
ToggleLed (
  IN  EFI_EVENT                Event,
  IN  VOID                     *Context
  )
{
  if (PcdGet32 (PcdDiagBootPhasesLedBlinkRate) != 0) {
    Led2Toggle ( );
  }
}

/**
  The user Entry Point for Application. The user code starts with this function
  as the real entry point for the image goes into a library that calls this
  function.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
BlinkLedIntialize (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;
  UINT64      LedBlinkInterval;
  EFI_EVENT   LedBlinkEvent;

  //
  //  Get Blink Interval
  //
  LedBlinkInterval = PcdGet32 (PcdDiagBootPhasesLedBlinkRate);
  if (LedBlinkInterval == 0) {
    LedBlinkInterval = 100;
  }
  LedBlinkInterval = MultU64x32 (LedBlinkInterval, 1000 * 10 );
  
  //
  // Initialize state of LEDs
  //
  Led1Off ();
  Led2On ();

  //
  // Create timer event
  //
  Status = gBS->CreateEvent ( 
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  ToggleLed,
                  NULL,
                  &LedBlinkEvent
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }    

  //
  // Set timer event rate
  //
  Status = gBS->SetTimer (
                  LedBlinkEvent,
                  TimerPeriodic,
                  LedBlinkInterval
                  );
  return Status;
}
 
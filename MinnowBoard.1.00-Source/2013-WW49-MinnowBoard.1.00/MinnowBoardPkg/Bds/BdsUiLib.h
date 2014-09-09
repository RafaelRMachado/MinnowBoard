/** @file
  User Interface Lib for BDS. This is the UI for the template BDS.

  The library instacne would include all the basic UI for a plaform. This
  would include all setup screens and utilities launched from the UI.

  Copyright (c) 2008, Intel Corporation                                                         
  All rights reserved. This program and the accompanying materials                          
  are licensed and made available under the terms and conditions of the BSD License         
  which accompanies this distribution.  The full text of the license may be found at        
  http://opensource.org/licenses/bsd-license.php                                            

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

**/

#ifndef __BDS_UI_LIB_H__
#define __BDS_UI_LIB_H__

#include "BdsLib.h"

/**
  This callback allows the UI code to call back into the BDS to perform a standard
  EFI boot.

  @param  BootOptions   Pointer to the data structure that represnts the boot object

**/
typedef 
VOID
(EFIAPI *BDS_UI_EFI_BOOT_CALL_BACK) (
  IN  EFI_BOOT_MANAGER_BOOT_OPTION   *BootOptions
  );

/**
  Called by BDS after the first time consoles are connected. This API allows a UI and
  hot key processing prior to L"DriverOption" variable processing. Also its possible to
  take boot mode specific action in this function as the boot mode is passed in. It's 
  also legal to put up a logo in this function.

  The EfiBootCallBack function allows the UI to callback into the BDS to do an EFI boot.

  [Note: If we need other callbacks they would get added to this function]

  @param  BootMode          System PI Boot Mode.
  @param  EfiBootCallBack   Register BDS callback to do an EFI boot

**/
VOID
EFIAPI
BdsUiInitialize (
  IN  EFI_BOOT_MODE               BootMode,
  IN  BDS_UI_EFI_BOOT_CALL_BACK   EfiBootCallBack   OPTIONAL
  );

/**
  Called by BDS prior to processing the UEFI boot options. A current set of UEFI
  boot options are passed in. This function produces the UEFI L"Timeout" behavior.
  A Timeout value of zero means the UI may be updated and this function will return
  after the UI is updated. A timeout value of non zero will make this routine wait 
  the prescribed number of seconds. If a key is pressed BdsUiInteractiveMenus () must
  be called and the Key value and CallBack are passed to it. It is legal to process 
  hotkeys in this function via calls to BdsUiInteractiveMenus().

  @param  Timeout         UEFI L"Timeout" variable value.
  @param  BootOptions     Array of boot option information
  @param  BootOptionCount Number of array entries in BootOption

**/
VOID
EFIAPI
BdsUiUpdateBootOptions (
 IN EFI_BOOT_MANAGER_BOOT_OPTION    *BootOptions,
 IN UINTN                           BootOptionCount,
 OUT  EFI_KEY_DATA                  *Key
 );

/**
  Delete BootOptions added via BdsUiUpdateBootOptions().

  @param  BootOptions     Array of boot option information
  @param  BootOptionCount Number of array entries in BootOption

**/
VOID
EFIAPI
BdsUiDeleteBootOptions (
 IN EFI_BOOT_MANAGER_BOOT_OPTION    *BootOptions,
 IN UINTN                           BootOptionCount
 );

/**
  Called by BDS to invoke an interactive memu. This is the end state for the boot
  processing if all boot options fail. An Optional pointer to a key pressed value 
  is supported to indicate that a key has been pressed and this value should be used
  by the UI.

  @param  KeyPress    Optionally pass in a value from the Simple Text In Protocol 

**/
VOID
EFIAPI
BdsUiInteractiveMenus (
  IN  EFI_KEY_DATA     *Key, 
  IN  UINT16           Timeout
  );

/**
  Called by the BDS prior to attempting to boot a boot option. BootOptionIndex is an array
  index to the BootOptions argument passed to BdsUiUpdateBootOptions().

  @param  BootOptionIndex   Index into BootOptions the BDS is trying to boot from.

**/
VOID
EFIAPI
BdsUiUpdateBootAttempt (
  IN UINTN      BootOptionIndex
  );

EFI_STATUS
AppendPickListEntry (
  IN  EFI_BOOT_MANAGER_BOOT_OPTION  *BootOption,
  IN  BOOLEAN                       Removable
  );


#endif

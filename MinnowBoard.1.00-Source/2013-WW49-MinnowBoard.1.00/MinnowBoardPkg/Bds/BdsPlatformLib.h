/** @file
  BDS Platform Library. Each of the library functions are hooks points to 
  allow for customization of the generic BDS behavior.

  Copyright (c) 2008, Intel Corporation                                                         
  All rights reserved. This program and the accompanying materials                          
  are licensed and made available under the terms and conditions of the BSD License         
  which accompanies this distribution.  The full text of the license may be found at        
  http://opensource.org/licenses/bsd-license.php                                            

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

**/

#ifndef __BDS_PLATFORM_LIB_H__
#define __BDS_PLATFORM_LIB_H__

#include "BdsLib.h"

/**
  Platform code is called on BdsEntry after the BootMode and Timeout values
  have been calculated. This routine can override the default BootMode or 
  Timeout settings or do special processing based on the boot mode.

  @param[in,out] BootMode    System PI Boot mode
  @param[in,out] Timeout     UEFI L"Timeout" variable setting.
  
**/
VOID
BdsPlatformEntry (
  IN  EFI_BOOT_MODE   *BootMode,
  IN  UINT16          *Timeout
  );

#endif

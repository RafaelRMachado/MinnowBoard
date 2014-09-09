//
// This file contains 'Framework Code' and is licensed as such 
// under the terms of your license agreement with Intel or your
// vendor.  This file may not be modified, except as allowed by
// additional terms of your license agreement.                 
//
/**@file
  The common DXE header file
  
Copyright (c) 2010-2013 Intel Corporation. All rights reserved.<BR>
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

**/

#ifndef __TCPLATFORM_DXE_H__
#define __TCPLATFORM_DXE_H__

#define EFI_EVENT_NOTIFY_SIGNAL_ALL     0x00000400

#define EFI_EVENT_SIGNAL_READY_TO_BOOT  0x00000203
#define EFI_EVENT_SIGNAL_LEGACY_BOOT    0x00000204

//#include <SetupVariable.h>
//#include <Guid/VpdData.h>

//
// Definition for PPM function enable
// 
typedef union {
  struct {
	  UINT16   EnableGv                   :1; // 0: Disabled; 1: Enabled
	  UINT16   EnableCx                   :1;
	  UINT16   EnableCxe                  :1;
	  UINT16   EnableHardC4E              :1;
	  UINT16   EnableTm1                  :1;
	  UINT16   EnableTm2                  :1;
	  UINT16   EnableProcHot              :1;
	  UINT16   TStatesEnable              :1;
	  UINT16   Rsvd                       :8;
  } Bits;
  UINT16 Uint16;
} PPM_FUNCTION_ENABLES;


#endif

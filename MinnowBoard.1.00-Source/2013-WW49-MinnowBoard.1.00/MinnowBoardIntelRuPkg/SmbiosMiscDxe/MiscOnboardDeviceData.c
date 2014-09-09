/*++
  This file contains an 'Intel Peripheral Driver' and is        
  licensed for Intel CPUs and chipsets under the terms of your  
  license agreement with Intel or your vendor.  This file may   
  be modified by the user, subject to additional terms of the   
  license agreement                                             
--*/
/*++

Copyright (c) 2006 - 2009, Intel Corporation. All rights reserved. <BR>
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  MiscOnboardDeviceData.c

Abstract:

  This driver parses the mMiscSubclassDataTable structure and reports
  any generated data to smbios.

--*/


#include "CommonHeader.h"

#include "SmbiosMisc.h"


//
// Static (possibly build generated) Bios Vendor data.
//
MISC_SMBIOS_TABLE_DATA(EFI_MISC_ONBOARD_DEVICE, MiscOnboardDeviceVideo) = {
  STRING_TOKEN(STR_MISC_ONBOARD_DEVICE_VIDEO),    // OnBoardDeviceDescription
  {                                               // OnBoardDeviceStatus
    EfiOnBoardDeviceTypeVideo,                    // DeviceType
    1,                                            // DeviceEnabled
    0                                             // Reserved
  },
  0                                               // OnBoardDevicePath
};

MISC_SMBIOS_TABLE_DATA(EFI_MISC_ONBOARD_DEVICE, MiscOnboardDeviceNetwork) = {
  STRING_TOKEN(STR_MISC_ONBOARD_DEVICE_NETWORK),  // OnBoardDeviceDescription
  {                                               // OnBoardDeviceStatus
    EfiOnBoardDeviceTypeEthernet,                 // DeviceType
    1,                                            // DeviceEnabled
    0                                             // Reserved
  },
  0                                               // OnBoardDevicePath
};

MISC_SMBIOS_TABLE_DATA(EFI_MISC_ONBOARD_DEVICE, MiscOnboardDeviceAudio) = {
  STRING_TOKEN(STR_MISC_ONBOARD_DEVICE_AUDIO),    // OnBoardDeviceDescription
  {                                               // OnBoardDeviceStatus
    EfiOnBoardDeviceTypeSound,                    // DeviceType
    1,                                            // DeviceEnabled
    0                                             // Reserved
  },
  DP_END                                          // OnBoardDevicePath
};

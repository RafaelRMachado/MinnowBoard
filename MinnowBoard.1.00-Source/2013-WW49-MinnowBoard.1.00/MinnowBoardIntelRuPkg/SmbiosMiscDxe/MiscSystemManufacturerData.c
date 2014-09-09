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

  MiscSystemManufacturerData.c
  
Abstract: 

  This driver parses the mMiscSubclassDataTable structure and reports
  any generated data using smbios protocol.

--*/


#include "CommonHeader.h"

#include "SmbiosMisc.h"


//
// Static (possibly build generated) System Manufacturer data.
//
MISC_SMBIOS_TABLE_DATA(EFI_MISC_SYSTEM_MANUFACTURER, MiscSystemManufacturer) = {
  STRING_TOKEN(STR_MISC_SYSTEM_MANUFACTURER),   // SystemManufactrurer
  STRING_TOKEN(STR_MISC_SYSTEM_PRODUCT_NAME),   // SystemProductName
  STRING_TOKEN(STR_MISC_SYSTEM_VERSION),        // SystemVersion
  STRING_TOKEN(STR_MISC_SYSTEM_SERIAL_NUMBER),  // SystemSerialNumber
  {                                             // SystemUuid
    0x35f954f0, 0x3b81, 0x4717, 0xb6, 0x17, 0x97, 0x83, 0xd9, 0x63, 0x1f, 0xea
  },
  EfiSystemWakeupTypePowerSwitch,               // SystemWakeupType  
  STRING_TOKEN(STR_MISC_SYSTEM_SKU_NUMBER),     // SystemSKUNumber
  STRING_TOKEN(STR_MISC_SYSTEM_FAMILY),         // SystemFamily
};

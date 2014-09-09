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

  MiscBoardData.c
  
Abstract: 

  This driver parses the mMiscSubclassDataTable structure and reports
  any generated data using smbios protocol.

--*/


#include "CommonHeader.h"

#include "SmbiosMisc.h"


//
// Static (possibly build generated) System Manufacturer data.
//
MISC_SMBIOS_TABLE_DATA(EFI_MISC_BASE_BOARD_MANUFACTURER, MiscBoard) = {
  STRING_TOKEN(STR_MISC_BOARD_MANUFACTURER),    // Manufactrurer
  STRING_TOKEN(STR_MISC_BOARD_PRODUCT_NAME),    // ProductName
  STRING_TOKEN(STR_MISC_BOARD_VERSION),         // Version
  STRING_TOKEN(STR_MISC_BOARD_SERIAL_NUMBER),   // SerialNumber
  STRING_TOKEN(STR_MISC_BOARD_ASSET_TAG)        // AssetTag
};

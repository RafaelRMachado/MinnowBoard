//
// This file contains an 'Intel Peripheral Driver' and is      
// licensed for Intel CPUs and chipsets under the terms of your
// license agreement with Intel or your vendor.  This file may 
// be modified by the user, subject to additional terms of the 
// license agreement                                           
//
/*++

Copyright (c) 2006, Intel Corporation. All rights reserved.
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  MiscDevicePath.h

Abstract:

  Misc class required EFI Device Path definitions (Ports, slots &
  onboard devices)

--*/

#ifndef _MISC_DEVICE_PATH_H
#define _MISC_DEVICE_PATH_H

#pragma pack(1)

//USB
/* For reference:
#define USB1_1_STR  "ACPI(PNP0A03,0)/PCI(1D,0)."
#define USB1_2_STR  "ACPI(PNP0A03,0)/PCI(1D,1)."
#define USB1_3_STR  "ACPI(PNP0A03,0)/PCI(1D,2)."
#define USB2_1_STR  "ACPI(PNP0A03,0)/PCI(1D,7)."
*/

#define DP_ACPI { ACPI_DEVICE_PATH,\
    ACPI_DP, (UINT8) (sizeof (ACPI_HID_DEVICE_PATH)),\
   (UINT8) ((sizeof (ACPI_HID_DEVICE_PATH)) >> 8), EISA_PNP_ID(0x0A03), 0 }
#define DP_PCI( device,function)  { HARDWARE_DEVICE_PATH,\
    HW_PCI_DP, (UINT8) (sizeof (PCI_DEVICE_PATH)),\
    (UINT8) ((sizeof (PCI_DEVICE_PATH)) >> 8), function, device }
#define DP_END  { END_DEVICE_PATH_TYPE, \
    END_ENTIRE_DEVICE_PATH_SUBTYPE, END_DEVICE_PATH_LENGTH, 0 }

#define DP_LPC(eisaid,function ){ ACPI_DEVICE_PATH, \
ACPI_DP,(UINT8) (sizeof (ACPI_HID_DEVICE_PATH)),\
(UINT8) ((sizeof (ACPI_HID_DEVICE_PATH)) >> 8),EISA_PNP_ID(eisaid), function }


#pragma pack()


#endif

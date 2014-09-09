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

  MiscBiosVendorData.c
  
Abstract: 

  BIOS vendor information static data.
  Misc. subclass type 2.
  SMBIOS type 0.

--*/


#include "CommonHeader.h"

#include "SmbiosMisc.h"


//
// Static (possibly build generated) Bios Vendor data.
//
MISC_SMBIOS_TABLE_DATA(EFI_MISC_BIOS_VENDOR, MiscBiosVendor) = {
  STRING_TOKEN (STR_MISC_BIOS_VENDOR),       // BiosVendor
  STRING_TOKEN (STR_MISC_BIOS_VERSION),      // BiosVersion
  STRING_TOKEN (STR_MISC_BIOS_RELEASE_DATE), // BiosReleaseDate
  0xE0000,                    // BiosStartingAddress
  {                           // BiosPhysicalDeviceSize
    4,                        // Value
    20,                       // Exponent
  },
  {                           // BiosCharacteristics1
    0,                        // Reserved1                         :2
    0,                        // Unknown                           :1
    0,                        // BiosCharacteristicsNotSupported   :1
    0,                        // IsaIsSupported                    :1
    0,                        // McaIsSupported                    :1

    0,                        // EisaIsSupported                   :1
    1,                        // PciIsSupported                    :1
    0,                        // PcmciaIsSupported                 :1
    0,                        // PlugAndPlayIsSupported            :1
    0,                        // ApmIsSupported                    :1

    1,                        // BiosIsUpgradable                  :1
    1,                        // BiosShadowingAllowed              :1
    0,                        // VlVesaIsSupported                 :1
    0,                        // EscdSupportIsAvailable            :1
    1,                        // BootFromCdIsSupported             :1

    1,                        // SelectableBootIsSupported         :1
    0,                        // RomBiosIsSocketed                 :1
    0,                        // BootFromPcmciaIsSupported         :1
    1,                        // EDDSpecificationIsSupported       :1
    0,                        // JapaneseNecFloppyIsSupported      :1

    0,                        // JapaneseToshibaFloppyIsSupported  :1
    0,                        // Floppy525_360IsSupported          :1
    0,                        // Floppy525_12IsSupported           :1
    0,                        // Floppy35_720IsSupported           :1
    0,                        // Floppy35_288IsSupported           :1

    1,                        // PrintScreenIsSupported            :1
    1,                        // Keyboard8042IsSupported           :1
    1,                        // SerialIsSupported                 :1
    1,                        // PrinterIsSupported                :1
    1,                        // CgaMonoIsSupported                :1

    0,                        // NecPc98                           :1
    1,                        // AcpiIsSupported                   :1
    1,                        // UsbLegacyIsSupported              :1
    0,                        // AgpIsSupported                    :1
    0,                        // I20BootIsSupported                :1

    0,                        // Ls120BootIsSupported              :1
    0,                        // AtapiZipDriveBootIsSupported      :1
    0,                        // Boot1394IsSupported               :1
    0,                        // SmartBatteryIsSupported           :1
    1,                        // BiosBootSpecIsSupported           :1

    1,                        // FunctionKeyNetworkBootIsSupported :1
    0                         // Reserved                          :22
  },
  {                           // BiosCharacteristics2
    0,                        // BiosReserved                      :16
    0,                        // SystemReserved                    :16
    0                         // Reserved                          :32
  },
  0x1,                        // System BIOS Major Release         
  0x0,                        // System BIOS Minor Release  
  0xFF,                       // Embedded controller firmware major Release  
  0xFF,                       // Embedded controller firmware minor Release   
};

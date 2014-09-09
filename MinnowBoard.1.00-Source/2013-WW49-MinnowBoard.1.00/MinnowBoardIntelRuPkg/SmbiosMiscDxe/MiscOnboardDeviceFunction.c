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

  MiscOnboardDeviceFunction.c

Abstract:

  Onboard device information boot time changes.
  SMBIOS type 10.

--*/


#include "CommonHeader.h"

#include "SmbiosMisc.h"


/**
  This function makes boot time changes to the contents of the
  MiscOnboardDevice (Type 10).

  @param  RecordData                 Pointer to copy of RecordData from the Data Table.  

  @retval EFI_SUCCESS                All parameters were valid.
  @retval EFI_UNSUPPORTED            Unexpected RecordType value.
  @retval EFI_INVALID_PARAMETER      Invalid parameter was found.

**/
MISC_SMBIOS_TABLE_FUNCTION(MiscOnboardDevice)
{
  CHAR8                         *OptionalStrStart;
  UINT8                         StatusAndType;
  UINTN                         DescriptionStrLen;
  EFI_STRING                    DeviceDescription;
  STRING_REF                    TokenToGet;
  EFI_STATUS                    Status;
  EFI_SMBIOS_HANDLE             SmbiosHandle;
  SMBIOS_TABLE_TYPE10           *SmbiosRecord;
  EFI_MISC_ONBOARD_DEVICE       *ForType10InputData;

  ForType10InputData = (EFI_MISC_ONBOARD_DEVICE *)RecordData;
  //
  // First check for invalid parameters.
  //
  if (RecordData == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  TokenToGet = 0;
  switch (ForType10InputData->OnBoardDeviceDescription) {
    case STR_MISC_ONBOARD_DEVICE_VIDEO:
      TokenToGet = STRING_TOKEN (STR_MISC_ONBOARD_DEVICE_VIDEO);      
      break;
    case STR_MISC_ONBOARD_DEVICE_AUDIO:
      TokenToGet = STRING_TOKEN (STR_MISC_ONBOARD_DEVICE_AUDIO);
      break;
  }
  
  DeviceDescription = HiiGetPackageString(&gEfiCallerIdGuid, TokenToGet, NULL);
  DescriptionStrLen = StrLen(DeviceDescription);
  if (DescriptionStrLen > SMBIOS_STRING_MAX_LENGTH) {
    return EFI_UNSUPPORTED;
  }

  //
  // Two zeros following the last string.
  //
  SmbiosRecord = AllocatePool(sizeof (SMBIOS_TABLE_TYPE10) + DescriptionStrLen + 1 + 1);
  ZeroMem(SmbiosRecord, sizeof (SMBIOS_TABLE_TYPE10) + DescriptionStrLen + 1 + 1);

  SmbiosRecord->Hdr.Type = EFI_SMBIOS_TYPE_ONBOARD_DEVICE_INFORMATION;
  SmbiosRecord->Hdr.Length = sizeof (SMBIOS_TABLE_TYPE10);
  //
  // Make handle chosen by smbios protocol.add automatically.
  //
  SmbiosRecord->Hdr.Handle = 0;

  //
  // Status & Type: Bit 7 Devicen Status, Bits 6:0 Type of Device
  //
  StatusAndType = (UINT8) ForType10InputData->OnBoardDeviceStatus.DeviceType;
  if (ForType10InputData->OnBoardDeviceStatus.DeviceEnabled != 0) {
    StatusAndType |= 0x80;
  } else {
    StatusAndType &= 0x7F;
  }

  SmbiosRecord->Device[0].DeviceType = StatusAndType;
  SmbiosRecord->Device[0].DescriptionString = 1;
  OptionalStrStart = (CHAR8 *)(SmbiosRecord + 1);
  UnicodeStrToAsciiStr(DeviceDescription, OptionalStrStart);

  //
  // Now we have got the full smbios record, call smbios protocol to add this record.
  //
  Status = AddSmbiosRecord (Smbios, &SmbiosHandle, (EFI_SMBIOS_TABLE_HEADER *) SmbiosRecord);
  FreePool(SmbiosRecord);
  return Status;
}

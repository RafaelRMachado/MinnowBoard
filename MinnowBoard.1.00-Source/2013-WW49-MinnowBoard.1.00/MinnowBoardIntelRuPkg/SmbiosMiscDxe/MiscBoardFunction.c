/*++
  This file contains an 'Intel Peripheral Driver' and is        
  licensed for Intel CPUs and chipsets under the terms of your  
  license agreement with Intel or your vendor.  This file may   
  be modified by the user, subject to additional terms of the   
  license agreement                                             
--*/
/*++

Copyright (c) 2006 - 2010, Intel Corporation. All rights reserved. <BR> 
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  MiscSystemBoardFunction.c
  
Abstract: 

  This driver parses the mMiscSubclassDataTable structure and reports
  any generated data to smbios.

--*/


#include "CommonHeader.h"

#include "SmbiosMisc.h"

/**
  This function makes boot time changes to the contents of the
  MiscSystemBoard (Type 2).

  @param  RecordData                 Pointer to copy of RecordData from the Data Table.  

  @retval EFI_SUCCESS                All parameters were valid.
  @retval EFI_UNSUPPORTED            Unexpected RecordType value.
  @retval EFI_INVALID_PARAMETER      Invalid parameter was found.

**/
MISC_SMBIOS_TABLE_FUNCTION(MiscBoard)
{
  CHAR8                             *OptionalStrStart;
  UINTN                             ManuStrLen;
  UINTN                             VerStrLen;
  UINTN                             PdNameStrLen;
  UINTN                             SerialNumStrLen;
  UINTN                             AssetTagLen;
  EFI_STATUS                        Status;
  CHAR16                            Manufacturer[SMBIOS_STRING_MAX_LENGTH];
  CHAR16                            ProductName[SMBIOS_STRING_MAX_LENGTH];
  CHAR16                            Version[SMBIOS_STRING_MAX_LENGTH];
  CHAR16                            SerialNumber[SMBIOS_STRING_MAX_LENGTH];
  CHAR16                            AssetTag[SMBIOS_STRING_MAX_LENGTH];
  EFI_STRING                        ManufacturerPtr;
  EFI_STRING                        ProductNamePtr;
  EFI_STRING                        VersionPtr;
  EFI_STRING                        SerialNumberPtr;
  EFI_STRING                        AssetTagPtr;
  STRING_REF                        TokenToGet;
  STRING_REF                        TokenToUpdate;
  EFI_SMBIOS_HANDLE                 SmbiosHandle;
  SMBIOS_TABLE_TYPE2                *SmbiosRecord;
  EFI_MISC_BASE_BOARD_MANUFACTURER  *ForType2InputData;
  UINTN                             RecordLengthInBytes;

DEBUG (( DEBUG_ERROR,
            "\r\n"
            "------------------------------------------------------------\r\n"
            "      SMBIOS Base Board\r\n"
            "------------------------------------------------------------\r\n"
            "\r\n" ));

  ForType2InputData = (EFI_MISC_BASE_BOARD_MANUFACTURER *)RecordData;

  //
  // First check for invalid parameters.
  //
  if (RecordData == NULL) {
    DEBUG (( DEBUG_ERROR, "RecordData = NULL\r\n" ));
    return EFI_INVALID_PARAMETER;
  }

  //
  // Update strings from PCD
  //
  AsciiStrToUnicodeStr ((const CHAR8*)PcdGetPtr(PcdSMBIOSBoardManufacturer), Manufacturer);
  if (StrLen (Manufacturer) > 0) {     
    TokenToUpdate = STRING_TOKEN (STR_MISC_BOARD_MANUFACTURER);
    HiiSetString (mHiiHandle, TokenToUpdate, Manufacturer, NULL);
  }
  TokenToGet = STRING_TOKEN (STR_MISC_BOARD_MANUFACTURER);
  ManufacturerPtr = HiiGetPackageString(&gEfiCallerIdGuid, TokenToGet, NULL);
  ManuStrLen = StrLen(ManufacturerPtr);
  if (ManuStrLen > SMBIOS_STRING_MAX_LENGTH) {
    DEBUG (( DEBUG_ERROR, "ManuStrLen (0x%08x) > 0x%08x\r\n", ManuStrLen, SMBIOS_STRING_MAX_LENGTH ));
    return EFI_UNSUPPORTED;
  }
  
  AsciiStrToUnicodeStr ((const CHAR8*)PcdGetPtr(PcdSMBIOSBoardProductName), ProductName);
  if (StrLen (ProductName) > 0) {     
    TokenToUpdate = STRING_TOKEN (STR_MISC_BOARD_PRODUCT_NAME);
    HiiSetString (mHiiHandle, TokenToUpdate, ProductName, NULL);
  }
  TokenToGet = STRING_TOKEN (STR_MISC_BOARD_PRODUCT_NAME);
  ProductNamePtr = HiiGetPackageString(&gEfiCallerIdGuid, TokenToGet, NULL);
  PdNameStrLen = StrLen(ProductNamePtr);
  if (PdNameStrLen > SMBIOS_STRING_MAX_LENGTH) {
    DEBUG (( DEBUG_ERROR, "PdNameStrLen (0x%08x) > 0x%08x\r\n", PdNameStrLen, SMBIOS_STRING_MAX_LENGTH ));
    return EFI_UNSUPPORTED;
  }
  
  AsciiStrToUnicodeStr ((const CHAR8*)PcdGetPtr(PcdSMBIOSBoardVersion), Version);
  if (StrLen (Version) > 0) {     
    TokenToUpdate = STRING_TOKEN (STR_MISC_BOARD_VERSION);
    HiiSetString (mHiiHandle, TokenToUpdate, Version, NULL);
  }
  TokenToGet = STRING_TOKEN (STR_MISC_BOARD_VERSION);
  VersionPtr = HiiGetPackageString(&gEfiCallerIdGuid, TokenToGet, NULL);
  VerStrLen = StrLen(VersionPtr);
  if (VerStrLen > SMBIOS_STRING_MAX_LENGTH) {
    DEBUG (( DEBUG_ERROR, "VerStrLen (0x%08x) > 0x%08x\r\n", VerStrLen, SMBIOS_STRING_MAX_LENGTH ));
    return EFI_UNSUPPORTED;
  }
  
  AsciiStrToUnicodeStr ((const CHAR8*)PcdGetPtr(PcdSMBIOSBoardSerialNumber), SerialNumber);
  if (StrLen (SerialNumber) > 0) {     
    TokenToUpdate = STRING_TOKEN (STR_MISC_BOARD_SERIAL_NUMBER);
    HiiSetString (mHiiHandle, TokenToUpdate, SerialNumber, NULL);
  }
  TokenToGet = STRING_TOKEN (STR_MISC_BOARD_SERIAL_NUMBER);
  SerialNumberPtr = HiiGetPackageString(&gEfiCallerIdGuid, TokenToGet, NULL);
  SerialNumStrLen = StrLen(SerialNumberPtr);
  if (SerialNumStrLen > SMBIOS_STRING_MAX_LENGTH) {
    DEBUG (( DEBUG_ERROR, "SerialNumStrLen (0x%08x) > 0x%08x\r\n", SerialNumStrLen, SMBIOS_STRING_MAX_LENGTH ));
    return EFI_UNSUPPORTED;
  }

  AsciiStrToUnicodeStr ((const CHAR8*)PcdGetPtr(PcdSMBIOSBoardAssetTag), AssetTag);
  if (StrLen (AssetTag) > 0) {     
    TokenToUpdate = STRING_TOKEN (STR_MISC_BOARD_ASSET_TAG);
    HiiSetString (mHiiHandle, TokenToUpdate, AssetTag, NULL);
  }
  TokenToGet = STRING_TOKEN (STR_MISC_BOARD_ASSET_TAG);
  AssetTagPtr = HiiGetPackageString(&gEfiCallerIdGuid, TokenToGet, NULL);
  AssetTagLen = StrLen(AssetTagPtr);
  if (AssetTagLen > SMBIOS_STRING_MAX_LENGTH) {
    DEBUG (( DEBUG_ERROR, "AssetTagLen (0x%08x) > 0x%08x\r\n", AssetTagLen, SMBIOS_STRING_MAX_LENGTH ));
    return EFI_UNSUPPORTED;
  }
  
  //
  // Two zeros following the last string.
  //
  RecordLengthInBytes = sizeof (SMBIOS_TABLE_TYPE2)
                        + ManuStrLen + 1
                        + PdNameStrLen + 1
                        + VerStrLen + 1
                        + SerialNumStrLen + 1
                        + AssetTagLen + 1
                        + 1;
  SmbiosRecord = AllocatePool(RecordLengthInBytes);
  ZeroMem(SmbiosRecord, RecordLengthInBytes);
  SmbiosRecord->Hdr.Type = EFI_SMBIOS_TYPE_BASEBOARD_INFORMATION;
  SmbiosRecord->Hdr.Length = sizeof (SMBIOS_TABLE_TYPE2);
  //
  // Make handle chosen by smbios protocol.add automatically.
  // 
  SmbiosRecord->Hdr.Handle = 0;  
  //
  // Manu will be the 1st optional string following the formatted structure.
  //
  SmbiosRecord->Manufacturer = 1;
  //
  // ProductName will be the 2nd optional string following the formatted structure.
  //
  SmbiosRecord->ProductName = 2;  
  //
  // Version will be the 3rd optional string following the formatted structure.  
  //
  SmbiosRecord->Version = 3;  
  //
  // Serial number will be the 4th optional string following the formatted structure.
  //
  SmbiosRecord->SerialNumber = 4;
  //
  // Asset Tag will be the 5th optional string following the formatted structure.
  //
  SmbiosRecord->AssetTag = 5;
  SmbiosRecord->BoardType = BaseBoardTypeMotherBoard;
  SmbiosRecord->FeatureFlag.Motherboard = 1;

  OptionalStrStart = (CHAR8 *)(SmbiosRecord + 1);
  UnicodeStrToAsciiStr(ManufacturerPtr, OptionalStrStart);
  UnicodeStrToAsciiStr(ProductNamePtr, OptionalStrStart + ManuStrLen + 1);
  UnicodeStrToAsciiStr(VersionPtr, OptionalStrStart + ManuStrLen + 1 + PdNameStrLen + 1);
  UnicodeStrToAsciiStr(SerialNumberPtr, OptionalStrStart + ManuStrLen + 1 + PdNameStrLen + 1 + VerStrLen + 1);
  UnicodeStrToAsciiStr(AssetTagPtr, OptionalStrStart + ManuStrLen + 1 + PdNameStrLen + 1 + VerStrLen + 1 + SerialNumStrLen+ 1);

  //
  // Now we have got the full smbios record, call smbios protocol to add this record.
  //
  Status = AddSmbiosRecord (Smbios, &SmbiosHandle, (EFI_SMBIOS_TABLE_HEADER *) SmbiosRecord);
  FreePool(SmbiosRecord);
DEBUG (( DEBUG_ERROR, "MiscBoard returning Status: %r\r\n", Status ));
  return Status;
}

//
// This file contains an 'Intel Peripheral Driver' and is      
// licensed for Intel CPUs and chipsets under the terms of your
// license agreement with Intel or your vendor.  This file may 
// be modified by the user, subject to additional terms of the 
// license agreement                                           
//
/*++
 
Copyright (c) 2006 - 2009, Intel Corporation. All rights reserved. 
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  SmbiosMisc.h

Abstract:

  Header file for the SmbiosMisc Driver.

--*/

#ifndef _SMBIOS_MISC_DRIVER_H
#define _SMBIOS_MISC_DRIVER_H

#include "MiscDevicePath.h"

///
/// Reference SMBIOS 2.6, chapter 3.1.3.
/// Each text string is limited to 64 significant characters due to system MIF limitations.
///
#define SMBIOS_STRING_MAX_LENGTH        64
#define SMBIOS_PORT_CONNECTOR_MAX_NUM   16

typedef struct {
  CHAR16   PortInternalConnectorDesignator[SMBIOS_STRING_MAX_LENGTH];
  CHAR16   PortExternalConnectorDesignator[SMBIOS_STRING_MAX_LENGTH];
  UINT8    PortInternalConnectorType;
  UINT8    PortExternalConnectorType;
  UINT8    PortType;
} SMBIOS_PORT_CONNECTOR_DESIGNATOR;   

typedef struct {
  UINT8                             SMBIOSConnectorNumber;
  SMBIOS_PORT_CONNECTOR_DESIGNATOR  SMBIOSPortConnector[SMBIOS_PORT_CONNECTOR_MAX_NUM];
} SMBIOS_PORT_CONNECTOR_DESIGNATOR_COFNIG;

#define SMBIOS_SYSTEM_SLOT_MAX_NUM  14

typedef struct {
  CHAR16    SlotDesignation[SMBIOS_STRING_MAX_LENGTH];
  UINT8     SlotType;
  UINT8     SlotDataBusWidth;
  UINT8     SlotUsage;
  UINT8     SlotLength;
  UINT16    SlotId;
  UINT32    SlotCharacteristics;
} SMBIOS_SLOT_DESIGNATION;

typedef struct {
  UINT8                    SMBIOSSystemSlotNumber;
  SMBIOS_SLOT_DESIGNATION  SMBIOSSystemSlot[SMBIOS_SYSTEM_SLOT_MAX_NUM];
} SMBIOS_SLOT_COFNIG;

//
// Data table entry update function.
//
typedef EFI_STATUS (EFIAPI EFI_MISC_SMBIOS_DATA_FUNCTION) (
  IN  VOID                 *RecordData,
  IN  EFI_SMBIOS_PROTOCOL  *Smbios
  );


//
// Data table entry definition.
//
typedef struct {
  //
  // intermediat input data for SMBIOS record
  //
  VOID                              *RecordData;
  EFI_MISC_SMBIOS_DATA_FUNCTION     *Function;
} EFI_MISC_SMBIOS_DATA_TABLE;

//
// Data Table extern definitions.
//
#define MISC_SMBIOS_DATA_TABLE_POINTER(NAME1) \
& NAME1 ## Data

//
// Data Table extern definitions.
//
#define MISC_SMBIOS_DATA_TABLE_EXTERNS(NAME1, NAME2) \
extern NAME1 NAME2 ## Data

//
// Data and function Table extern definitions.
//
#define MISC_SMBIOS_TABLE_EXTERNS(NAME1, NAME2, NAME3) \
extern NAME1 NAME2 ## Data; \
extern EFI_MISC_SMBIOS_DATA_FUNCTION NAME3 ## Function


//
// Data Table entries
//

#define MISC_SMBIOS_TABLE_ENTRY_DATA_AND_FUNCTION(NAME1, NAME2) \
{ \
  & NAME1 ## Data, \
  & NAME2 ## Function \
}


//
// Global definition macros.
//
#define MISC_SMBIOS_TABLE_DATA(NAME1, NAME2) \
  NAME1 NAME2 ## Data

#define MISC_SMBIOS_TABLE_FUNCTION(NAME2) \
  EFI_STATUS EFIAPI NAME2 ## Function( \
  IN  VOID                  *RecordData, \
  IN  EFI_SMBIOS_PROTOCOL   *Smbios \
  )


// Data Table Array
//
extern EFI_MISC_SMBIOS_DATA_TABLE   mSmbiosMiscDataTable[];

//
// Data Table Array Entries
//
extern UINTN                        mSmbiosMiscDataTableEntries;
extern EFI_HII_HANDLE               mHiiHandle;
/**
  Add an SMBIOS record.

  @param  Smbios                The EFI_SMBIOS_PROTOCOL instance.
  @param  SmbiosHandle          A unique handle will be assigned to the SMBIOS record.
  @param  Record                The data for the fixed portion of the SMBIOS record. The format of the record is
                                determined by EFI_SMBIOS_TABLE_HEADER.Type. The size of the formatted area is defined 
                                by EFI_SMBIOS_TABLE_HEADER.Length and either followed by a double-null (0x0000) or 
                                a set of null terminated strings and a null.

  @retval EFI_SUCCESS           Record was added.
  @retval EFI_OUT_OF_RESOURCES  Record was not added due to lack of system resources.

**/
EFI_STATUS
AddSmbiosRecord (
  IN EFI_SMBIOS_PROTOCOL        *Smbios,
  OUT EFI_SMBIOS_HANDLE         *SmbiosHandle,
  IN EFI_SMBIOS_TABLE_HEADER    *Record
  );

#endif

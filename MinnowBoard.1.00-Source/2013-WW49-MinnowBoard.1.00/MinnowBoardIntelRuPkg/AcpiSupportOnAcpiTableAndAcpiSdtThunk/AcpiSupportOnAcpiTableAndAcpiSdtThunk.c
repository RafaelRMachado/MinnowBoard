/*++
  This file contains an 'Intel Peripheral Driver' and is        
  licensed for Intel CPUs and chipsets under the terms of your  
  license agreement with Intel or your vendor.  This file may   
  be modified by the user, subject to additional terms of the   
  license agreement                                             
--*/
/** @file
  Implementation of AcpiSupport Protocol based on AcpiTable and AcpiSdt Protocol.

  Intel's Framework AcpiSupport Protocol is replaced by AcpiTable Protocol in UEFI specification
  and AcpiSdt Protocol in PI specification.
  This module produces AcpiSupport Protocol on top of AcpiTable and AcpiSdt Protocol.

Copyright (c) 2010 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

**/

#include <PiDxe.h>
#include <Protocol/AcpiTable.h>
#include <Protocol/AcpiSystemDescriptionTable.h>
#include <Protocol/AcpiSupport.h>
#include <IndustryStandard/Acpi.h>

#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>

/**
  Returns a requested ACPI table.

  @param  This                  A pointer to the EFI_ACPI_SUPPORT_PROTOCOL instance.
  @param  Index                 The zero-based index of the table to retrieve.
  @param  Table                 The pointer for returning the table buffer.
  @param  Version               Updated with the ACPI versions to which this table belongs.
  @param  Handle                The pointer for identifying the table.

  @retval EFI_SUCCESS           The function completed successfully.
  @retval EFI_NOT_FOUND         The requested index is too large and a table was not found.

**/
EFI_STATUS
EFIAPI
GetAcpiTable (
  IN EFI_ACPI_SUPPORT_PROTOCOL            *This,
  IN INTN                                 Index,
  OUT VOID                                **Table,
  OUT EFI_ACPI_TABLE_VERSION              *Version,
  OUT UINTN                               *Handle
  );

/**
  Used to add, remove, or update ACPI tables.

  @param  This                  A pointer to the EFI_ACPI_SUPPORT_PROTOCOL instance.
  @param  Table                 The pointer to the new table to add or update.
  @param  Checksum              If TRUE, indicates that the checksum should be
                                calculated for this table.
  @param  Version               Indicates to which version(s) of ACPI the table should be added.
  @param  Handle                The pointer to the handle of the table to remove or update.

  @retval EFI_SUCCESS           The function completed successfully.
  @retval EFI_INVALID_PARAMETER *Handle was zero and Table was NULL.
  @retval EFI_ABORTED           Could not complete the desired action.

**/
EFI_STATUS
EFIAPI
SetAcpiTable (
  IN EFI_ACPI_SUPPORT_PROTOCOL            *This,
  IN VOID                                 *Table OPTIONAL,
  IN BOOLEAN                              Checksum,
  IN EFI_ACPI_TABLE_VERSION               Version,
  IN OUT UINTN                            *Handle
  );

/**
  Causes one or more versions of the ACPI tables to be published in
  the EFI system configuration tables.

  The PublishTables() function installs the ACPI tables for the versions that are specified in 
  Version. No tables are published for Version equal to EFI_ACPI_VERSION_NONE. Once 
  published, tables will continue to be updated as tables are modified with 
  EFI_ACPI_SUPPORT_PROTOCOL.SetAcpiTable(). 

  @param  This                  A pointer to the EFI_ACPI_SUPPORT_PROTOCOL instance.
  @param  Version               Indicates to which version(s) of ACPI the table should be published.

  @retval EFI_SUCCESS           The function completed successfully.
  @retval EFI_ABORTED           An error occurred and the function could not complete successfully.

**/
EFI_STATUS
EFIAPI
PublishTables (
  IN EFI_ACPI_SUPPORT_PROTOCOL            *This,
  IN EFI_ACPI_TABLE_VERSION               Version
  );

EFI_HANDLE   mAcpiSupportHandle = NULL;

EFI_ACPI_SDT_PROTOCOL   *mAcpiSdt;
EFI_ACPI_TABLE_PROTOCOL *mAcpiTable;

EFI_ACPI_SUPPORT_PROTOCOL  mAcpiSupport = {
  GetAcpiTable,
  SetAcpiTable,
  PublishTables
};

/**
  This function calculates and updates an UINT8 checksum.

  @param  Buffer          Pointer to buffer to checksum
  @param  Size            Number of bytes to checksum
  @param  ChecksumOffset  Offset to place the checksum result in

**/
VOID
AcpiPlatformChecksum (
  IN VOID       *Buffer,
  IN UINTN      Size,
  IN UINTN      ChecksumOffset
  )
{
  UINT8 Sum;
  UINT8 *Ptr;

  Sum = 0;
  //
  // Initialize pointer
  //
  Ptr = Buffer;

  //
  // set checksum to 0 first
  //
  Ptr[ChecksumOffset] = 0;

  //
  // add all content of buffer
  //
  while ((Size--) != 0) {
    Sum = (UINT8) (Sum + (*Ptr++));
  }
  //
  // set checksum
  //
  Ptr                 = Buffer;
  Ptr[ChecksumOffset] = (UINT8) (0xff - Sum + 1);

  return ;
}

/**
  Returns a requested ACPI table.

  @param  This                  A pointer to the EFI_ACPI_SUPPORT_PROTOCOL instance.
  @param  Index                 The zero-based index of the table to retrieve.
  @param  Table                 The pointer for returning the table buffer.
  @param  Version               Updated with the ACPI versions to which this table belongs.
  @param  Handle                The pointer for identifying the table.

  @retval EFI_SUCCESS           The function completed successfully.
  @retval EFI_NOT_FOUND         The requested index is too large and a table was not found.

**/
EFI_STATUS
EFIAPI
GetAcpiTable (
  IN EFI_ACPI_SUPPORT_PROTOCOL            *This,
  IN INTN                                 Index,
  OUT VOID                                **Table,
  OUT EFI_ACPI_TABLE_VERSION              *Version,
  OUT UINTN                               *Handle
  )
{
  EFI_ACPI_SDT_HEADER                 *CurrentTable;
  EFI_STATUS                          Status;

  Status = mAcpiSdt->GetAcpiTable (
                       Index,
                       &CurrentTable,
                       Version,
                       Handle
                       );
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  //
  // Good. Table is found.
  // We need allocate table, because AcpiSdt only return table pointer.
  //
  *Table = AllocateCopyPool (CurrentTable->Length, CurrentTable);
  ASSERT (*Table);

  return EFI_SUCCESS;
}

/**
  Used to add, remove, or update ACPI tables.

  @param  This                  A pointer to the EFI_ACPI_SUPPORT_PROTOCOL instance.
  @param  Table                 The pointer to the new table to add or update.
  @param  Checksum              If TRUE, indicates that the checksum should be
                                calculated for this table.
  @param  Version               Indicates to which version(s) of ACPI the table should be added.
  @param  Handle                The pointer to the handle of the table to remove or update.

  @retval EFI_SUCCESS           The function completed successfully.
  @retval EFI_INVALID_PARAMETER *Handle was zero and Table was NULL.
  @retval EFI_ABORTED           Could not complete the desired action.

**/
EFI_STATUS
EFIAPI
SetAcpiTable (
  IN EFI_ACPI_SUPPORT_PROTOCOL            *This,
  IN VOID                                 *Table OPTIONAL,
  IN BOOLEAN                              Checksum,
  IN EFI_ACPI_TABLE_VERSION               Version,
  IN OUT UINTN                            *Handle
  )
{
  EFI_ACPI_SDT_HEADER                 *CurrentTable;
  EFI_STATUS                          Status;

  if (Version == EFI_ACPI_TABLE_VERSION_NONE) {
    return EFI_SUCCESS;
  }

  CurrentTable = (EFI_ACPI_SDT_HEADER *)Table;

  if (*Handle == 0) {
    if (CurrentTable == NULL) {
      //
      // Invalid parameter combination
      //
      return EFI_INVALID_PARAMETER;
    } else {
      //
      // Add table
      //
      if (Checksum) {
        AcpiPlatformChecksum (
          (VOID *)CurrentTable,
          CurrentTable->Length,
          OFFSET_OF (EFI_ACPI_SDT_HEADER, Checksum)
          );
      }
      
      //
      // BUGBUG: Need handle FADT specially.
      // Platform may install FADTv1 and FADTv2.
      // But invoking InstallAcpiTable() will cause SetAcpiTable (V1 | V2 | V3).
      // Finally, checking version in AddTableToList() will error on duplication.
      // So we have to add work-around here.
      // We just add latest version FADT and ignore early version.
      //
      if (CurrentTable->Signature == EFI_ACPI_1_0_FIXED_ACPI_DESCRIPTION_TABLE_SIGNATURE) {
        if (Version == EFI_ACPI_TABLE_VERSION_1_0B) {
          return EFI_SUCCESS;
        }
      }

      Status = mAcpiTable->InstallAcpiTable (
                             mAcpiTable,
                             CurrentTable,
                             CurrentTable->Length,
                             Handle
                             );
    }
  } else {
    if (Table != NULL) {
      //
      // Update table
      //
      //
      // Delete the table list entry
      //
      Status = mAcpiTable->UninstallAcpiTable (mAcpiTable, *Handle);
      if (EFI_ERROR (Status)) {
        //
        // Should not get an error here ever, but abort if we do.
        //
        return EFI_ABORTED;
      }
      //
      // Add the table
      //
      if (Checksum) {
        AcpiPlatformChecksum (
          (VOID *)CurrentTable,
          CurrentTable->Length,
          OFFSET_OF (EFI_ACPI_SDT_HEADER, Checksum)
          );
      }
      Status = mAcpiTable->InstallAcpiTable (
                             mAcpiTable,
                             CurrentTable,
                             CurrentTable->Length,
                             Handle
                             );
    } else {
      //
      // Delete table
      //
      Status = mAcpiTable->UninstallAcpiTable (mAcpiTable, *Handle);
    }
  }

  if (EFI_ERROR (Status)) {
    return EFI_ABORTED;
  }

  return EFI_SUCCESS;
}

/**
  Causes one or more versions of the ACPI tables to be published in
  the EFI system configuration tables.

  The PublishTables() function installs the ACPI tables for the versions that are specified in 
  Version. No tables are published for Version equal to EFI_ACPI_VERSION_NONE. Once 
  published, tables will continue to be updated as tables are modified with 
  EFI_ACPI_SUPPORT_PROTOCOL.SetAcpiTable(). 

  @param  This                  A pointer to the EFI_ACPI_SUPPORT_PROTOCOL instance.
  @param  Version               Indicates to which version(s) of ACPI the table should be published.

  @retval EFI_SUCCESS           The function completed successfully.
  @retval EFI_ABORTED           An error occurred and the function could not complete successfully.

**/
EFI_STATUS
EFIAPI
PublishTables (
  IN EFI_ACPI_SUPPORT_PROTOCOL            *This,
  IN EFI_ACPI_TABLE_VERSION               Version
  )
{
  //
  // There is no publish concept in UEFI or PI spec for ACPI table management.
  // SetAcpiTable will already install AcpiTable or uninstall AcpiTable, so no action is needed here.
  //
  return EFI_SUCCESS;
}

/**
  The user Entry Point for module AcpiSupportOnAcpiTableAndAcpiSdtThunk.
  The user code starts with this function.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.  
  @param[in] SystemTable    A pointer to the EFI System Table.
  
  @retval EFI_SUCCESS       The entry point is executed successfully.

**/
EFI_STATUS
EFIAPI
AcpiSupportOnAcpiTableAndAcpiSdtThunkInitialize (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;
  
  //
  // Make sure the AcpiSupport Protocol is not already installed in the system
  //
  ASSERT_PROTOCOL_ALREADY_INSTALLED (NULL, &gEfiAcpiSupportProtocolGuid);

  //
  // Locate and cache UEFI AcpiTable Protocol.
  //
  Status = gBS->LocateProtocol (
                  &gEfiAcpiTableProtocolGuid, 
                  NULL, 
                  (VOID **) &mAcpiTable
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // Locate and cache PI AcpiSdt Protocol.
  //
  Status = gBS->LocateProtocol (
                  &gEfiAcpiSdtProtocolGuid, 
                  NULL, 
                  (VOID **) &mAcpiSdt
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // Install the protocol on a new handle.
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mAcpiSupportHandle,
                  &gEfiAcpiSupportProtocolGuid, &mAcpiSupport,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  return Status;
}

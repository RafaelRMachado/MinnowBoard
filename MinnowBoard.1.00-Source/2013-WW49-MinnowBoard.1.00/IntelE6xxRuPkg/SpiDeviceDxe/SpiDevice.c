/*++
  This file contains an 'Intel Peripheral Driver' and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/** @file

Copyright (c) 2011 - 2012, Intel Corporation. All rights reserved.<BR>
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

**/

#include <Protocol/Spi.h>
#include <Protocol/SpiDevice.h>

#include "SpiDevice.h"

EFI_SPI_PROTOCOL  *mSpiProtocol;

UINTN mNvStorageBase = 0;

EFI_STATUS
EFIAPI
SpiRead (
  IN      UINTN       SpiOffset,
  IN OUT  UINTN       *Size,
  OUT     UINT8       *Buffer
  )
{
  EFI_STATUS  Status;
  VOID        *BiosMmioAddress;

  //
  // Validate parameters.
  //
  if (Size == NULL || Buffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  if (SpiOffset + *Size > PcdGet32 (PcdFlashAreaSize)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Check to see if the read is taking place in a memory mapped part of the flash.
  // Some flash regions may not be mapped for runtime access by the OS and must
  // be accessed through the controller and not MMIO.
  //
  if (ReadUsingMmio (SpiOffset)) {
    //
    // Convert BIOS region offset into an actual memory address.
    //
    BiosMmioAddress = (VOID*) (SpiOffset + PcdGet32 (PcdFlashAreaBase));

    //
    // Do memory copy instead of using SPI controller.
    //
    CopyMem ((VOID*) Buffer, BiosMmioAddress, *Size);
  } else if ((SpiOffset >= VN_STORAGE_REGION_FLASH_OFFSET) && (SpiOffset < (VN_STORAGE_REGION_FLASH_OFFSET + PcdGet32 (PcdFlashAreaSize)))) {
    //
    // Convert the offset into a memory address into the NV Storage region.  At
    // runtime this is the only region of the flash that is mapped for runtime
    // access.  Prior to runtime the preceding case will cover MMIO flash access.
    //
    BiosMmioAddress = (VOID*) ((SpiOffset - VN_STORAGE_REGION_FLASH_OFFSET) + mNvStorageBase);

    //
    // Do memory copy instead of using SPI controller.
    //
    CopyMem ((VOID*) Buffer, BiosMmioAddress, *Size);
  } else {
    //
    // Perform flash read.
    //
    Status = mSpiProtocol->Execute (
      mSpiProtocol,
      SPI_DEVICE_READ_STATUS_CMD_INDEX,
      SPI_DEVICE_READ_CMD_INDEX,
      0,
      TRUE,
      FALSE,
      FALSE,
      SpiOffset,
      (UINT32) *Size,
      Buffer,
      EnumSpiRegionAll
      );
    if (EFI_ERROR(Status)) {
      DEBUG((EFI_D_ERROR, "Failed to read SPI region.\n"));
      return Status;
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SpiWrite (
  IN      UINTN       SpiOffset,
  IN OUT  UINTN       *Size,
  IN      UINT8       *Buffer
  )
{
  EFI_STATUS  Status;

  //
  // Validate the input parameters
  //
  if (Size == NULL || Buffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  if (SpiOffset + *Size > PcdGet32 (PcdFlashAreaSize)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Perform the flash write.
  //
  mSpiProtocol->EnableWrite (mSpiProtocol, TRUE);
  Status = mSpiProtocol->Execute (
    mSpiProtocol,
    SPI_DEVICE_READ_STATUS_CMD_INDEX,
    SPI_DEVICE_WRITE_CMD_INDEX,
    SPI_DEVICE_PREFIX_OP_WRITE_EN_INDEX,
    TRUE,
    TRUE,
    TRUE,
    SpiOffset,
    (UINT32) *Size,
    Buffer,
    EnumSpiRegionAll
    );
  mSpiProtocol->EnableWrite (mSpiProtocol, FALSE);
  if (EFI_ERROR (Status)) {
    DEBUG((EFI_D_ERROR, "Failed to write SPI region.\n"));
  }

  return Status;
}

EFI_STATUS
EFIAPI
SpiErase (
  IN      UINTN       SpiOffset,
  IN OUT  UINTN       Size
  )
{
  EFI_STATUS  Status;
  UINTN       RegionOffset;
  UINTN       EraseSize;
  UINTN       BytesRemaining;
  UINT8       EraseOpCodeIndex;

  //
  // Validate the input parameters
  //
  Status = EFI_INVALID_PARAMETER;
  if (SpiOffset + Size > PcdGet32 (PcdFlashAreaSize)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Force the minimal alignment of 4k.
  //
  BytesRemaining = Size;
  RegionOffset = SpiOffset;
  if (RegionOffset & (SIZE_4KB - 1)) {
    DEBUG((EFI_D_INFO, "Forcing SPI Device Erase alignment to a 4k base.\n"));
    BytesRemaining += (RegionOffset & (SIZE_4KB - 1));
    RegionOffset = RegionOffset & (SIZE_4KB - 1);
  }

  //
  // Perform as many erase operations as needed to erase requested region.
  //
  while (BytesRemaining > 0) {
    //
    // Determine the erase size (4k or 64k)
    //
    EraseSize = SIZE_4KB;
    EraseOpCodeIndex = SPI_DEVICE_ERASE_4K_CMD_INDEX;
    if ((BytesRemaining >= SIZE_64KB) && ((RegionOffset & (SIZE_64KB - 1)) == 0)) {
      EraseSize = SIZE_64KB;
      EraseOpCodeIndex = SPI_DEVICE_ERASE_64K_CMD_INDEX;
    }

    //
    // Perform erase
    //
    mSpiProtocol->EnableWrite (mSpiProtocol, TRUE);
    Status = mSpiProtocol->Execute (
      mSpiProtocol,
      SPI_DEVICE_READ_STATUS_CMD_INDEX,
      EraseOpCodeIndex,
      SPI_DEVICE_PREFIX_OP_WRITE_EN_INDEX,
      FALSE,
      TRUE,
      FALSE,
      RegionOffset,
      0,
      NULL,
      EnumSpiRegionAll
      );
    mSpiProtocol->EnableWrite (mSpiProtocol, FALSE);
    if (EFI_ERROR(Status)) {
      DEBUG((EFI_D_ERROR, "Failed to erase SPI region.\n"));
      return Status;
    }

    //
    // Update the number of bytes left to erase.
    //
    BytesRemaining -= EraseSize;
  }

  return Status;
}

EFI_STATUS
EFIAPI
SpiLock (
  IN      UINTN       SpiOffset,
  IN OUT  UINTN       Size,
  IN      BOOLEAN     Lock
  )
{
  //
  // Block/Sector locking is not supported in this implementation.  Use SpiSetRange
  // and SpiLockRanges to protect areas of the flash.
  //
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
SpiSetRange (
  IN      UINTN       SpiOffset,
  IN      UINTN       Size,
  IN      BOOLEAN     ReadLock,
  IN      BOOLEAN     WriteLock
  )
{
  EFI_STATUS    Status;
  SPI_PRR_DATA  PrrData;
  UINT8         Index;

  //
  // Validate parameters with flash size.
  //
  if (SpiOffset >= PcdGet32 (PcdFlashAreaSize) || (SpiOffset + Size) > PcdGet32 (PcdFlashAreaSize)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Check parameter alignment.
  //
  if ((SpiOffset & 0xFFF) != 0 || ((SpiOffset + Size) & 0xFFF) != 0) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Get the current PRR settings.
  //
  Status = mSpiProtocol->GetPrr (mSpiProtocol, &PrrData);
  if (EFI_ERROR (Status)) {
    DEBUG((EFI_D_ERROR, "Failed to get SPI PRR information.\n"));
    return EFI_ACCESS_DENIED;
  }

  //
  // Validate that the PRRs have not been locked.
  //
  if (PrrData.FlashConfigLock) {
    return EFI_ACCESS_DENIED;
  }

  //
  // Find location of the region if it exists or the next available location to
  // add the region.  Also do overlap detection at this point.  A region is in
  // use if at least on protection is enabled.
  //
  for (Index = 0; Index < SPI_PRR_COUNT; Index++) {
    if (!PrrData.PrrEntries[Index].ReadProtect && !PrrData.PrrEntries[Index].WriteProtect) {
      //
      // An empty slot was found so region does not match.  Use this to index to
      // set the region and access policies.
      //
      break;
    } else if (SpiOffset == PrrData.PrrEntries[Index].RangeBase && (SpiOffset + Size - 1) == PrrData.PrrEntries[Index].RangeLimit) {
      //
      // Just updating an existing range.
      //
      break;
    } else if (SpiOffset <= PrrData.PrrEntries[Index].RangeBase && (SpiOffset + Size - 1) >= PrrData.PrrEntries[Index].RangeBase) {
      //
      // Overlap on base.
      //
      return EFI_INVALID_PARAMETER;
    } else if (SpiOffset <= PrrData.PrrEntries[Index].RangeLimit && (SpiOffset + Size - 1) >= PrrData.PrrEntries[Index].RangeLimit) {
      //
      // Overlap on limit.
      //
      return EFI_INVALID_PARAMETER;
    } else if (SpiOffset > PrrData.PrrEntries[Index].RangeBase && (SpiOffset + Size) < PrrData.PrrEntries[Index].RangeLimit) {
      //
      // Inclusive overlap.
      //
      return EFI_INVALID_PARAMETER;
    }
  }

  //
  // Set the region and access policies.
  //
  PrrData.PrrEntries[Index].RangeBase = (UINT32) SpiOffset;
  PrrData.PrrEntries[Index].RangeLimit = (UINT32) (SpiOffset + Size - 1);
  PrrData.PrrEntries[Index].ReadProtect = ReadLock;
  PrrData.PrrEntries[Index].WriteProtect = WriteLock;

  //
  // Update PRR values in SPI controller.
  //
  Status = mSpiProtocol->SetPrr (mSpiProtocol, &PrrData);

  return Status;
}

EFI_STATUS
EFIAPI
SpiLockRanges (
  )
{
  //
  // Call lock on the SPI interface.  This will lock down further configuration
  // changes in the SPI controller.
  //
  return mSpiProtocol->Lock (mSpiProtocol);
}

/**
  Get the JEDED ID from the SPI flash part.

  @param  Context       Pointer to a context data structure
                        needed by the SPI controller driver
  @param  Description   Description of the flash device
  @param  BufferLength  Length of the JedecId buffer
  @param  JedecId       Pointer to a buffer to fill with
                        the JEDEC ID value

  @return  EFI_SUCCESS            The JEDEC ID value is in the buffer
  @return  EFI_INVALID_PARAMETER  JedecId is NULL
  @return  EFI_INVALID_PARAMETER  Description is NULL
  @return  EFI_INVALID_PARAMETER  Too few opcode entries
  @return  EFI_INVALID_PARAMETER  JEDEC ID response buffer too small
  @return  EFI_UNSUPPORTED        JEDEC ID opcode not found

**/
EFI_STATUS
EFIAPI
JedecIdRead (
  IN       VOID                    *Context,
  IN CONST FLASH_PART_DESCRIPTION  *Description,
  IN       UINTN                   BufferLength,
  OUT      UINT8                   *JedecId
 )
{
  UINTN       Index;
  UINTN       JedecIdIndex;
  UINTN       OpcodeIndex;
  UINT32      RequiredOpcodeIndexes;
  EFI_STATUS  Status;

  //
  //  Validate parameters.
  //
  if ((JedecId == NULL)
    || (Description == NULL)
    || (Description->OpcodeTableEntries <= SPI_FLASH_PART_OPCODE_ERASE_64K_BYTE_BLOCK)
    || (BufferLength < Description->JededIdResponseLengthInBytes)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  //  Locate the JEDEC ID opcode
  //
  RequiredOpcodeIndexes =
        (1 << SPI_FLASH_PART_OPCODE_JEDEC_ID)
      | (1 << SPI_FLASH_PART_OPCODE_READ_STATUS)
      | (1 << SPI_FLASH_PART_OPCODE_WRITE_STATUS)
      | (1 << SPI_FLASH_PART_OPCODE_READ_BYTES)
      | (1 << SPI_FLASH_PART_OPCODE_WRITE_256_BYTE_PAGE)
      | (1 << SPI_FLASH_PART_OPCODE_ERASE_4K_BYTE_BLOCK)
      | (1 << SPI_FLASH_PART_OPCODE_ERASE_64K_BYTE_BLOCK);
  JedecIdIndex = 0;
  for ( Index = 0; Index < Description->OpcodeTableEntries; Index++ ) {
    OpcodeIndex = Description->OpcodeTable [Index].OpcodeIndex;
    if ( OpcodeIndex < 32 ) {
      RequiredOpcodeIndexes &= ~(1 << OpcodeIndex);
    }
    if ( OpcodeIndex == SPI_FLASH_PART_OPCODE_JEDEC_ID ) {
      JedecIdIndex = Index;
    }
  }
  if (RequiredOpcodeIndexes != 0 ) {
    return EFI_UNSUPPORTED;
  }

  //
  //  Initialize the SPI controller with the JEDEC ID opcode
  //
  Status = mSpiProtocol->LoadOpcode (
                           mSpiProtocol,
                           &Description->OpcodeTable [JedecIdIndex],
                           SPI_DEVICE_READ_JEDEC_ID_CMD_INDEX
                           );
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "ERROR - SPI failed to load JEDEC ID opcode, Status: %r\r\n", Status ));
  } else {
    //
    //  Display the operation
    //
    DEBUG ((
      DEBUG_INFO,
      "SPI JEDEC ID (0x%02x): %d bytes\r\n",
      Description->OpcodeTable [SPI_FLASH_PART_OPCODE_JEDEC_ID].Opcode,
      Description->JededIdResponseLengthInBytes
      ));

    //
    //  Read the JEDEC ID from the SPI flash part
    //
    Status = mSpiProtocol->Execute (
      mSpiProtocol,
      SPI_DEVICE_READ_STATUS_CMD_INDEX,
      SPI_DEVICE_READ_JEDEC_ID_CMD_INDEX,
      0,
      TRUE,
      FALSE,
      FALSE,
      0,
      Description->JededIdResponseLengthInBytes,
      JedecId,
      EnumSpiRegionAll
      );
    if (EFI_ERROR(Status)) {
      DEBUG((EFI_D_ERROR, "Failed to read JEDEC ID, Status: %r\r\n", Status));
    }
  }

  //
  //  Return the operation status
  //
  return Status;
}

/**
  Load the specified opcode into the SPI controller

  @param  OpcodeIndex            Index of the opcode
  @param  ControllerOpcodeIndex  Index into the controller's opcode table
  @param  FlashDescription       Description of the flash device

  @return  EFI_SUCCESS           The opcode was successfully loaded
  @return  EFI_UNSUPPORTED       The opcode was not found

**/
EFI_STATUS
LoadOpcode (
  IN       UINTN                   OpcodeIndex,
  IN       UINTN                   ControllerOpcodeIndex,
  IN CONST FLASH_PART_DESCRIPTION  *FlashDescription
 )
{
  UINTN       Index;

  //
  //  Locate the opcode
  //
  for (Index = 0; Index < FlashDescription->OpcodeTableEntries; Index++) {
    if (OpcodeIndex == FlashDescription->OpcodeTable [Index].OpcodeIndex) {
      return mSpiProtocol->LoadOpcode (
                             mSpiProtocol,
                             &FlashDescription->OpcodeTable [Index],
                             ControllerOpcodeIndex );
    }
  }

  //
  //  The opcode was not found
  //
  DEBUG ((EFI_D_ERROR, "ERROR - Required opcode mask did not include opcode index %d\r\n", OpcodeIndex ));
  return EFI_UNSUPPORTED;
}


/**
  Determine the flash size and description

  @param  PerformJedecIdOperation  Callback routine to initiate
                                   the JEDEC ID operation using
                                   the SPI controller to identify
                                   the flash part.
  @param  Context                  Pointer to a context structure to pass
                                   to PerformJedecIdOperation
  @param  FlashDescription         Pointer to a buffer to receive a
                                   pointer to a FLASH_PART_DESCRIPTION
                                   data structure containing the flash
                                   part information.

  @return  This routine returns the size of the flash part if it is
           supported.  Zero is returned if the flash part is not
           supported.

**/
UINT64
EFIAPI
FindFlashSupport (
  IN        PERFORM_JEDEC_ID_OPERATION  PerformJedecIdOperation,
  IN        VOID                        *Context,
  OUT CONST FLASH_PART_DESCRIPTION      **FlashDescription
 )
{
  UINTN                         BufferLength;
  CONST FLASH_PART_DESCRIPTION  *Description;
  UINT64                        FlashSize;
  EFI_HANDLE                    *HandleArray;
  UINTN                         HandleCount;
  UINTN                         HandleIndex;
  UINT8                         *JedecId;
  UINT32                        MaxPriority;
  UINT32                        Priority;
  SPI_FLASH_PART_PROTOCOL       * Protocol;
  SPI_FLASH_PART_PROTOCOL       **SpiFlashPartProtocol;
  EFI_STATUS                    Status;

  //
  //  Assume failure
  //
  FlashSize = 0;
  HandleArray = NULL;
  JedecId = NULL;
  SpiFlashPartProtocol = NULL;

  //
  //  Locate handles containing SPI_FLASH_PART_PROTOCOLS
  //
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gSpiFlashPartProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleArray
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "ERROR - Failed to locate SPI_FLASH_PART_PROTOCOL, Status: %r\r\n", Status));
  } else {
    //
    //  Allocate and fill in the protocol array
    //
    DEBUG ((DEBUG_INFO, "%d SPI flash part descriptions found\r\n", HandleCount));
    SpiFlashPartProtocol = AllocatePool (HandleCount * sizeof (*SpiFlashPartProtocol));
    if (SpiFlashPartProtocol == NULL) {
      DEBUG ((DEBUG_ERROR, "ERROR - Failed to allocate SpiFlashDataProtocol buffer\r\n"));
    } else {
      for (HandleIndex = 0; HandleCount > HandleIndex; HandleIndex++) {
        Status = gBS->OpenProtocol (
                        HandleArray [HandleIndex],
                        &gSpiFlashPartProtocolGuid,
                        (VOID **)&SpiFlashPartProtocol [HandleIndex],
                        NULL,
                        NULL,
                        EFI_OPEN_PROTOCOL_GET_PROTOCOL
                        );
        if (EFI_ERROR (Status)) {
          DEBUG ((DEBUG_ERROR, "ERROR - Failed to open SPI_FLASH_DATA_PROTOCOL, Status: %r\r\n", Status));
          break;
        }
      }
      if (!EFI_ERROR (Status)) {
        //
        //  Allocate the JEDEC ID buffer
        //
        BufferLength = 0;
        for (HandleIndex = 0; HandleCount > HandleIndex; HandleIndex++) {
          //
          //  Get the JEDEC ID opcode description
          //
          Protocol = SpiFlashPartProtocol [HandleIndex];
          Description = Protocol->GetFlashDescription (
                                    Protocol,
                                    NULL,
                                    NULL
                                    );
          if (BufferLength < Description->JededIdResponseLengthInBytes) {
            BufferLength = Description->JededIdResponseLengthInBytes;
          }
        }
        JedecId = AllocatePool (BufferLength);
        if (JedecId == NULL) {
          DEBUG ((DEBUG_ERROR, "ERROR - Failed to allocate JedecId buffer\r\n"));
        } else {
          //
          //  Start with the first flash type description;
          //
          MaxPriority = 0xffffffff;
          do {
            //
            //  Determine the highest priority protocol
            //
            Priority = 0;
            for (HandleIndex = 0; HandleCount > HandleIndex; HandleIndex++) {
              Protocol = SpiFlashPartProtocol [HandleIndex];
              if ((MaxPriority >= Protocol->Priority)
                && (Priority < Protocol->Priority))
                Priority = Protocol->Priority;
            }
            if (Priority == 0) {
              //
              //  The flash is not supported
              //
              break;
            }

            //
            //  Walk the handles containing the SPI flash part protocol
            //
            HandleIndex = 0;
            do {
              //
              //  Verify the description type matches and the opcode table
              //  supports the minimum number of entries required for the code
              //
              Protocol = SpiFlashPartProtocol [HandleIndex];
              if (Priority == Protocol->Priority) {
                //
                //  Get the JEDEC ID opcode description
                //
                Description = Protocol->GetFlashDescription (
                                          Protocol,
                                          NULL,
                                          NULL
                                          );
                if ((Description == NULL)
                  || (SPI_FLASH_PART_OPCODE_JEDEC_ID == Description->OpcodeTableEntries)) {
                  DEBUG ((DEBUG_ERROR, "ERROR - JEDEC ID opcode not available\r\n"));
                } else {
                  //
                  //  Display the flash part
                  //
                  DEBUG ((DEBUG_INFO, "Priority: 0x%08x, SPI Flash Part: %s\r\n", Priority, Description->PartNumber ));

                  //
                  //  Attempt to read the JEDEC ID
                  //
                  Status = PerformJedecIdOperation (
                             Context,
                             Description,
                             Description->JededIdResponseLengthInBytes,
                             JedecId
                             );
                  if (!EFI_ERROR (Status)) {
                    //
                    //  Display the JEDEC ID
                    //
                    DEBUG_CODE_BEGIN ();
                    {
                      UINTN Index;

                      DEBUG ((DEBUG_INFO, "JEDEC ID:"));
                      for (Index = 0; Description->JededIdResponseLengthInBytes > Index; Index++) {
                        DEBUG ((DEBUG_INFO, " 0x%02x", JedecId [Index]));
                      }
                      DEBUG ((DEBUG_INFO, "\r\n"));
                    }
                    DEBUG_CODE_END ();

                    //
                    //  Verify support and determine flash size
                    //
                    Description = Protocol->GetFlashDescription (
                                              Protocol,
                                              JedecId,
                                              &FlashSize
                                              );
                    if (Description != NULL) {
                      //
                      //  The flash device is supported
                      //  Return the table for this flash device
                      //
                      DEBUG ((DEBUG_INFO, "SPI flash device found: %s\r\n", Description->PartNumber));
                      *FlashDescription = Description;
                      goto PartFound;
                    }
                  }
                }
              }

              //
              //  Set next handle
              //
              HandleIndex += 1;
            } while (HandleCount > HandleIndex);

            //
            //  Set the next priority
            //
            MaxPriority = Priority - 1;
          } while (Priority != 0);

          //
          //  No flash device found
          //
          DEBUG ((DEBUG_ERROR, "Matching SPI flash description not found\r\n"));
        }
      }
    }
  }

PartFound:
  //
  //  Free the buffers
  //
  if (JedecId != NULL) {
    FreePool (JedecId);
  }
  if (SpiFlashPartProtocol != NULL) {
    FreePool (SpiFlashPartProtocol);
  }
  if (HandleArray != NULL) {
    FreePool (HandleArray);
  }

  //
  //  Return the flash size
  //  Zero (0) indicates flash not found or not supported
  //
  return FlashSize;
}

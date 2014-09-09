//
// This file contains an 'Intel Peripheral Driver' and is
// licensed for Intel CPUs and chipsets under the terms of your
// license agreement with Intel or your vendor.  This file may
// be modified by the user, subject to additional terms of the
// license agreement
//
/** @file

Copyright (c) 2010 - 2012, Intel Corporation. All rights reserved.<BR>
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

**/

#ifndef _SPI_DEVICE_H_
#define _SPI_DEVICE_H_

#include <Protocol/Spi.h>
#include <Protocol/SpiFlashPart.h>

#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiRuntimeLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>

//
// Prefix opcodes allowed on the SPI interface
//
#define SPI_DEVICE_PREFIX_OP_WRITE_EN_INDEX     0
#define SPI_DEVICE_PREFIX_OP_WRITE_S_EN_INDEX   1

//
// Reserved index values for the SPI controller
//
#define SPI_DEVICE_READ_CMD_INDEX           0
#define SPI_DEVICE_ERASE_64K_CMD_INDEX      1
#define SPI_DEVICE_ERASE_4K_CMD_INDEX       2
#define SPI_DEVICE_WRITE_CMD_INDEX          3
#define SPI_DEVICE_READ_STATUS_CMD_INDEX    4
#define SPI_DEVICE_WRITE_STATUS_CMD_INDEX   5

#define SPI_DEVICE_NUM_FIXED_OPCODES        6

#define SPI_DEVICE_READ_JEDEC_ID_CMD_INDEX  6

//
// Defines the offset in the SPI device where the BIOS region starts.
//
#define BIOS_REGION_FLASH_OFFSET        (PcdGet32 (PcdBiosImageBase) - PcdGet32 (PcdFlashAreaBase))
#define VN_STORAGE_REGION_FLASH_OFFSET  (PcdGet32 (PcdFlashNvStorageBase) - PcdGet32 (PcdFlashAreaBase))

extern EFI_SPI_PROTOCOL  *mSpiProtocol;
extern UINTN mNvStorageBase;

EFI_STATUS
EFIAPI
InitSpiDevice (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
EFIAPI
SpiRead (
  IN      UINTN       SpiOffset,
  IN OUT  UINTN       *Size,
  OUT     UINT8       *Buffer
  );

EFI_STATUS
EFIAPI
SpiWrite (
  IN      UINTN       SpiOffset,
  IN OUT  UINTN       *Size,
  IN      UINT8       *Buffer
  );

EFI_STATUS
EFIAPI
SpiErase (
  IN      UINTN       SpiOffset,
  IN OUT  UINTN       Size
  );

EFI_STATUS
EFIAPI
SpiLock (
  IN      UINTN       SpiOffset,
  IN OUT  UINTN       Size,
  IN      BOOLEAN     Lock
  );

EFI_STATUS
EFIAPI
SpiSetRange (
  IN      UINTN       SpiOffset,
  IN      UINTN       Size,
  IN      BOOLEAN     ReadLock,
  IN      BOOLEAN     WriteLock
  );

EFI_STATUS
EFIAPI
SpiLockRanges (
  );

BOOLEAN
ReadUsingMmio (
  IN UINTN  SpiOffset
  );

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
 );

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
 );

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

**/
typedef
EFI_STATUS
(EFIAPI  *PERFORM_JEDEC_ID_OPERATION) (
  IN       VOID                    *Context,
  IN CONST FLASH_PART_DESCRIPTION  *Description,
  IN       UINTN                   BufferLength,
  OUT      UINT8                   *JedecId
  );
 
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
  );
  
#endif

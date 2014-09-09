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
#ifndef _EFI_SPI_DEVICE_H_
#define _EFI_SPI_DEVICE_H_

//
// GUID definition for protocol interface
//
#define SPI_DEVICE_PROTOCOL_GUID \
  {0x37DCF59A, 0x944A, 0x11DF, {0x97, 0xD4, 0xE3, 0xAA, 0xC9, 0x24, 0x56, 0x53}}

extern EFI_GUID gSpiDeviceProtocolGuid;

//
// Converts a flash memory mapped address into a flash linear address.
//
#define FLASH_LINEAR_ADDRESS(Base)   ((Base) - PcdGet32 (PcdFlashAreaBase))

/**
  Performs a read from the flash device.

  @param[in]      SpiOffset Offset into the flash device.
  @param[in,out]  Size      Size of the region to be read on input and the number
                            of bytes read on exit.
  @param[out]     Buffer    A pointer to a caller allocated buffer large enough to hold
                            the resulting transaction.
  @retval EFI_SUCCESS             The transaction completed.
  @retval EFI_INVALID_PARAMETER   One or more parameters are invalid.
  @retval EFI_DEVICE_ERROR        Unable to read data due to device state.
**/
typedef
EFI_STATUS
(EFIAPI* SPI_DEVICE_READ) (
  IN      UINTN       SpiOffset,
  IN OUT  UINTN       *Size,
  OUT     UINT8       *Buffer
  );

/**
  Performs a write to the flash device.

  @param[in]      SpiOffset Offset into the flash device.
  @param[in,out]  Size      Size of the region to be written to on input and the number
                            of bytes written on exit.
  @param[out]     Buffer    A pointer to a caller allocated buffer that contains the
                            data to be written.
  @retval EFI_SUCCESS             The transaction completed.
  @retval EFI_INVALID_PARAMETER   One or more parameters are invalid.
  @retval EFI_DEVICE_ERROR        Unable to write data due to device state.
**/
typedef
EFI_STATUS
(EFIAPI* SPI_DEVICE_WRITE) (
  IN      UINTN       SpiOffset,
  IN OUT  UINTN       *Size,
  IN      UINT8       *Buffer
  );

/**
  Performs an erase operation to the flash device.

  @param[in]      SpiOffset Offset into the flash device.
  @param[in,out]  Size      Size of the region to be erased on input and the number
                            of bytes erased on exit.
  @retval EFI_SUCCESS             The transaction completed.
  @retval EFI_INVALID_PARAMETER   One or more parameters are invalid.
  @retval EFI_DEVICE_ERROR        Unable to erase data due to device state.
**/
typedef
EFI_STATUS
(EFIAPI* SPI_DEVICE_ERASE) (
  IN      UINTN       SpiOffset,
  IN OUT  UINTN       Size
  );

/**
  Locks one or more blocks of the flash device.

  @param[in]      SpiOffset Offset into the flash device.
  @param[in,out]  Size      Size of the region to be locked/unloced on input and the number
                            of bytes locked/unlocked on exit.
  @param[in]      Lock      Selects the Lock or Unlock operation.

  @retval EFI_SUCCESS             The transaction completed.
  @retval EFI_INVALID_PARAMETER   One or more parameters are invalid.
  @retval EFI_DEVICE_ERROR        Unable to erase data due to device state.
  @retval EFI_UNSUPPORTED         Feature is not supported at this time.  All
                                  blocks are to remain unlocked.
**/
typedef
EFI_STATUS
(EFIAPI* SPI_DEVICE_LOCK) (
  IN      UINTN       SpiOffset,
  IN OUT  UINTN       Size,
  IN      BOOLEAN     Lock
  );

/**
  Sets a range to be protected by the flash controller.  This will either add
  a new range or update an existing range.  Overlapping regions are not supported.

  @param[in]  SpiOffset Defines the base address of the range as an SPI offset.
  @param[in]  Size      Defines the size of the range.
  @param[in]  ReadLock  Defines the policy for reads to the specified range.
  @param[in]  WriteLock Defines the policy for writes to the specified range.

  @retval EFI_SUCCESS           Range added successfully.
  @retval EFI_ACCESS_DENIED     Unable to add range due to configuration lock-down.
  @retval EFI_OUT_OF_RESOURCES  Unable to complete request with current PRRs available.
  @retval EFI_INVALID_PARAMETER An invalid flash mapping was provided or the region
                                overlaps a region already set.
**/
typedef
EFI_STATUS
(EFIAPI *SPI_DEVICE_SET_RANGE) (
  IN      UINTN       SpiOffset,
  IN      UINTN       Size,
  IN      BOOLEAN     ReadLock,
  IN      BOOLEAN     WriteLock
  );

/**
  Locks out all further changes to protected ranges.

  @retval EFI_SUCCESS       Lock completed.
  @retval EFI_ACCESS_DENIED Already locked.
  @retval EFI_DEVICE_ERROR  Unable to lock.
**/
typedef
EFI_STATUS
(EFIAPI *SPI_DEVICE_LOCK_RANGES) (
  );

typedef struct {
  SPI_DEVICE_READ         SpiRead;
  SPI_DEVICE_WRITE        SpiWrite;
  SPI_DEVICE_ERASE        SpiErase;
  SPI_DEVICE_LOCK         SpiLock;
  SPI_DEVICE_SET_RANGE    SpiSetRange;
  SPI_DEVICE_LOCK_RANGES  SpiLockRanges;
} SPI_DEVICE_PROTOCOL;

#endif

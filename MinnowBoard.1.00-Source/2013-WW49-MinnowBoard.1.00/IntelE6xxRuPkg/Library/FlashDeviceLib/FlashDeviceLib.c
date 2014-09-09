/*++
  This file contains an 'Intel Peripheral Driver' and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/** @file
  Libray implementation for Flash Device Library based on Multiple Flash Support
  library intance.

  Copyright (c) 2006 - 2013, Intel Corporation. All rights reserved.<BR>
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by such
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.

**/


#include <PiDxe.h>

#include <Library/DebugLib.h>
#include <Library/FlashDeviceLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>


#include <Protocol/SpiDevice.h>

SPI_DEVICE_PROTOCOL *mSpiDeviceProtocol = NULL;

STATIC
UINTN
AdjustAddress (
  IN  UINTN   Address
  )
{
  return (Address - PcdGet32 (PcdFlashAreaBase));
}

/**
  Read NumBytes bytes of data from the address specified by
  PAddress into Buffer.

  @param[in]      PAddress      The starting physical address of the read.
  @param[in,out]  NumBytes      On input, the number of bytes to read. On output, the number
                                of bytes actually read.
  @param[out]     Buffer        The destination data buffer for the read.

  @retval         EFI_SUCCESS.      Opertion is successful.
  @retval         EFI_DEVICE_ERROR  If there is any device errors.  
  
**/
EFI_STATUS
EFIAPI
LibFvbFlashDeviceRead (
  IN      UINTN                           PAddress,
  IN  OUT UINTN                           *NumBytes,
      OUT UINT8                           *Buffer
  )
{
  ASSERT (mSpiDeviceProtocol != NULL);
  return mSpiDeviceProtocol->SpiRead (AdjustAddress (PAddress), NumBytes, Buffer);
}



/**
  Write NumBytes bytes of data from Buffer to the address specified by
  PAddresss.

  @param[in]      PAddress        The starting physical address of the write.
  @param[in,out]  NumBytes        On input, the number of bytes to write. On output,
                                  the actual number of bytes written.
  @param[in]      Buffer          The source data buffer for the write.

  @retval         EFI_SUCCESS.      Opertion is successful.
  @retval         EFI_DEVICE_ERROR  If there is any device errors.  

**/
EFI_STATUS 
EFIAPI
LibFvbFlashDeviceWrite (
  IN        UINTN                           PAddress,
  IN OUT    UINTN                           *NumBytes,
  IN        UINT8                           *Buffer
  )
{
  ASSERT (mSpiDeviceProtocol != NULL);
  return mSpiDeviceProtocol->SpiWrite (AdjustAddress (PAddress), NumBytes, Buffer);
}



/**
  Erase the block staring at PAddress.

  @param[in]  PAddress        The starting physical address of the block to be erased.
                              This library assume that caller garantee that the PAddress
                              is at the starting address of this block.
  @param[in]  LbaLength       The length of the logical block to be erased.
  
  @retval     EFI_SUCCESS.      Opertion is successful.
  @retval     EFI_DEVICE_ERROR  If there is any device errors.  

**/
EFI_STATUS 
EFIAPI
LibFvbFlashDeviceBlockErase (
  IN    UINTN                     PAddress,
  IN    UINTN                     LbaLength
  )
{
  ASSERT (mSpiDeviceProtocol != NULL);
  return mSpiDeviceProtocol->SpiErase (AdjustAddress (PAddress), LbaLength);
}



/**
  Lock or unlock the block staring at PAddress.

  @param[in]  PAddress        The starting physical address of region to be (un)locked.
  @param[in]  LbaLength       The length of the logical block to be erased.
  @param[in]  Lock            TRUE to lock. FALSE to unlock.

  @retval  EFI_SUCCESS.       Opertion is successful.
  @retval  EFI_DEVICE_ERROR   If there is any device errors.  
  @retval  EFI_UNSUPPORTED    Locking is not supported

**/
EFI_STATUS
EFIAPI
LibFvbFlashDeviceBlockLock (
  IN    UINTN                          PAddress,
  IN    UINTN                          LbaLength,
  IN    BOOLEAN                        Lock
  )
{
  ASSERT (mSpiDeviceProtocol != NULL);
  return mSpiDeviceProtocol->SpiLock (AdjustAddress (PAddress), LbaLength, Lock);
}


EFI_STATUS
EFIAPI
InitializeFlashDeviceLib (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS                            Status;

  //
  // Locate required protocol for access to flash.
  //
  Status = gBS->LocateProtocol (
    &gSpiDeviceProtocolGuid,
    NULL,
    (VOID**) &mSpiDeviceProtocol
    );
  ASSERT_EFI_ERROR(Status);

  return Status;
}


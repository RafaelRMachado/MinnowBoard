/*++
  This file contains an 'Intel Peripheral Driver' and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/** @file

Copyright (c) 2010 - 2013, Intel Corporation. All rights reserved.<BR>
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

**/

#include <PiSmm.h>

#include <Protocol/SmmSpi.h>
#include <Protocol/SmmSpiDevice.h>

#include <Library/BaseLib.h>
#include <Library/SmmServicesTableLib.h>

#include "SpiDevice.h"
#include "SpiDeviceSmmComm.h"

SPI_DEVICE_PROTOCOL mSpiDevProtocol = {
  SpiRead,
  SpiWrite,
  SpiErase,
  SpiLock,
  SpiSetRange,
  SpiLockRanges
};

EFI_STATUS
EFIAPI
SpiDeviceSmmHandler (
  IN       EFI_HANDLE                 DispatchHandle,
  IN CONST VOID                       *RegisterContext,
  IN OUT   VOID                       *CommBuffer,
  IN OUT   UINTN                      *CommBufferSize
  )
{
  EFI_STATUS                                Status;
  SMM_SPI_DEV_COMMUNICATE_FUNCTION_HEADER   *SpiDevCommHeader;
  SMM_SPI_DEV_READ_WRITE_ERASE_HEADER       *SpiDevDataOpHeader;
  SMM_SPI_DEV_LOCK_HEADER                   *SpiDevLockHeader;
  SMM_SPI_DEV_SET_RANGE_HEADER              *SpiDevSetRangeHeader;

  ASSERT (CommBuffer != NULL);

  SpiDevCommHeader = (SMM_SPI_DEV_COMMUNICATE_FUNCTION_HEADER*) CommBuffer;
  switch (SpiDevCommHeader->Function) {
    case SPI_DEV_FUNCTION_READ:
      SpiDevDataOpHeader = (SMM_SPI_DEV_READ_WRITE_ERASE_HEADER*) SpiDevCommHeader->Data;
      Status = SpiRead (
        SpiDevDataOpHeader->Offset,
        &SpiDevDataOpHeader->Size,
        (UINT8*) (SpiDevDataOpHeader + 1)
        );
      break;
    case SPI_DEV_FUNCTION_WRITE:
      SpiDevDataOpHeader = (SMM_SPI_DEV_READ_WRITE_ERASE_HEADER*) SpiDevCommHeader->Data;
      Status = SpiWrite (
        SpiDevDataOpHeader->Offset,
        &SpiDevDataOpHeader->Size,
        (UINT8*) (SpiDevDataOpHeader + 1)
        );
      break;
    case SPI_DEV_FUNCTION_ERASE:
      SpiDevDataOpHeader = (SMM_SPI_DEV_READ_WRITE_ERASE_HEADER*) SpiDevCommHeader->Data;
      Status = SpiErase (
        SpiDevDataOpHeader->Offset,
        SpiDevDataOpHeader->Size
        );
      break;
    case SPI_DEV_FUNCTION_LOCK:
      SpiDevLockHeader = (SMM_SPI_DEV_LOCK_HEADER*) SpiDevCommHeader->Data;
      Status = SpiLock (
        SpiDevLockHeader->Offset,
        SpiDevLockHeader->Size,
        SpiDevLockHeader->Lock
        );
      break;
    case SPI_DEV_FUNCTION_SET_RANGE:
      SpiDevSetRangeHeader = (SMM_SPI_DEV_SET_RANGE_HEADER*) SpiDevCommHeader->Data;
      Status = SpiSetRange (
        SpiDevSetRangeHeader->Offset,
        SpiDevSetRangeHeader->Size,
        SpiDevSetRangeHeader->ReadLock,
        SpiDevSetRangeHeader->WriteLock
        );
      break;
    case SPI_DEV_FUNCTION_LOCK_RANGES:
      Status = SpiLockRanges ();
      break;
    default:
      ASSERT (FALSE);
      Status = EFI_UNSUPPORTED;
      break;
  }

  //
  // Set the return value.
  //
  SpiDevCommHeader->ReturnStatus = Status;

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
InitSpiDevice (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  UINT8                         Data;
  CONST FLASH_PART_DESCRIPTION  *FlashDescription;
  UINT64                        FlashSize;
  EFI_HANDLE                    Handle;
  UINTN                         Index;
  UINT8                         Prefix [SPI_NUM_PREFIX_OPCODE];
  EFI_STATUS                    Status;
  CONST UINT8                   OpcodeMap [] = {
    SPI_DEVICE_READ_JEDEC_ID_CMD_INDEX, //  SPI_FLASH_PART_OPCODE_JEDEC_ID
    SPI_DEVICE_READ_STATUS_CMD_INDEX,   //  SPI_FLASH_PART_OPCODE_READ_STATUS
    SPI_DEVICE_WRITE_STATUS_CMD_INDEX,  //  SPI_FLASH_PART_OPCODE_WRITE_STATUS
    SPI_DEVICE_READ_CMD_INDEX,          //  SPI_FLASH_PART_OPCODE_READ_BYTES
    SPI_DEVICE_WRITE_CMD_INDEX,         //  SPI_FLASH_PART_OPCODE_WRITE_256_BYTE_PAGE
    SPI_DEVICE_ERASE_4K_CMD_INDEX,      //  SPI_FLASH_PART_OPCODE_ERASE_4K_BYTE_BLOCK
    SPI_DEVICE_ERASE_64K_CMD_INDEX      //  SPI_FLASH_PART_OPCODE_ERASE_64K_BYTE_BLOCK
  };

  //--------------------------------------------------
  //
  //  Note only this routine is able to make calls
  //  into the DXE environment since it is called
  //  synchronously from that environment and DXE
  //  is still executing in physical mode.
  //
  //--------------------------------------------------

  mNvStorageBase = PcdGet32 (PcdFlashNvStorageBase);

  //
  // Locate the SPI controller protocol and save it for later.
  //
  DEBUG((EFI_D_INFO, "Locating SPI Controller Protocol.\n"));
  Status = gSmst->SmmLocateProtocol (
    &gEfiSmmSpiProtocolGuid,
    NULL,
    (VOID**) &mSpiProtocol
    );
  ASSERT_EFI_ERROR(Status);
  
  //
  // Loop through all the flash devices that are supported and see if one will
  // initialize the SPI Controller interface.
  //
  FlashSize = FindFlashSupport (
                &JedecIdRead,
                NULL,
                &FlashDescription
                );
  if (FlashSize == 0) {
    DEBUG((EFI_D_ERROR, "No SPI flash part description found!\r\n"));
  } else {
    //
    // Attempt to configure the SPI controller for this device.
    //
    DEBUG((EFI_D_INFO, "SPI flash size: %d MBytes\n", DivU64x32(FlashSize, 1024 * 1024 )));
    DEBUG((EFI_D_INFO, "Configuring SPI Controller.\n"));
    for ( Index = 0; Index < (sizeof (OpcodeMap) / sizeof (OpcodeMap [0])); Index++ ) {
      Status = LoadOpcode (
                 Index,
                 OpcodeMap [Index],
                 FlashDescription
                 );
      if ( EFI_ERROR (Status)) {
        return Status;
      }
    }
    Prefix [SPI_DEVICE_PREFIX_OP_WRITE_EN_INDEX] = FlashDescription->WriteEnable;
    Prefix [SPI_DEVICE_PREFIX_OP_WRITE_S_EN_INDEX] = FlashDescription->WriteStatusEnable;
    Status = mSpiProtocol->LoadPrefixByte (
                             mSpiProtocol,
                             Prefix
                             );
    if (EFI_ERROR(Status)) {
      DEBUG((EFI_D_ERROR, "Failed to load SPI flash prefix bytes, Status: %r\n", Status));
      return Status;
    }

    //
    // Write 0 to SPI Device Status Register 
    // (clear BP0 and BP1 (block protection bits))
    // We made an assumption that unlock can be done
    // by writting a ZERO to SPI chip's status register.
    // This is true for all SPI flash chip I have seen so far.
    //
    mSpiProtocol->EnableWrite (mSpiProtocol, TRUE);
    Data = 0;
    Status = mSpiProtocol->Execute (
      mSpiProtocol,
      SPI_DEVICE_READ_STATUS_CMD_INDEX,
      SPI_FLASH_PART_OPCODE_WRITE_STATUS,
      SPI_DEVICE_PREFIX_OP_WRITE_S_EN_INDEX,
      TRUE,
      TRUE,
      TRUE,
      0,
      sizeof (Data),
      &Data,
      EnumSpiRegionAll
      );
    mSpiProtocol->EnableWrite (mSpiProtocol, FALSE);
    if (EFI_ERROR(Status)) {
      DEBUG((EFI_D_ERROR, "Failed to write SPI flash status, Status: %r\n", Status));
      return Status;
    }

    //
    //  Complete the initialization of the SPI controller
    //
    Status = mSpiProtocol->FlashLocation (
                             mSpiProtocol,
                             PcdGet32 (PcdBiosImageBase) - PcdGet32 (PcdFlashAreaBase),
                             PcdGet32 (PcdBiosImageSize)
                             );
    if (EFI_ERROR(Status)) {
      DEBUG((EFI_D_ERROR, "Failed to set SPI flash location, Status: %r\n", Status));
      return Status;
    }

    //
    // Publish the SMM SPI Device protocol for FVB service
    //
    DEBUG((EFI_D_INFO, "Installing SPI Device Protocol.\n"));
    Handle = NULL;
    Status = gSmst->SmmInstallProtocolInterface (
      &Handle,
      &gSmmSpiDeviceProtocolGuid,
      EFI_NATIVE_INTERFACE,
      &mSpiDevProtocol
      );
    if (EFI_ERROR(Status)) {
      return EFI_OUT_OF_RESOURCES;
    }
    
    //
    // Install protocol to inform other DXE drivers the SMM service is available.
    //
    Handle = NULL;
    Status = gBS->InstallProtocolInterface (
      &Handle,
      &gSmmSpiDeviceProtocolGuid,
      EFI_NATIVE_INTERFACE,
      &mSpiDevProtocol
      );
    if (EFI_ERROR(Status)) {
      DEBUG((EFI_D_ERROR, "Unable to install SMM SPI device protocol, Status: %r\n", Status));
      return Status;
    }

    //
    // Install communication handler.
    //
    Handle = NULL;
    Status = gSmst->SmiHandlerRegister (SpiDeviceSmmHandler, &gSmmSpiDeviceProtocolGuid, &Handle);
    if (EFI_ERROR(Status)) {
      DEBUG((EFI_D_ERROR, "Unable to register SMM SPI handler, Status: %r\n", Status));
      return Status;
    }

    DEBUG((EFI_D_INFO, "SPI flash controller configured successfully\n", Status));
    return EFI_SUCCESS;
  }

  //
  // Unable to find a supported SPI device
  //
  DEBUG((EFI_D_ERROR, "Unable to configure SPI Controller for SPI device present.\n"));
  return EFI_UNSUPPORTED;
}

BOOLEAN
ReadUsingMmio (
  IN UINTN  SpiOffset
  )
{
  return (BOOLEAN) ((SpiOffset >= BIOS_REGION_FLASH_OFFSET) && (SpiOffset < (BIOS_REGION_FLASH_OFFSET + PcdGet32 (PcdBiosImageSize))));
}

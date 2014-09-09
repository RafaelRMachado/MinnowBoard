/*++
  This file contains an 'Intel Peripheral Driver' and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c) 2006 - 2011, Intel Corporation. All rights reserved.<BR>
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  FvbInfo.c

Abstract:

  Defines data structure that is the volume header found.
  These data is intent to decouple FVB driver with FV header.

--*/

#include <PiDxe.h>
#include "FvbService.h"

#define FVB_MEDIA_BLOCK_SIZE        0x10000

typedef struct {
  EFI_PHYSICAL_ADDRESS        BaseAddress;
  EFI_FIRMWARE_VOLUME_HEADER  FvbInfo;
  //
  //EFI_FV_BLOCK_MAP_ENTRY    ExtraBlockMap[n];//n=0
  //
  EFI_FV_BLOCK_MAP_ENTRY      End[1];
} EFI_FVB2_MEDIA_INFO;

//
// This data structure contains a template of all correct FV headers, which is used to restore
// Fv header if it's corrupted.
//
EFI_FVB2_MEDIA_INFO mPlatformFvbMediaInfo[] = {
  //
  // Main BIOS FVB
  //
  {
    0,
    {
      {0,}, //ZeroVector[16]
      EFI_FIRMWARE_FILE_SYSTEM2_GUID,
      0,
      EFI_FVH_SIGNATURE,
      0x0004feff, // check MdePkg/Include/Pi/PiFirmwareVolume.h for details on EFI_FVB_ATTRIBUTES_2
      sizeof (EFI_FIRMWARE_VOLUME_HEADER) + sizeof (EFI_FV_BLOCK_MAP_ENTRY),
      0,    //CheckSum, check the FD for the value.
      0,    //ExtHeaderOffset
      {0,}, //Reserved[1]
      2,    //Revision
      {
        {
          0,
          0,
        }
      }
    },
    {
      {
        0,
        0
      }
    }
  },
  //
  // Systen NvStorage FVB
  //
  {
    0,
    {
      {0,}, //ZeroVector[16]
      EFI_SYSTEM_NV_DATA_FV_GUID,
      0,
      EFI_FVH_SIGNATURE,
      0x0004feff, // check MdePkg/Include/Pi/PiFirmwareVolume.h for details on EFI_FVB_ATTRIBUTES_2
      sizeof (EFI_FIRMWARE_VOLUME_HEADER) + sizeof (EFI_FV_BLOCK_MAP_ENTRY),
      0,    //CheckSum which will be calucated dynamically.
      0,    //ExtHeaderOffset
      {0,}, //Reserved[1]
      2,    //Revision
      {
        {
          0,
          0,
        }
      }
    },
    {
      {
        0,
        0
      }
    }
  },
  //
  // Recovery BIOS FVB
  //
  {
    0,
    {
      {0,}, //ZeroVector[16]
      EFI_FIRMWARE_FILE_SYSTEM2_GUID,
      0,
      EFI_FVH_SIGNATURE,
      0x0004feff, // check MdePkg/Include/Pi/PiFirmwareVolume.h for details on EFI_FVB_ATTRIBUTES_2
      sizeof (EFI_FIRMWARE_VOLUME_HEADER) + sizeof (EFI_FV_BLOCK_MAP_ENTRY),
      0,    //CheckSum which will be calucated dynamically.
      0,    //ExtHeaderOffset
      {0,}, //Reserved[1]
      2,    //Revision
      {
        {
          0,
          0,
        }
      }
    },
    {
      {
        0,
        0
      }
    }
  }
};

EFI_STATUS
GetFvbInfo (
  IN  EFI_PHYSICAL_ADDRESS         FvBaseAddress,
  OUT EFI_FIRMWARE_VOLUME_HEADER   **FvbInfo
  )
{
  UINTN                       Index;
  EFI_FIRMWARE_VOLUME_HEADER  *FvHeader;

  //
  // Init Fvb data
  //
  mPlatformFvbMediaInfo[0].BaseAddress      = PcdGet32 (PcdFlashFvMainBase);
  mPlatformFvbMediaInfo[0].FvbInfo.FvLength = PcdGet32 (PcdFlashFvMainSize);
  mPlatformFvbMediaInfo[0].FvbInfo.BlockMap[0].NumBlocks = PcdGet32 (PcdFlashFvMainSize) / FVB_MEDIA_BLOCK_SIZE;
  mPlatformFvbMediaInfo[0].FvbInfo.BlockMap[0].Length    = FVB_MEDIA_BLOCK_SIZE;

  mPlatformFvbMediaInfo[1].BaseAddress      = PcdGet32 (PcdFlashNvStorageBase);
  mPlatformFvbMediaInfo[1].FvbInfo.FvLength = PcdGet32 (PcdFlashNvStorageSize);
  mPlatformFvbMediaInfo[1].FvbInfo.BlockMap[0].NumBlocks = PcdGet32 (PcdFlashNvStorageSize) / FVB_MEDIA_BLOCK_SIZE;
  mPlatformFvbMediaInfo[1].FvbInfo.BlockMap[0].Length    = FVB_MEDIA_BLOCK_SIZE;

  mPlatformFvbMediaInfo[2].BaseAddress      = PcdGet32 (PcdFlashFvRecoveryBase);
  mPlatformFvbMediaInfo[2].FvbInfo.FvLength = PcdGet32 (PcdFlashFvRecoverySize);
  mPlatformFvbMediaInfo[2].FvbInfo.BlockMap[0].NumBlocks = PcdGet32 (PcdFlashFvRecoverySize) / FVB_MEDIA_BLOCK_SIZE;
  mPlatformFvbMediaInfo[2].FvbInfo.BlockMap[0].Length    = FVB_MEDIA_BLOCK_SIZE;

 
  for (Index=0; Index < sizeof (mPlatformFvbMediaInfo)/sizeof (mPlatformFvbMediaInfo[0]); Index += 1) {
    if (mPlatformFvbMediaInfo[Index].BaseAddress == FvBaseAddress) {
      FvHeader =  &mPlatformFvbMediaInfo[Index].FvbInfo;
      //
      // Update the checksum value of FV header.
      //
      FvHeader->Checksum = CalculateCheckSum16 ((UINT16 *) FvHeader, FvHeader->HeaderLength / sizeof (UINT16));

      *FvbInfo = FvHeader;

      DEBUG ((EFI_D_INFO, "\nBaseAddr: 0x%lx \n", FvBaseAddress));
      DEBUG ((EFI_D_INFO, "FvLength: 0x%lx \n", (*FvbInfo)->FvLength));
      DEBUG ((EFI_D_INFO, "HeaderLength: 0x%x \n", (*FvbInfo)->HeaderLength));
      DEBUG ((EFI_D_INFO, "FvBlockMap[0].NumBlocks: 0x%x \n", (*FvbInfo)->BlockMap[0].NumBlocks));
      DEBUG ((EFI_D_INFO, "FvBlockMap[0].BlockLength: 0x%x \n", (*FvbInfo)->BlockMap[0].Length));
      DEBUG ((EFI_D_INFO, "FvBlockMap[1].NumBlocks: 0x%x \n",   (*FvbInfo)->BlockMap[1].NumBlocks));
      DEBUG ((EFI_D_INFO, "FvBlockMap[1].BlockLength: 0x%x \n\n", (*FvbInfo)->BlockMap[1].Length));

      return EFI_SUCCESS;
    }
  }
  return EFI_NOT_FOUND;
}

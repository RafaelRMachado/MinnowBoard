/** @file
  Generic Platform hook points for the BDS

  Copyright (c) 2008, Intel Corporation                                                         
  All rights reserved. This program and the accompanying materials                          
  are licensed and made available under the terms and conditions of the BSD License         
  which accompanies this distribution.  The full text of the license may be found at        
  http://opensource.org/licenses/bsd-license.php                                            

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

**/

#include <PiDxe.h>

#include <Protocol/SimpleFileSystem.h>
#include <Protocol/BlockIo.h>
#include <Protocol/DevicePath.h>
#include <Protocol/FirmwareVolume2.h>
#include <Protocol/LoadFile.h>
#include <Protocol/SimpleNetwork.h>

#include <Guid/Gpt.h>

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DevicePathLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <IndustryStandard/Pci.h>

#include "BdsPlatformLib.h"

typedef struct {
  ACPI_HID_DEVICE_PATH      PciRootBridge;
  PCI_DEVICE_PATH           PciDevice;
  EFI_DEVICE_PATH_PROTOCOL  End;
} PLATFORM_ONBOARD_CONTROLLER_DEVICE_PATH;

//
//
//
EFI_BOOT_MODE                 gBootMode;
UINTN                         mDefaultBootOptionsCount    = 0;
UINTN                         mDefaultBootOptionsMaxCount = 0;
EFI_BOOT_MANAGER_BOOT_OPTION  *mDefaultBootOptions        = NULL;

//
// Graphics Controller Device Path: PciRoot(0x0)/Pci(0x2,0x0)
//
PLATFORM_ONBOARD_CONTROLLER_DEVICE_PATH  mGraphicsControllerDevicePath = {
  {
    {
      ACPI_DEVICE_PATH,
      ACPI_DP,
      {
        (UINT8)(sizeof(ACPI_HID_DEVICE_PATH)),
        (UINT8)((sizeof(ACPI_HID_DEVICE_PATH)) >> 8)
      }
    },
    EISA_PNP_ID(0x0A03),
    0
  },
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_PCI_DP,
      {
        (UINT8) (sizeof (PCI_DEVICE_PATH)),
        (UINT8) ((sizeof (PCI_DEVICE_PATH)) >> 8)
      }
    },
    0x0,
    0x2
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      END_DEVICE_PATH_LENGTH,
      0
    }
  }
};


/**
  Platform code is called on BdsEntry after the BootMode and Timeout values
  have been calculated. This routine can override the default BootMode or 
  Timeout settings or do special processing based on the boot mode.

  @param[in,out] BootMode    System PI Boot mode
  @param[in,out] Timeout     UEFI L"Timeout" variable setting.
  
**/
VOID
BdsPlatformEntry (
  IN  EFI_BOOT_MODE   *BootMode,
  IN  UINT16          *Timeout
  )
{
  gBootMode = *BootMode;

  if (*BootMode == BOOT_WITH_DEFAULT_SETTINGS) {
    //
    // Clear the UEFI variables. Recovery code is done when the variables are accessed
    //
    EfiBootManagerDeleteVariables ();
  }

  //
  // Force connect of Graphics Console at PciRoot(0x0)/Pci(0x2,0x0)
  //
  EfiConnectDevicePaths ((EFI_DEVICE_PATH_PROTOCOL *)&mGraphicsControllerDevicePath);
  
//  *Timeout = 0;
}


VOID
BdsPlatformAddBootableImage (
  IN  CHAR16                    *NameString,
  IN  EFI_DEVICE_PATH_PROTOCOL  *FileDevicePath
  )
{
  UINTN  Size;
  UINTN  Index;

  if (mDefaultBootOptionsCount >= mDefaultBootOptionsMaxCount) {
    if (mDefaultBootOptionsMaxCount == 0) {
      // Very first time set a default size
      mDefaultBootOptionsMaxCount = 4;
    }
    Size = mDefaultBootOptionsMaxCount * sizeof (EFI_BOOT_MANAGER_BOOT_OPTION);
    mDefaultBootOptions = ReallocateCopyPool (Size * 2, Size, mDefaultBootOptions);

    mDefaultBootOptionsMaxCount *= 2;
  }
  
  for (Index = 0; Index < mDefaultBootOptionsCount; Index++ ) {
    //
    // If the boot option has already in buffer, don't add it.
    //
    if (!CompareMem (&mDefaultBootOptions[Index].FilePath, FileDevicePath, GetDevicePathSize (FileDevicePath))){
      return;
    }
  }
  EfiBootManagerInitializeBootOption (
    &mDefaultBootOptions[mDefaultBootOptionsCount++],
    NameString,
    FileDevicePath,
    LOAD_OPTION_ACTIVE,
    NULL,
    0
    );
}

VOID
AddLoadFileBootOptions (
  VOID
  )
{
  EFI_STATUS                            Status;
  EFI_HANDLE                            *HandleArray;
  UINTN                                 HandleArrayCount;
  UINTN                                 Index;
  EFI_DEVICE_PATH_PROTOCOL              *FilePath;
  EFI_DEVICE_PATH_PROTOCOL              *DevicePathNode;
  CHAR16                                *FileName;
  EFI_SIMPLE_NETWORK_PROTOCOL           *Snp;

  //
  // If there is a removable media device that does not have media present do a read.
  // The read will cause media to be detected and the partition drivers and file system
  // drivers to layer on top following the EFI driver model
  //
  Status = gBS->LocateHandleBuffer (ByProtocol, &gEfiLoadFileProtocolGuid, NULL, &HandleArrayCount, &HandleArray);
  if (EFI_ERROR (Status)) {
    return;
  }
  for (Index = 0; Index < HandleArrayCount; Index++) {
    //
    // if the LoadFileProtocol is provided by PXE, skip it. BdsLibBootOptionNetwork() will handle it.
    //
    Status = gBS->HandleProtocol (HandleArray[Index], &gEfiSimpleNetworkProtocolGuid, (VOID **)&Snp);
    if (!EFI_ERROR (Status)) {
      continue;
    }

    //
    // Add the file name
    //
    Status = gBS->HandleProtocol (HandleArray[Index], &gEfiDevicePathProtocolGuid, (VOID **)&FilePath);
    if (!EFI_ERROR (Status)) {
      FileName = L"Load File";
      DevicePathNode = FilePath;
      while (!IsDevicePathEnd (DevicePathNode)) {
        if (DevicePathNode->Type == MEDIA_DEVICE_PATH && DevicePathNode->SubType == MEDIA_FILEPATH_DP) {
          FileName = (CHAR16 *)(DevicePathNode + 1);
          break;
        }
        DevicePathNode = NextDevicePathNode (DevicePathNode);
      }
      BdsPlatformAddBootableImage (FileName, FilePath);
    }
  }
  FreePool (HandleArray);
}


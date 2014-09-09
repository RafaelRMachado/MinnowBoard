/** @file
  Produce Load File Protocol for UEFI Applications in Firmware Volumes

  Copyright (c) 2011 - 2013, Intel Corporation                                                         
  All rights reserved. This program and the accompanying materials                          
  are licensed and made available under the terms and conditions of the BSD License         
  which accompanies this distribution.  The full text of the license may be found at        
  http://opensource.org/licenses/bsd-license.php                                            

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

**/

#include <PiDxe.h>

#include <Guid/LzmaDecompress.h>
#include <Protocol/LoadFile.h>
#include <Protocol/DevicePath.h>
#include <Protocol/FirmwareVolume2.h>
#include <Protocol/FirmwareVolumeBlock.h>

#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DevicePathLib.h>

#define LOAD_FILE_ON_FV2_PRIVATE_DATA_SIGNATURE  SIGNATURE_32 ('l', 'f', 'f', 'v')

typedef struct {
  UINTN                          Signature;
  EFI_LOAD_FILE_PROTOCOL         LoadFile;
  EFI_DEVICE_PATH_PROTOCOL       *DevicePath;
  EFI_FIRMWARE_VOLUME2_PROTOCOL  *Fv;
  EFI_GUID                       NameGuid;
  LIST_ENTRY                     Link;
} LOAD_FILE_ON_FV2_PRIVATE_DATA;

#define LOAD_FILE_ON_FV2_PRIVATE_DATA_FROM_THIS(a) CR (a, LOAD_FILE_ON_FV2_PRIVATE_DATA, LoadFile, LOAD_FILE_ON_FV2_PRIVATE_DATA_SIGNATURE)
#define LOAD_FILE_ON_FV2_PRIVATE_DATA_FROM_LINK(a) CR (a, LOAD_FILE_ON_FV2_PRIVATE_DATA, Link, LOAD_FILE_ON_FV2_PRIVATE_DATA_SIGNATURE)

EFI_STATUS
EFIAPI
LoadFileOnFv2LoadFile (
  IN     EFI_LOAD_FILE_PROTOCOL    *This,
  IN     EFI_DEVICE_PATH_PROTOCOL  *FilePath,
  IN     BOOLEAN                   BootPolicy,
  IN OUT UINTN                     *BufferSize,
  IN     VOID                      *Buffer       OPTIONAL
  );

LOAD_FILE_ON_FV2_PRIVATE_DATA  mLoadFileOnFv2PrivateDataTemplate = {
  LOAD_FILE_ON_FV2_PRIVATE_DATA_SIGNATURE,
  {
    LoadFileOnFv2LoadFile
  }
};

EFI_EVENT  mFvRegistration;

LIST_ENTRY mPrivateDataList;

/**
  Causes the driver to load a specified file.

  @param[in]      This                Protocol instance pointer.
  @param[in]      FilePath            The device specific path of the file to load.
  @param[in]      BootPolicy          If TRUE, indicates that the request originates from the
                                      boot manager is attempting to load FilePath as a boot
                                      selection. If FALSE, then FilePath must match an exact file
                                      to be loaded.
  @param[in, out] BufferSize          On input the size of Buffer in bytes. On output with a return
                                      code of EFI_SUCCESS, the amount of data transferred to
                                      Buffer. On output with a return code of EFI_BUFFER_TOO_SMALL,
                                      the size of Buffer required to retrieve the requested file.
  @param[in]      Buffer              The memory buffer to transfer the file to. IF Buffer is NULL,
                                      then no the size of the requested file is returned in
                                      BufferSize.

  @retval EFI_SUCCESS                 The file was loaded.
  @retval EFI_UNSUPPORTED             The device does not support the provided BootPolicy.
  @retval EFI_INVALID_PARAMETER       FilePath is not a valid device path, or
                                      BufferSize is NULL.
  @retval EFI_NO_MEDIA                No medium was present to load the file.
  @retval EFI_DEVICE_ERROR            The file was not loaded due to a device error.
  @retval EFI_NO_RESPONSE             The remote system did not respond.
  @retval EFI_NOT_FOUND               The file was not found.
  @retval EFI_ABORTED                 The file load process was manually cancelled.

**/
EFI_STATUS
EFIAPI
LoadFileOnFv2LoadFile (
  IN     EFI_LOAD_FILE_PROTOCOL    *This,
  IN     EFI_DEVICE_PATH_PROTOCOL  *FilePath,
  IN     BOOLEAN                   BootPolicy,
  IN OUT UINTN                     *BufferSize,
  IN     VOID                      *Buffer       OPTIONAL
  )
{
  EFI_STATUS                     Status;
  LOAD_FILE_ON_FV2_PRIVATE_DATA  *Private;
  VOID                           *Pe32Buffer;
  UINTN                          Pe32BufferSize;
  UINT32                         AuthenticationStatus;

  if (This == NULL || BufferSize == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Only support BootPolicy
  //
  if (!BootPolicy) {
    return EFI_UNSUPPORTED;
  }

  //
  // Get private context data
  //  
  Private = LOAD_FILE_ON_FV2_PRIVATE_DATA_FROM_THIS (This);

  //
  // Determine the size of the PE32 section
  //
  Pe32Buffer     = NULL;
  Pe32BufferSize = 0;
  Status = Private->Fv->ReadSection (
                        Private->Fv,
                        &Private->NameGuid,
                        EFI_SECTION_PE32,
                        0,
                        &Pe32Buffer,
                        &Pe32BufferSize,
                        &AuthenticationStatus
                        );
  if (EFI_ERROR (Status)) {
    return Status;
  }    

  //
  // If the buffer passed in is not large enough, return the size of the required
  // buffer in BufferSize and return EFI_BUFFER_TOO_SMALL
  //  
  if (*BufferSize < Pe32BufferSize || Buffer == NULL) {
    *BufferSize = Pe32BufferSize;
    return EFI_BUFFER_TOO_SMALL;
  }
  
  //
  // The buffer passed in is large enough, so read the PE32 section directly into
  // the buffer, update BufferSize with the actual size read, and return the status
  // from ReadSection()
  //
  return Private->Fv->ReadSection (
                        Private->Fv,
                        &Private->NameGuid,
                        EFI_SECTION_PE32,
                        0,
                        &Buffer,
                        BufferSize,
                        &AuthenticationStatus
                        );
}

/** 
  Check if the FFS has been installed LoadFileProtocol for it.

  @param EFI_GUID     File GUID.
  
  @retval TRUE        The FFS's FileLoadProtocol is in list.
  @retval FALSE       The FFS's FileLoadProtocol is not in list.
  
**/
BOOLEAN
EFIAPI
IsInPrivateList (
  IN EFI_GUID      *NameGuid
) 
{
 LIST_ENTRY  *Entry;
 LOAD_FILE_ON_FV2_PRIVATE_DATA *PrivateData;
 
 if (IsListEmpty (&mPrivateDataList)) {
   return FALSE;
 }

 for(Entry = (&mPrivateDataList)->ForwardLink; Entry != (&mPrivateDataList); Entry = Entry->ForwardLink) {
   PrivateData = LOAD_FILE_ON_FV2_PRIVATE_DATA_FROM_LINK (Entry);
   if (CompareGuid (NameGuid, &PrivateData->NameGuid)) {
     DEBUG ((EFI_D_INFO, "LoadFileOnFv2:FileLoadProtocol has been installed in:%g\n", NameGuid)); 
     return TRUE;
   }
 }
 return FALSE;
}


/** 
  Install LoadFile Protocol for FFS.

  @param Handle          FV Handle.
  
**/
VOID
EFIAPI
InstallFileLoadProtocol (
  EFI_HANDLE Handle
)
{
  EFI_STATUS                     Status;
  LOAD_FILE_ON_FV2_PRIVATE_DATA  *Private;
  EFI_FIRMWARE_VOLUME2_PROTOCOL  *Fv;
  EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL *Fvb;
  EFI_PHYSICAL_ADDRESS           Address;
  EFI_FV_FILETYPE                FileType;
  UINTN                          Key;
  EFI_GUID                       NameGuid;
  EFI_FV_FILE_ATTRIBUTES         Attributes;
  UINTN                          Size;
  EFI_HANDLE                     LoadFileHandle;
  UINT32                         AuthenticationStatus;
  CHAR16                         *UiName;
  UINTN                          UiNameSize;

  DEBUG ((EFI_D_INFO, "LoadFileOnFv2:Find a FV!\n"));
  Status = gBS->HandleProtocol (Handle, &gEfiFirmwareVolume2ProtocolGuid, (VOID **)&Fv);    
  ASSERT_EFI_ERROR (Status);
  Status = gBS->HandleProtocol (Handle, &gEfiFirmwareVolumeBlockProtocolGuid, (VOID **)&Fvb);
  Fvb->GetPhysicalAddress (Fvb, &Address);
  DEBUG ((EFI_D_INFO, "LoadFileOnFv2:Fvb->Address=%x \n", Address));
  
  //
  // Use Firmware Volume 2 Protocol to search for aFFS files of type 
  // EFI_FV_FILETYPE_APPLICATION and produce a LoadFile protocol for 
  // each one found.
  //
  FileType = EFI_FV_FILETYPE_APPLICATION;
  Key = 0;
  while (TRUE) {
    Status = Fv->GetNextFile (Fv, &Key, &FileType, &NameGuid, &Attributes, &Size);
    if (EFI_ERROR (Status)) {
      break;
    }
    
    UiName = NULL;
    Status = Fv->ReadSection (
                   Fv,
                   &NameGuid,
                   EFI_SECTION_USER_INTERFACE,
                   0,
                   (VOID **)&UiName,
                   &UiNameSize,
                   &AuthenticationStatus
                   );
    if (EFI_ERROR (Status)) {
      continue;
    }        
    if (!IsInPrivateList (&NameGuid)) {
      Private = (LOAD_FILE_ON_FV2_PRIVATE_DATA *)AllocateCopyPool (sizeof (mLoadFileOnFv2PrivateDataTemplate), &mLoadFileOnFv2PrivateDataTemplate);
      ASSERT (Private != NULL);
      Private->Fv = Fv;
      Private->DevicePath = FileDevicePath (Handle, UiName);
      CopyGuid (&Private->NameGuid, &NameGuid);
      LoadFileHandle = NULL;
      DEBUG ((EFI_D_INFO, "Find a APPLICATION in this FV!\n"));
      Status = gBS->InstallMultipleProtocolInterfaces (
                      &LoadFileHandle,
                      &gEfiDevicePathProtocolGuid, Private->DevicePath,
                      &gEfiLoadFileProtocolGuid, &Private->LoadFile,
                      NULL
                      );
      ASSERT_EFI_ERROR (Status);
      InsertTailList (&mPrivateDataList, &Private->Link);
    }
  }
}

/**
  This notification function is invoked when an instance of the
  LzmaCustomDecompressGuid is produced. It installs another instance of the
  EFI_FIRMWARE_VOLUME_PROTOCOL on the handle of the FFS. This notification function
  also handles the situation when LZMA decoder driver loaded later than FirmwareVolume driver.

  @param  Event                 The event that occured
  @param  Context               Context of event. Not used in this nofication function.

**/
VOID
EFIAPI
FvNotificationEvent (
  IN  EFI_EVENT       Event,
  IN  VOID            *Context
  )
{
  EFI_STATUS                     Status;
  UINTN                          BufferSize;
  EFI_HANDLE                     *Handle;
  UINTN                          Index;
  EFI_HANDLE                     *CurHandle;


  Handle     = NULL;
  Index      = 0;
  BufferSize = sizeof (EFI_HANDLE);
  Handle     = AllocateZeroPool (BufferSize);  
  Status = gBS->LocateHandle (
                    ByProtocol,
                    &gEfiFirmwareVolume2ProtocolGuid,
                    NULL,
                    &BufferSize,
                    Handle
                    );
  if (EFI_BUFFER_TOO_SMALL == Status) {
    FreePool (Handle);
    Handle = AllocateZeroPool (BufferSize);
    Status = gBS->LocateHandle (
                    ByProtocol,
                    &gEfiFirmwareVolume2ProtocolGuid,
                    NULL,
                    &BufferSize,
                    Handle
                    );
    if (EFI_ERROR (Status)) {
      return;
    }    
  } else if (EFI_ERROR (Status)) {
    return;
  }

  CurHandle = Handle;
  //DEBUG ((EFI_D_ERROR, "LoadFileFv2: NumofHandle=%x\n", BufferSize/sizeof (EFI_HANDLE)));
  for (Index=0; Index < BufferSize/sizeof (EFI_HANDLE); Index++) {
    CurHandle = Handle + Index;
    //DEBUG ((EFI_D_ERROR, "LoadFileFv2: Index= %x\n", Index));
    //
    // Install LoadFile Protocol
    //
    InstallFileLoadProtocol (*CurHandle);    
    //DEBUG ((EFI_D_ERROR, "LocadFileFv: CurHandle=%x\n", *CurHandle));
  }
  if (Handle != NULL) {
    FreePool (Handle);
  } 
}

/**
  The user Entry Point for Application. The user code starts with this function
  as the real entry point for the image goes into a library that calls this 
  function.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.  
  @param[in] SystemTable    A pointer to the EFI System Table.
  
  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
LoadFileOnFv2Intialize (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  InitializeListHead (&mPrivateDataList);
  
  EfiCreateProtocolNotifyEvent (
    &gEfiFirmwareVolume2ProtocolGuid,
    TPL_CALLBACK,
    FvNotificationEvent,
    NULL,
    &mFvRegistration
    );

  EfiCreateProtocolNotifyEvent (
     &gLzmaCustomDecompressGuid,
     TPL_CALLBACK,
     FvNotificationEvent,
     NULL,
     &mFvRegistration
    );

  return EFI_SUCCESS;         
}

/** @file
  Common BDS library routines

  Copyright (c) 2008 - 2013, Intel Corporation                                                         
  All rights reserved. This program and the accompanying materials                          
  are licensed and made available under the terms and conditions of the BSD License         
  which accompanies this distribution.  The full text of the license may be found at        
  http://opensource.org/licenses/bsd-license.php                                            

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

**/

#include "BdsLib.h"
#include "BdsUiLib.h"
#include "BdsUtility.h"

#define  BDS_MAX_STRING_LENTH                  100

BOOLEAN   mConnectAll = FALSE;

extern EFI_BOOT_MODE                 gBootMode;
extern UINTN                         mDefaultBootOptionsCount;
extern UINTN                         mDefaultBootOptionsMaxCount;
extern EFI_BOOT_MANAGER_BOOT_OPTION  *mDefaultBootOptions;

EFI_STATUS
EFIAPI
BdsLibGetImageHeader (
  IN  EFI_HANDLE                  Device,
  IN  CHAR16                      *FileName,
  OUT EFI_IMAGE_DOS_HEADER        *DosHeader,
  OUT EFI_IMAGE_OPTIONAL_HEADER_PTR_UNION   Hdr
  );

/**
  Get the EFI Boot Manager L"Timeout" variable setting. If the variable is
  not valid set it to a default value if SetDefaultOnError is TRUE. The
  default value is configurable via a PCD setting.

  @param  SetDefaultOnError   Set default value on error

  @return Timeout value.

**/
UINT16
EfiBootManagerGetTimeout (
  IN  BOOLEAN       SetDefaultOnError
  )
{
  UINT16      Timeout;
  UINTN       Size;
  EFI_STATUS  Status;

  //
  // Return Timeout variable or 0xffff if no valid
  // Timeout variable exists.
  //
  Size    = sizeof (UINT16);
  Status  = gRT->GetVariable (L"Timeout", &gEfiGlobalVariableGuid, NULL, &Size, &Timeout);
  if (!EFI_ERROR (Status)) {
    return Timeout;
  }

  Timeout = (UINT16) PcdGet16 (PcdPlatformBootTimeOut);
  //
  // If O.K. with caller set the default value
  //
  if (SetDefaultOnError) {
    gRT->SetVariable (
          L"Timeout",
          &gEfiGlobalVariableGuid,
          EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
          sizeof (UINT16),
          &Timeout
          );
  }
  return Timeout;
}

/**
  Delete all UEFI specification variables. All EFI variables with the gEfiGlobalVariableGuid
  are owned by the UEFI specification and this routine will delete them.

  Warning: If this functions breaks your code you are probably using gEfiGlobalVariableGuid
  in violation of the EFI specificaiton. Only things defined in the EFI specification can
  use this GUID. Please change your GUID.

**/
VOID
EfiBootManagerDeleteVariables (
  VOID
  )
{
  EFI_STATUS    Status;
  UINTN         VariableNameSize;
  CHAR16        VariableName[256];
  EFI_GUID      VendorGuid;

  VariableName[0] = 0x0000;
  do {
    Status = gRT->GetNextVariableName (&VariableNameSize, VariableName, &VendorGuid);
    if (!EFI_ERROR (Status)) {
      if (CompareGuid (&VendorGuid, &gEfiGlobalVariableGuid)) {
        //
        // If its the EFI GUID delete it
        //
        gRT->SetVariable (VariableName, &gEfiGlobalVariableGuid, 0, 0, NULL);
        //
        // Start search over after delete operation
        //
        VariableName[0] = 0x0000;
      } else if (CompareGuid (&VendorGuid, &gEfiCallerIdGuid)) {
        //
        // If its the CallerId GUID delete it
        //
        gRT->SetVariable (VariableName, &gEfiCallerIdGuid, 0, 0, NULL);
        //
        // Start search over after delete operation
        //
        VariableName[0] = 0x0000;
      }
    } else {
      //
      // sizeof (VariableName) is big enough for all currently defined variables.
      // If you get this ASSERT there are two probable causes.
      // 1) UEFI specification has been updated and the size of VariableName needs to grow
      // 2) The gEfiGlobalVariableGuid is being used in error (It's owned by the UEFI spec)
      //    so your private variable can not use the EFI GUID, change your GUID!
      //
      ASSERT (!(Status == EFI_BUFFER_TOO_SMALL));
    }

  } while (!EFI_ERROR (Status));
}


VOID
EfiBootManagerInitializeBootOption (
  IN OUT EFI_BOOT_MANAGER_BOOT_OPTION *Option,
  IN  CHAR16                          *Description,
  IN  EFI_DEVICE_PATH_PROTOCOL        *FilePath,
  IN  UINT32                          Attributes,
  IN  UINT8                           *OptionalData,
  IN  UINT32                          OptionalDataSize
  )
{
  ZeroMem (Option, sizeof (EFI_BOOT_MANAGER_BOOT_OPTION));

  Option->FilePath  = AllocateZeroPool (GetDevicePathSize (FilePath));
  CopyMem (Option->FilePath, FilePath, GetDevicePathSize (FilePath));
  Option->Attributes = Attributes;
  Option->Description = AllocateZeroPool (StrSize (Description));
  CopyMem (Option->Description, Description, StrSize (Description));
  Option->OptionalData = AllocateZeroPool (OptionalDataSize);
  CopyMem (Option->OptionalData, OptionalData, OptionalDataSize);
  Option->OptionalDataSize = OptionalDataSize;
}


/**
  Build the boot#### or driver#### option from the VariableName, the
  build boot#### or driver#### will also be linked to BdsCommonOptionList.

  @param  BdsCommonOptionList   The header of the boot#### or driver#### option
                                link list
  @param  VariableName          EFI Variable name indicate if it is boot#### or
                                driver####

  @retval BDS_COMMON_OPTION     Get the option just been created
  @retval NULL                  Failed to get the new option

**/
EFI_BOOT_MANAGER_BOOT_OPTION *
EFIAPI
MinuteBdsLibVariableToOption (
  IN OUT LIST_ENTRY                   *BdsCommonOptionList,
  IN  CHAR16                          *VariableName
  )
{
  UINT32                    Attribute;
  UINT16                    FilePathSize;
  UINT8                     *Variable;
  UINT8                     *TempPtr;
  UINTN                     VariableSize;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  EFI_BOOT_MANAGER_BOOT_OPTION *Option;
  VOID                      *LoadOptions;
  UINT32                    LoadOptionsSize;
  CHAR16                    *Description;
  UINT8                     NumOff;
  //
  // Read the variable. We will never free this data.
  //
  Variable = BdsLibGetVariableAndSize (
              VariableName,
              &gEfiGlobalVariableGuid,
              &VariableSize
              );
  if (Variable == NULL) {
    return NULL;
  }
  //
  // Notes: careful defined the variable of Boot#### or
  // Driver####, consider use some macro to abstract the code
  //
  //
  // Get the option attribute
  //
  TempPtr   =  Variable;
  Attribute =  *(UINT32 *) Variable;
  TempPtr   += sizeof (UINT32);

  //
  // Get the option's device path size
  //
  FilePathSize =  *(UINT16 *) TempPtr;
  TempPtr      += sizeof (UINT16);

  //
  // Get the option's description string
  //
  Description = (CHAR16 *) TempPtr;

  //
  // Get the option's description string size
  //
  TempPtr     += StrSize ((CHAR16 *) TempPtr);

  //
  // Get the option's device path
  //
  DevicePath =  (EFI_DEVICE_PATH_PROTOCOL *) TempPtr;
  TempPtr    += FilePathSize;

  LoadOptions     = TempPtr;
  LoadOptionsSize = (UINT32) (VariableSize - (UINTN) (TempPtr - Variable));

  //
  // The Console variables may have multiple device paths, so make
  // an Entry for each one.
  //
  Option = AllocateZeroPool (sizeof (EFI_BOOT_MANAGER_BOOT_OPTION));
  if (Option == NULL) {
    return NULL;
  }

  //Option->Signature   = BDS_LOAD_OPTION_SIGNATURE;
  Option->FilePath  = AllocateZeroPool (GetDevicePathSize (DevicePath));
  ASSERT(Option->FilePath != NULL);
  CopyMem (Option->FilePath, DevicePath, GetDevicePathSize (DevicePath));

  Option->Attributes   = Attribute;
  Option->Description = AllocateZeroPool (StrSize (Description));
  ASSERT(Option->Description != NULL);
  CopyMem (Option->Description, Description, StrSize (Description));

  Option->OptionalData = AllocateZeroPool (LoadOptionsSize);
  ASSERT(Option->OptionalData != NULL);
  CopyMem (Option->OptionalData, LoadOptions, LoadOptionsSize);
  Option->OptionalDataSize = LoadOptionsSize;

  //
  // Get the value from VariableName Unicode string
  // since the ISO standard assumes ASCII equivalent abbreviations, we can be safe in converting this
  // Unicode stream to ASCII without any loss in meaning.
  //
  if (*VariableName == 'B') {
    NumOff = (UINT8) (sizeof (L"Boot") / sizeof(CHAR16) - 1);
    Option->BootCurrent = (UINT16) ((VariableName[NumOff]  -'0') * 0x1000);
    Option->BootCurrent = (UINT16) (Option->BootCurrent + ((VariableName[NumOff+1]-'0') * 0x100));
    Option->BootCurrent = (UINT16) (Option->BootCurrent +  ((VariableName[NumOff+2]-'0') * 0x10));
    Option->BootCurrent = (UINT16) (Option->BootCurrent + ((VariableName[NumOff+3]-'0')));
  }
  //
  // Insert active entry to BdsDeviceList
  //
  if ((Option->Attributes & LOAD_OPTION_ACTIVE) == LOAD_OPTION_ACTIVE) {
    //
    //InsertTailList (BdsCommonOptionList, &Option->Link);
    //
    FreePool (Variable);
    return Option;
  }

  FreePool (Variable);
  FreePool (Option);
  return NULL;

}

/**
  Internal function to check if the input boot option is a valid EFI NV Boot####.

  @param OptionToCheck  Boot option to be checked.

  @retval TRUE      This boot option matches a valid EFI NV Boot####.
  @retval FALSE     If not.

**/
BOOLEAN
BdsIsBootOptionValidNVVarialbe (
  IN  EFI_BOOT_MANAGER_BOOT_OPTION      *OptionToCheck
  )
{
  LIST_ENTRY                   TempList;
  EFI_BOOT_MANAGER_BOOT_OPTION *BootOption;
  BOOLEAN                      Valid;
  CHAR16                       OptionName[20];

  Valid = FALSE;

  InitializeListHead (&TempList);
  UnicodeSPrint (OptionName, sizeof (OptionName), L"Boot%04x\n", OptionToCheck->BootCurrent);

  BootOption = MinuteBdsLibVariableToOption (&TempList, OptionName);
  if (BootOption == NULL) {
    return FALSE;
  }

  //
  // If the Boot Option Number and Device Path matches, OptionToCheck matches a
  // valid EFI NV Boot####.
  //
  if ((OptionToCheck->BootCurrent == BootOption->BootCurrent) &&
      (CompareMem (OptionToCheck->FilePath, BootOption->FilePath, GetDevicePathSize (OptionToCheck->FilePath)) == 0))
      {
    Valid = TRUE;
  }

  FreePool (BootOption);

  return Valid;
}


EFI_STATUS
EfiBootManagerLoadImage (
  IN  EFI_BOOT_MANAGER_BOOT_OPTION    *BootOption,
  IN  EFI_HANDLE                      *ImageHandle,
  IN  BOOLEAN                         RemovableMediaSupport
  )
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *WorkingDevicePath;
  EFI_HANDLE                Handle;
  EFI_DEVICE_PATH_PROTOCOL  *FilePath;
  EFI_LOADED_IMAGE_PROTOCOL *ImageInfo;

  PERF_START (0, "BDS Load Image", NULL, 1);
  PERF_END   (0, "BDS Load Image", NULL, 0);

  Status = EFI_SUCCESS;
  //
  // If it's Device Path that starts with a hard drive path, append it with the front part to compose a
  // full device path
  //
  WorkingDevicePath = NULL;
  if ((DevicePathType (BootOption->FilePath) == MEDIA_DEVICE_PATH) &&
      (DevicePathSubType (BootOption->FilePath) == MEDIA_HARDDRIVE_DP)) {
    WorkingDevicePath = BdsExpandPartitionPartialDevicePathToFull (
                          (HARDDRIVE_DEVICE_PATH *)BootOption->FilePath
                          );
    if (WorkingDevicePath != NULL) {
      BootOption->FilePath = WorkingDevicePath;
    }
  }

  //
  // Set Boot Current
  //
  if (BdsIsBootOptionValidNVVarialbe (BootOption)) {
    //
    // For a temporary boot (i.e. a boot by selected a EFI Shell using "Boot From File"), Boot Current is actually not valid.
    // In this case, "BootCurrent" is not created.
    // Only create the BootCurrent variable when it points to a valid Boot#### variable.
    //
    gRT->SetVariable (
          L"BootCurrent",
          &gEfiGlobalVariableGuid,
          EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
          sizeof (UINT16),
          &BootOption->BootCurrent
          );
  }

  //
  // Signal the EVT_SIGNAL_READY_TO_BOOT event
  //
  EfiSignalEventReadyToBoot();

  //
  // Expand USB Class or USB WWID device path node to be full device path of a USB
  // device in platform then load the boot file on this full device path and get the
  // image handle.
  //
  *ImageHandle = BdsExpandUsbShortFormDevicePath (BootOption->FilePath);

  //
  // Adjust the different type memory page number just before booting
  // and save the updated info into the variable for next boot to use
  //
  BdsSetMemoryTypeInformationVariable ();

  //
  // By expanding the USB Class or WWID device path, the ImageHandle has returnned.
  // Here get the ImageHandle for the non USB class or WWID device path.
  //
  if (*ImageHandle == NULL) {
    ASSERT (BootOption->FilePath != NULL);
    if ((DevicePathType (BootOption->FilePath) == BBS_DEVICE_PATH) &&
        (DevicePathSubType (BootOption->FilePath) == BBS_BBS_DP)
       ) {
      //
      // ToDo: Check to see if we should legacy BOOT. If yes then do the legacy boot
      //

    }

    DEBUG_CODE_BEGIN();

    if (BootOption->Description == NULL) {
      DEBUG ((DEBUG_INFO | DEBUG_LOAD, "Booting from unknown device path\n"));
    } else {
      DEBUG ((DEBUG_INFO | DEBUG_LOAD, "Booting %S\n", BootOption->Description));
    }

    DEBUG_CODE_END();

    //
    // Report status code for OS Loader LoadImage.
    //
    REPORT_STATUS_CODE (EFI_PROGRESS_CODE, PcdGet32 (PcdProgressCodeOsLoaderLoad));
    Status = gBS->LoadImage (
                    TRUE,
                    gImageHandle,
                    BootOption->FilePath,
                    NULL,
                    0,
                    ImageHandle
                    );

    //
    // If we didn't find an image directly, we need to try as if it is a removable device boot option
    // and load the image according to the default boot behavior for removable device.
    //
    if (EFI_ERROR (Status)) {
      //
      // check if there is a bootable removable media could be found in this device path ,
      // and get the bootable media handle
      //
      Handle = BdsLibGetBootableHandle(BootOption->FilePath);
      if (Handle == NULL) {
        goto Done;
      }
      //
      // Load the default boot file \EFI\BOOT\boot{machinename}.EFI from removable Media
      //  machinename is ia32, ia64, x64, ...
      //
      FilePath = FileDevicePath (Handle, EFI_REMOVABLE_MEDIA_FILE_NAME);
      if (FilePath != NULL) {
        REPORT_STATUS_CODE (EFI_PROGRESS_CODE, PcdGet32 (PcdProgressCodeOsLoaderLoad));
        Status = gBS->LoadImage (
                        TRUE,
                        gImageHandle,
                        FilePath,
                        NULL,
                        0,
                        ImageHandle
                        );
       if (EFI_ERROR (Status)) {
          //
          // The DevicePath failed, and it's not a valid
          // removable media device.
          //
          goto Done;
        }
      }
    }

    if (EFI_ERROR (Status)) {
      //
      // If there is any error from the Boot attempt exit now.
      //
      goto Done;
    }
  }

  //
  // Provide the image with it's load options
  //
  if (*ImageHandle == NULL) {
    goto Done;
  }
  Status = gBS->HandleProtocol (*ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **) &ImageInfo);
  ASSERT_EFI_ERROR (Status);

  if (BootOption->OptionalDataSize != 0) {
    ImageInfo->LoadOptionsSize  = BootOption->OptionalDataSize;
    ImageInfo->LoadOptions      = BootOption->OptionalData;
  }
  //
  // Before calling the image, enable the Watchdog Timer for
  // the 5 Minute period
  //
  gBS->SetWatchdogTimer (5 * 60, 0x0000, 0x00, NULL);

  //
  // Write boot to OS performance data for UEFI boot
  //
  PERF_CODE (
    WriteBootToOsPerformanceData ();
  );

/*
  //
  // If image being started is loaded from Firmware Volume, then do a connect all 
  // This is required to support Setup application in FLASH and Shell in FLASH.
  //
  if (DevicePathType    (BootOption->FilePath) == MEDIA_DEVICE_PATH    && 
      DevicePathSubType (BootOption->FilePath) == MEDIA_PIWG_FW_VOL_DP    ) {
    EfiBootManagerConnectAll (TRUE);
  }
*/

  //
  //  Connect all of the devices before loading the shell
  //
  if (FeaturePcdGet(PcdFeatureBdsConnectAll)) {
    if (BootOption->ConnectAll) {
      if (!mConnectAll) {
        Print (L"Connecting All Devices ...\r\n");
      }
      EfiBootManagerConnectAll (TRUE);
    }
  }

  //
  //  Boot phase notification
  //
  if (FeaturePcdGet (PcdDiagBootPhasesSerial)) {
    Print (L"Starting UEFI Application ...\r\n");
  } else {
    DEBUG ((DEBUG_INFO, "Starting UEFI Application ...\n"));
  }

  //
  // Report status code for OS Loader StartImage.
  //
  REPORT_STATUS_CODE (EFI_PROGRESS_CODE, PcdGet32 (PcdProgressCodeOsLoaderStart));

  Status = gBS->StartImage (*ImageHandle, &BootOption->ExitDataSize, &BootOption->ExitData);
  DEBUG ((DEBUG_INFO | DEBUG_LOAD, "Image Return Status = %r\n", Status));

  //
  // Clear the Watchdog Timer after the image returns
  //
  gBS->SetWatchdogTimer (0x0000, 0x0000, 0x0000, NULL);

Done:

  //
  //  Connect all of the devices on failure
  //
  if (FeaturePcdGet(PcdFeatureBdsConnectAll)) {
    EfiBootManagerConnectAll (TRUE);
  }

  //
  // Clear Boot Current
  //
  gRT->SetVariable (
        L"BootCurrent",
        &gEfiGlobalVariableGuid,
        EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
        0,
        &BootOption->BootCurrent
        );

  return Status;
}


/**
  Attempt to boot the EFI boot option. This routine sets L"BootCurent" and
  also singles the EFI ready to boot event. If the device path for the option
  starts with a BBS device path a legacy boot is attempted via the callback
  registered in EfiBootManagerInitialize(). Short form device paths are
  also supported via this rountine. A device path starting with
  MEDIA_HARDDRIVE_DP, MSG_USB_WWID_DP, MSG_USB_CLASS_DP gets expaned out
  to find the first device that matches. If the BootOption Device Path
  fails the removable media boot algorithm is attempted (\EFI\BOOTIA32.EFI,
  \EFI\BOOTX64.EFI,... only one file type is tried per processor type)

  @param  BootOption    Boot Option to try and boot.

  @return EFI_SUCCESS     BootOption was booted
  @return EFI_UNSUPPORTED A BBS device path was found with no valid callback
                          registered via EfiBootManagerInitialize().
  @return EFI_NOT_FOUND   The BootOption was not found on the system
  @return !EFI_SUCCESS  BootOption failed with this error status

**/
VOID
EFIAPI
EfiBootManagerBootOption (
  IN  EFI_BOOT_MANAGER_BOOT_OPTION    *BootOption
  )
{
  EFI_STATUS                Status;
  EFI_HANDLE                ImageHandle;

  // Connect the device if necessary. No such need for internal shell.
  if (BdsGetBootTypeFromDevicePath (BootOption->FilePath) != BDS_EFI_MEM_DEVICE_BOOT) {
    BdsLibConnectDevicePath (BootOption->FilePath);
    BootOption->ConnectAll = FALSE;
  } else {
    BootOption->ConnectAll = TRUE;
  }

  //
  // Record the performance data for End of BDS
  //
  PERF_END(NULL, "BDS", NULL, 0);

  if ((DevicePathType (BootOption->FilePath) == BBS_DEVICE_PATH) &&
      (DevicePathSubType (BootOption->FilePath) == BBS_BBS_DP)) {
    return;
  }
  
  PERF_START (NULL, "Signal ReadyToBoot", "BDS", 0);
  EfiSignalEventReadyToBoot ();
  PERF_END (NULL, "Signal ReadyToBoot", "BDS", 0);

  Status = EfiBootManagerLoadImage (BootOption, &ImageHandle, TRUE);
}

VOID
EfiBootManagerConnectAll (
  IN  BOOLEAN   OnlyOncePerBoot
  )
{
  EFI_STATUS  Status;
  UINTN       HandleCount;
  EFI_HANDLE  *HandleBuffer;
  UINTN       Index;

  if (OnlyOncePerBoot && mConnectAll) {
    return;
  }
  mConnectAll = TRUE;

  Status = gBS->LocateHandleBuffer (
                  AllHandles,
                  NULL,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return;
  }



  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->ConnectController (HandleBuffer[Index], NULL, NULL, TRUE);
  }

  FreePool (HandleBuffer);
  return;
}

EFI_STATUS
EFIAPI
EfiConnectDevicePaths (
  IN EFI_DEVICE_PATH_PROTOCOL *DevicePathToConnect
  )
{

  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *CopyOfDevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *Instance;
  EFI_DEVICE_PATH_PROTOCOL  *RemainingDevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *Next;
  EFI_HANDLE                Handle;
  EFI_HANDLE                PreviousHandle;
  UINTN                     Size;

    if (DevicePathToConnect == NULL) {
      return EFI_SUCCESS;
    }

    DevicePath        = DuplicateDevicePath (DevicePathToConnect);
    if (DevicePath == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    CopyOfDevicePath  = DevicePath;

    do {
      //
      // The outer loop handles multi instance device paths.
      // Only console variables contain multiple instance device paths.
      //
      // After this call DevicePath points to the next Instance
      //
      Instance  = GetNextDevicePathInstance (&DevicePath, &Size);
      if (Instance == NULL) {
        FreePool (CopyOfDevicePath);
        return EFI_OUT_OF_RESOURCES;
      }

      Next      = Instance;
      while (!IsDevicePathEndType (Next)) {
        Next = NextDevicePathNode (Next);
      }

      SetDevicePathEndNode (Next);

      //
      // Start the real work of connect with RemainingDevicePath
      //
      PreviousHandle = NULL;
      do {
        //
        // Find the handle that best matches the Device Path. If it is only a
        // partial match the remaining part of the device path is returned in
        // RemainingDevicePath.
        //
        RemainingDevicePath = Instance;
        Status              = gBS->LocateDevicePath (&gEfiDevicePathProtocolGuid, &RemainingDevicePath, &Handle);
        if (!IsDevicePathEndType( RemainingDevicePath)) {
          gBS->DisconnectController (Handle, NULL, NULL);
        }

        if (!EFI_ERROR (Status)) {
          if (Handle == PreviousHandle) {
            //
            // If no forward progress is made try invoking the Dispatcher.
            // A new FV may have been added to the system an new drivers
            // may now be found.
            // Status == EFI_SUCCESS means a driver was dispatched
            // Status == EFI_NOT_FOUND means no new drivers were dispatched
            //
            Status = gDS->Dispatch ();
          }

          if (!EFI_ERROR (Status)) {
            PreviousHandle = Handle;
            //
            // Connect all drivers that apply to Handle and RemainingDevicePath,
            // the Recursive flag is FALSE so only one level will be expanded.
            //
            // Do not check the connect status here, if the connect controller fail,
            // then still give the chance to do dispatch, because partial
            // RemainingDevicepath may be in the new FV
            //
            // 1. If the connect fail, RemainingDevicepath and handle will not
            //    change, so next time will do the dispatch, then dispatch's status
            //    will take effect
            // 2. If the connect success, the RemainingDevicepath and handle will
            //    change, then avoid the dispatch, we have chance to continue the
            //    next connection
            //
            gBS->ConnectController (Handle, NULL, RemainingDevicePath, FALSE);
          }
        }
        //
        // Loop until RemainingDevicePath is an empty device path
        //
      } while (!EFI_ERROR (Status) && !IsDevicePathEnd (RemainingDevicePath));

    } while (DevicePath != NULL);

    if (CopyOfDevicePath != NULL) {
      FreePool (CopyOfDevicePath);
    }
    //
    // All handle with DevicePath exists in the handle database
    //
    return Status;
  }


/**
  Return Key from Simple Text In Protocol. Optional return after a timeout expires
  even if no key was pressed. A timeout of zero mean no timeout is used.

  @param  Key             Key press information from Simple Text In protocol
  @param  Period          100ns increments to wait for key press


  @retval EFI_SUCCESS      The keystroke information was returned.
  @retval EFI_TIMEOUT      TimeoutInPeriod expired with no key being input
  @retval EFI_NOT_READY    There was no keystroke data availiable.
  @retval EFI_DEVICE_ERROR The keystroke information was not returned due to
                           hardware errors.

**/
EFI_STATUS
GetCharKeyWithTimeout (
  IN OUT EFI_KEY_DATA             *Key,
  IN     UINT64                   Period
  )
{
  EFI_STATUS    Status;
  UINTN         WaitCount;
  UINTN         WaitIndex;
  EFI_EVENT     WaitList[2];

  WaitCount   = 1;
  WaitList[0] = gST->ConIn->WaitForKey;
  if (Period != 0) {
    // Create a time event for 1 sec duration if we have a timeout
    gBS->CreateEvent (EVT_TIMER, 0, NULL, NULL, &WaitList[1]);
    gBS->SetTimer (WaitList[1], TimerPeriodic, Period);
    WaitCount++;
  }

  for (;;) {
    Status = gBS->WaitForEvent (WaitCount, WaitList, &WaitIndex);
    ASSERT_EFI_ERROR (Status);

    switch (WaitIndex) {
    case 0:
      // Key event signaled
      Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key->Key);
      if (!EFI_ERROR (Status)) {
        if (WaitCount == 2) {
          gBS->CloseEvent (WaitList[1]);
        }
        return Status;
      }
      break;

    case 1:
      // Timeout
      gBS->CloseEvent (WaitList[1]);
      return EFI_TIMEOUT;
      break;

    default:
      ASSERT (FALSE);
    }
  }
}



/**
  Copies an allocated buffer of OldSize to an allocated buffer of type EfiBootServicesData
  of AllocationSize. If Buffer is NULL this function works like AllocatePool (). If Buffer
  is not NULL FreePool ()

  Allocates the number bytes specified by AllocationSize of type EfiBootServicesData, copies
  OldSize bytes from Buffer to the newly allocated buffer, and returns a pointer to the
  allocated buffer.  If AllocationSize is 0, then a valid buffer of 0 size is returned.  If there
  is not enough memory remaining to satisfy the request, then NULL is returned.
  If AllocationSize is greater than (MAX_ADDRESS ? Buffer + 1), then ASSERT().
  If AllocateionSize < OldSize then ASSERT ().

  @param  AllocationSize        The number of bytes to allocate.
  @param  OldSize               Size of current buffer.
  @param  Buffer                The buffer to copy to the allocated buffer.

  @return A pointer to the allocated buffer or NULL if allocation fails.

**/
VOID *
EFIAPI
ReallocateCopyPool (
  IN UINTN       AllocationSize,
  IN UINTN       OldSize,
  IN VOID        *Buffer
  )
{
  VOID  *Memory;

  ASSERT (OldSize < AllocationSize);

  Memory = AllocatePool (AllocationSize);
  if ((Buffer != NULL) && (Memory != NULL)) {
    CopyMem (Memory, Buffer, OldSize);
    FreePool (Buffer);
  }

  return Memory;
}

EFI_BOOT_MANAGER_BOOT_OPTION *
EFIAPI
BdsLibBuildOptionFromPath (
  IN OUT  EFI_DEVICE_PATH_PROTOCOL        *DevicePath,
  IN      CHAR16                          *Description,
  IN      UINTN                           BootOrder
  )
{
  EFI_BOOT_MANAGER_BOOT_OPTION      *BootOption;
  UINT16                            DevicePathSize;
  UINTN                             DescriptionSize;
  UINTN                             VariableSize;
  UINT8                             *OptionPtr;
  UINT16                            OptionDevicePathSize;
  UINTN                             OptionDescriptionSize;
  UINT8                             *OptionDevicePath;
  UINT8                             *OptionDescription;
  EFI_STATUS                        Status;

  //
  // The Console variables may have multiple device paths, so make
  // an Entry for each one.
  //
  BootOption = AllocateZeroPool (sizeof (EFI_BOOT_MANAGER_BOOT_OPTION));

  ASSERT (BootOption != NULL);

  BootOption->Attributes = LOAD_OPTION_ACTIVE;
  BootOption->BootCurrent = (UINT16) BootOrder;
  BootOption->OptionalDataSize = 0;
  BootOption->OptionalData   = NULL;

  DevicePathSize = (UINT16)GetDevicePathSize (DevicePath);
  BootOption->FilePath = AllocateZeroPool (DevicePathSize);
  ASSERT (BootOption->FilePath != NULL);

  CopyMem (BootOption->FilePath, DevicePath, DevicePathSize);

  DescriptionSize = StrSize (Description);
  if (DescriptionSize > 0) {
    BootOption->Description = AllocateZeroPool (DescriptionSize);
    ASSERT(BootOption->Description != NULL);
    CopyMem (BootOption->Description, Description, DescriptionSize);
  } else {
    BootOption->Description = NULL;
    DescriptionSize = 0;
  }

  UnicodeSPrint (BootOption->BootCurrentStr, 20, L"Boot%04x", BootOrder);

  //
  // Check BootXXXX variable to see if there's a variable for this boot option.
  //
  OptionPtr = BdsLibGetVariableAndSize (
                BootOption->BootCurrentStr,
                &gEfiGlobalVariableGuid,
                &VariableSize
                );
  if (OptionPtr == NULL ||
      (OptionDevicePathSize = *(UINT16 *)(OptionPtr + sizeof (UINT32))) != DevicePathSize ||
      (OptionDescription = (OptionPtr + sizeof (UINT32) + sizeof (UINT16))) == NULL ||
      (OptionDescriptionSize = StrSize((UINT16*)OptionDescription)) != DescriptionSize ||
      (OptionDevicePath = OptionDescription + OptionDescriptionSize) == NULL ||
      CompareMem (OptionDescription, Description, DescriptionSize) != 0 ||
      CompareMem (OptionDevicePath, DevicePath, DevicePathSize) != 0) {
    //
    // Create a new or update the existing variable with new value
    //
    if (OptionPtr != NULL) {
      FreePool (OptionPtr);
    }
    VariableSize = sizeof(BootOption->Attributes) + sizeof (DevicePathSize) + DescriptionSize + DevicePathSize;
    OptionPtr = AllocateZeroPool (VariableSize);
    *(UINT32 *)OptionPtr = BootOption->Attributes;
    OptionPtr += sizeof (UINT32);
    *(UINT16 *)OptionPtr = DevicePathSize;
    OptionPtr += sizeof (UINT16);
    CopyMem (OptionPtr, Description, DescriptionSize);
    OptionPtr += DescriptionSize;
    CopyMem (OptionPtr, DevicePath, DevicePathSize);

    //
    // Get back to the starting point
    //
    OptionPtr = OptionPtr + DevicePathSize - VariableSize;
    Status = gRT->SetVariable (
                    BootOption->BootCurrentStr,
                    &gEfiGlobalVariableGuid,
                    EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                    VariableSize,
                    OptionPtr
                    );
    if (EFI_ERROR (Status)) {
      FreePool (OptionPtr);
    }

  }

  return BootOption;
}

EFI_STATUS
EFIAPI
BdsLibBootOptionUsb (
  IN OUT UINTN                            *BootOrder
  )
{
  UINTN                             NumberBlockIoHandles;
  EFI_HANDLE                        *BlockIoHandles;
  UINTN                             Index;
  UINTN                             UsbNumber;
  EFI_STATUS                        Result;
  CHAR16                            Buffer[40];
  EFI_BOOT_MANAGER_BOOT_OPTION      *BootOption;
  EFI_DEVICE_PATH_PROTOCOL          *DevicePath;
  UINTN                              DevicePathType;
  EFI_STATUS                        Status;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL   *FfsProtocol;

  //
  // Parse removable media
  //
  BlockIoHandles = NULL;
  gBS->LocateHandleBuffer (
        ByProtocol,
        &gEfiBlockIoProtocolGuid,
        NULL,
        &NumberBlockIoHandles,
        &BlockIoHandles
        );

  Result = EFI_NOT_FOUND;
  UsbNumber = 0;
  for (Index = 0; Index < NumberBlockIoHandles; Index++) {
    Status = gBS->HandleProtocol (BlockIoHandles[Index], &gEfiSimpleFileSystemProtocolGuid, (VOID **)&FfsProtocol);
    if (!EFI_ERROR (Status)) {
      DevicePath  = DevicePathFromHandle (BlockIoHandles[Index]);
      DEBUG((EFI_D_INFO, "USB Path = %s\n", DevicePathToStr (DevicePath)));
      DevicePathType = BdsGetBootTypeFromDevicePath (DevicePath);

      switch (DevicePathType) {
      case BDS_EFI_MESSAGE_USB_DEVICE_BOOT:
        if (UsbNumber != 0) {
          UnicodeSPrint (Buffer, sizeof (Buffer), L"%s %d", L"EFI USB Device", UsbNumber);
        } else {
          UnicodeSPrint (Buffer, sizeof (Buffer), L"%s", L"EFI USB Device");
        }
        BootOption = BdsLibBuildOptionFromPath (DevicePath, Buffer, *BootOrder);
        if (!EFI_ERROR (AppendPickListEntry (BootOption, TRUE))) {
          UsbNumber++;
          (*BootOrder)++;
          Result = EFI_SUCCESS;
          break;
        }
      default:
        break;
      }
    }
  }

  if (BlockIoHandles) {
    FreePool (BlockIoHandles);
  }

  return Result;
}

EFI_STATUS
EFIAPI
BdsLibBootOptionCdDvd (
  IN OUT UINTN                            *BootOrder
  )
{
  UINTN                             NumberBlockIoHandles;
  EFI_HANDLE                        *BlockIoHandles;
  UINTN                             Index;
  UINTN                             CdromNumber;
  EFI_STATUS                        Result;
  EFI_DEVICE_PATH_PROTOCOL          *DevicePath;
  CHAR16                            Buffer[40];
  EFI_BOOT_MANAGER_BOOT_OPTION      *BootOption;
  UINTN                             DevicePathType;
  EFI_STATUS                        Status;
  EFI_BLOCK_IO_PROTOCOL             *BlkIo;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL   *Sfsp;

  gBS->LocateHandleBuffer (
         ByProtocol,
         &gEfiBlockIoProtocolGuid,
         NULL,
         &NumberBlockIoHandles,
         &BlockIoHandles
         );

  Result = EFI_NOT_FOUND;
  CdromNumber = 0;
  for (Index = 0; Index < NumberBlockIoHandles; Index++) {
    Status = gBS->HandleProtocol (
                    BlockIoHandles[Index],
                    &gEfiBlockIoProtocolGuid,
                    (VOID **) &BlkIo
                    );
    if (!EFI_ERROR (Status)) {
      if (!BlkIo->Media->RemovableMedia) {
        //
        // skip the non-removable block devices
        //
        continue;
      }
    }

    DevicePath  = DevicePathFromHandle (BlockIoHandles[Index]);
    DevicePathType = BdsGetBootTypeFromDevicePath (DevicePath);

    switch (DevicePathType) {
    //
    // Assume a removable SATA device should be the DVD/CD device
    //
    case BDS_EFI_MESSAGE_ATAPI_BOOT:
    case BDS_EFI_MESSAGE_SATA_BOOT:
      Status = gBS->HandleProtocol (
                      BlockIoHandles[Index],
                      &gEfiSimpleFileSystemProtocolGuid,
                      (VOID **) &Sfsp
                      );
      if (EFI_ERROR (Status)) {
        //
        //  Skip if the file system handle don't supports a SimpleFileSystem protocol,
        //
        break;
      }

      if (CdromNumber != 0) {
        UnicodeSPrint (Buffer, sizeof (Buffer), L"%s %d", L"EFI DVD/CDROM", CdromNumber);
      } else {
        UnicodeSPrint (Buffer, sizeof (Buffer), L"%s", L"EFI DVD/CDROM");
      }
      DEBUG((EFI_D_INFO, "%s Path = %s\n", Buffer, DevicePathToStr (DevicePath)));
      
      BootOption = BdsLibBuildOptionFromPath (DevicePath, Buffer, *BootOrder);
      //if (BdsBootOptionList != NULL) {
      //  InsertTailList(BdsBootOptionList, &BootOption->Link);
      //} else {
      //  FreePool(BootOption);
      //}
      if (!EFI_ERROR (AppendPickListEntry (BootOption, TRUE))) {
        CdromNumber++;
        (*BootOrder)++;
        Result = EFI_SUCCESS;
      }
      break;
    }
  }

  if (BlockIoHandles != NULL) {
    FreePool (BlockIoHandles);
  }

  return Result;
}


EFI_STATUS
EFIAPI
BdsLibBootOptionNetwork (
  IN OUT UINTN                            *BootOrder
  )
{
  EFI_HANDLE                        *LoadFileHandles;
  UINTN                             Index;
  UINTN                             NumOfLoadFileHandles;
  UINTN                             EthernetAdapterNumber;
  EFI_STATUS                        Status;
  EFI_SIMPLE_NETWORK_PROTOCOL       *Snp;
  EFI_STATUS                        Result;
  EFI_DEVICE_PATH_PROTOCOL          *DevicePath;
  CHAR16                            Buffer[40];
  EFI_BOOT_MANAGER_BOOT_OPTION      *BootOption;
  UINTN                             EthernetAdapterCount;
  NETWORK_ADAPTER                   *EthernetAdapter;
  EFI_DEVICE_PATH_PROTOCOL          *TempDevicePath;
  UINTN                             SubType;
  UINTN                             PathLength;

  //
  // Search Load File protocol for PXE boot option.
  //
  LoadFileHandles = NULL;
  gBS->LocateHandleBuffer (
        ByProtocol,
        &gEfiLoadFileProtocolGuid,
        NULL,
        &NumOfLoadFileHandles,
        &LoadFileHandles
        );

  //
  // Get the buffer space for the network adapters
  //
  EthernetAdapterCount = 0;
  EthernetAdapter = NULL;
  if ( 0 != NumOfLoadFileHandles ) {
    EthernetAdapter = AllocatePool ( NumOfLoadFileHandles * sizeof ( NETWORK_ADAPTER ));
    if ( NULL == EthernetAdapter ) {
      FreePool ( LoadFileHandles );
      NumOfLoadFileHandles = 0;
      LoadFileHandles = NULL;
    }
  }

  //
  // Create the boot entries for the adapters/protocols found
  //
  Result = EFI_NOT_FOUND;
  for (Index = 0; Index < NumOfLoadFileHandles; Index++) {
    Status = gBS->HandleProtocol (LoadFileHandles[Index], &gEfiSimpleNetworkProtocolGuid, (VOID **)&Snp);
    if (EFI_ERROR(Status)) {
      continue;
    }
    DevicePath = DevicePathFromHandle (LoadFileHandles[Index]);
    if (DevicePath == NULL) {
      continue;
    }

    DEBUG ((DEBUG_INFO, "SNP Found.  Index = %d.  DevPath = %s\n", Index, DevicePathToStr (DevicePath)));
    
    //
    //  Walk the device path to determine the network number
    //
    SubType = 0;
    PathLength = 0;
    TempDevicePath = DevicePath;
    while (!IsDevicePathEndType (TempDevicePath)) {
      if (DevicePathType (TempDevicePath) == MESSAGING_DEVICE_PATH) {
        SubType = DevicePathSubType (TempDevicePath);
        if (SubType == MSG_IPv4_DP || SubType == MSG_IPv6_DP) {
          PathLength = (UINT8 *)TempDevicePath - (UINT8 *)DevicePath;
          break;
        }
      }
      
      //
      //  Set the next portion of the device path
      //
      TempDevicePath = NextDevicePathNode (TempDevicePath);
    }
    
    if (PathLength == 0) {
      PathLength = (UINT8 *)TempDevicePath - (UINT8 *)DevicePath;
    }
      
    //
    //  Determine the network number
    //
    for (EthernetAdapterNumber = 0; EthernetAdapterNumber < EthernetAdapterCount; EthernetAdapterNumber++) {
      if ((EthernetAdapter[EthernetAdapterNumber].DevicePathLength == PathLength) &&
          (CompareMem (EthernetAdapter[EthernetAdapterNumber].DevicePath, DevicePath,PathLength) == 0)) {
        //
        //  Network adapter found
        //
        DEBUG ((DEBUG_INFO, "Found adapter ETH%d\n", EthernetAdapterNumber));
        break;          
      }
    }
    if (EthernetAdapterNumber >= EthernetAdapterCount) {
      //
      //  Remember the next adapter
      //
      DEBUG ((
        DEBUG_INFO,
        "Adapter ETH%d, %d bytes: %s\n",
        EthernetAdapterCount,
        PathLength,
        DevicePathToStr (DevicePath)
        ));
      EthernetAdapter[EthernetAdapterCount].DevicePath       = DevicePath;
      EthernetAdapter[EthernetAdapterCount].DevicePathLength = PathLength;
      EthernetAdapterCount += 1;
    }

    //
    //  Build the network name
    //
    switch (SubType) {
    case MSG_IPv4_DP:
      //
      // The IPv4 Handle with Load File from NetworkPkg PXE Base Code driver is not working
      //
      UnicodeSPrint (Buffer, sizeof (Buffer), L"ETH%d - IPv4", EthernetAdapterNumber);
      break;
    case MSG_IPv6_DP:
      UnicodeSPrint (Buffer, sizeof (Buffer), L"ETH%d - IPv6", EthernetAdapterNumber);
      break;
    default:
      //
      // The Handle with Load File from MdeModulePkg PXE Base Code driver is working
      //
      UnicodeSPrint (Buffer, sizeof (Buffer), L"ETH%d - IPv4", EthernetAdapterNumber);
      break;
    }
    DEBUG((EFI_D_INFO, "Network Path = %s\r\n", Buffer));

    //
    //  Build the boot option
    //
    BootOption = BdsLibBuildOptionFromPath (DevicePath, Buffer, *BootOrder);
    if (!EFI_ERROR (AppendPickListEntry (BootOption, TRUE))) {
     (*BootOrder)++;
      Result = EFI_SUCCESS;
    }
  }

  if ( NULL != EthernetAdapter ) {
    FreePool ( EthernetAdapter );
  }
  if (LoadFileHandles) {
    FreePool (LoadFileHandles);
  }

  return Result;
}


EFI_STATUS
EFIAPI
BdsLibBootOptionHardDisk (
  IN OUT UINTN                            *BootOrder
  )
{
  UINTN                                 NumberBlockIoHandles;
  EFI_HANDLE                            *BlockIoHandles;
  EFI_BLOCK_IO_PROTOCOL                 *BlkIo;
  UINTN                                 Index;
  EFI_DEVICE_PATH_PROTOCOL              *DevicePath;
  EFI_STATUS                            Result;
  EFI_STATUS                            Status;
  UINTN                                 HdNumber;
  CHAR16                                Buffer[40];
  EFI_BOOT_MANAGER_BOOT_OPTION          *BootOption;
  UINTN                                 DevicePathType;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL       *Sfsp;
  EFI_IMAGE_OPTIONAL_HEADER_PTR_UNION   Hdr;
  EFI_IMAGE_OPTIONAL_HEADER_UNION       HdrData;
  EFI_IMAGE_DOS_HEADER                  DosHeader;
  EFI_DEVICE_PATH_PROTOCOL              *FilePath;

  BlockIoHandles = NULL;
  ZeroMem (Buffer, sizeof(Buffer));
  gBS->LocateHandleBuffer (
         ByProtocol,
         &gEfiBlockIoProtocolGuid,
         NULL,
         &NumberBlockIoHandles,
         &BlockIoHandles
         );

  Result = EFI_NOT_FOUND;
  HdNumber = 0;
  for (Index = 0; Index < NumberBlockIoHandles; Index++) {
    Status = gBS->HandleProtocol (
                    BlockIoHandles[Index],
                    &gEfiBlockIoProtocolGuid,
                    (VOID **) &BlkIo
                    );
    //
    // Ignore error
    //
    if (EFI_ERROR(Status)) {
      continue;
    }

    DevicePath  = DevicePathFromHandle (BlockIoHandles[Index]);
    DEBUG((EFI_D_INFO, "HardDisk Path = %s\n", DevicePathToStr (DevicePath)));
    DevicePathType = BdsGetBootTypeFromDevicePath (DevicePath);
    DEBUG ((EFI_D_INFO, "HardDisk DevicePathType=%x\n", DevicePathType));

    //
    // Ignore removable media
    //
    if (BlkIo->Media->RemovableMedia == TRUE && DevicePathType != BDS_EFI_MEDIA_HD_BOOT) {
      DEBUG ((EFI_D_INFO, "Ignored because it is removable\r\n" ));
      continue;
    }

    switch (DevicePathType) {
      case BDS_EFI_MESSAGE_ATAPI_BOOT:
      case BDS_EFI_MESSAGE_SATA_BOOT:
      case BDS_EFI_MEDIA_HD_BOOT:

      Status = gBS->HandleProtocol (
                      BlockIoHandles[Index],
                      &gEfiSimpleFileSystemProtocolGuid,
                      (VOID **) &Sfsp
                      );
      if (EFI_ERROR (Status)) {
        //
        //  Skip if the file system handle don't supports a SimpleFileSystem protocol,
        //
        break;
      }

      FilePath = FileDevicePath (BlockIoHandles[Index], EFI_REMOVABLE_MEDIA_FILE_NAME);
      DEBUG((EFI_D_INFO, "FILE PATH = %s\n", DevicePathToStr (FilePath)));
      Hdr.Union  = &HdrData;
      Status     = BdsLibGetImageHeader (
                     BlockIoHandles[Index],
                     EFI_REMOVABLE_MEDIA_FILE_NAME,
                     &DosHeader,
                     Hdr
                     );
      if (!EFI_ERROR (Status) &&
          EFI_IMAGE_MACHINE_TYPE_SUPPORTED (Hdr.Pe32->FileHeader.Machine) &&
          Hdr.Pe32->OptionalHeader.Subsystem == EFI_IMAGE_SUBSYSTEM_EFI_APPLICATION) {

        if (HdNumber != 0) {
          UnicodeSPrint (Buffer, sizeof (Buffer), L"%s %d", L"EFI HD Device", HdNumber);
        } else {
          UnicodeSPrint (Buffer, sizeof (Buffer), L"%s", L"EFI HD Device");
        }
      
        DEBUG ((EFI_D_INFO, "********* Hd String Buffer=%s\n", Buffer));
        BootOption = BdsLibBuildOptionFromPath (DevicePath, Buffer, *BootOrder);

        if (!EFI_ERROR(AppendPickListEntry (BootOption, TRUE))) {
          HdNumber++;
          (*BootOrder)++;
          Result = EFI_SUCCESS;
        }
      }

      break;
    default:
      break;
    }
  }

  if (BlockIoHandles != NULL) {
    FreePool (BlockIoHandles);
  }

  return Result;
}


EFI_STATUS
EFIAPI
PlatformBdsGetBootOptions (
  IN OUT UINTN                           *BootOptionCount,
  IN     BOOLEAN                          ReEnumerate
  )
{
  UINTN                             Order;
  UINT16                            Index;
  UINT16                            OptionAccounts;
  EFI_STATUS                        Status;
  PLATFORM_PCD_BOOT_OPTION_TYPE     BootOrder[BOOTORDERMAX];
  UINT16                            *BootOrderVarData;
  UINTN                              BootOrderSize;

  EFI_BOOT_MANAGER_BOOT_OPTION      *BootOption;
  CHAR16            BootOptionName[20];

  //
  // Try to find boot options from variable
  //
  BootOption = NULL;
  if (!ReEnumerate) {
    //
    // Read the BootOrder
    //
    BootOrderVarData = BdsLibGetVariableAndSize (
                         L"BootOrder",
                         &gEfiGlobalVariableGuid,
                         &BootOrderSize
                         );

    if (BootOrderVarData != NULL) {
      ZeroMem (BootOptionName, sizeof (BootOptionName));
      for (Index = 0; Index < BootOrderSize / sizeof (UINT16); Index++) {
        UnicodeSPrint (BootOptionName, sizeof (BootOptionName), L"Boot%04x", BootOrderVarData[Index]);
        BootOption = MinuteBdsLibVariableToOption (NULL, BootOptionName);
        if (BootOption != NULL) {
        if (!AppendPickListEntry (BootOption, TRUE)) {
          (*BootOptionCount)++;
        }
      }
    }

    FreePool (BootOrderVarData);

    if (*BootOptionCount > 0)
      return EFI_SUCCESS;
    }
  }

  //
  // Initialize BootOrder array.
  //
  for (Index = 0; Index < BOOTORDERMAX; ++Index) {
    BootOrder[Index] = Index;
  }

  if (PcdGetBool(PcdBootOrderPolicyEnable) == TRUE) {
    //
    // Disable all boot selections
    //
    for (Index = 0; Index < BOOTORDERMAX; ++Index) {
      BootOrder[Index] = BOOTORDERSKIP;
    }

    //
    // Get the User defined BootOrder via PCD
    //
    Order = PcdGet16 (PcdBootOrderHD);
    if (Order < BOOTORDERMAX) {
      BootOrder[Order] = BOOTORDERHD;
    }

    Order = PcdGet16 (PcdBootOrderPayload);
    if (Order < BOOTORDERMAX) {
      BootOrder[Order] = BOOTORDERMEMD;
    }

    Order = PcdGet16 (PcdBootOrderDvdCd);
    if (Order < BOOTORDERMAX) {
      BootOrder[Order] = BOOTORDERDVDCD;
    }

    Order = PcdGet16 (PcdBootOrderUsb);
    if (Order < BOOTORDERMAX) {
      BootOrder[Order] = BOOTORDERUSB;
    }

    //Order = PcdGet16 (PcdBootOrderFloppy);
    //if (Order < BOOTORDERMAX) {
    //  BootOrder[Order] = BOOTORDERFLOPPY;
    //}

    //Order = PcdGet16 (PcdBootOrderScsi);
    //if (Order < BOOTORDERMAX) {
    //  BootOrder[Order] = BOOTORDERSCSI;
    //}

    Order = PcdGet16 (PcdBootOrderNetwork);
    if (Order < BOOTORDERMAX) {
      BootOrder[Order] = BOOTORDERNETWORK;
    }
  }

  //
  // Search legacy boot options
  //
  Order = *BootOptionCount;
  DEBUG ((EFI_D_INFO, "***** Initial****Boot Option Order=%x****\n", Order));

  //
  // Find concrete EFI boot option(s) for each boot type
  //
  for (Index = 0; Index < BOOTORDERMAX; ++Index) {
    DEBUG (( DEBUG_INFO, "Index: %d\r\n", Index ));
    DEBUG (( DEBUG_INFO, "Boot Device Type: %d\r\n", BootOrder [ Index ]));
    switch (BootOrder[Index]) {
    case BOOTORDERNETWORK:
      DEBUG ((EFI_D_INFO, "*****Network****Boot Option Order=%x\n", Order));
      Status = BdsLibBootOptionNetwork(&Order);
      break;

    case BOOTORDERMEMD:
      DEBUG ((EFI_D_INFO, "*****Payload****Boot Option Order=%x\n", Order));
      AddLoadFileBootOptions ();
      if (mDefaultBootOptionsCount != 0) {
        for (OptionAccounts = 0; OptionAccounts <mDefaultBootOptionsCount; OptionAccounts++) {
          BootOption = BdsLibBuildOptionFromPath (mDefaultBootOptions[OptionAccounts].FilePath, mDefaultBootOptions[OptionAccounts].Description, Order);
          FreePool (BootOption);
          DEBUG (( DEBUG_INFO, "Order: %x\r\n", Order ));
          if (!AppendPickListEntry (&mDefaultBootOptions[OptionAccounts], TRUE)) {
            Order++;
          }
        }
      }
      break;

    case BOOTORDERHD:
      DEBUG ((EFI_D_INFO, "*****HardDisk****Boot Option Order=%x\n", Order));
      Status = BdsLibBootOptionHardDisk(&Order);
      break;

    //case BOOTORDERFLOPPY:
    //  DEBUG ((EFI_D_INFO, "*****Floppy****Boot Option Order=%x\n", Order));
    //  Status = BdsLibBootOptionFloppy(&Order);
    //  break;

    case BOOTORDERDVDCD:
      DEBUG ((EFI_D_INFO, "*****CdDvd****Boot Option Order=%x\n", Order));
      Status = BdsLibBootOptionCdDvd(&Order);
      break;

    //case BOOTORDERSCSI:
    //  DEBUG ((EFI_D_INFO, "*****SCSI****Boot Option Order=%x\n", Order));
    //  Status = BdsLibBootOptionScsi(&Order);
    //  break;

    case BOOTORDERUSB:
      DEBUG ((EFI_D_INFO, "*****USB****Boot Option Order=%x\n", Order));
      Status = BdsLibBootOptionUsb(&Order);
      break;

    case BOOTORDERSKIP:
      DEBUG ((EFI_D_INFO, "*****Skip****Boot Option Order=%x\n", Order));
      Status = EFI_NOT_FOUND;
      break;

    default:
      DEBUG ((EFI_D_INFO, "*****Not Found****Boot Option Order=%x\n", Order));
      Status = EFI_NOT_FOUND;
      break;
    }
  }

  *BootOptionCount = Order;
  DEBUG ((EFI_D_INFO, "***** Finished****Boot Option Order=%x\n", Order));

  BootOrderVarData = NULL;
  if (Order > 0) {
    BootOrderVarData = AllocateZeroPool (sizeof(UINT16) * Order);

    for (Index = 0; Index < Order; ++Index) {
      BootOrderVarData[Index] = Index;
    }
  }

  //
  // Write 'BootOrder' variable
  //
  Status = gRT->SetVariable (
                L"BootOrder",
                &gEfiGlobalVariableGuid,
                EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                Order * sizeof (UINT16),
                BootOrderVarData
                );
  //
  // There must be boot options in variable
  // Status = BdsLibBuildOptionFromVar (BootOptionList, L"BootOrder");
  //
  return Status;
}


/** @file
  Generic BDS for EDK II.

  Platform customizations are done via BdsPlatformLib. The User Interface (UI) is
  implemented via the BdsUiLib. The design goal is to not have to modify this driver
  that follows the UEFI and PI specification required behavior. The customization can
  be done in the Platform and UI librarys. Also having a seperate UI library allows
  a single platform so to support multiple user interface flavors.

  Copyright (c) 2008, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PiDxe.h>

#include <Protocol/Bds.h>
#include <Guid/Performance.h>
#include <Protocol/SerialIo.h>
#include <Protocol/SimpleTextOut.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/SimpleTextInEx.h>
#include <Protocol/SimplePointer.h>
#include <Protocol/GenericMemoryTest.h>
#include <Protocol/AcpiS3Save.h>
#include <Protocol/SmmAccess2.h>
#include <Protocol/DxeSmmReadyToLock.h>

#include <Guid/GlobalVariable.h>

#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/HobLib.h>
#include <Library/DevicePathLib.h>
#include <Library/PcdLib.h>
#include <Library/PerformanceLib.h>
#include <Library/PrintLib.h>
#include <Library/CapsuleLib.h>

#include <IndustryStandard/Pci.h>

#include "BdsLib.h"
#include "BdsUiLib.h"
#include "BdsUtility.h"

#include "BdsPlatformLib.h"

///
/// The size of a 3 character ISO639 language code.
///
#define ISO_639_2_ENTRY_SIZE    3

BOOLEAN   gConnectAllHappened = FALSE;
EFI_SIMPLE_TEXT_OUTPUT_MODE mBdsTextOutMode = {
  1,
  0,
  0,
  0,
  0,
  FALSE
};

/**
  Bds Dummy SimpleTextOutput protocol interface.

  @param  This                     Protocol instance pointer.
  @param  ExtendedVerification     Driver may perform more exhaustive verfication
                                   operation of the device during reset.

  @retval EFI_SUCCESS              Always return EFI_SUCCESS.

**/
EFI_STATUS
EFIAPI
BdsTextOutReset (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *This,
  IN  BOOLEAN                            ExtendedVerification
  )
{

  mBdsTextOutMode.Mode = 0;
  mBdsTextOutMode.CursorColumn = 0;
  mBdsTextOutMode.CursorRow    = 0;
  mBdsTextOutMode.CursorVisible = TRUE;
  return EFI_SUCCESS;
}


/**
  Bds Dummy SimpleTextOutput protocol interface.

  @param  This                     Protocol instance pointer.
  @param  WString                  The NULL-terminated Unicode string to be
                                   displayed on the output device(s). All output
                                   devices must also support the Unicode drawing
                                   defined in this file.

  @retval EFI_SUCCESS              Always return EFI_SUCCESS.

**/
EFI_STATUS
EFIAPI
BdsTextOutOutputString (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *This,
  IN  CHAR16                             *WString
  )
{
  return EFI_SUCCESS;
}


/**
  Bds Dummy SimpleTextOutput protocol interface.

  @param  This                     Protocol instance pointer.
  @param  WString                  The NULL-terminated Unicode string to be
                                   examined for the output device(s).

  @retval EFI_SUCCESS              Always return EFI_SUCCESS.


**/
EFI_STATUS
EFIAPI
BdsTextOutTestString (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *This,
  IN  CHAR16                             *WString
  )
{
  return EFI_SUCCESS;
}


/**
  Bds Dummy SimpleTextOutput protocol interface.

  @param  This                     Protocol instance pointer.
  @param  ModeNumber               The mode number to return information on.
  @param  Columns                  Returns the columns of the text output device
                                   for the requested ModeNumber.
  @param  Rows                     Returns the rows of the text output device
                                   for the requested ModeNumber.

  @retval EFI_SUCCESS              Always return EFi_SUCCESS.


**/
EFI_STATUS
EFIAPI
BdsTextOutQueryMode (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *This,
  IN  UINTN                              ModeNumber,
  OUT UINTN                              *Columns,
  OUT UINTN                              *Rows
  )
{
  //
  // Check whether param ModeNumber is valid.
  // ModeNumber should be within range 0 ~ MaxMode - 1.
  //
  if ( (ModeNumber > (UINTN)(((UINT32)-1)>>1)) ) {
    return EFI_UNSUPPORTED;
  }

  if ((INT32) ModeNumber >= This->Mode->MaxMode) {
    return EFI_UNSUPPORTED;
  }

  *Columns  = 80;
  *Rows     = 25;
  return EFI_SUCCESS;
}


/**
  Bds Dummy SimpleTextOutput protocol interface.

  @param  This                     Protocol instance pointer.
  @param  ModeNumber               The mode number to set.

  @retval EFI_SUCCESS              Always return EFi_SUCCESS.

**/
EFI_STATUS
EFIAPI
BdsTextOutSetMode (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *This,
  IN  UINTN                              ModeNumber
  )
{
  if ( (ModeNumber > (UINTN)(((UINT32)-1)>>1)) ) {
    return EFI_UNSUPPORTED;
  }

  if ((INT32) ModeNumber >= This->Mode->MaxMode) {
    return EFI_UNSUPPORTED;
  }

  mBdsTextOutMode.Mode = (INT32)ModeNumber;
  mBdsTextOutMode.CursorColumn = 0;
  mBdsTextOutMode.CursorRow    = 0;
  mBdsTextOutMode.CursorVisible = TRUE;

  return EFI_SUCCESS;
}


/**
  Bds Dummy SimpleTextOutput protocol interface.

  @param  This                     Protocol instance pointer.
  @param  Attribute                The attribute to set. Bits 0..3 are the
                                   foreground color, and bits 4..6 are the
                                   background color. All other bits are undefined
                                   and must be zero. The valid Attributes are
                                   defined in this file.

  @retval EFI_SUCCESS              Always return EFI_SUCCESS.

**/
EFI_STATUS
EFIAPI
BdsTextOutSetAttribute (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *This,
  IN  UINTN                              Attribute
  )
{
  mBdsTextOutMode.Attribute = (INT32)Attribute;
  return EFI_SUCCESS;
}


/**
  Bds Dummy SimpleTextOutput protocol interface.

  @param  This                     Protocol instance pointer.

  @retval EFI_SUCCESS              Always return EFI_SUCCESS.

**/
EFI_STATUS
EFIAPI
BdsTextOutClearScreen (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *This
  )
{

  mBdsTextOutMode.CursorColumn = 0;
  mBdsTextOutMode.CursorRow    = 0;
  mBdsTextOutMode.CursorVisible = TRUE;
  return EFI_SUCCESS;
}


/**
  Bds Dummy SimpleTextOutput protocol interface.

  @param  This                     Protocol instance pointer.
  @param  Column                   The column position to set the cursor to. Must be
                                   greater than or equal to zero and less than the
                                   number of columns by QueryMode ().
  @param  Row                      The row position to set the cursor to. Must be
                                   greater than or equal to zero and less than the
                                   number of rows by QueryMode ().

  @retval EFI_SUCCESS              Always return EFI_SUCCESS.

**/
EFI_STATUS
EFIAPI
BdsTextOutSetCursorPosition (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *This,
  IN  UINTN                              Column,
  IN  UINTN                              Row
  )
{
  if (Column >= 80 || Row >= 25) {
    return EFI_UNSUPPORTED;
  }

  mBdsTextOutMode.CursorColumn = (INT32) Column;
  mBdsTextOutMode.CursorRow    = (INT32) Row;
  return EFI_SUCCESS;
}


/**
  Bds Dummy SimpleTextOutput protocol interface.

  @param  This                     Protocol instance pointer.
  @param  Visible                  If TRUE, the cursor is set to be visible. If
                                   FALSE, the cursor is set to be invisible.

  @retval EFI_SUCCESS              Always return EFI_SUCCESS.
**/
EFI_STATUS
EFIAPI
BdsTextOutEnableCursor (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *This,
  IN  BOOLEAN                            Visible
  )
{
   mBdsTextOutMode.CursorVisible = Visible;
  return EFI_SUCCESS;
}

/**
  Bds Dummy SimpleTextInput protocol interface.

  @param  This                     Protocol instance pointer.
  @param  ExtendedVerification     Driver may perform diagnostics on reset.

  @retval EFI_SUCCESS              Always return EFI_SUCCESS.

**/
EFI_STATUS
EFIAPI
BdsTextInReset (
  IN  EFI_SIMPLE_TEXT_INPUT_PROTOCOL  *This,
  IN  BOOLEAN                         ExtendedVerification
  )
{
  return EFI_SUCCESS;
}

/**
  Bds Dummy SimpleTextInput protocol interface.

  @param  This                     Protocol instance pointer.
  @param  Key                      Driver may perform diagnostics on reset.

  @retval EFI_SUCCESS              Always return EFI_SUCCESS.

**/
EFI_STATUS
EFIAPI
BdsTextInReadKeyStroke (
  IN  EFI_SIMPLE_TEXT_INPUT_PROTOCOL  *This,
  OUT EFI_INPUT_KEY                   *Key
  )
{
  return EFI_SUCCESS;
}

/**
  Bds Dummy SimpleTextInputEx protocol interface.

  @param  This                     Protocol instance pointer.
  @param  ExtendedVerification     Driver may perform diagnostics on reset.

  @retval EFI_SUCCESS              Always return EFI_SUCCESS.
**/
EFI_STATUS
EFIAPI
BdsTextInResetEx (
  IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL  *This,
  IN BOOLEAN                            ExtendedVerification
  )
{
  return EFI_SUCCESS;
}


/**
  Bds Dummy SimpleTextInputEx protocol interface.

  @param  This                     Protocol instance pointer.
  @param  KeyData                  A pointer to a buffer that is filled in with the
                                   keystroke state data for the key that was
                                   pressed.

  @retval EFI_SUCCESS              Always return EFI_SUCCESS.

**/
EFI_STATUS
EFIAPI
BdsTextInReadKeyStrokeEx (
  IN  EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *This,
  OUT EFI_KEY_DATA                      *KeyData
  )
{
  return EFI_SUCCESS;
}

/**
  Set certain state for the input device.

  @param  This                     Protocol instance pointer.
  @param  KeyToggleState           A pointer to the EFI_KEY_TOGGLE_STATE to set the
                                   state for the input device.

  @retval EFI_SUCCESS              The device state was set successfully.
  @retval EFI_DEVICE_ERROR         The device is not functioning correctly and
                                   could not have the setting adjusted.
  @retval EFI_UNSUPPORTED          The device does not have the ability to set its
                                   state.
  @retval EFI_INVALID_PARAMETER    KeyToggleState is NULL.

**/
EFI_STATUS
EFIAPI
BdsTextInSetState (
  IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL  *This,
  IN EFI_KEY_TOGGLE_STATE               *KeyToggleState
  )
{
  return EFI_SUCCESS;
}

/**
  Register a notification function for a particular keystroke for the input device.

  @param  This                     Protocol instance pointer.
  @param  KeyData                  A pointer to a buffer that is filled in with the
                                   keystroke information data for the key that was
                                   pressed.
  @param  KeyNotificationFunction  Points to the function to be called when the key
                                   sequence is typed specified by KeyData.
  @param  NotifyHandle             Points to the unique handle assigned to the
                                   registered notification.

  @retval EFI_SUCCESS              The notification function was registered
                                   successfully.
  @retval EFI_OUT_OF_RESOURCES     Unable to allocate resources for necesssary data
                                   structures.
  @retval EFI_INVALID_PARAMETER    KeyData or KeyNotificationFunction or NotifyHandle is NULL.

**/
EFI_STATUS
EFIAPI
BdsTextInRegisterKeyNotify (
  IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL  *This,
  IN EFI_KEY_DATA                       *KeyData,
  IN EFI_KEY_NOTIFY_FUNCTION            KeyNotificationFunction,
  OUT EFI_HANDLE                        *NotifyHandle
  )
{
  return EFI_SUCCESS;
}

/**
  Remove a registered notification function from a particular keystroke.

  @param  This                     Protocol instance pointer.
  @param  NotificationHandle       The handle of the notification function being
                                   unregistered.

  @retval EFI_SUCCESS              The notification function was unregistered
                                   successfully.
  @retval EFI_INVALID_PARAMETER    The NotificationHandle is invalid.

**/
EFI_STATUS
EFIAPI
BdsTextInUnregisterKeyNotify (
  IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL  *This,
  IN EFI_HANDLE                         NotificationHandle
  )
{
  return EFI_SUCCESS;
}

EFI_SIMPLE_TEXT_INPUT_PROTOCOL mBdsDummyConIn = {
  BdsTextInReset,
  BdsTextInReadKeyStroke,
  (EFI_EVENT) NULL
};

EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL mBdsDummyConInEx = {
  BdsTextInResetEx,
  BdsTextInReadKeyStrokeEx,
  (EFI_EVENT) NULL,
  BdsTextInSetState,
  BdsTextInRegisterKeyNotify,
  BdsTextInUnregisterKeyNotify
};

EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL mBdsDummyConOut = {
  BdsTextOutReset,
  BdsTextOutOutputString,
  BdsTextOutTestString,
  BdsTextOutQueryMode,
  BdsTextOutSetMode,
  BdsTextOutSetAttribute,
  BdsTextOutClearScreen,
  BdsTextOutSetCursorPosition,
  BdsTextOutEnableCursor,
  (EFI_SIMPLE_TEXT_OUTPUT_MODE *) &mBdsTextOutMode
};


VOID
BdsPrintMemoryMap (
  UINT16        *PrintString
  )
{
  EFI_STATUS  Status;


  UINTN                         EfiMemoryMapSize;
  EFI_MEMORY_DESCRIPTOR         *EfiMemoryMap;
  EFI_MEMORY_DESCRIPTOR         *CopyEfiMemoryMap;
  UINTN                         EfiMapKey;
  UINTN                         EfiDescriptorSize;
  UINT32                        EfiDescriptorVersion;

  EfiMemoryMapSize  = 0;
  EfiMemoryMap      = NULL;
  Status = gBS->GetMemoryMap (
                  &EfiMemoryMapSize,
                  EfiMemoryMap,
                  &EfiMapKey,
                  &EfiDescriptorSize,
                  &EfiDescriptorVersion
                  );
  ASSERT (Status == EFI_BUFFER_TOO_SMALL);

  do {
    //
    // Use size returned back plus 1 descriptor for the AllocatePool.
    // We don't just multiply by 2 since the "for" loop below terminates on
    // EfiMemoryMapEnd which is dependent upon EfiMemoryMapSize. Otherwize
    // we process bogus entries and create bogus E820 entries.
    //
    EfiMemoryMap = (EFI_MEMORY_DESCRIPTOR *) AllocatePool (EfiMemoryMapSize);
    ASSERT (EfiMemoryMap != NULL);
    Status = gBS->GetMemoryMap (
                    &EfiMemoryMapSize,
                    EfiMemoryMap,
                    &EfiMapKey,
                    &EfiDescriptorSize,
                    &EfiDescriptorVersion
                    );
    if (EFI_ERROR (Status)) {
      FreePool (EfiMemoryMap);
    }
  } while (Status == EFI_BUFFER_TOO_SMALL);
  ASSERT_EFI_ERROR (Status);

  DEBUG ((EFI_D_ERROR, "****EfiDecriptorSize=%x****\n", EfiDescriptorSize));
  DEBUG ((EFI_D_ERROR, "****EfiMemoryMapSize=%lx****\n", EfiMemoryMapSize));
  CopyEfiMemoryMap = EfiMemoryMap;

  for (;(UINT8*) CopyEfiMemoryMap < ((UINT8 *)EfiMemoryMap + EfiMemoryMapSize);) {
    DEBUG ((EFI_D_ERROR, "***********MemoryMay in %s ******************!\n", PrintString));
    DEBUG ((EFI_D_ERROR, "****Type=%x****\n", CopyEfiMemoryMap->Type));
    DEBUG ((EFI_D_ERROR, "****PhysicalStart=%lx****\n", CopyEfiMemoryMap->PhysicalStart));
    DEBUG ((EFI_D_ERROR, "****VirtualStart=%lx****\n", CopyEfiMemoryMap->VirtualStart));
    DEBUG ((EFI_D_ERROR, "****Attribute=%lx****\n", CopyEfiMemoryMap->Attribute));
    DEBUG ((EFI_D_ERROR, "****NumberOfPages=%lx****\n", CopyEfiMemoryMap->NumberOfPages));
    CopyEfiMemoryMap = (EFI_MEMORY_DESCRIPTOR*)((UINT8*)CopyEfiMemoryMap + EfiDescriptorSize);
  }
  FreePool (EfiMemoryMap);
}


//
//
//
VOID
EFIAPI
BdsEntry (
  IN EFI_BDS_ARCH_PROTOCOL  *This
  );


EFI_STATUS
EFIAPI
BdsProcessCapsules (
 EFI_BOOT_MODE BootMode
  );


//
// Template for BDS Architectural Protocol. Global template is a size optimization
//
EFI_HANDLE            mHandle = NULL;
EFI_BDS_ARCH_PROTOCOL mBds    = {
  BdsEntry
};

//
// Event is global as a size reduction
//
EFI_EVENT                     mReadyToBootEvent;

//
// Global variable for the EFI multi instance device paths for the console devices
// The BDS intitializes these values when its entry point is called.
//
EFI_DEVICE_PATH_PROTOCOL      *gConInDevices  = NULL;
EFI_DEVICE_PATH_PROTOCOL      *gConOutDevices = NULL;
EFI_DEVICE_PATH_PROTOCOL      *gErrOutDevices = NULL;

VOID
InstallReadyToLock (
  VOID
  )
{
  EFI_STATUS                Status;
  EFI_HANDLE                Handle;
  EFI_SMM_ACCESS2_PROTOCOL  *SmmAccess;
  EFI_ACPI_S3_SAVE_PROTOCOL *AcpiS3Save;

  //
  // Install DxeSmmReadyToLock protocol prior to the processing of boot options
  //
  Status = gBS->LocateProtocol (&gEfiSmmAccess2ProtocolGuid, NULL, (VOID **) &SmmAccess);
  if (!EFI_ERROR (Status)) {

    //
    // Prepare S3 information, this MUST be done before DxeSmmReadyToLock
    //
    Status = gBS->LocateProtocol (&gEfiAcpiS3SaveProtocolGuid, NULL, (VOID **)&AcpiS3Save);
    if (!EFI_ERROR (Status)) {
      AcpiS3Save->S3Save (AcpiS3Save, NULL);
    }

    Handle = NULL;
    Status = gBS->InstallProtocolInterface (
                    &Handle,
                    &gEfiDxeSmmReadyToLockProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    NULL
                    );
    ASSERT_EFI_ERROR (Status);
  }

  return ;
}

/**
  Perform the memory test base on the memory test intensive level,
  and update the memory resource.

  @param  Level         The memory test intensive level.

  @retval EFI_STATUS    Success test all the system memory and update
                        the memory resource

**/
EFI_STATUS
EFIAPI
BdsMemoryTest (
  IN EXTENDMEM_COVERAGE_LEVEL Level
  )
{
  EFI_STATUS                        Status;
  EFI_STATUS                        KeyStatus;
  EFI_STATUS                        InitStatus;
  EFI_STATUS                        ReturnStatus;
  BOOLEAN                           RequireSoftECCInit;
  EFI_GENERIC_MEMORY_TEST_PROTOCOL  *GenMemoryTest;
  UINT64                            TestedMemorySize;
  UINT64                            TotalMemorySize;
  UINTN                             TestPercent;
  UINT64                            PreviousValue;
  BOOLEAN                           ErrorOut;
  BOOLEAN                           TestAbort;
  EFI_INPUT_KEY                     Key;
  CHAR16                            StrPercent[80];
  CHAR16                            *StrTotalMemory;
  CHAR16                            *Pos;
  //CHAR16                            *TmpStr;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL     Foreground;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL     Background;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL     Color;
  UINT32                            TempData;

  ReturnStatus = EFI_SUCCESS;
  ZeroMem (&Key, sizeof (EFI_INPUT_KEY));

  Pos = AllocatePool (128);

  if (Pos == NULL) {
    return ReturnStatus;
  }

  StrTotalMemory    = Pos;

  TestedMemorySize  = 0;
  TotalMemorySize   = 0;
  PreviousValue     = 0;
  ErrorOut          = FALSE;
  TestAbort         = FALSE;

  SetMem (&Foreground, sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL), 0xff);
  SetMem (&Background, sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL), 0x0);
  SetMem (&Color, sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL), 0xff);

  RequireSoftECCInit = FALSE;

  Status = gBS->LocateProtocol (
                  &gEfiGenericMemTestProtocolGuid,
                  NULL,
                  (VOID **) &GenMemoryTest
                  );
  if (EFI_ERROR (Status)) {
    FreePool (Pos);
    return EFI_SUCCESS;
  }

  InitStatus = GenMemoryTest->MemoryTestInit (
                                GenMemoryTest,
                                Level,
                                &RequireSoftECCInit
                                );
  if (InitStatus == EFI_NO_MEDIA) {
    //
    // The PEI codes also have the relevant memory test code to check the memory,
    // it can select to test some range of the memory or all of them. If PEI code
    // checks all the memory, this BDS memory test will has no not-test memory to
    // do the test, and then the status of EFI_NO_MEDIA will be returned by
    // "MemoryTestInit". So it does not need to test memory again, just return.
    //
    FreePool (Pos);
    return EFI_SUCCESS;
  }

  //TmpStr = GetStringById (STRING_TOKEN (STR_ESC_TO_SKIP_MEM_TEST));

  //if (TmpStr != NULL) {
  //  //PrintXY (10, 10, NULL, NULL, TmpStr);
  //  FreePool (TmpStr);
  //}

  do {
    Status = GenMemoryTest->PerformMemoryTest (
                              GenMemoryTest,
                              &TestedMemorySize,
                              &TotalMemorySize,
                              &ErrorOut,
                              TestAbort
                              );
    if (ErrorOut && (Status == EFI_DEVICE_ERROR)) {
      //TmpStr = GetStringById (STRING_TOKEN (STR_SYSTEM_MEM_ERROR));
      //if (TmpStr != NULL) {
      //  //PrintXY (10, 10, NULL, NULL, TmpStr);
      //  FreePool (TmpStr);
      //}

      ASSERT (0);
    }

    TempData = (UINT32) DivU64x32 (TotalMemorySize, 16);
    TestPercent = (UINTN) DivU64x32 (
                            DivU64x32 (MultU64x32 (TestedMemorySize, 100), 16),
                            TempData
                            );
    if (TestPercent != PreviousValue) {
      UnicodeValueToString (StrPercent, 0, TestPercent, 0);
      //TmpStr = GetStringById (STRING_TOKEN (STR_MEMORY_TEST_PERCENT));
      //if (TmpStr != NULL) {
        //
        // TmpStr size is 64, StrPercent is reserved to 16.
        //
        //StrCat (StrPercent, TmpStr);
       // PrintXY (10, 10, NULL, NULL, StrPercent);
      //  FreePool (TmpStr);
      //}

      //TmpStr = GetStringById (STRING_TOKEN (STR_PERFORM_MEM_TEST));
      //if (TmpStr != NULL) {
#if 0
   PlatformBdsShowProgress (
          Foreground,
          Background,
          TmpStr,
          Color,
          TestPercent,
          (UINTN) PreviousValue
          );
#endif
        //FreePool (TmpStr);
      //}
    }

    PreviousValue = TestPercent;

    KeyStatus     = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
    if (!EFI_ERROR (KeyStatus) && (Key.ScanCode == SCAN_ESC)) {
      if (!RequireSoftECCInit) {
        //TmpStr = GetStringById (STRING_TOKEN (STR_PERFORM_MEM_TEST));
        //if (TmpStr != NULL) {
#if 0
          PlatformBdsShowProgress (
            Foreground,
            Background,
            TmpStr,
            Color,
            100,
            (UINTN) PreviousValue
            );
#endif
          //FreePool (TmpStr);
        //}

        //PrintXY (10, 10, NULL, NULL, L"100");
        Status = GenMemoryTest->Finished (GenMemoryTest);
        goto Done;
      }

      TestAbort = TRUE;
    }
  } while (Status != EFI_NOT_FOUND);

  Status = GenMemoryTest->Finished (GenMemoryTest);

Done:
  UnicodeValueToString (StrTotalMemory, COMMA_TYPE, TotalMemorySize, 0);
  if (StrTotalMemory[0] == L',') {
    StrTotalMemory++;
  }

  //TmpStr = GetStringById (STRING_TOKEN (STR_MEM_TEST_COMPLETED));
  //if (TmpStr != NULL) {
  //  StrCat (StrTotalMemory, TmpStr);
  //  FreePool (TmpStr);
  //}

  //PrintXY (10, 10, NULL, NULL, StrTotalMemory);
 #if 0
  PlatformBdsShowProgress (
    Foreground,
    Background,
    StrTotalMemory,
    Color,
    100,
    (UINTN) PreviousValue
    );
#endif
  FreePool (Pos);

  return ReturnStatus;
}

/**
  Check if lang is in supported language codes according to language string.

  This code is used to check if lang is in in supported language codes. It can handle
  RFC4646 and ISO639 language tags.
  In ISO639 language tags, take 3-characters as a delimitation to find matched string.
  In RFC4646 language tags, take semicolon as a delimitation to find matched string.

  For example:
    SupportedLang  = "engfraengfra"
    Iso639Language = TRUE
    Lang           = "eng", the return value is "TRUE", or
    Lang           = "chs", the return value is "FALSE".
  Another example:
    SupportedLang  = "en;fr;en-US;fr-FR"
    Iso639Language = FALSE
    Lang           = "en", the return value is "TRUE", or
    Lang           = "zh", the return value is "FALSE".

  @param  SupportedLang               Platform supported language codes.
  @param  Lang                        Configured language.
  @param  Iso639Language              A bool value to signify if the handler is operated on ISO639 or RFC4646.

  @retval TRUE  lang is in supported language codes.
  @retval FALSE lang is not in supported language codes.

**/
BOOLEAN
IsLangInSupportedLangCodes(
  IN  CHAR8            *SupportedLang,
  IN  CHAR8            *Lang,
  IN  BOOLEAN          Iso639Language
  )
{
  UINTN    Index;
  UINTN    CompareLength;
  UINTN    LanguageLength;

  if (Iso639Language) {
    CompareLength = ISO_639_2_ENTRY_SIZE;
    for (Index = 0; Index < AsciiStrLen (SupportedLang); Index += CompareLength) {
      if (AsciiStrnCmp (Lang, SupportedLang + Index, CompareLength) == 0) {
        //
        // Successfully find the Lang string in SupportedLang string.
        //
        return TRUE;
      }
    }
    return FALSE;
  } else {
    //
    // Compare RFC4646 language code
    //
    for (LanguageLength = 0; Lang[LanguageLength] != '\0'; LanguageLength++);

    for (; *SupportedLang != '\0'; SupportedLang += CompareLength) {
      //
      // Skip ';' characters in SupportedLang
      //
      for (; *SupportedLang != '\0' && *SupportedLang == ';'; SupportedLang++);
      //
      // Determine the length of the next language code in SupportedLang
      //
      for (CompareLength = 0; SupportedLang[CompareLength] != '\0' && SupportedLang[CompareLength] != ';'; CompareLength++);

      if ((CompareLength == LanguageLength) &&
          (AsciiStrnCmp (Lang, SupportedLang, CompareLength) == 0)) {
        //
        // Successfully find the Lang string in SupportedLang string.
        //
        return TRUE;
      }
    }
    return FALSE;
  }
}

/**
  Initialize Lang or PlatformLang variable, if Lang or PlatformLang variable is not found,
  or it has been set to an unsupported value(not one of platform supported language codes),
  set the default language code to it.

  @param  LangName                    Language name, L"Lang" or L"PlatformLang".
  @param  SupportedLang               Platform supported language codes.
  @param  DefaultLang                 Default language code.
  @param  Iso639Language              A bool value to signify if the handler is operated on ISO639 or RFC4646,
                                      TRUE for L"Lang" LangName or FALSE for L"PlatformLang" LangName.

**/
VOID
InitializeLangVariable (
  IN CHAR16     *LangName,
  IN CHAR8      *SupportedLang,
  IN CHAR8      *DefaultLang,
  IN BOOLEAN    Iso639Language
  )
{
  EFI_STATUS  Status;
  CHAR8       *Lang;

  //
  // Find current Lang or PlatformLang from EFI Variable.
  //
  Status = GetEfiGlobalVariable2 (LangName, (VOID **) &Lang, NULL);
  //
  // If Lang or PlatformLang variable is not found,
  // or it has been set to an unsupported value(not one of the supported language codes),
  // set the default language code to it.
  //
  if (EFI_ERROR (Status) || !IsLangInSupportedLangCodes (SupportedLang, Lang, Iso639Language)) {
    //
    // The default language code should be one of the supported language codes.
    //
    ASSERT (IsLangInSupportedLangCodes (SupportedLang, DefaultLang, Iso639Language));
    Status = gRT->SetVariable (
                    LangName,
                    &gEfiGlobalVariableGuid,
                    EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                    AsciiStrSize (DefaultLang),
                    DefaultLang
                    );
  }

  if (Lang != NULL) {
    FreePool (Lang);
  }
}

/**
  Determine the current language that will be used based on language related EFI
  Variables. Optionally support the old EFI variables. The PlatformLangCodes is
  volatile and set on every boot directly from a PCD setting. So the build system
  is repsoncible for generating this value. The PlatformLang checked to make sure
  it is a memeber of PlatformLangCodes. If PlatformLang is not a member of
  PlatformLangCodes then the default value for PlatformLang is written to the NV
  variable.

**/
VOID
BdsInitializeLanguageVariables (
  VOID
  )
{
  EFI_STATUS  Status;
  CHAR8       *PlatformLangCodes;

  //
  // Set the PlatformLangCodes and PlatformLang based on the PCD setting from the build
  //
  PlatformLangCodes = (CHAR8 *)PcdGetPtr (PcdUefiVariableDefaultPlatformLangCodes);
  Status = gRT->SetVariable (
                  L"PlatformLangCodes",
                  &gEfiGlobalVariableGuid,
                  EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                  AsciiStrSize (PlatformLangCodes),
                  PlatformLangCodes
                  );

  InitializeLangVariable (L"PlatformLang", PlatformLangCodes, (CHAR8 *) PcdGetPtr (PcdUefiVariableDefaultPlatformLang), FALSE);
}

VOID
BdsPlatformGetConsoleVariables (
  IN EFI_DEVICE_PATH_PROTOCOL   **StdErrDevices,
  IN EFI_DEVICE_PATH_PROTOCOL   **StdInDevices,
  IN EFI_DEVICE_PATH_PROTOCOL   **StdOutDevices
  )
{
  UINTN                     DevicePathSize;

  DevicePathSize  = 0;

  *StdErrDevices = NULL;
  *StdInDevices  = NULL;
  *StdOutDevices = NULL;

  *StdErrDevices = BdsLibGetVariableAndSize (
                L"ErrOut",
                &gEfiGlobalVariableGuid,
                &DevicePathSize
                );

  *StdInDevices = BdsLibGetVariableAndSize (
            L"ConIn",
            &gEfiGlobalVariableGuid,
            &DevicePathSize
            );

  *StdOutDevices = BdsLibGetVariableAndSize (
            L"ConOut",
            &gEfiGlobalVariableGuid,
            &DevicePathSize
            );

}

VOID
BdsPlatformChangeSerialPortAttr (
  IN  EFI_DEVICE_PATH_PROTOCOL              *DevicePath,
  OUT EFI_DEVICE_PATH_PROTOCOL              **SerialDev
  )
{
  UINT8                                 LineControl;
  UART_DEVICE_PATH                      *UrtDevicePath;
  VENDOR_DEVICE_PATH                    *VendorDevicePath;
  EFI_DEVICE_PATH_PROTOCOL              *LastDeviceNode;
  
  DEBUG((EFI_D_INFO, "Before Changing Serial Port Device = %s\n", DevicePathToStr (DevicePath)));
  (*SerialDev) = DuplicateDevicePath (DevicePath);

  if (DevicePathType (*SerialDev)== HARDWARE_DEVICE_PATH &&
      DevicePathSubType (*SerialDev) == HW_VENDOR_DP
    ){
    LastDeviceNode = NextDevicePathNode (*SerialDev);
    while (!IsDevicePathEndType (LastDeviceNode)) {
      if (DevicePathType (LastDeviceNode) == MESSAGING_DEVICE_PATH &&
        DevicePathSubType (LastDeviceNode) == MSG_VENDOR_DP
      ) {
        VendorDevicePath = (VENDOR_DEVICE_PATH *)LastDeviceNode;
        switch (PcdGet8(PcdTerminalType)) {
        case 1:
         CopyGuid(&VendorDevicePath->Guid, &gEfiVT100Guid);
         return;

        case 2:
         CopyGuid(& VendorDevicePath->Guid, &gEfiVT100PlusGuid);
         return;
        case 3:
         CopyGuid(&VendorDevicePath->Guid, &gEfiVTUTF8Guid);
         return;

        case 0:
        default:
          CopyGuid(& VendorDevicePath->Guid, &gEfiPcAnsiGuid);
          return;
        }
      }
      LastDeviceNode = NextDevicePathNode (LastDeviceNode);
    }
    return;
  }

  LastDeviceNode = NextDevicePathNode (*SerialDev);
  while (!IsDevicePathEndType (LastDeviceNode)) {

    if (DevicePathType (LastDeviceNode) == MESSAGING_DEVICE_PATH &&
        DevicePathSubType (LastDeviceNode) == MSG_UART_DP
    ) {
      break;
    }
    LastDeviceNode = NextDevicePathNode (LastDeviceNode);

  }

  UrtDevicePath = (UART_DEVICE_PATH *) LastDeviceNode;

  UrtDevicePath->BaudRate = PcdGet32(PcdSerialBaudRate);

  //
  // Get the LCR register setting from pcd
  //
  LineControl = PcdGet8 (PcdSerialLineControl);

  //
  // Set the data bits
  //
  switch (LineControl & 0x03) {
    case 0:
      UrtDevicePath->DataBits = 5;
      break;
    case 1:
      UrtDevicePath->DataBits = 6;
      break;
    case 2:
      UrtDevicePath->DataBits = 7;
      break;
    case 3:
      UrtDevicePath->DataBits = 8;
      break;
    default:
      UrtDevicePath->DataBits = 8;
      break;
  }
  //
  // Set the stop bits
  //  Stop Bits 0x00 - Default Stop Bits.
  //  Stop Bits 0x01 - 1 Stop Bit.
  //  Stop Bits 0x02 - 1.5 Stop Bits.
  //  Stop Bits 0x03 - 2 Stop Bits.
  //
  if (LineControl & BIT2) {
    if (UrtDevicePath->DataBits == 5)
      UrtDevicePath->StopBits = 2;
    else
      UrtDevicePath->StopBits = 3;
  } else {
      UrtDevicePath->StopBits = 1;
  }

  //
  // Set the Parity
  //  Parity 0x00 - Default Parity.
  //  Parity 0x01 - No Parity.
  //  Parity 0x02 - Even Parity.
  //  Parity 0x03 - Odd Parity.
  //  Parity 0x04 - Mark Parity.
  //  Parity 0x05 - Space Parity.
  //
  if (LineControl & BIT3) {
    switch ((LineControl >> 4) & 0x03) {
    case 0: // Odd Parity
      UrtDevicePath->Parity = 3;
      break;
    case 1: // Even Parity
      UrtDevicePath->Parity = 2;
      break;
    case 2: // Mark Parity
      UrtDevicePath->Parity = 4;
      break;
    case 3: // Space Parity
      UrtDevicePath->Parity = 5;
      break;
    default:
      UrtDevicePath->Parity = 0;
      break;
    }
  } else {
    UrtDevicePath->Parity = 1;
  }
  DEBUG ((DEBUG_ERROR, "SerialDev->Uart.Parity = %x\n", UrtDevicePath->Parity));
  DEBUG ((DEBUG_ERROR, "SerialDev->Uart.StopBits = %x\n", UrtDevicePath->StopBits));
  DEBUG ((DEBUG_ERROR, "SerialDev->Uart.DataBits = %x\n", UrtDevicePath->DataBits));
  DEBUG ((DEBUG_ERROR, "SerialDev->Uart.BaudRate = %x\n", UrtDevicePath->BaudRate));

  if (!IsDevicePathEnd (LastDeviceNode)) {
    LastDeviceNode = NextDevicePathNode (LastDeviceNode);
    if (DevicePathType (LastDeviceNode) == MESSAGING_DEVICE_PATH &&
        DevicePathSubType (LastDeviceNode) == MSG_VENDOR_DP
    ) {
      VendorDevicePath = (VENDOR_DEVICE_PATH *)LastDeviceNode;
      switch (PcdGet8(PcdTerminalType)) {
      case 1:
       CopyGuid(&VendorDevicePath->Guid, &gEfiVT100Guid);
      break;

      case 2:
       CopyGuid(& VendorDevicePath->Guid, &gEfiVT100PlusGuid);
      break;
      case 3:
       CopyGuid(&VendorDevicePath->Guid, &gEfiVTUTF8Guid);
       break;

      case 0:
      default:
        CopyGuid(& VendorDevicePath->Guid, &gEfiPcAnsiGuid);
        break;
      }
    }
  }
}

/**
  Fill console handle in System Table if there are no valid console handle in.

  Firstly, check the validation of console handle in System Table. If it is invalid,
  update it by the first console device handle from EFI console variable.

  @param  VarName            The name of the EFI console variable.
  @param  ConsoleGuid        Specified Console protocol GUID.
  @param  ConsoleHandle      On IN,  console handle in System Table to be checked.
                             On OUT, new console hanlde in system table.
  @param  ProtocolInterface  On IN,  console protocol on console handle in System Table to be checked.
                             On OUT, new console protocol on new console hanlde in system table.

  @retval TRUE               System Table has been updated.
  @retval FALSE              System Table hasn't been updated.

**/
BOOLEAN
UpdateSystemTableConsole (
  IN     CHAR16                          *VarName,
  IN     EFI_GUID                        *ConsoleGuid,
  IN OUT EFI_HANDLE                      *ConsoleHandle,
  IN OUT VOID                            **ProtocolInterface
  )
{
  EFI_STATUS                Status;
  UINTN                     DevicePathSize;
  EFI_DEVICE_PATH_PROTOCOL  *FullDevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *VarConsole;
  EFI_DEVICE_PATH_PROTOCOL  *Instance;
  VOID                      *Interface;
  EFI_HANDLE                NewHandle;

  ASSERT (VarName != NULL);
  ASSERT (ConsoleHandle != NULL);
  ASSERT (ConsoleGuid != NULL);
  ASSERT (ProtocolInterface != NULL);

  if (*ConsoleHandle != NULL) {
    Status = gBS->HandleProtocol (
                   *ConsoleHandle,
                   ConsoleGuid,
                   &Interface
                   );
    if (Status == EFI_SUCCESS && Interface == *ProtocolInterface) {
      //
      // If ConsoleHandle is valid and console protocol on this handle also
      // also matched, just return.
      //
      return FALSE;
    }
  }

  //
  // Get all possible consoles device path from EFI variable
  //
  VarConsole = BdsLibGetVariableAndSize (
                VarName,
                &gEfiGlobalVariableGuid,
                &DevicePathSize
                );
  if (VarConsole == NULL) {
    //
    // If there is no any console device, just return.
    //
    return FALSE;
  }

  FullDevicePath = VarConsole;

  do {
    //
    // Check every instance of the console variable
    //
    Instance  = GetNextDevicePathInstance (&VarConsole, &DevicePathSize);
    if (Instance == NULL) {
      FreePool (FullDevicePath);
      ASSERT (FALSE);
    }

    //
    // Find console device handle by device path instance
    //
    Status = gBS->LocateDevicePath (
                   ConsoleGuid,
                   &Instance,
                   &NewHandle
                   );
    if (!EFI_ERROR (Status)) {
      //
      // Get the console protocol on this console device handle
      //
      Status = gBS->HandleProtocol (
                     NewHandle,
                     ConsoleGuid,
                     &Interface
                     );
      if (!EFI_ERROR (Status)) {
        //
        // Update new console handle in System Table.
        //
        *ConsoleHandle     = NewHandle;
        *ProtocolInterface = Interface;
        return TRUE;
      }
    }

  } while (Instance != NULL);

  //
  // No any available console devcie found.
  //
  return FALSE;
}


VOID
BdsSetgST () {
  //
  // Update gST Console related.
  //
  UpdateSystemTableConsole (L"ConIn", &gEfiSimpleTextInProtocolGuid, &gST->ConsoleInHandle, (VOID **) &gST->ConIn);
  UpdateSystemTableConsole (L"ConOut", &gEfiSimpleTextOutProtocolGuid, &gST->ConsoleOutHandle, (VOID **) &gST->ConOut);
  UpdateSystemTableConsole (L"ErrOut", &gEfiSimpleTextOutProtocolGuid, &gST->StandardErrorHandle, (VOID **) &gST->StdErr);

  gST->Hdr.CRC32 = 0;
  gBS->CalculateCrc32 (
        (UINT8 *) &gST->Hdr,
        gST->Hdr.HeaderSize,
        &gST->Hdr.CRC32
        );

}
/**
  Return the platform specific defaults for the 3 EFI non-volatile console varibles.
  Each varible is a multi-instance device path and each device path instances
  represetns a console device. If a device path instance starts with a USB Class
  device path then all USB devices of that class will we enabled.

  @param  StdErrDevices     Return proposed L"ErrOut" console variable setting
  @param  StdInDevices      Return proposed L"ConIn" console variable setting
  @param  StdOutDevices     Return proposed L"ConOut" console variable setting
  @param  StdErrEnabled     TRUE if StdOut needs to be enabled
  @param  ReplaceVariables  TRUE if EFI console variable should be replaced if
                            they exist.

**/
VOID
BdsPlatformRepairConsoleVariables (
  IN EFI_DEVICE_PATH_PROTOCOL   **StdErrDevices,
  IN EFI_DEVICE_PATH_PROTOCOL   **StdInDevices,
  IN EFI_DEVICE_PATH_PROTOCOL   **StdOutDevices,
  IN BOOLEAN                    StdErrEnabled,
  IN BOOLEAN                    ReplaceVariables
  )
{
  EFI_STATUS                            Status;
  UINTN                                 Size;
  EFI_HANDLE                            Handle;
  UINT8                                 Index;
  EFI_HANDLE                            *ConOutHandles;
  EFI_GRAPHICS_OUTPUT_PROTOCOL          *GraphicsOutput;
  EFI_SIMPLE_POINTER_PROTOCOL           *SimplePointer;
  EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL     *SimpleTextInputEx;
  EFI_SIMPLE_TEXT_INPUT_PROTOCOL        *SimpleTextInput;
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL       *SimpleTextOutput;
  EFI_DEVICE_PATH_PROTOCOL              *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL              *SerialDev;



  if (!ReplaceVariables && (*StdInDevices != NULL) && (*StdOutDevices != NULL)) {
    //
    // If the variables exist try them.
    // You chould chose to filter here to add or remove options, but this
    // example does not do that.
    //
    return;
  }

  //
  // Phase 0 - Intialize infrastructure
  //
  *StdErrDevices = NULL;
  *StdInDevices  = NULL;
  *StdOutDevices = NULL;

  //
  // Phase 1 - Connect the console devices (start the drivers)
  //

  //
  // Phase 2 - Pick the devices to go in the console varialbes
  //
  GraphicsOutput    = NULL;
  SimpleTextInput   = NULL;
  SimpleTextOutput  = NULL;
  SimplePointer     = NULL;
  SimpleTextInputEx = NULL;


  Status = gBS->LocateProtocol (&gEfiSimpleTextOutProtocolGuid,  NULL, (VOID **)&SimpleTextOutput);
  DEBUG ((DEBUG_ERROR, "STO (1) Status = %r\n", Status));

  Status = gBS->LocateProtocol (&gEfiGraphicsOutputProtocolGuid, NULL, (VOID **)&GraphicsOutput);
  DEBUG ((DEBUG_ERROR, "GOP Status = %r\n", Status));

  Status = gBS->LocateProtocol (&gEfiSimpleTextInputExProtocolGuid,   NULL, (VOID **)&SimpleTextInputEx);
  DEBUG ((DEBUG_ERROR, "STIEx (1) Status = %r\n", Status));

  Status = gBS->LocateProtocol (&gEfiSimpleTextInProtocolGuid,   NULL, (VOID **)&SimpleTextInput);
  DEBUG ((DEBUG_ERROR, "STI Status = %r\n", Status));

  Status = gBS->LocateProtocol (&gEfiSimplePointerProtocolGuid,  NULL, (VOID **)&SimplePointer);
  DEBUG ((DEBUG_ERROR, "SP Status = %r\n", Status));

  if (SimpleTextInput == NULL) {
    //
    // Install Dummy SimpleTextInPutProtocol
    //
    if (SimpleTextInputEx == NULL) {
      gBS->InstallMultipleProtocolInterfaces (
        &gST->ConsoleInHandle,
        &gEfiSimpleTextInputExProtocolGuid,
        &mBdsDummyConInEx,
        &gEfiSimpleTextInProtocolGuid,
        &mBdsDummyConIn,
        NULL
        );
      SimpleTextInputEx = &mBdsDummyConInEx;
    } else {
      gBS->InstallMultipleProtocolInterfaces (
        &gST->ConsoleInHandle,
        &gEfiSimpleTextInProtocolGuid,
        &mBdsDummyConIn,
        NULL
        );
    }
    gST->ConIn = &mBdsDummyConIn;
  }

  if (SimpleTextOutput == NULL) {
    //
    // Install Dummy SimpleTextOutputProtocol
    //
    gBS->InstallMultipleProtocolInterfaces (
        &gST->ConsoleOutHandle,
        &gEfiSimpleTextOutProtocolGuid,
        &mBdsDummyConOut,
        NULL
        );
     gST->ConOut = &mBdsDummyConOut;
  }

  //
  // Locate all handles which has SimpleTextOutProtocol and get the device path from each of them,
  // then save them as Device Path List. Late the Device Path list will be stored in 'ConOut' variable.
  //
  Size = sizeof (EFI_HANDLE);
  Status = gBS->LocateHandle (
                  ByProtocol,
                  &gEfiSimpleTextOutProtocolGuid,
                  NULL,
                  &Size,
                  (VOID **) &Handle
                  );
  DEBUG ((DEBUG_ERROR, "STO Handle Status = %r\n", Status));
  if (Status == EFI_BUFFER_TOO_SMALL) {
    ConOutHandles = (EFI_HANDLE*) AllocateZeroPool (Size);
    Status = gBS->LocateHandle (
                  ByProtocol,
                  &gEfiSimpleTextOutProtocolGuid,
                  NULL,
                  &Size,
                  (VOID **) ConOutHandles
                  );
    DEBUG ((DEBUG_ERROR, "STO(2) Handle Status = %r\n", Status));
    DEBUG ((EFI_D_INFO, "STO(2) Handles Size = %x\n", Size/sizeof(EFI_HANDLE)));
    for ( Index = 0; Index < (Size/sizeof (EFI_HANDLE)); Index++) {
      DEBUG ((EFI_D_INFO, "Handle(0)=%x\n", Handle));
      DEBUG ((EFI_D_INFO, "ConOutHanles+Index*sizeof(EFI_HANDLE)=%x; Index=%x\n", (UINT8 *) ConOutHandles + Index * sizeof (EFI_HANDLE), Index));
      CopyMem ( (UINT8*) &Handle, ((UINT8 *) ConOutHandles) + Index * sizeof (EFI_HANDLE), sizeof (EFI_HANDLE));
      DEBUG ((EFI_D_INFO, "Handle(1)=%x\n", Handle));
      DevicePath = NULL;
      DevicePath = DevicePathFromHandle (Handle);
      if ( DevicePath != NULL && BDS_EFI_MESSAGE_SERIAL == BdsGetBootTypeFromDevicePath (DevicePath)) {
        BdsPlatformChangeSerialPortAttr (DevicePath, &SerialDev);
        DEBUG ((EFI_D_INFO, "After Replaced what the Serial Io device Path = %s\n",  DevicePathToStr (SerialDev)));
        *StdOutDevices = AppendDevicePathInstance (*StdOutDevices, SerialDev);
        *StdErrDevices = AppendDevicePathInstance (*StdErrDevices, SerialDev);
        FreePool (SerialDev);
      }else {
        DEBUG((EFI_D_INFO, "After Serial Port Device changed= %s\n", DevicePathToStr (DevicePath)));
        *StdOutDevices = AppendDevicePathInstance (*StdOutDevices, DevicePath);
        *StdErrDevices = AppendDevicePathInstance (*StdErrDevices, DevicePath);
      }
    }
    FreePool (ConOutHandles);
  } else {
    DevicePath = NULL;
    DevicePath = DevicePathFromHandle (Handle);
    if (DevicePath != NULL && BDS_EFI_MESSAGE_SERIAL == BdsGetBootTypeFromDevicePath (DevicePath)) {
      BdsPlatformChangeSerialPortAttr (DevicePath, &SerialDev);
      DEBUG ((EFI_D_INFO, "After Replaced what the Serial Io device Path = %s\n",  DevicePathToStr (SerialDev)));
      *StdOutDevices = AppendDevicePathInstance (*StdOutDevices, SerialDev);
      *StdErrDevices = AppendDevicePathInstance (*StdErrDevices, SerialDev);
      FreePool (SerialDev);
    } else {
      *StdOutDevices = AppendDevicePathInstance (*StdOutDevices, DevicePath);
      *StdErrDevices = AppendDevicePathInstance (*StdErrDevices, DevicePath);
    }
  }

  //
  // Locate all handles which has SimpleTextInProtocol and get the device path of each of them,
  // then save them as Device Path List. Lately the Device Path list will be stored in 'ConIn'.
  //
  Size = sizeof (EFI_HANDLE);
  Status = gBS->LocateHandle (
                  ByProtocol,
                  &gEfiSimpleTextInProtocolGuid,
                  NULL,
                  &Size,
                  (VOID **) &Handle
                  );
  if (Status == EFI_BUFFER_TOO_SMALL) {
    ConOutHandles = (EFI_HANDLE*) AllocateZeroPool (Size);
    Status = gBS->LocateHandle (
                  ByProtocol,
                  &gEfiSimpleTextInProtocolGuid,
                  NULL,
                  &Size,
                  (VOID **) ConOutHandles
                  );
    DEBUG ((DEBUG_ERROR, "STI(2) Handle Status = %r\n", Status));
    DEBUG ((EFI_D_INFO, "STI(2) Handles Size = %x\n", Size/sizeof(EFI_HANDLE)));
    for ( Index = 0; Index < (Size/sizeof (EFI_HANDLE)); Index++) {
      DEBUG ((EFI_D_ERROR, "Handle(0)=%x\n", Handle));
      DEBUG ((EFI_D_ERROR, "ConOutHanles+Index*sizeof(EFI_HANDLE)=%x; Index=%x\n", (UINT8 *) ConOutHandles + Index * sizeof (EFI_HANDLE), Index));
      CopyMem ( (UINT8*) &Handle, ((UINT8 *) ConOutHandles) + Index * sizeof (EFI_HANDLE), sizeof (EFI_HANDLE));
      DEBUG ((EFI_D_ERROR, "Handle(1)=%x\n", Handle));
      DevicePath = NULL;
      DevicePath = DevicePathFromHandle (Handle);

      if ( DevicePath != NULL && BDS_EFI_MESSAGE_SERIAL == BdsGetBootTypeFromDevicePath (DevicePath)) {
        BdsPlatformChangeSerialPortAttr (DevicePath, &SerialDev);
        *StdInDevices = AppendDevicePathInstance (*StdInDevices, SerialDev);
        FreePool (SerialDev);
      } else {
        *StdInDevices = AppendDevicePathInstance (*StdInDevices, DevicePath);
      }
    }
    FreePool (ConOutHandles);
  }else {
    DevicePath = NULL;
    DevicePath = DevicePathFromHandle (Handle);
    if (DevicePath != NULL && BDS_EFI_MESSAGE_SERIAL == BdsGetBootTypeFromDevicePath (DevicePath)) {
      BdsPlatformChangeSerialPortAttr (DevicePath, &SerialDev);
      *StdInDevices = AppendDevicePathInstance (*StdInDevices, SerialDev);
      FreePool (SerialDev);
    } else {
      *StdInDevices = AppendDevicePathInstance (*StdInDevices, DevicePath);
    }
  }

  //
  // Update the variables
  //
  gRT->SetVariable (
        L"ConIn",
        &gEfiGlobalVariableGuid,
        EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
        GetDevicePathSize (*StdInDevices),
        *StdInDevices
        );
  DEBUG ((DEBUG_ERROR, "ConIn Variable Size = %x\n", GetDevicePathSize (*StdInDevices)));

  if (*StdOutDevices != NULL) {
    gRT->SetVariable (
        L"ConOut",
        &gEfiGlobalVariableGuid,
        EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
        GetDevicePathSize (*StdOutDevices),
        *StdOutDevices
        );
  }
  DEBUG ((DEBUG_ERROR, "ConOut Variable Size = %x\n", GetDevicePathSize (*StdOutDevices)));

  if (*StdErrDevices != NULL) {
    gRT->SetVariable (
        L"ErrOut",
        &gEfiGlobalVariableGuid,
        EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
        GetDevicePathSize (*StdErrDevices),
        *StdErrDevices
        );
  }
  DEBUG ((DEBUG_ERROR, "ErrOut Variable Size = %x\n", GetDevicePathSize (*StdErrDevices)));

  gST->Hdr.CRC32 = 0;
  gBS->CalculateCrc32 (
        (UINT8 *) &gST->Hdr,
        gST->Hdr.HeaderSize,
        &gST->Hdr.CRC32
        );


}

/**
  Display a message on the serial port

  @param  AnsiString    String to display

**/
VOID
BdsPrint (
  IN  CHAR8 * AsciiString
  )
{
  if (FeaturePcdGet (PcdDiagBootPhasesSerial)) {
    UINTN StringLength;
    EFI_SERIAL_IO_PROTOCOL * SerialPort;
    EFI_STATUS Status;

    //
    //  Locate the serial port
    //
    Status = gBS->LocateProtocol ( &gEfiSerialIoProtocolGuid,
                                   NULL,
                                   (VOID**)&SerialPort );
    if ( !EFI_ERROR ( Status )) {
      //
      //  Determine the length of the string
      //
      StringLength = AsciiStrLen ( AsciiString );
      Status = SerialPort->Write ( SerialPort,
                                   &StringLength,
                                   AsciiString );
    }
  }
}

/**
  This function uses policy data from the platform to determine what operating
  system or system utility should be loaded and invoked.  This function call
  also optionally make the use of user input to determine the operating system
  or system utility to be loaded and invoked.  When the DXE Core has dispatched
  all the drivers on the dispatch queue, this function is called.  This
  function will attempt to connect the boot devices required to load and invoke
  the selected operating system or system utility.  During this process,
  additional firmware volumes may be discovered that may contain addition DXE
  drivers that can be dispatched by the DXE Core.   If a boot device cannot be
  fully connected, this function calls the DXE Service Dispatch() to allow the
  DXE drivers from any newly discovered firmware volumes to be dispatched.
  Then the boot device connection can be attempted again.  If the same boot
  device connection operation fails twice in a row, then that boot device has
  failed, and should be skipped.  This function should never return.

  @param  This             The EFI_BDS_ARCH_PROTOCOL instance.

  @return None.

**/
VOID
EFIAPI
BdsEntry (
  IN EFI_BDS_ARCH_PROTOCOL  *This
  )
{
  UINT16                        Timeout;
  EFI_BOOT_MODE                 BootMode;
  EFI_BOOT_MANAGER_BOOT_OPTION  *BootOptions;
  UINTN                         BootOptionsCount;
  EFI_KEY_DATA                  Key;
  BOOLEAN                       IsFirstBoot;


  //
  // Use a DynamicHii type pcd to save the boot status, which is used to
  // control configuration mode, such as FULL/MINIMAL/NO_CHANGES configuration.
  //
  IsFirstBoot = PcdGetBool(PcdBootState);
  if (IsFirstBoot == TRUE) {
    PcdSetBool(PcdBootState, FALSE);
  }

  //
  // Insert the performance probe
  //
  PERF_END (0, "DXE", NULL, 0);
  PERF_START (0, "BDS", NULL, 0);

  //
  //  Boot phase notification
  //
  if ( FeaturePcdGet ( PcdDiagBootPhasesSerial )) {
    DEBUG ((DEBUG_INFO, "\n\n----------\n"));
    DEBUG ((DEBUG_INFO, "BDS Starting\n"));
    DEBUG ((DEBUG_INFO, "----------\n"));
  }

  //
  // Close boot script and install ready to lock
  //
  InstallReadyToLock ();

  Timeout  = EfiBootManagerGetTimeout (TRUE);
  BootMode = GetBootModeHob ();

  DEBUG ((EFI_D_ERROR, "BootMode = %x\n", BootMode));

  //
  // Library hook point for BDS entry prior to console connection.
  // Also allows BootMode and Timeout to be over ridden by the platform
  //
  BdsPlatformEntry (&BootMode, &Timeout);

  //
  // Initialize the EFI Language variables if they are not valid.
  //
  BdsInitializeLanguageVariables ();

  //
  // Process Capsule when boot mode is BOOT_ON_FLASH_UPDATE
  //
  if(BootMode == BOOT_ON_FLASH_UPDATE) {
    BdsProcessCapsules(BootMode);
  }

  if (BootMode == BOOT_WITH_MINIMAL_CONFIGURATION || BootMode == BOOT_ASSUMING_NO_CONFIGURATION_CHANGES) {
    BdsPlatformGetConsoleVariables (&gErrOutDevices, &gConInDevices, &gConOutDevices);
  }

  DEBUG (( DEBUG_INFO, "PCIe bus configuration\r\n" ));
  if (!gErrOutDevices || !gConInDevices || !gConOutDevices)
  {
    //
    // Connect all devices
    //
    EfiBootManagerConnectAll (TRUE);

    //
    // Update UEFI System Table with console devices and set console variables
    //
    BdsPlatformRepairConsoleVariables (&gErrOutDevices, &gConInDevices, &gConOutDevices, TRUE, TRUE);
  }

  //
  // After set the Variable reconnect
  //
  EfiConnectDevicePaths (gConOutDevices);
  EfiConnectDevicePaths (gConInDevices);
  EfiConnectDevicePaths (gErrOutDevices);
  BdsSetgST();
  //
  // The EFI System table must contain valid console infomration at this point.
  // If the ConSplitter was loaded the handles would be valid even though no console devices
  // are currently present. Hot plug of console devices is supported with the ConSpiltter.
  // If the ConSplitter is not present the BdsPlatformRepairConsoleVariables() is responsible
  // for updating the EFI system table with the values for the single console device in the system.
  //
  ASSERT (!((gST->ConIn == NULL) || (gST->ConsoleInHandle == NULL)));
  ASSERT (!((gST->ConOut == NULL) || (gST->ConsoleOutHandle == NULL)));

  //
  //  Boot phase notification
  //
  if ( FeaturePcdGet ( PcdDiagBootPhasesSerial )) {
    BdsPrint ( "Graphics Console Setup Complete ...\r\n" );
  }

  //
  // Now that consoles are initialized start up the UI.
  //
  BdsUiInitialize (BootMode, (BDS_UI_EFI_BOOT_CALL_BACK)EfiBootManagerBootOption);

  //
  // Insert the performance probe for Console Out
  //
  PERF_START (NULL, "ConOut", "BDS", 1);
  PERF_END (NULL, "ConOut", "BDS", 0);

  //
  // Load EFI Drivers based on Variable settings and/or platform overrides
  // On this platform, do not process driver options.  They are never expected to be set.
  //
  PERF_START (NULL, "DriverOptions", "BDS", 0);
  PERF_END (NULL, "DriverOptions", "BDS", 0);

  //
  // Boot Options
  // Update or repair boot options based on platform policy
  //
  BootOptions      = NULL;
  BootOptionsCount = 0;
  ZeroMem (&Key, sizeof (EFI_KEY_DATA));

  //
  // Clear up BootOptions.
  //
  BdsUiDeleteBootOptions (BootOptions, BootOptionsCount);

  //
  // Build up Boot Options
  //
  if (BootMode == BOOT_WITH_MINIMAL_CONFIGURATION || BootMode == BOOT_ASSUMING_NO_CONFIGURATION_CHANGES) {
    PlatformBdsGetBootOptions (&BootOptionsCount, FALSE);
  }
  else {
    PlatformBdsGetBootOptions (&BootOptionsCount, TRUE);
  }
  DEBUG ((EFI_D_ERROR, "***********Bds BootOptionsCount = %d ************\n", BootOptionsCount));

  //
  // Update the UI with the information in case you need to see a list
  // of boot options while the timeout is counting down.
  //
  if (!PcdGetBool (PcdEnableFastBoot) || !(BootMode == BOOT_WITH_MINIMAL_CONFIGURATION || BootMode == BOOT_ASSUMING_NO_CONFIGURATION_CHANGES))
  BdsUiUpdateBootOptions (BootOptions, BootOptionsCount, &Key);

  BdsMemoryTest (IGNORE);
  BdsUiInteractiveMenus (&Key, Timeout);
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
BdsIntialize (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                    Status;
  CHAR16                        *Str;

  //BdsPrintMemoryMap (L"BDS Intiliaze");

  gST->FirmwareRevision = PcdGet32 (PcdFirmwareRevision);

  Str = (CHAR16 *)PcdGetPtr (PcdFirmwareVendor);
  gST->FirmwareVendor = AllocateRuntimeCopyPool (StrSize (Str), Str);

  //
  // Fixup Tasble CRC after we updated Firmware Vendor and Revision
  //
  gBS->CalculateCrc32 ((VOID *)gST, sizeof (EFI_SYSTEM_TABLE), &gST->Hdr.CRC32);

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mHandle,
                  &gEfiBdsArchProtocolGuid,  &mBds,
                  NULL
                  );
  return Status;
}

/**

  This routine is called to see if there are any capsules we need to process.
  If the boot mode is not UPDATE, then we do nothing. Otherwise find the
  capsule HOBS and produce firmware volumes for them via the DXE service.
  Then call the dispatcher to dispatch drivers from them. Finally, check
  the status of the updates.

  This function should be called by BDS in case we need to do some
  sort of processing even if there is no capsule to process. We
  need to do this if an earlier update went away and we need to
  clear the capsule variable so on the next reset PEI does not see it and
  think there is a capsule available.

  @param BootMode                 the current boot mode

  @retval EFI_INVALID_PARAMETER   boot mode is not correct for an update
  @retval EFI_SUCCESS             There is no error when processing capsule

**/
EFI_STATUS
EFIAPI
BdsProcessCapsules (
  EFI_BOOT_MODE BootMode
  )
{
  EFI_STATUS                  Status;
  EFI_PEI_HOB_POINTERS        HobPointer;
  EFI_CAPSULE_HEADER          *CapsuleHeader;
  UINT32                      Size;
  UINT32                      CapsuleNumber;
  UINT32                      CapsuleTotalNumber;
  EFI_CAPSULE_TABLE           *CapsuleTable;
  UINT32                      Index;
  UINT32                      CacheIndex;
  UINT32                      CacheNumber;
  VOID                        **CapsulePtr;
  VOID                        **CapsulePtrCache;
  EFI_GUID                    *CapsuleGuidCache;

  CapsuleNumber = 0;
  CapsuleTotalNumber = 0;
  CacheIndex   = 0;
  CacheNumber  = 0;
  CapsulePtr        = NULL;
  CapsulePtrCache   = NULL;
  CapsuleGuidCache  = NULL;

  //
  // We don't do anything else if the boot mode is not flash-update
  //
  if (BootMode != BOOT_ON_FLASH_UPDATE) {
    DEBUG ((EFI_D_ERROR, "Boot mode is not correct for capsule update.\n"));
    return EFI_INVALID_PARAMETER;
  }

  Status = EFI_SUCCESS;
  //
  // Find all capsule images from hob
  //
  HobPointer.Raw = GetHobList ();
  while ((HobPointer.Raw = GetNextHob (EFI_HOB_TYPE_UEFI_CAPSULE, HobPointer.Raw)) != NULL) {
    CapsuleTotalNumber ++;
    HobPointer.Raw = GET_NEXT_HOB (HobPointer);
  }
  if (CapsuleTotalNumber == 0) {
    //
    // We didn't find a hob, so had no errors.
    //
    DEBUG ((EFI_D_ERROR, "We can not find capsule data in capsule update boot mode.\n"));
    DEBUG ((EFI_D_ERROR, "Please check the followings are correct if unexpected capsule update error happens.\n"));
    DEBUG ((EFI_D_ERROR, "1. CapsuleX64 is built as X64 module when PEI is IA32 and DXE is X64\n"));
    DEBUG ((EFI_D_ERROR, "2. Capsule data should persist in memory across a system reset.\n"));
    return EFI_SUCCESS;
  }

  //
  // Init temp Capsule Data table.
  //
  CapsulePtr       = (VOID **) AllocateZeroPool (sizeof (VOID *) * CapsuleTotalNumber);
  ASSERT (CapsulePtr != NULL);
  CapsulePtrCache  = (VOID **) AllocateZeroPool (sizeof (VOID *) * CapsuleTotalNumber);
  ASSERT (CapsulePtrCache != NULL);
  CapsuleGuidCache = (EFI_GUID *) AllocateZeroPool (sizeof (EFI_GUID) * CapsuleTotalNumber);
  ASSERT (CapsuleGuidCache != NULL);

  //
  // Find all capsule images from hob
  //
  HobPointer.Raw = GetHobList ();
  while ((HobPointer.Raw = GetNextHob (EFI_HOB_TYPE_UEFI_CAPSULE, HobPointer.Raw)) != NULL) {
    CapsulePtr [CapsuleNumber++] = (VOID *) (UINTN) HobPointer.Capsule->BaseAddress;
    HobPointer.Raw = GET_NEXT_HOB (HobPointer);
  }

  //
  //Check the capsule flags,if contains CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE, install
  //capsuleTable to configure table with EFI_CAPSULE_GUID
  //

  //
  // Capsules who have CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE always are used for operating
  // System to have information persist across a system reset. EFI System Table must
  // point to an array of capsules that contains the same CapsuleGuid value. And agents
  // searching for this type capsule will look in EFI System Table and search for the
  // capsule's Guid and associated pointer to retrieve the data. Two steps below describes
  // how to sorting the capsules by the unique guid and install the array to EFI System Table.
  // Firstly, Loop for all coalesced capsules, record unique CapsuleGuids and cache them in an
  // array for later sorting capsules by CapsuleGuid.
  //
  for (Index = 0; Index < CapsuleTotalNumber; Index++) {
    CapsuleHeader = (EFI_CAPSULE_HEADER*) CapsulePtr [Index];
    if ((CapsuleHeader->Flags & CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE) != 0) {
      //
      // For each capsule, we compare it with known CapsuleGuid in the CacheArray.
      // If already has the Guid, skip it. Whereas, record it in the CacheArray as
      // an additional one.
      //
      CacheIndex = 0;
      while (CacheIndex < CacheNumber) {
        if (CompareGuid(&CapsuleGuidCache[CacheIndex],&CapsuleHeader->CapsuleGuid)) {
          break;
        }
        CacheIndex++;
      }
      if (CacheIndex == CacheNumber) {
        CopyMem(&CapsuleGuidCache[CacheNumber++],&CapsuleHeader->CapsuleGuid,sizeof(EFI_GUID));
      }
    }
  }

  //
  // Secondly, for each unique CapsuleGuid in CacheArray, gather all coalesced capsules
  // whose guid is the same as it, and malloc memory for an array which preceding
  // with UINT32. The array fills with entry point of capsules that have the same
  // CapsuleGuid, and UINT32 represents the size of the array of capsules. Then install
  // this array into EFI System Table, so that agents searching for this type capsule
  // will look in EFI System Table and search for the capsule's Guid and associated
  // pointer to retrieve the data.
  //
  CacheIndex = 0;
  while (CacheIndex < CacheNumber) {
    CapsuleNumber = 0;
    for (Index = 0; Index < CapsuleTotalNumber; Index++) {
      CapsuleHeader = (EFI_CAPSULE_HEADER*) CapsulePtr [Index];
      if ((CapsuleHeader->Flags & CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE) != 0) {
        if (CompareGuid (&CapsuleGuidCache[CacheIndex], &CapsuleHeader->CapsuleGuid)) {
          //
          // Cache Caspuleheader to the array, this array is uniqued with certain CapsuleGuid.
          //
          CapsulePtrCache[CapsuleNumber++] = (VOID*)CapsuleHeader;
        }
      }
    }
    if (CapsuleNumber != 0) {
      Size = sizeof(EFI_CAPSULE_TABLE) + (CapsuleNumber - 1) * sizeof(VOID*);
      CapsuleTable = AllocateRuntimePool (Size);
      ASSERT (CapsuleTable != NULL);
      CapsuleTable->CapsuleArrayNumber =  CapsuleNumber;
      CopyMem(&CapsuleTable->CapsulePtr[0], CapsulePtrCache, CapsuleNumber * sizeof(VOID*));
      Status = gBS->InstallConfigurationTable (&CapsuleGuidCache[CacheIndex], (VOID*)CapsuleTable);
      ASSERT_EFI_ERROR (Status);
    }
    CacheIndex++;
  }

  //
  // Besides ones with CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE flag, all capsules left are
  // recognized by platform with CapsuleGuid. For general platform driver, UpdateFlash
  // type is commonly supported, so here only deal with encapsuled FVs capsule. Additional
  // type capsule transaction could be extended. It depends on platform policy.
  //
  for (Index = 0; Index < CapsuleTotalNumber; Index++) {
    CapsuleHeader = (EFI_CAPSULE_HEADER*) CapsulePtr [Index];
    if ((CapsuleHeader->Flags & CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE) == 0) {
      //
      // Call capsule library to process capsule image.
      //
      ProcessCapsuleImage (CapsuleHeader);
    }
  }

  //
  // Free the allocated temp memory space.
  //
  FreePool (CapsuleGuidCache);
  FreePool (CapsulePtrCache);
  FreePool (CapsulePtr);

  return Status;
}


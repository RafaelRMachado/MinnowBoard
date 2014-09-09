/** @file
  User Interface Lib for BDS. This is the UI for the template BDS.

  UI based on Simple Text Out protoocl. Hii is not implemented in this 
  example library instance.

  Copyright (c) 2008, Intel Corporation                                                         
  All rights reserved. This program and the accompanying materials                          
  are licensed and made available under the terms and conditions of the BSD License         
  which accompanies this distribution.  The full text of the license may be found at        
  http://opensource.org/licenses/bsd-license.php                                            

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

**/

#include <PiDxe.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/PrintLib.h>
#include <Library/IoLib.h>
#include <Library/DxeServicesLib.h>
#include <Protocol/OEMBadging.h>
#include <Protocol/UgaDraw.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/BootLogo.h>
#include <Protocol/Smbios.h>
#include <IndustryStandard/SmBios.h>


#include "BdsLib.h"
#include "BdsUiLib.h"
#include "BdsPlatformLib.h"
#include "BdsUtility.h"

//
//
//
#define MAX_OPTION_STRING_SIZE  256

typedef struct {
  EFI_BOOT_MANAGER_BOOT_OPTION  *BootOption;
  CHAR16                        String[MAX_OPTION_STRING_SIZE];    // Display including spaces needed for inverse
  UINTN                         Attribute;      // Simple Text Out Attributes 
  UINTN                         CursorColumn;   // Starting Cursor Column for String[]
  UINTN                         MaxCursorColumn;// Ending Cursor Column for String[]
  UINTN                         CursorRow;      // Cuursor Row for String
  BOOLEAN                       Removable;
} PLATFORM_UI_PICK_LIST_ENTRY;

//
// Variables set up in BdsUiInitialize ()
//
BDS_UI_EFI_BOOT_CALL_BACK   mEfiBootCallBack   = NULL;
UINTN                       mAttributes;
UINTN                       mInverseAttributes;
EFI_SIMPLE_TEXT_OUTPUT_MODE mStoMode;
UINTN                       mStoMaxColumns;
UINTN                       mStoMaxRows;
UINT32                      mStartRow;
UINT32                      mStartColumn;

//
// UI List globals
//
UINTN                       mPickListMaxCount = 0;
UINTN                       gPickListCount = 0;
PLATFORM_UI_PICK_LIST_ENTRY *gPickList = NULL;
UINTN                       gPickListSelection = 0;

/**
  Called by BDS after the first time consoles are connected. This API allows a UI and
  hot key processing prior to L"DriverOption" variable processing. Also its possible to
  take boot mode specific action in this function as the boot mode is passed in. It's 
  also legal to put up a logo in this function.

  The EfiBootCallBack function allows the UI to callback into the BDS to do an EFI boot.

  [Note: If we need other callbacks they would get added to this function]

  @param  BootMode          System PI Boot Mode.
  @param  EfiBootCallBack   Register BDS callback to do an EFI boot

**/
VOID
EFIAPI
BdsUiInitialize (
  IN  EFI_BOOT_MODE               BootMode,
  IN  BDS_UI_EFI_BOOT_CALL_BACK   EfiBootCallBack  OPTIONAL
  )
{
  mEfiBootCallBack = EfiBootCallBack;

  CopyMem (&mStoMode, gST->ConOut->Mode, sizeof (EFI_SIMPLE_TEXT_OUTPUT_MODE));
  gST->ConOut->QueryMode (gST->ConOut, gST->ConOut->Mode->Mode, &mStoMaxColumns, &mStoMaxRows);

  mAttributes        = PcdGet32 (PcdBdsUiTextAttribute); 
  mInverseAttributes = PcdGet32 (PcdBdsUiTextInverseAttribute); 

  //
  // The data hub strings take up the first 4 rows
  //
  mStartRow       = PcdGet32 (PcdBdsUiTextStartRow) + 4;
  mStartColumn    = PcdGet32 (PcdBdsUiTextStartColumn);
}



/**
  Update the Option Line with the bang and value and the string.
  Needed to change the highlighted option and bang for a given Boot Option.

  @param  BootOptionIndex   Boot Option to update

**/
VOID
UpdateOptionLine (
  IN UINTN      BootOptionIndex
  )
{
  CHAR16    PreChar[2];

  if (gPickList == NULL) {
    return;
  }

  //
  // Print pre character.
  //
  PreChar[1] = 0;
  gST->ConOut->SetCursorPosition (gST->ConOut, gPickList[BootOptionIndex].CursorColumn - 1, gPickList[BootOptionIndex].CursorRow);
  gST->ConOut->SetAttribute (gST->ConOut, mAttributes);
  PreChar[0] = ' ';
  gST->ConOut->OutputString (gST->ConOut, PreChar);
    
  //
  // Print string
  //
  gST->ConOut->SetAttribute (gST->ConOut, gPickList[BootOptionIndex].Attribute);
  gST->ConOut->OutputString (gST->ConOut, gPickList[BootOptionIndex].String);
}

EFI_STATUS
AppendPickListEntry (
  IN  EFI_BOOT_MANAGER_BOOT_OPTION  *BootOption,
  IN  BOOLEAN                       Removable
  )
{
  PLATFORM_UI_PICK_LIST_ENTRY   NewEntry;
  UINTN                         Len;
  UINTN                         Size;

  if (mPickListMaxCount <= gPickListCount) {
    //
    // Allocate more space for the entry
    //
    if (mPickListMaxCount == 0) {
      mPickListMaxCount = 10;
    }
    Size = mPickListMaxCount * sizeof (PLATFORM_UI_PICK_LIST_ENTRY);
    gPickList = ReallocateCopyPool (Size * 2, Size, gPickList);
    mPickListMaxCount *= 2;
  }    

  NewEntry.BootOption        = BootOption;
  NewEntry.Attribute         = (gPickListCount == 0) ? mInverseAttributes : mAttributes;
  NewEntry.CursorColumn      = mStartColumn;
  NewEntry.MaxCursorColumn   = PcdGet32 (PcdBdsUiTextMaxColumn);
  NewEntry.CursorRow         = mStartRow + gPickListCount;
  NewEntry.Removable         = Removable;

  //
  // Update the String and pad it with spaces
  //
  NewEntry.String[0] = 0x0000;
  StrnCpy (NewEntry.String, BootOption->Description, 255);
  Len = StrLen (NewEntry.String);
  SetMem16 (NewEntry.String + Len, ((NewEntry.MaxCursorColumn - NewEntry.CursorColumn) - Len) * sizeof (UINT16), 0x0020); //

  // Append Entry to the end of the pick list
  CopyMem (gPickList + gPickListCount,  &NewEntry, sizeof (PLATFORM_UI_PICK_LIST_ENTRY));
  
  gPickListCount++;
  return EFI_SUCCESS;
}

/**

  Acquire the string associated with the Index from smbios structure and return it.
  The caller is responsible for free the string buffer.

  @param    OptionalStrStart  The start position to search the string
  @param    Index             The index of the string to extract
  @param    String            The string that is extracted

  @retval   EFI_SUCCESS       The function returns EFI_SUCCESS always.

**/
EFI_STATUS
GetOptionalStringByIndex (
  IN      CHAR8                   *OptionalStrStart,
  IN      UINT8                   Index,
  OUT     CHAR16                  **String
  )
{
  UINTN          StrSize;

  if (Index == 0) {
    *String = AllocateZeroPool (sizeof (CHAR16));
    return EFI_SUCCESS;
  }

  StrSize = 0;
  do {
    Index--;
    OptionalStrStart += StrSize;
    StrSize           = AsciiStrSize (OptionalStrStart);
  } while (OptionalStrStart[StrSize] != 0 && Index != 0);

  if ((Index != 0) || (StrSize == 1)) {
    //
    // Meet the end of strings set but Index is non-zero, or
    // Find an empty string
    //
    *String = AllocatePool (sizeof (L"Miss String"));
    CopyMem (String, L"Miss String", sizeof (L"Miss String"));
  } else {
    *String = AllocatePool (StrSize * sizeof (CHAR16));
    AsciiStrToUnicodeStr (OptionalStrStart, *String);
  }

  return EFI_SUCCESS;
}

/**
  Convert Processor Frequency Data to a string.

  @param ProcessorFrequency The frequency data to process
  @param Base10Exponent     The exponent based on 10
  @param String             The string that is created

**/
VOID
ConvertProcessorToString (
  IN  UINT16                               ProcessorFrequency,
  IN  UINT16                               Base10Exponent,
  OUT CHAR16                               **String
  )
{
  CHAR16  *StringBuffer;
  UINTN   Index;
  UINT32  FreqMhz;

  if (Base10Exponent >= 6) {
    FreqMhz = ProcessorFrequency;
    for (Index = 0; Index < (UINTN) (Base10Exponent - 6); Index++) {
      FreqMhz *= 10;
    }
  } else {
    FreqMhz = 0;
  }

  StringBuffer = AllocateZeroPool (0x20);
  ASSERT (StringBuffer != NULL);
  Index = UnicodeValueToString (StringBuffer, LEFT_JUSTIFY, FreqMhz / 1000, 3);
  StrCat (StringBuffer, L".");
  UnicodeValueToString (StringBuffer + Index + 1, PREFIX_ZERO, (FreqMhz % 1000) / 10, 2);
  StrCat (StringBuffer, L" GHz");
  *String = (CHAR16 *) StringBuffer;
  return ;
}


/**
  Update the banner information for the Front Page based on DataHub information.

**/
VOID
BdsGetCpuInfo (
  OUT  CHAR16       **CpuSpeed,
  OUT  CHAR16       **CpuModel
  )
{
  UINT8                             StrIndex;
  BOOLEAN                           Find;
  EFI_STATUS                        Status;
  EFI_SMBIOS_HANDLE                 SmbiosHandle;
  EFI_SMBIOS_PROTOCOL               *Smbios;
  EFI_SMBIOS_TABLE_HEADER           *Record;
  SMBIOS_TABLE_TYPE4                *Type4Record;
  
  Find = FALSE;
  //
  // Update Front Page strings
  //
  Status = gBS->LocateProtocol (
                  &gEfiSmbiosProtocolGuid,
                  NULL,
                  (VOID **) &Smbios
                  );
  if (EFI_ERROR (Status)) {
    return;
  }  

  SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;
  do {
    Status = Smbios->GetNext (Smbios, &SmbiosHandle, NULL, &Record, NULL);
    if (EFI_ERROR(Status)) {
      break;
    }

    switch (Record->Type) {        
    case EFI_SMBIOS_TYPE_PROCESSOR_INFORMATION:
      Type4Record = (SMBIOS_TABLE_TYPE4 *) Record;

      //
      // Get Cpu Model
      //
      StrIndex = Type4Record->ProcessorVersion;
      GetOptionalStringByIndex ((CHAR8*)((UINT8*)Type4Record + Type4Record->Hdr.Length), StrIndex, CpuModel);

      //
      // Get Cpu Speed
      //
      ConvertProcessorToString(Type4Record->CurrentSpeed, 6, CpuSpeed);
      Find = TRUE;
      break;

    default:
      break;
    }
  } while (!Find);
  return ;
}


VOID
EFIAPI
BdsUiRepaintMenu (
  VOID
  )
{
  UINTN   Index;
  CHAR8   *BoardName;
  CHAR16  *ComputerModel;
  CHAR16  *CpuModel;
  CHAR16  *CpuSpeed;
  CHAR16  BiosVersion[80];
  CHAR16  MemorySize[80];
  UINTN   Size;

  ComputerModel = NULL;
  CpuModel = NULL;
  CpuSpeed = NULL;
  BoardName = (CHAR8 *)PcdGetPtr (PcdSMBIOSBoardProductName);
  if ( NULL != BoardName ) {
    Size = AsciiStrLen (BoardName) + 1;
    ComputerModel = (CHAR16 *)AllocatePool (Size * sizeof (CHAR16));
    AsciiStrToUnicodeStr (BoardName, ComputerModel);
  }

  Size = 4;
  BdsGetCpuInfo(&CpuSpeed, &CpuModel);

  //
  // If CPU info is not available, then use default names
  //
  if (CpuModel == NULL) {
    DEBUG ((EFI_D_INFO, "Can't get CpuModule n"));
    CpuModel = AllocateZeroPool (sizeof (L"CPU Model Unknown"));
    CopyMem (CpuModel, L"Pentium", sizeof (L"CPU Model Unknown"));
  }
  if (CpuSpeed == NULL) {
    DEBUG ((EFI_D_INFO, "Can't get CpuSpeed\n"));
    CpuSpeed = AllocateZeroPool (sizeof (L"CPU Speed Unknown"));
    CopyMem (CpuSpeed, L"CPU Speed Unknown", sizeof (L"CPU Speed Unknown"));
  }
  
  UnicodeSPrint (BiosVersion, sizeof (BiosVersion), L"%s Build %d on %a", gST->FirmwareVendor, gST->FirmwareRevision, __DATE__);
  UnicodeSPrint (MemorySize, sizeof (MemorySize), L"%d MB", Size);

  gST->ConOut->SetAttribute      (gST->ConOut, EFI_TEXT_ATTR (EFI_LIGHTGRAY, EFI_BLACK));
  gST->ConOut->ClearScreen       (gST->ConOut);
  gST->ConOut->EnableCursor      (gST->ConOut, FALSE);

  gST->ConOut->SetCursorPosition (gST->ConOut, mStartColumn, 1);
  gST->ConOut->OutputString      (gST->ConOut, ComputerModel);

  gST->ConOut->SetCursorPosition (gST->ConOut, mStartColumn, 2);
  if (CpuModel != NULL) {
    gST->ConOut->OutputString    (gST->ConOut, CpuModel);
  }
  gST->ConOut->SetCursorPosition (gST->ConOut, 60, 2);
  if (CpuSpeed != NULL) {
    gST->ConOut->OutputString    (gST->ConOut, CpuSpeed);
  }

  gST->ConOut->SetCursorPosition (gST->ConOut, mStartColumn, 3);
  gST->ConOut->OutputString      (gST->ConOut, BiosVersion);
  gST->ConOut->SetCursorPosition (gST->ConOut, 60, 3);
  gST->ConOut->OutputString      (gST->ConOut, MemorySize);

  for (Index = 0; Index < gPickListCount; Index++) {
    UpdateOptionLine (Index);
  }

  gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR (EFI_LIGHTGRAY, EFI_BLACK));
  if (CpuSpeed != NULL) {
    FreePool (CpuSpeed);
  }
  if (CpuModel != NULL) {
    FreePool (CpuModel);
  }
  if (ComputerModel != NULL) {
    FreePool (ComputerModel);
  }
}

/**
  Check if the current specific mode supported the user defined resolution
  for the Graphics Console device based on Graphics Output Protocol.

  If yes, set the graphic devcice's current mode to this specific mode.

  @param  GraphicsOutput        Graphics Output Protocol instance pointer.
  @param  HorizontalResolution  User defined horizontal resolution
  @param  VerticalResolution    User defined vertical resolution.
  @param  CurrentModeNumber     Current specific mode to be check.

  @retval EFI_SUCCESS       The mode is supported.
  @retval EFI_UNSUPPORTED   The specific mode is out of range of graphics
                            device supported.
  @retval other             The specific mode does not support user defined
                            resolution or failed to set the current mode to the
                            specific mode on graphics device.

**/
EFI_STATUS
CheckModeSupported (
  EFI_GRAPHICS_OUTPUT_PROTOCOL  *GraphicsOutput,
  IN  UINT32                    HorizontalResolution,
  IN  UINT32                    VerticalResolution,
  OUT UINT32                    *CurrentModeNumber
  )
{
  UINT32     ModeNumber;
  EFI_STATUS Status;
  UINTN      SizeOfInfo;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
  UINT32     MaxMode;

  Status  = EFI_SUCCESS;
  MaxMode = GraphicsOutput->Mode->MaxMode;

  for (ModeNumber = 0; ModeNumber < MaxMode; ModeNumber++) {
    Status = GraphicsOutput->QueryMode (
                       GraphicsOutput,
                       ModeNumber,
                       &SizeOfInfo,
                       &Info
                       );
    if (!EFI_ERROR (Status)) {
      if ((Info->HorizontalResolution == HorizontalResolution) &&
          (Info->VerticalResolution == VerticalResolution)) {
        if ((GraphicsOutput->Mode->Info->HorizontalResolution == HorizontalResolution) &&
            (GraphicsOutput->Mode->Info->VerticalResolution == VerticalResolution)) {
          //
          // If video device has been set to this mode, we do not need to SetMode again
          //
          FreePool (Info);
          break;
        } else {
          Status = GraphicsOutput->SetMode (GraphicsOutput, ModeNumber);
          if (!EFI_ERROR (Status)) {
            FreePool (Info);
            break;
          }
        }
      }
      FreePool (Info);
    }
  }

  if (ModeNumber == GraphicsOutput->Mode->MaxMode) {
    Status = EFI_UNSUPPORTED;
  }

  *CurrentModeNumber = ModeNumber;
  return Status;
}

/**
  Initialize the Graphic Device Resolution.

  @param[in]  GraphicsOutput   Pointer of Graphic Output Protocol.
  @param[in]  UgaDraw          Pointer of UGA DRAW Protocol.

  @retval EFI_SUCCESS     Initilized successfully.
  @retval EFI_UNSUPPORTED Operation failed. 

**/

EFI_STATUS
SetGraphicDevice (
  IN    EFI_GRAPHICS_OUTPUT_PROTOCOL  *GraphicsOutput,
  IN    EFI_UGA_DRAW_PROTOCOL         *UgaDraw
  )
{
  EFI_STATUS                           Status;
  UINT32                               HorizontalResolution;
  UINT32                               VerticalResolution;
  UINT32                               ColorDepth;
  UINT32                               RefreshRate;
  UINT32                               ModeIndex;
  UINTN                                MaxMode;
  UINT32                               ModeNumber;
  UINTN                                SizeOfInfo;  
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;

  Status = EFI_SUCCESS;
  
  HorizontalResolution  = PcdGet32 (PcdVideoHorizontalResolution);
  VerticalResolution    = PcdGet32 (PcdVideoVerticalResolution);

  if (GraphicsOutput != NULL) {
    //
    // The console is build on top of Graphics Output Protocol, find the mode number
    // for the user-defined mode; if there are multiple video devices,
    // graphic console driver will set all the video devices to the same mode.
    //
    if ((HorizontalResolution == 0x0) || (VerticalResolution == 0x0)) {
      //
      // Find the highest resolution which GOP supports.
      //    
      MaxMode = GraphicsOutput->Mode->MaxMode;
      
      for (ModeIndex = 0; ModeIndex < MaxMode; ModeIndex++) {
        Status = GraphicsOutput->QueryMode (
                           GraphicsOutput,
                           ModeIndex,
                           &SizeOfInfo,
                           &Info
                           );
        if (!EFI_ERROR (Status)) {
          if ((Info->HorizontalResolution >= HorizontalResolution) &&
              (Info->VerticalResolution >= VerticalResolution)) {
            HorizontalResolution = Info->HorizontalResolution;
            VerticalResolution   = Info->VerticalResolution;
            ModeNumber           = ModeIndex;
          }
          FreePool (Info);
        }
      }
      if ((HorizontalResolution == 0x0) || (VerticalResolution == 0x0)) {
        Status = EFI_UNSUPPORTED;
        return Status;
      }
    } else {
      //
      // Use user-defined resolution
      //
      Status = CheckModeSupported (
                   GraphicsOutput,
                   HorizontalResolution,
                   VerticalResolution,
                   &ModeNumber
                   );
      if (EFI_ERROR (Status)) {
        //
        // if not supporting current mode, try 800x600 which is required by UEFI/EFI spec
        //
        Status = CheckModeSupported (
                     GraphicsOutput,
                     800,
                     600,
                     &ModeNumber
                     );
      }
    }
  } else if (FeaturePcdGet (PcdUgaConsumeSupport)) {
    //
    // At first try to set user-defined resolution
    //
    ColorDepth            = 32;
    RefreshRate           = 60;
    Status = UgaDraw->SetMode (
                        UgaDraw,
                        HorizontalResolution,
                        VerticalResolution,
                        ColorDepth,
                        RefreshRate
                        );
    if (EFI_ERROR (Status)) {
      //
      // Try to set 800*600 which is required by UEFI/EFI spec
      //
      Status = UgaDraw->SetMode (
                 UgaDraw,
                 800,
                 600,
                 ColorDepth,
                 RefreshRate
                 );
      if (EFI_ERROR (Status)) {
        Status = UgaDraw->GetMode (
                   UgaDraw,
                   &HorizontalResolution,
                   &VerticalResolution,
                   &ColorDepth,
                   &RefreshRate
                   );
        if (EFI_ERROR (Status)) {
          return Status;
        }
      }
    } else {
      Status = EFI_UNSUPPORTED;      
    }
  }
  return Status;
}

EFI_STATUS
EFIAPI
FindGraphicAndUgaProtocol (
  OUT EFI_GRAPHICS_OUTPUT_PROTOCOL  **GraphicsOutput, 
  OUT EFI_UGA_DRAW_PROTOCOL         **UgaDraw
  ) 

{
  UINTN                           Size;
  EFI_HANDLE                      Handle;
  UINTN                           Index;
  EFI_HANDLE                      *GraphicHandle;
  EFI_HANDLE                      *UgaDrawHandle;
  EFI_DEVICE_PATH_PROTOCOL        *DevicePath;
  EFI_STATUS                      Status;
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *SimpleTextOut;

  *GraphicsOutput = NULL;
  *UgaDraw        = NULL;
  GraphicHandle   = NULL;
  UgaDrawHandle   = NULL;

  Size = sizeof (EFI_HANDLE);
  Status = gBS->LocateHandle (
                  ByProtocol,
                  &gEfiGraphicsOutputProtocolGuid,
                  NULL,
                  &Size,
                  (VOID **) &Handle
                  );
  DEBUG ((DEBUG_ERROR, "GraphicsOutput LocateHandle  = %r\n", Status));
  if (Status == EFI_BUFFER_TOO_SMALL) {
    GraphicHandle = (EFI_HANDLE*) AllocateZeroPool (Size);
    Status = gBS->LocateHandle (
                    ByProtocol,
                    &gEfiGraphicsOutputProtocolGuid,
                    NULL,
                    &Size,
                    (VOID **) GraphicHandle
                    );
  } else if (Status != EFI_SUCCESS) {
    return EFI_UNSUPPORTED;
  }
    
  for ( Index = 0; Index < (Size/sizeof (EFI_HANDLE)); Index++) {
    if (GraphicHandle != NULL) {
      CopyMem ( (UINT8*) &Handle, ((UINT8 *) GraphicHandle) + Index * sizeof (EFI_HANDLE), sizeof (EFI_HANDLE));
    }
    DEBUG ((EFI_D_INFO, "Graphic Handle(1)=%x\n", Handle));
    DevicePath = NULL;
    DevicePath = DevicePathFromHandle (Handle);
    if ( DevicePath != NULL) {
      Status = gBS->HandleProtocol (Handle, &gEfiGraphicsOutputProtocolGuid, (VOID **) GraphicsOutput);
      if (Status == EFI_SUCCESS) {
        Status = gBS->HandleProtocol (Handle, &gEfiSimpleTextOutProtocolGuid, (VOID **) &SimpleTextOut);
        if (EFI_ERROR (Status)) {
         //
         // If no GraphicConsole, initialize graphic device.
          //
          SetGraphicDevice (*GraphicsOutput, *UgaDraw);
        }
        DEBUG ((EFI_D_INFO, "Graphic Handle GraphicOUtput(2)=%x\n", *GraphicsOutput));
        break;
      }      
    }
  }  
  
  if (GraphicsOutput == NULL && FeaturePcdGet (PcdUgaConsumeSupport)) {
    //
    // Open GOP failed, try to open UGA
    //
    Size = sizeof (EFI_HANDLE);
    Status = gBS->LocateHandle (
                    ByProtocol,
                    &gEfiUgaDrawProtocolGuid,
                    NULL,
                    &Size,
                    (VOID **) &Handle
                    );
    DEBUG ((DEBUG_ERROR, "UgaDrawProtocol LocateHandle  = %r\n", Status));
    if (Status == EFI_BUFFER_TOO_SMALL) {
      UgaDrawHandle = (EFI_HANDLE*) AllocateZeroPool (Size);
      Status = gBS->LocateHandle (
                      ByProtocol,
                      &gEfiUgaDrawProtocolGuid,
                      NULL,
                      &Size,
                      (VOID **) UgaDrawHandle
                      );
    }
    
    for ( Index = 0; Index < (Size/sizeof (EFI_HANDLE)); Index++) {
      CopyMem ( (UINT8*) &Handle, ((UINT8 *) UgaDrawHandle) + Index * sizeof (EFI_HANDLE), sizeof (EFI_HANDLE));
      DEBUG ((EFI_D_INFO, "UgaDrawProtocol Handle(1)=%x\n", Handle));
      DevicePath = NULL;
      DevicePath = DevicePathFromHandle (Handle);
      if ( DevicePath != NULL) {
        gBS->HandleProtocol (Handle, &gEfiUgaDrawProtocolGuid, (VOID **) UgaDraw);
        Status = gBS->HandleProtocol (Handle, &gEfiSimpleTextOutProtocolGuid, (VOID **) &SimpleTextOut);
        if (EFI_ERROR (Status)) {
          //
          // If no GraphicConsole, initialize Uga device.
          //
          SetGraphicDevice (*GraphicsOutput, *UgaDraw);
        }
        DEBUG ((EFI_D_INFO, "UgaDraw Handle Draw(2)=%x\n", *UgaDraw));
        break;
      }
    }
  }
  
  if (GraphicHandle != NULL) {
    FreePool (GraphicHandle);
  }
  if (UgaDrawHandle != NULL) {
    FreePool (UgaDrawHandle);
  }
  return Status;
}

/**
  Use SystemTable Conout to stop video based Simple Text Out consoles from going
  to the video device. Put up LogoFile on every video device that is a console.

  @param[in]  LogoFile   File name of logo to display on the center of the screen.

  @retval EFI_SUCCESS     ConsoleControl has been flipped to graphics and logo displayed.
  @retval EFI_UNSUPPORTED Logo not found

**/
EFI_STATUS
EFIAPI
EnableQuietBoot (
  IN  EFI_GUID  *LogoFile
  )
{
  EFI_STATUS                    Status;
  EFI_OEM_BADGING_PROTOCOL      *Badging;
  UINT32                        SizeOfX;
  UINT32                        SizeOfY;
  INTN                          DestX;
  INTN                          DestY;
  UINT8                         *ImageData;
  UINTN                         ImageSize;
  UINTN                         BltSize;
  UINT32                        Instance;
  EFI_BADGING_FORMAT            Format;
  EFI_BADGING_DISPLAY_ATTRIBUTE Attribute;
  UINTN                         CoordinateX;
  UINTN                         CoordinateY;
  UINTN                         Height;
  UINTN                         Width;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Blt;
  EFI_UGA_DRAW_PROTOCOL         *UgaDraw;
  UINT32                        ColorDepth;
  UINT32                        RefreshRate;
  EFI_GRAPHICS_OUTPUT_PROTOCOL  *GraphicsOutput;
  EFI_BOOT_LOGO_PROTOCOL        *BootLogo;
  UINTN                         NumberOfLogos;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *LogoBlt;
  UINTN                         LogoDestX;
  UINTN                         LogoDestY;
  UINTN                         LogoHeight;
  UINTN                         LogoWidth;
  UINTN                         NewDestX;
  UINTN                         NewDestY;
  UINTN                         NewHeight;
  UINTN                         NewWidth;

  if (LogoFile == NULL) return EFI_SUCCESS;
  //
  // Find Graphic Protocol or UgaDraw Protocol.
  //
  FindGraphicAndUgaProtocol (&GraphicsOutput, &UgaDraw);  
  if (GraphicsOutput == NULL && UgaDraw == NULL) {
      return EFI_UNSUPPORTED;
  }

  //
  // Try to open Boot Logo Protocol.
  //
  BootLogo = NULL;
  gBS->LocateProtocol (&gEfiBootLogoProtocolGuid, NULL, (VOID **) &BootLogo);

  //
  // Erase Cursor from screen
  //
  gST->ConOut->EnableCursor (gST->ConOut, FALSE);

  Badging = NULL;
  Status  = gBS->LocateProtocol (&gEfiOEMBadgingProtocolGuid, NULL, (VOID **) &Badging);

  if (GraphicsOutput != NULL) {
    SizeOfX = GraphicsOutput->Mode->Info->HorizontalResolution;
    SizeOfY = GraphicsOutput->Mode->Info->VerticalResolution;

  } else if (UgaDraw != NULL && FeaturePcdGet (PcdUgaConsumeSupport)) {
    Status = UgaDraw->GetMode (UgaDraw, &SizeOfX, &SizeOfY, &ColorDepth, &RefreshRate);
    if (EFI_ERROR (Status)) {
      return EFI_UNSUPPORTED;
    }
  } else {
    return EFI_UNSUPPORTED;
  }

  Blt = NULL;
  NumberOfLogos = 0;
  LogoDestX = 0;
  LogoDestY = 0;
  LogoHeight = 0;
  LogoWidth = 0;
  NewDestX = 0;
  NewDestY = 0;
  NewHeight = 0;
  NewWidth = 0;
  Instance = 0;
  while (1) {
    ImageData = NULL;
    ImageSize = 0;

    if (Badging != NULL) {
      //
      // Get image from OEMBadging protocol.
      //
      Status = Badging->GetImage (
                          Badging,
                          &Instance,
                          &Format,
                          &ImageData,
                          &ImageSize,
                          &Attribute,
                          &CoordinateX,
                          &CoordinateY
                          );
      if (EFI_ERROR (Status)) {
        goto Done;
      }

      //
      // Currently only support BMP format.
      //
      if (Format != EfiBadgingFormatBMP) {
        if (ImageData != NULL) {
          FreePool (ImageData);
        }
        continue;
      }
    } else {
      //
      // Get the specified image from FV.
      //
      Status = GetSectionFromAnyFv (LogoFile, EFI_SECTION_RAW, 0, (VOID **) &ImageData, &ImageSize);
      if (EFI_ERROR (Status)) {
        return EFI_UNSUPPORTED;
      }

      CoordinateX = 0;
      CoordinateY = 0;
      Attribute   = EfiBadgingDisplayAttributeCenter;
    }

    if (Blt != NULL) {
      FreePool (Blt);
    }
    Blt = NULL;
    Status = ConvertBmpToGopBlt (
              ImageData,
              ImageSize,
              (VOID **) &Blt,
              &BltSize,
              &Height,
              &Width
              );
    if (EFI_ERROR (Status)) {
      FreePool (ImageData);

      if (Badging == NULL) {
        return Status;
      } else {
        continue;
      }
    }

    //
    // Calculate the display position according to Attribute.
    //
    switch (Attribute) {
    case EfiBadgingDisplayAttributeLeftTop:
      DestX = CoordinateX;
      DestY = CoordinateY;
      break;

    case EfiBadgingDisplayAttributeCenterTop:
      DestX = (SizeOfX - Width) / 2;
      DestY = CoordinateY;
      break;

    case EfiBadgingDisplayAttributeRightTop:
      DestX = (SizeOfX - Width - CoordinateX);
      DestY = CoordinateY;;
      break;

    case EfiBadgingDisplayAttributeCenterRight:
      DestX = (SizeOfX - Width - CoordinateX);
      DestY = (SizeOfY - Height) / 2;
      break;

    case EfiBadgingDisplayAttributeRightBottom:
      DestX = (SizeOfX - Width - CoordinateX);
      DestY = (SizeOfY - Height - CoordinateY);
      break;

    case EfiBadgingDisplayAttributeCenterBottom:
      DestX = (SizeOfX - Width) / 2;
      DestY = (SizeOfY - Height - CoordinateY);
      break;

    case EfiBadgingDisplayAttributeLeftBottom:
      DestX = CoordinateX;
      DestY = (SizeOfY - Height - CoordinateY);
      break;

    case EfiBadgingDisplayAttributeCenterLeft:
      DestX = CoordinateX;
      DestY = (SizeOfY - Height) / 2;
      break;

    case EfiBadgingDisplayAttributeCenter:
      DestX = (SizeOfX - Width) / 2;
      DestY = (SizeOfY - Height) / 2;
      break;

    default:
      DestX = CoordinateX;
      DestY = CoordinateY;
      break;
    }

    if ((DestX >= 0) && (DestY >= 0)) {
      if (GraphicsOutput != NULL) {
        Status = GraphicsOutput->Blt (
                            GraphicsOutput,
                            Blt,
                            EfiBltBufferToVideo,
                            0,
                            0,
                            (UINTN) DestX,
                            (UINTN) DestY,
                            Width,
                            Height,
                            Width * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
                            );
      } else if (UgaDraw != NULL && FeaturePcdGet (PcdUgaConsumeSupport)) {
        Status = UgaDraw->Blt (
                            UgaDraw,
                            (EFI_UGA_PIXEL *) Blt,
                            EfiUgaBltBufferToVideo,
                            0,
                            0,
                            (UINTN) DestX,
                            (UINTN) DestY,
                            Width,
                            Height,
                            Width * sizeof (EFI_UGA_PIXEL)
                            );
      } else {
        Status = EFI_UNSUPPORTED;
      }

      //
      // Report displayed Logo information.
      //
      if (!EFI_ERROR (Status)) {
        NumberOfLogos++;

        if (LogoWidth == 0) {
          //
          // The first Logo.
          //
          LogoDestX = (UINTN) DestX;
          LogoDestY = (UINTN) DestY;
          LogoWidth = Width;
          LogoHeight = Height;
        } else {
          //
          // Merge new logo with old one.
          //
          NewDestX = MIN ((UINTN) DestX, LogoDestX);
          NewDestY = MIN ((UINTN) DestY, LogoDestY);
          NewWidth = MAX ((UINTN) DestX + Width, LogoDestX + LogoWidth) - NewDestX;
          NewHeight = MAX ((UINTN) DestY + Height, LogoDestY + LogoHeight) - NewDestY;

          LogoDestX = NewDestX;
          LogoDestY = NewDestY;
          LogoWidth = NewWidth;
          LogoHeight = NewHeight;
        }
      }
    }

    FreePool (ImageData);

    if (Badging == NULL) {
      break;
    }
  }

Done:
  if (BootLogo == NULL || NumberOfLogos == 0) {
    //
    // No logo displayed.
    //
    if (Blt != NULL) {
      FreePool (Blt);
    }

    return Status;
  }

  //
  // Advertise displayed Logo information.
  //
  if (NumberOfLogos == 1) {
    //
    // Only one logo displayed, use its Blt buffer directly for BootLogo protocol.
    //
    LogoBlt = Blt;
    Status = EFI_SUCCESS;
  } else {
    //
    // More than one Logo displayed, get merged BltBuffer using VideoToBuffer operation. 
    //
    if (Blt != NULL) {
      FreePool (Blt);
    }

    LogoBlt = AllocateZeroPool (LogoWidth * LogoHeight * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
    if (LogoBlt == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }

    if (GraphicsOutput != NULL) {
      Status = GraphicsOutput->Blt (
                          GraphicsOutput,
                          LogoBlt,
                          EfiBltVideoToBltBuffer,
                          LogoDestX,
                          LogoDestY,
                          0,
                          0,
                          LogoWidth,
                          LogoHeight,
                          LogoWidth * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
                          );
    } else if (UgaDraw != NULL && FeaturePcdGet (PcdUgaConsumeSupport)) {
      Status = UgaDraw->Blt (
                          UgaDraw,
                          (EFI_UGA_PIXEL *) LogoBlt,
                          EfiUgaVideoToBltBuffer,
                          LogoDestX,
                          LogoDestY,
                          0,
                          0,
                          LogoWidth,
                          LogoHeight,
                          LogoWidth * sizeof (EFI_UGA_PIXEL)
                          );
    } else {
      Status = EFI_UNSUPPORTED;
    }
  }

  if (!EFI_ERROR (Status)) {
    BootLogo->SetBootLogo (BootLogo, LogoBlt, LogoDestX, LogoDestY, LogoWidth, LogoHeight);
  }
  FreePool (LogoBlt);

  return Status;
}


/**
  Called by BDS prior to processing the UEFI boot options. A current set of UEFI
  boot options are passed in. This function produces the UEFI L"Timeout" behavior.
  A Timeout value of zero means the UI may be updated and this function will return
  after the UI is updated. A timeout value of non zero will make this routine wait 
  the prescribed number of seconds. If a key is pressed BdsUiInteractiveMenus () must
  be called and the Key value and CallBack are passed to it. It is legal to process 
  hotkeys in this function via calls to BdsUiInteractiveMenus().

  @param  Timeout         UEFI L"Timeout" variable value.
  @param  BootOptions     Array of boot option information
  @param  BootOptionCount Number of array entries in BootOption

**/
VOID
EFIAPI
BdsUiUpdateBootOptions (
 IN EFI_BOOT_MANAGER_BOOT_OPTION    *BootOptions,
 IN UINTN                           BootOptionCount,
 OUT  EFI_KEY_DATA                  *Key
 )
{
  UINTN      Index;
  EFI_EVENT  TimerEvent;
  EFI_STATUS Status;
  
  if (BootOptions != NULL) {
    for (Index = 0; Index < BootOptionCount; Index++) {
      AppendPickListEntry (&BootOptions[Index], FALSE);
    }
  }

  EnableQuietBoot (PcdGetPtr (PcdLogoFile));

  TimerEvent = NULL;
  Status = gBS->CreateEvent (
                  EVT_TIMER,
                  0,
                  NULL,
                  NULL,
                  &TimerEvent
                  );
  if (!EFI_ERROR (Status)) {
    //
    // Set 1s timer.
    //
    gBS->SetTimer (TimerEvent, TimerPeriodic, 10000000);
    DEBUG ((EFI_D_ERROR, "Create Timer Event!\n"));
    if (!EFI_ERROR (Status)) {
      gBS->WaitForEvent (1, &TimerEvent, &Index);
      gBS->CloseEvent (&TimerEvent);
    }
  }

  BdsUiRepaintMenu ();
}

/**
  Delete BootOptions added via BdsUiUpdateBootOptions().

  @param  BootOptions     Array of boot option information
  @param  BootOptionCount Number of array entries in BootOption

**/
VOID
EFIAPI
BdsUiDeleteBootOptions (
 IN EFI_BOOT_MANAGER_BOOT_OPTION    *BootOptions,
 IN UINTN                           BootOptionCount
 )
{
  if (gPickList != NULL) {
    FreePool (gPickList);
    gPickList = NULL;
    gPickListCount = 0;
    mPickListMaxCount = 0;
    gPickListSelection = 0;
  }

  if (mDefaultBootOptionsCount != 0) {
    if (mDefaultBootOptions != NULL) {
      FreePool (mDefaultBootOptions);
      mDefaultBootOptions = NULL;
      mDefaultBootOptionsCount = 0;
      mDefaultBootOptionsMaxCount = 0;
    }
  }
}


VOID
RestoreScreenToInitialState (
  VOID
  )
{
  gST->ConOut->SetAttribute (gST->ConOut, mStoMode.Attribute);
  gST->ConOut->ClearScreen  (gST->ConOut);
  gST->ConOut->EnableCursor (gST->ConOut, mStoMode.CursorVisible);
}

/**
  Internal function for BDS to handle the interactive menus. 
  @param  Period          100ns increments to wait for key press   
 
**/

EFI_STATUS
BdsUiInteractiveMenusInternal (
  EFI_KEY_DATA    *Key, 
  UINT64          Period
  )
{
  EFI_STATUS    Status;
  BOOLEAN       ScreenUpdate;
  BOOLEAN       UiKeyHit;
  UINTN         BootOptionsCount;    
  

  ScreenUpdate = FALSE;
  UiKeyHit = TRUE;
  
  if ((Key->Key.ScanCode == SCAN_UP) || (Key->Key.UnicodeChar == '+')) {
    if (gPickListSelection > 0) {
      ScreenUpdate = TRUE;
      if (gPickList != NULL) {
        gPickList[gPickListSelection].Attribute = mAttributes;
        UpdateOptionLine (gPickListSelection);
        gPickList[--gPickListSelection].Attribute = mInverseAttributes;
      }
    }
  } else if ((Key->Key.ScanCode == SCAN_DOWN) || (Key->Key.UnicodeChar == '-')) {
    if (gPickListSelection < (gPickListCount - 1)) {
      ScreenUpdate = TRUE;
      if (gPickList != NULL) {
        gPickList[gPickListSelection].Attribute = mAttributes;
        UpdateOptionLine (gPickListSelection);
        gPickList[++gPickListSelection].Attribute = mInverseAttributes;
      }
    }
  } else if (Key->Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
    RestoreScreenToInitialState ();
    if ((mEfiBootCallBack != NULL) && (gPickList != NULL)) {
      mEfiBootCallBack (gPickList[gPickListSelection].BootOption);
    }
    //
    // Clear up BootOptions.
    //
    BootOptionsCount = 0;
    BdsUiDeleteBootOptions (NULL, BootOptionsCount); 
    PlatformBdsGetBootOptions (&BootOptionsCount, TRUE);
    BdsUiRepaintMenu ();     
  } else if ((Key->Key.ScanCode == SCAN_END) || (Key->Key.ScanCode == SCAN_ESC) ||
            (Key->Key.UnicodeChar == 'e')  || (Key->Key.UnicodeChar == 'E')) {
    //
    // Restore console to initial state
    //    
    RestoreScreenToInitialState ();
    BdsUiRepaintMenu ();
  } else {
    UiKeyHit = FALSE;
  }
  if (ScreenUpdate) {
    UpdateOptionLine (gPickListSelection);        
  }

  ZeroMem (Key, sizeof (EFI_KEY_DATA));
  Status = GetCharKeyWithTimeout (Key, Period);
  return Status;
}


/**
  Called by BDS to invoke an interactive memu. This is the end state for the boot
  processing if all boot options fail. An Optional pointer to a key pressed value 
  is supported to indicate that a key has been pressed and this value should be used
  by the UI.

  @param  KeyPress    Optionally pass in a value from the Simple Text In Protocol 

**/
VOID
EFIAPI
BdsUiInteractiveMenus (
  IN  EFI_KEY_DATA      *Key, 
  IN  UINT16            Timeout
  )
{
  EFI_STATUS      Status;
  UINT64          Period;
  EFI_BOOT_MODE     BootMode;

  BootMode = GetBootModeHob ();

  //
  // If it is headless configuration, go to first boot option directly.
  //
  if (gST->ConIn->WaitForKey == NULL) {
    if (gPickList != NULL) {
      mEfiBootCallBack (gPickList[0].BootOption);
    }    
  }

  if (PcdGetBool (PcdEnableFastBoot)  && (BootMode == BOOT_WITH_MINIMAL_CONFIGURATION || BootMode == BOOT_ASSUMING_NO_CONFIGURATION_CHANGES)) {
    if (gPickList != NULL) {
      mEfiBootCallBack (gPickList[0].BootOption);
    }
  }
  
  //
  // Check the KeyStroke 
  //
  Period = ONE_SECOND * Timeout;
  if ((Key->Key.ScanCode == 0) && (Key->Key.UnicodeChar == 0)) {
    Status = GetCharKeyWithTimeout (Key, Period);
  } else {
    Status = EFI_SUCCESS;
  }    

  //
  // Check the KeyStroke again. 
  //
  //if (Status == EFI_TIMEOUT) {
  //  Status = GetCharKeyWithTimeout (Key, Period);
  //}

  if (!EFI_ERROR (Status)) {
    Status = BdsUiInteractiveMenusInternal (Key, Period);
  }else {
    //
    // For first time, if there is no action from user, goto first boot option.
    //
    RestoreScreenToInitialState ();
    if (gPickList != NULL) {
      mEfiBootCallBack (gPickList[0].BootOption);
      BdsUiRepaintMenu (); 
    }    
  }

  Period = ONE_SECOND;
  for (;;) {
    if (Status == EFI_TIMEOUT) {
      Status = GetCharKeyWithTimeout (Key, Period);
    }

    if (!EFI_ERROR (Status)) {
      Status = BdsUiInteractiveMenusInternal (Key, Period);
    }
  }
}

/**
  Called by the BDS prior to attempting to boot a boot option. BootOptionIndex is an array
  index to the BootOptions argument passed to BdsUiUpdateBootOptions().

  @param  BootOptionIndex   Index into BootOptions the BDS is trying to boot from.

**/
VOID
EFIAPI
BdsUiUpdateBootAttempt (
  IN UINTN      BootOptionIndex
  )
{
  RestoreScreenToInitialState ();
}

/** @file
  User Interface Lib for BDS. This is the UI for the template BDS.

  The library instacne would include all the basic UI for a plaform. This
  would include all setup screens and utilities launched from the UI.
  Common BDS library routines

  Copyright (c) 2008, Intel Corporation                                                         
  All rights reserved. This program and the accompanying materials                          
  are licensed and made available under the terms and conditions of the BSD License         
  which accompanies this distribution.  The full text of the license may be found at        
  http://opensource.org/licenses/bsd-license.php                                            

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

**/

#ifndef __BDS_LIB_H__
#define __BDS_LIB_H__

#include <Protocol/DevicePath.h>
#include <Library/DebugLib.h>

#define ONE_SECOND  10000000

//
//  Specify the default boot order for the device
//  Add SKIP to enable zero or one based array indexing
//
typedef enum{
  BOOTORDERSKIP = 0,
  BOOTORDERDVDCD,
  BOOTORDERUSB,
  BOOTORDERSCSI,
  BOOTORDERFLOPPY,
  BOOTORDERHD,
  BOOTORDERMEMD,
  BOOTORDERNETWORK,
  BOOTORDERMAX
} PLATFORM_PCD_BOOT_OPTION_TYPE;

//
// Network adapter list
//
typedef struct {
  UINTN DevicePathLength;                 // Length of the device path in bytes
  EFI_DEVICE_PATH_PROTOCOL * DevicePath;  // Device path buffer
} NETWORK_ADAPTER;

// 
// Data structure to contain EFI Load Option information.
//
typedef struct {
  UINT16                    BootCurrent;        // XXXX numerical value
  CHAR16                    BootCurrentStr[20]; // BootXXXX DriverXXXX variable name string
  UINT32                    Attributes;         // Load Option Attributes
  CHAR16                    *Description;       // Load Option Description
  EFI_DEVICE_PATH_PROTOCOL  *FilePath;          // Load Option Device Path
  UINT8                     *OptionalData;      // Load Option optional data to pass into image
  UINT32                    OptionalDataSize;   // Load Option size of OptionalData
  BOOLEAN                   BootNext;           // TRUE if this was a L"BootNext" Variable
  EFI_STATUS                BootStatus;         // Status returned from boot attempt gBS->StartImage ()
  CHAR16                    *ExitData;          // Exit data returned from gBS->StartImage () 
  UINTN                     ExitDataSize;       // Size of ExitData
  BOOLEAN                   ConnectAll;         // If TRUE connect all devices before starting image
} EFI_BOOT_MANAGER_BOOT_OPTION;

typedef enum {
  PciConnectEmbedded,
  PciConnectPlugIn,  
  PciConnectBoth,
  PciConnectMaxEntry
} PCI_CONNECT_TYPE;

/**
  Get the EFI Boot Manager L"Timeout" variable setting. If the variable is 
  not valid set it to a default value if SetDefaultOnError is TRUE. Reguardless
  of SetDefaultOnError a valid default is always returned if the EFI variable
  is not valid.

  @param  SetDefaultOnError   Set default value on error

  @return Timeout value.

**/
UINT16
EfiBootManagerGetTimeout (
  IN  BOOLEAN       SetDefaultOnError
  );

/**
  Delete all UEFI specification variables. All EFI variables with the gEfiGlobalVariableGuid
  are owned by the UEFI specification and this routine will delete them. 

**/
VOID
EfiBootManagerDeleteVariables (
  VOID
  );

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
  );

EFI_STATUS
EfiBootManagerLoadImage (
  IN  EFI_BOOT_MANAGER_BOOT_OPTION    *BootOption,
  IN  EFI_HANDLE                      *ImageHandle,
  IN  BOOLEAN                         RemovableMediaSupport
  );

VOID 
EfiBootManagerConnectAll (
  IN  BOOLEAN   OnlyOncePerBoot
  );

VOID
EfiBootManagerInitializeBootOption (
  IN OUT EFI_BOOT_MANAGER_BOOT_OPTION *Option,
  IN  CHAR16                          *Description,
  IN  EFI_DEVICE_PATH_PROTOCOL        *FilePath,
  IN  UINT32                          Attributes,
  IN  UINT8                           *OptionalData,
  IN  UINT32                          OptionalDataSize
  );

VOID
BdsFlushKeyBuffer (
  VOID
  );

EFI_STATUS
BdsProcessKeyPress (
  IN  EFI_KEY_DATA                        *Key,
  OUT EFI_BOOT_MANAGER_BOOT_OPTION        **BootOption
  );

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
  );

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
  );

EFI_STATUS
EFIAPI
EfiConnectDevicePaths (
  IN EFI_DEVICE_PATH_PROTOCOL *DevicePathToConnect
  );

EFI_STATUS
EFIAPI
BdsLibBootOptionShell (
  IN OUT UINTN                            *BootOrder
  );

/*
EFI_STATUS
EFIAPI
BdsLibBootOptionPayload (
  IN OUT UINTN                            *BootOrder
  );
*/

EFI_STATUS
EFIAPI
BdsLibBootOptionNetwork (
  IN OUT UINTN                            *BootOrder
  );

EFI_STATUS
EFIAPI
BdsLibBootOptionUsb (
  IN OUT UINTN                            *BootOrder
  );

EFI_STATUS
EFIAPI
BdsLibBootOptionHardDisk (
  IN OUT UINTN                            *BootOrder
  );

EFI_STATUS
EFIAPI
BdsLibBootOptionCdDvd (
  IN OUT UINTN                            *BootOrder
  );

/*
EFI_STATUS
EFIAPI
BdsLibBootOptionOem (
  IN OUT UINTN                            *BootOrder
  );
*/

EFI_STATUS
EFIAPI
PlatformBdsGetBootOptions (
  OUT    UINTN                           *BootOptionCount,
  IN     BOOLEAN                          ReEnumerate
  );

/**
  This routine adjusts the memory information for different memory type and 
  saves them into the variables for next boot. It conditionally resets the
  system when the memory information changes. Platform can reserve memory 
  large enough (125% of actual requirement) to avoid the reset in the first boot.
**/
VOID
BdsSetMemoryTypeInformationVariable (
  VOID
  );


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
  );

/**
  Expand USB Class or USB WWID device path node to be full device path of a USB
  device in platform then load the boot file on this full device path and return the 
  image handle.

  This function support following 4 cases:
  1) Boot Option device path starts with a USB Class or USB WWID device path,
     and there is no Media FilePath device path in the end.
     In this case, it will follow Removable Media Boot Behavior.
  2) Boot Option device path starts with a USB Class or USB WWID device path,
     and ended with Media FilePath device path.
  3) Boot Option device path starts with a full device path to a USB Host Controller,
     contains a USB Class or USB WWID device path node, while not ended with Media
     FilePath device path. In this case, it will follow Removable Media Boot Behavior.
  4) Boot Option device path starts with a full device path to a USB Host Controller,
     contains a USB Class or USB WWID device path node, and ended with Media
     FilePath device path.

  @param  DevicePath    The Boot Option device path.

  @return  The image handle of boot file, or NULL if there is no boot file found in
           the specified USB Class or USB WWID device path.

**/
EFI_HANDLE *
BdsExpandUsbShortFormDevicePath (
  IN EFI_DEVICE_PATH_PROTOCOL       *DevicePath
  );

/**

  Allocates a block of memory and writes performance data of booting into it.
  OS can processing these record.
  
**/
VOID
WriteBootToOsPerformanceData (
  VOID
  );

VOID
AddLoadFileBootOptions (
  VOID
  );

VOID
BdsPrintMemoryMap (
  UINT16        *PrintString
  );

#endif

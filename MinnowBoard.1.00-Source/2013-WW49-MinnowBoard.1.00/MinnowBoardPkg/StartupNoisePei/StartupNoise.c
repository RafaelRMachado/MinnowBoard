/** @file
  Application wrapper for the E6XX GPIO ports.
  
  Copyright (c) 2012-2013, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <FrameworkPei.h>
#include <Library/DebugLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/SerialPortLib.h>
#include <Ppi/MemoryDiscovered.h>

/**
  Memory available notification callback.

  @param  PeiServices      Indirect reference to the PEI Services Table.
  @param  NotifyDescriptor Address of the notification descriptor data structure.
  @param  Ppi              Address of the PPI that was installed.

  @return Status of the notification.
**/
EFI_STATUS
MemoryAvailable(
  IN EFI_PEI_SERVICES           **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDescriptor,
  IN VOID                       *Ppi
  )
{
  CONST CHAR8 * String;

  //
  //  Tell the user that PEI is running
  //
  String = "Memory Available\r\n";
  SerialPortWrite ( (UINT8 *)String, AsciiStrLen ( String ));
  return EFI_SUCCESS;
}


//
//  Memory available notification descriptor
//
CONST EFI_PEI_NOTIFY_DESCRIPTOR mMemoryAvailableNotify = {
  ( EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST ),
  &gEfiPeiMemoryDiscoveredPpiGuid,
  MemoryAvailable
};


/**
  This is the entrypoint of PEIM
  
  @param  FileHandle  Handle of the file being invoked.
  @param  PeiServices Describes the list of possible PEI Services.

  @retval EFI_SUCCESS if it completed successfully.  
**/
EFI_STATUS
EFIAPI
StartupNoise (
  IN       EFI_PEI_FILE_HANDLE  FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
  )
{
  EFI_STATUS Status;
  CHAR8 * String;

  //
  //  Initialize the serial port
  //
  SerialPortInitialize ();

  //
  //  Tell the user that PEI is running
  //
  String = "\r\n\r\nRunning from SPI Flash\r\n";
  SerialPortWrite ( (UINT8 *)String, AsciiStrLen ( String ));

  //
  //  Request notification when RAM is available
  //
  Status = PeiServicesNotifyPpi ( &mMemoryAvailableNotify );
  ASSERT_EFI_ERROR (Status);
  return Status;
}

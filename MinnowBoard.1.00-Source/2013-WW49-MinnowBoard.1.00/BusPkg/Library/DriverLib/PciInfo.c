/** @file
  Read PCI device information
  
  Copyright (c) 2011 - 2013, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Library/DriverLib.h>


/**
  Read PCI device information

  This routine fills in the ::DL_PCI_INFO structure with the data
  from configuration space.

  @param [in] Controller          Handle for the controller.
  @param [in] DriverBindingHandle Handle for binding protocols.
  @param [in] Attributes          Attributes for OpenProtocol
  @param [in] PciInfo             Address of a ::DL_PCI_INFO structure.
  @param [out] PciIo              Optional address to receive the EFI_PCI_IO_PROTCOL,
                                  The caller must close the PCI I/O protocol if this
                                  address is not NULL.

  @retval EFI_SUCCESS   The structure was initialized.

**/
EFI_STATUS
DlPciInfo (
  IN EFI_HANDLE Controller,
  IN EFI_HANDLE DriverBindingHandle,
  IN UINT32 Attributes,
  IN DL_PCI_INFO * PciInfo,
  OUT EFI_PCI_IO_PROTOCOL ** PciIo
  )
{
  EFI_PCI_IO_PROTOCOL * PciIoProtocol;
  EFI_STATUS Status;

  //
  //  Determine if this is a PCI device.
  //
  Status = gBS->OpenProtocol ( Controller,
                               &gEfiPciIoProtocolGuid,
                               (VOID**)&PciIoProtocol,
                               DriverBindingHandle,
                               Controller,
                               Attributes );
  if ( !EFI_ERROR ( Status )) {
    //
    //  Read the vendor and device ID fields
    //
    Status = PciIoProtocol->Pci.Read ( PciIoProtocol,
                                       EfiPciIoWidthUint32,
                                       PCI_VENDOR_ID_OFFSET,
                                       1,
                                       (UINT32 *)&PciInfo->VendorID );

    //
    //  Read the class code and revision fields
    //
    if ( !EFI_ERROR ( Status )) {
      Status = PciIoProtocol->Pci.Read ( PciIoProtocol,
                                         EfiPciIoWidthUint32,
                                         PCI_REVISION_ID_OFFSET,
                                         1,
                                         (UINT32 *)&PciInfo->Revision );
    }

    //
    //  Done with the PCI I/O protocol
    //
    if ( NULL != PciIo ) {
      //
      //  Hand the protocol to the caller if requested
      //  The caller must release the protocol when done.
      //
      *PciIo = PciIoProtocol;
    }
  }

  //
  //  Return the operation status
  //
  return Status;
}

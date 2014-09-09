/** @file
  Implement the Ethernet driver binding protocol.
  
  Copyright (c) 2013, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Ethernet.h"

///
/// Driver binding protocol implementation for EG20T Ethernet driver.
///
EFI_DRIVER_BINDING_PROTOCOL gEg20tEthernetDriverBinding = {
  EthernetDriverSupported,
  EthernetDriverStart,
  EthernetDriverStop,
  0x10,
  NULL,
  NULL
};

VOID
EFIAPI
Eg20tEthernetNotifyExitBootServices (
  EFI_EVENT Event,
  VOID      *Context
  )
{
  ETHERNET_CONTEXT             *EthernetContext;
  
  EthernetContext = (ETHERNET_CONTEXT *)Context;

  //
  // Restore original PCI attributes
  //
  EthernetContext->PciIo->Attributes (
                            EthernetContext->PciIo,
                            EfiPciIoAttributeOperationDisable,
                            EthernetContext->PciSupports & EFI_PCI_DEVICE_ENABLE,
                            NULL
                            );
}

/**
  Verify the controller type

  This routine determines if a Ethernet controller is available.

  This routine is called by the UEFI driver framework during connect
  processing.

  @param [in] DriverBinding        Protocol instance pointer.
  @param [in] Controller           Handle of device to test.
  @param [in] RemainingDevicePath  Not used.

  @retval EFI_SUCCESS          This driver supports this device.
  @retval other                This driver does not support this device.

**/
EFI_STATUS
EFIAPI
EthernetDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL *DriverBinding,
  IN EFI_HANDLE Controller,
  IN EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath
 )
{
  PCI_TYPE00           Pci;
  EFI_PCI_IO_PROTOCOL  *PciIo;
  EFI_STATUS           Status;

  //
  //  Retrieve the Device Path Protocol
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  NULL,
                  DriverBinding->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  //
  //  Retrieve the PCI I/O Protocol
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  (VOID **)&PciIo,
                  DriverBinding->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  //  Read the PCI Configuration Header from the PCI device
  //
  Status = PciIo->Pci.Read (
                        PciIo,
                        EfiPciIoWidthUint32,
                        PCI_VENDOR_ID_OFFSET,
                        sizeof (Pci) / sizeof (UINT32),
                        &Pci
                        );
  if (!EFI_ERROR (Status)) {
    //
    //  Determine if this device requires the Ethernet driver
    //
    if (Pci.Hdr.VendorId == EG20T_PCI_VENDOR_ID && Pci.Hdr.DeviceId == EG20T_ETHERNET_PCI_DEVICE_ID) {
      DEBUG ((DEBUG_VERBOSE, "Found Intel Ethernet controller, DeviceID: 0x%04x\n", Pci.Hdr.DeviceId));
    } else {
      Status = EFI_UNSUPPORTED;
    }
  }

  //
  //  Close the PCI I/O protocol
  //
  gBS->CloseProtocol (
         Controller,
         &gEfiPciIoProtocolGuid,
         DriverBinding->DriverBindingHandle,
         Controller
         );

  return Status;
}

/**
  Connect to the Ethernet controller

  This routine initializes an instance of the Ethernet driver for this
  controller.

  This routine is called by the UEFI driver framework during connect
  processing if the controller passes the tests in EthernetDriverSupported.

  @param [in] DriverBinding        Protocol instance pointer.
  @param [in] Controller           Handle of device to work with.
  @param [in] RemainingDevicePath  Not used, always produce all possible children.

  @retval EFI_SUCCESS          This driver is added to Controller.
  @retval other                This driver does not support this device.

**/
EFI_STATUS
EFIAPI
EthernetDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *DriverBinding,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
 )
{
  EFI_STATUS                  Status;
  EFI_PCI_IO_PROTOCOL         *PciIo;
  ETHERNET_CONTEXT            *EthernetContext;
  MAC_ADDR_DEVICE_PATH        MacDeviceNode;
  EFI_DEVICE_PATH_PROTOCOL    *ParentDevicePath;

  EthernetContext = NULL;
  
  //
  //  Retrieve the PCI I/O Protocol
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  (VOID **)&PciIo,
                  DriverBinding->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **) &ParentDevicePath,
                  DriverBinding->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    goto Error;
  }

  //
  //  Found the Ethernet device
  //  Allocate the Ethernet context structure
  //
  EthernetContext = AllocateZeroPool (sizeof (*EthernetContext));
  if (EthernetContext == NULL) {
    DEBUG ((DEBUG_ERROR, "ERROR - No memory for Ethernet driver\n"));
    Status = EFI_OUT_OF_RESOURCES;
    goto Error;
  }
  DEBUG ((DEBUG_VERBOSE, "0x%p: EthernetContext allocated\n", EthernetContext));

  //
  //  Initialize the context structure
  //
  EthernetContext->Signature = ETHERNET_CONTEXT_SIGNATURE;
  EthernetContext->PciIo     = PciIo;

  //
  //  Start the driver
  //
  Status = EthernetApiStart (EthernetContext);
  if (EFI_ERROR (Status)) {
    //
    //  Release the API resources upon failure
    //
    goto Error;
  }
  
  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  Eg20tEthernetNotifyExitBootServices,
                  EthernetContext,
                  &gEfiEventExitBootServicesGuid,
                  &EthernetContext->ExitBootServicesEvent
                  );
  if (EFI_ERROR (Status)) {
    goto Error;
  }

  //
  //  Set the new Device Path
  //
  ZeroMem (&MacDeviceNode, sizeof (MAC_ADDR_DEVICE_PATH));
  MacDeviceNode.Header.Type    = MESSAGING_DEVICE_PATH;
  MacDeviceNode.Header.SubType = MSG_MAC_ADDR_DP;
  SetDevicePathNodeLength (&MacDeviceNode.Header, sizeof (MAC_ADDR_DEVICE_PATH));
  CopyMem (&MacDeviceNode.MacAddress, &EthernetContext->ModeData.PermanentAddress, sizeof (EFI_MAC_ADDRESS));
  MacDeviceNode.IfType = EthernetContext->SimpleNetworkProtocol.Mode->IfType;
  EthernetContext->DevPath = AppendDevicePathNode (
                               ParentDevicePath,
                               (EFI_DEVICE_PATH_PROTOCOL *) &MacDeviceNode
                               );
  if (EthernetContext->DevPath == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Error;
  }
  
  //
  //  Install the driver protocol
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &EthernetContext->DeviceHandle,
                  &gEfiSimpleNetworkProtocolGuid, &EthernetContext->SimpleNetworkProtocol,
                  &gEfiDevicePathProtocolGuid, EthernetContext->DevPath,
                  NULL
                 );
  if (EFI_ERROR (Status)) {
    goto Error;
  }

  //
  // Open For new Child Device
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  (VOID **)&PciIo,
                  DriverBinding->DriverBindingHandle,
                  EthernetContext->DeviceHandle,
                  EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                  );
  if (EFI_ERROR (Status)) {
    goto Error;
  }

  DEBUG ((DEBUG_INIT, "0x%p: Ethernet controller started\n", EthernetContext));
  return EFI_SUCCESS;

Error:
  
  gBS->CloseProtocol (
         Controller,
         &gEfiDevicePathProtocolGuid,
         DriverBinding->DriverBindingHandle,
         Controller
         );
  gBS->CloseProtocol (
         Controller,
         &gEfiPciIoProtocolGuid,
         DriverBinding->DriverBindingHandle,
         Controller
         );
  
  if (EthernetContext != NULL) {
    EthernetApiStop (EthernetContext);
    if (EthernetContext->DeviceHandle != NULL) {
      gBS->UninstallMultipleProtocolInterfaces (
             EthernetContext->DeviceHandle,
             &gEfiSimpleNetworkProtocolGuid, &EthernetContext->SimpleNetworkProtocol,
             &gEfiDevicePathProtocolGuid, EthernetContext->DevPath,
             NULL
             );
    }
    if (EthernetContext->ExitBootServicesEvent != NULL) {
      gBS->CloseEvent (EthernetContext->ExitBootServicesEvent);
    }
    if (EthernetContext->DevPath != NULL) {
      FreePool (EthernetContext->DevPath);
    }
    FreePool (EthernetContext);
  }
  
  return Status;
}

/**
  Disconnect from the Ethernet controller.

  This routine disconnects from the Ethernet controller.

  This routine is called by DriverUnload when the Ethernet driver
  is being unloaded.
  
  @param [in] DriverBinding        Protocol instance pointer.
  @param [in] Controller           Handle of device to stop driver on.
  @param [in] NumberOfChildren     How many children need to be stopped.
  @param [in] ChildHandleBuffer    Not used.

  @retval EFI_SUCCESS          This driver is removed Controller.
  @retval EFI_DEVICE_ERROR     The device could not be stopped due to a device error.
  @retval other                This driver was not removed from this device.

**/
EFI_STATUS
EFIAPI
EthernetDriverStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *DriverBinding,
  IN  EFI_HANDLE                   Controller,
  IN  UINTN                        NumberOfChildren,
  IN  EFI_HANDLE                   *ChildHandleBuffer
 )
{
  EFI_STATUS                        Status;
  EFI_SIMPLE_NETWORK_PROTOCOL       *Snp;
  ETHERNET_CONTEXT                  *EthernetContext;

  if (NumberOfChildren == 0) {
    //
    //  Close the PCI I/O Protocol
    //
    Status = gBS->CloseProtocol (
                    Controller,
                    &gEfiPciIoProtocolGuid,
                    DriverBinding->DriverBindingHandle,
                    Controller
                    );

    //
    //  Close the Device Path protocol
    //
    Status = gBS->CloseProtocol (
                    Controller,
                    &gEfiDevicePathProtocolGuid,
                    DriverBinding->DriverBindingHandle,
                    Controller
                    );
    return Status;
  }

  if (NumberOfChildren > 1) {
    return EFI_DEVICE_ERROR;
  }
  
  Status = gBS->OpenProtocol (
                  ChildHandleBuffer[0],
                  &gEfiSimpleNetworkProtocolGuid,
                  (VOID **) &Snp,
                  DriverBinding->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (!EFI_ERROR (Status)) {
    EthernetContext = ETHERNET_CONTEXT_FROM_SIMPLE_NETWORK_PROTOCOL (Snp);
    Status = gBS->CloseProtocol (
                    Controller,
                    &gEfiPciIoProtocolGuid,
                    DriverBinding->DriverBindingHandle,
                    ChildHandleBuffer[0]
                    );
    if (!EFI_ERROR (Status)) {
      gBS->UninstallMultipleProtocolInterfaces (
             ChildHandleBuffer[0],
             &gEfiSimpleNetworkProtocolGuid, &EthernetContext->SimpleNetworkProtocol,
             &gEfiDevicePathProtocolGuid, EthernetContext->DevPath,
             NULL
             );
      //
      // Close Exit Boot Services Event
      //
      gBS->CloseEvent (EthernetContext->ExitBootServicesEvent);
      
      //
      //  Stop the driver
      //
      EthernetApiStop (EthernetContext);
      FreePool (EthernetContext->DevPath); 
      FreePool (EthernetContext);
    }
  }

  return Status;
}

EFI_STATUS
EFIAPI
Eg20tEthernetUnload (
  IN EFI_HANDLE  ImageHandle
  )
/*++

Routine Description:

  Unload function for this image. Uninstall DriverBinding protocol.

Arguments:

  ImageHandle           - Handle for the image of this driver.

Returns:

  EFI_SUCCESS           - Driver unloaded successfully.
  other                 - Driver can not unloaded.

--*/
{
  EFI_STATUS  Status;
  EFI_HANDLE  *HandleBuffer;
  UINTN       HandleCount;
  UINTN       Index;
  VOID        *Interface;

  //
  // Make sure the request is for this driver
  //
  if (ImageHandle != gImageHandle) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Retrieve array of all handles in the handle database
  //
  Status = gBS->LocateHandleBuffer (
                  AllHandles,
                  NULL,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                 );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  //
  // Disconnect the current driver from handles in the handle database
  //
  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->DisconnectController (
                    HandleBuffer[Index],
                    ImageHandle,
                    NULL
                   );
  }
  
  //
  // Free the array of handles
  //
  FreePool (HandleBuffer);
  
  //
  // Uninstall protocols installed in the driver entry point
  //
  Status = gBS->HandleProtocol (
                  ImageHandle,
                  &gEfiComponentNameProtocolGuid,
                  &Interface
                 );
  if (!EFI_ERROR (Status)) {
    Status = gBS->UninstallProtocolInterface (
                    ImageHandle,
                    &gEfiComponentNameProtocolGuid, Interface
                   );
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }
  Status = gBS->HandleProtocol (
                  ImageHandle,
                  &gEfiComponentName2ProtocolGuid,
                  &Interface
                 );
  if (!EFI_ERROR (Status)) {
    Status = gBS->UninstallProtocolInterface (
                    ImageHandle,
                    &gEfiComponentName2ProtocolGuid, Interface
                   );
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  Status = gBS->UninstallProtocolInterface (
                  ImageHandle,
                  &gEfiDriverBindingProtocolGuid, &gEg20tEthernetDriverBinding
                 );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  //
  // Do any additional cleanup that is required for this driver
  //
  return EFI_SUCCESS;
}

/**
  The user Entry Point for module DiskIo. The user code starts with this function.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.  
  @param[in] SystemTable    A pointer to the EFI System Table.
  
  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
InitializeEg20tEthernet (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
 )
{
  EFI_STATUS  Status;

  //
  // Install driver model protocol(s).
  //
  Status = EfiLibInstallDriverBindingComponentName2 (
             ImageHandle,
             SystemTable,
             &gEg20tEthernetDriverBinding,
             ImageHandle,
             &gEg20tEthernetComponentName,
             &gEg20tEthernetComponentName2
            );
  ASSERT_EFI_ERROR (Status);

  return Status;
}


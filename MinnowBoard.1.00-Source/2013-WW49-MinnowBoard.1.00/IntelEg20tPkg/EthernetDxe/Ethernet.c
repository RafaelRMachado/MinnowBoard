/** @file
  Implement the Ethernet support routines.
  
  Copyright (c) 2013, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Ethernet.h"

/**
  Enable all multicast addresses

  This routine must be called at TPL_NOTIFY.

  @param[in] EthernetContext  Address of an ETHERNET_CONTEXT structure
  @param[in] Enable           TRUE to enable, FALSE to disable

**/
VOID
EthernetEnableAllMulticast (
  IN ETHERNET_CONTEXT  *EthernetContext,
  IN BOOLEAN           Enable
 )
{
  if (Enable) {
    //
    //  Receive all multicast addresses
    //
    PciIoMemAnd32 (EthernetContext, ETH_REG_RX_MODE, (UINT32)(~RX_MODE_MLT_FIL_EN));
  } else {
    //
    //  Receive specified multicast addresses only
    //
    PciIoMemOr32 (EthernetContext, ETH_REG_RX_MODE, RX_MODE_MLT_FIL_EN);
  }
}

/**
  Enable broadcast

  This routine must be called at TPL_NOTIFY.

  @param[in] EthernetContext  Address of an ETHERNET_CONTEXT structure
  @param[in] Enable           TRUE to enable, FALSE to disable

**/
VOID
EthernetEnableBroadcast (
  IN ETHERNET_CONTEXT  *EthernetContext,
  IN BOOLEAN           Enable
 )
{
  //
  //  Update the broadcast status
  //
  EthernetContext->ReceiveBroadcast = Enable;
}

/**
  Enable all addresses

  This routine must be called at TPL_NOTIFY.

  @param[in] EthernetContext  Address of an ETHERNET_CONTEXT structure
  @param[in] Enable           TRUE to enable, FALSE to disable

**/
VOID
EthernetEnablePromiscuous (
  IN ETHERNET_CONTEXT  *EthernetContext,
  IN BOOLEAN           Enable
 )
{
  if (Enable) {
    //
    //  Receive all addresses
    //
    PciIoMemAnd32 (EthernetContext, ETH_REG_RX_MODE, (UINT32)(~RX_MODE_ADD_FIL_EN));
  } else {
    //
    //  Receive specified addresses only
    //
    PciIoMemOr32 (EthernetContext, ETH_REG_RX_MODE, RX_MODE_ADD_FIL_EN);
  }
}

/**
  Enable the receiver

  This routine must be called at TPL_NOTIFY.

  @param[in] EthernetContext  Address of an ETHERNET_CONTEXT structure
  @param[in] Enable           TRUE to enable, FALSE to disable

**/
VOID
EthernetEnableReceiver (
  IN ETHERNET_CONTEXT  *EthernetContext,
  IN BOOLEAN           Enable
 )
{
  //
  //  Enable or disable the receiver
  //
  PciIoMemWrite32 (EthernetContext, ETH_REG_MAC_RX_ENABLE, Enable ? MAC_RX_ENABLE : 0);
}


/**
  Check for Ethernet interrupts

  This routine must be called at TPL_NOTIFY.

  @param[in] EthernetContext  Address of an ETHERNET_CONTEXT structure

  @return   This routine returns the interrupt status

**/
UINT32
EthernetInterrupts (
  IN ETHERNET_CONTEXT  *EthernetContext
 )
{
  UINT32  InterruptStatus;

  //
  // BUGBUG: MDK: Do we need this check???
  //
  if (EthernetContext->LinkState != LINK_STATE_UP) {
    return FALSE;
  }

  //
  //  Determine if any interrupts have occurred
  //
  InterruptStatus = PciIoMemRead32 (EthernetContext, ETH_REG_INTERRUPT_STATUS);
  if ((InterruptStatus & ETH_INT_RX_DMA_CMPLT) != 0) {
    EthernetContext->InterruptStatus |= EFI_SIMPLE_NETWORK_RECEIVE_INTERRUPT;
    gBS->SignalEvent (EthernetContext->SimpleNetworkProtocol.WaitForPacket);
  }
  if ((InterruptStatus & ETH_INT_TX_CMPLT) != 0) {
    EthernetContext->InterruptStatus |= EFI_SIMPLE_NETWORK_TRANSMIT_INTERRUPT;
    gBS->SignalEvent (EthernetContext->SimpleNetworkProtocol.WaitForPacket);
  }

  //
  //  Return the interrupt status
  //
  return InterruptStatus;
}


/**
  Reset the MAC

  This routine must be called at TPL_NOTIFY.

  @param[in] EthernetContext  Address of an ETHERNET_CONTEXT structure

**/
VOID
EthernetMacReset (
  IN ETHERNET_CONTEXT  *EthernetContext
 )
{
  //
  //  Disable the receiver and transmitter
  //
  PciIoMemWrite32 (EthernetContext, ETH_REG_DMA_CONTROL, 0);
  PciIoMemWrite32 (EthernetContext, ETH_REG_MAC_RX_ENABLE, 0);

  //
  //  Reset the MAC
  //
  PciIoMemWrite32 (EthernetContext, ETH_REG_SOFT_RESET, ETH_SOFT_RESET);

  //
  //  Update the context
  //
  EthernetContext->ModeData.MCastFilterCount = 0;
  EthernetContext->InterruptStatus           = 0;
  EthernetContext->ReceiveIndexMask          = RX_BUFFERS - 1;
  EthernetContext->ReceiveIndex              = EthernetContext->ReceiveIndexMask;
}


/**
  Determine if the MAC reset is complete

  @param[in] EthernetContext  Address of an ETHERNET_CONTEXT structure

**/
BOOLEAN
EthernetMacResetComplete (
  IN ETHERNET_CONTEXT  *EthernetContext
 )
{
  //
  //  Generate a wide enough reset pulse
  //
  if (EthernetContext->LinkTimerEventCounter != 0) {
    return FALSE;
  }

  //
  //  Remove the reset
  //
  PciIoMemWrite32 (EthernetContext, ETH_REG_SOFT_RESET, 0);

  //
  //  Return the reset complete status
  //
  return TRUE;
}


/**
  Determine if the MAC reset duration is complete

  @param[in] EthernetContext  Address of an ETHERNET_CONTEXT structure

**/
BOOLEAN
EthernetMacResetDurationComplete (
  IN ETHERNET_CONTEXT * EthernetContext
 )
{
  //
  //  Delay to allow the MAC to recover from reset
  //
  if (EthernetContext->LinkTimerEventCounter != 0) {
    return FALSE;
  }

  //
  //  Configure the MII
  //
  PciIoMemWrite32 (EthernetContext, ETH_REG_RGMII_CONTROL, RGMII_CTL_RGMII_RATE_125_MHZ | RGMII_CTL_RGMII_MODE);
  PciIoMemWrite32 (EthernetContext, ETH_REG_MODE, MODE_DUPLEX_MODE | MODE_ETHER_MODE);
  
  //
  //  Restore the MAC address
  //
  EthernetSetAddress (EthernetContext, UNICAST_ADDRESS_INDEX);

  return TRUE;
}


/**
  Save the PHY auto-negotiation results

  This routine must be called at TPL_NOTIFY.

  @param[in] EthernetContext  Address of an ETHERNET_CONTEXT structure
  @param[in] PhyStatus        Value of PHY status register

**/
VOID
EthernetPhySaveResults (
  ETHERNET_CONTEXT * EthernetContext,
  UINT16 PhyStatus
 )
{
  UINTN  PhyAddress;

  //
  //  Save the PHY registers
  //
  PhyAddress = EthernetContext->PhyAddress;
  EthernetContext->PhyControl.Control = EthernetPhyRead (EthernetContext,
                                                          PhyAddress,
                                                          PHY_CONTROL);
  EthernetContext->PhyStatus.Status = PhyStatus;
  EthernetContext->PhyControl.AutoNegotiationAdvertisement = EthernetPhyRead (EthernetContext,
                                                                               PhyAddress,
                                                                               PHY_AUTO_NEGOTIATION_ADVERTISEMENT);
  EthernetContext->PhyStatus.AutoNegotiationStatus = EthernetPhyRead (EthernetContext,
                                                                       PhyAddress,
                                                                       PHY_AUTO_NEGOTIATION_LINK_PARTNER_AVILABILITY);
  EthernetContext->PhyControl.MasterSlaveControl = EthernetPhyRead (EthernetContext,
                                                                     PhyAddress,
                                                                     PHY_MASTER_SLAVE_CONTROL);
  EthernetContext->PhyStatus.MasterSlaveStatus = EthernetPhyRead (EthernetContext,
                                                                   PhyAddress,
                                                                   PHY_MASTER_SLAVE_STATUS);
}

/**
  Receive an Ethernet frame

  This routine must be called at TPL_NOTIFY.

  @param[in] EthernetContext  Address of an ETHERNET_CONTEXT structure
  @param  BufferSize On entry, the size, in bytes, of Buffer. On exit, the size, in
                     bytes, of the packet that was received on the network interface.
  @param  Buffer     A pointer to the data buffer to receive both the media header and
                     the data.

  @retval  EFI_SUCCESS           The received data was stored in Buffer, and BufferSize has
                                 been updated to the number of bytes received.
  @retval  EFI_NOT_READY         No packets have been received on the network interface.
  @retval  EFI_BUFFER_TOO_SMALL  The BufferSize parameter is too small.

**/
EFI_STATUS
EthernetReceiveFrame (
  IN     ETHERNET_CONTEXT  *EthernetContext,
  IN OUT UINTN             *BufferSize,
  OUT    VOID              *Buffer
 )
{
  UINTN GmacStatus;
  UINTN Index;
  UINTN IndexEnd;
  UINTN IndexMask;
  EFI_PHYSICAL_ADDRESS PhysicalDescriptor;
  UINT8 * ReceiveBuffer;
  INTN ReceiveBytes;
  ETH_RECEIVE_DESCRIPTOR * ReceiveDescriptor;
  ETH_RECEIVE_DESCRIPTOR * ReceiveDescriptorEnd;
  BOOLEAN ReleasePacket;
  EFI_STATUS Status;

  //
  //  Load the receive descriptor hold register
  //
  EthernetInterrupts (EthernetContext);

  //
  //  Determine if a receive frame is present
  //
  if ((EthernetContext->InterruptStatus & EFI_SIMPLE_NETWORK_RECEIVE_INTERRUPT) == 0) {
    //
    //  No frame present
    //
    return EFI_NOT_READY;
  }

  //
  //  A frame exists in the ring
  //
  Status               = EFI_SUCCESS;
  IndexMask            = EthernetContext->ReceiveIndexMask;
  ReceiveDescriptorEnd = (ETH_RECEIVE_DESCRIPTOR *)(UINTN)PciIoMemRead32 (EthernetContext, ETH_REG_RX_DESCR_HARD_POINTER_HOLD);
  IndexEnd             = ReceiveDescriptorEnd - EthernetContext->ReceiveDescriptors;
  Index                = (EthernetContext->ReceiveIndex + 1) & IndexMask;
  ReceiveDescriptor    = &EthernetContext->ReceiveDescriptors[Index];
  if (IndexEnd == Index) {
    DEBUG ((DEBUG_ERROR, "0x%08x: IndexEnd\n", IndexEnd));
    DEBUG ((DEBUG_ERROR, "0x%08x: Index\n", Index));
    DEBUG ((DEBUG_ERROR, "0x%08x: ReceiveDescriptorEnd\n", ReceiveDescriptorEnd));
    DEBUG ((DEBUG_ERROR, "0x%08x: ReceiveDescriptor\n", ReceiveDescriptor));
    ASSERT (IndexEnd != Index);
  }
  ReleasePacket = TRUE;

  //
  //  Determine if the frame is valid
  //
  ReceiveBytes = ETH_RX_BYTES (ReceiveDescriptor) - 4;
  GmacStatus   = ReceiveDescriptor->GmacStatus;
  if (((GmacStatus & (RX_GMAC_STATUS_CRC_ERR | RX_GMAC_STATUS_NBL_ERR | RX_GMAC_STATUS_NOT_OCTAL)) != 0) ||
      (ReceiveBytes < ETHERNET_HEADER_SIZE) ||
      (((GmacStatus & RX_GMAC_STATUS_MAR_BR) != 0) && (!EthernetContext->ReceiveBroadcast))) {
    //
    //  Update the statistics
    //
    EthernetContext->Statistics.TxTotalFrames++;
    if (ReceiveBytes > 0) {
      EthernetContext->Statistics.RxTotalBytes += ReceiveBytes;
    }
    EthernetContext->Statistics.RxCrcErrorFrames++;
    if ((GmacStatus & RX_GMAC_STATUS_MAR_BR) != 0) {
      EthernetContext->Statistics.RxBroadcastFrames++;
    }

    //
    //  Skip over the error packets
    //
    Status = EFI_NOT_READY;
  } else {
    //
    //  A valid Ethernet frame is in the ring.
    //  Validate the packet size
    //
    if (*BufferSize < (UINTN)ReceiveBytes) {
      //
      //  The buffer is too small, keep the packet for later
      //
      ReleasePacket = FALSE;
      Status = EFI_BUFFER_TOO_SMALL;
    } else {
      //
      //  Update the statistics
      //
      EthernetContext->Statistics.TxTotalFrames++;
      if (ReceiveBytes > 0) {
        EthernetContext->Statistics.RxTotalBytes += ReceiveBytes;
      }
      EthernetContext->Statistics.RxGoodFrames++;
      if (ReceiveBytes < (64 - 4)) {
        EthernetContext->Statistics.RxUndersizeFrames++;
      } else if (ReceiveBytes > (1518 - 4)) {
        EthernetContext->Statistics.RxOversizeFrames++;
      }
      if ((GmacStatus & RX_GMAC_STATUS_MAR_BR) != 0) {
        EthernetContext->Statistics.RxBroadcastFrames++;
      } else if ((GmacStatus & RX_GMAC_STATUS_MAR_BR) != 0) {
        EthernetContext->Statistics.RxMulticastFrames++;
      } else {
        EthernetContext->Statistics.RxUnicastFrames++;
      }

      //
      //  Receive the packet
      //
      ReceiveBuffer = (UINT8 *)(UINTN)ReceiveDescriptor->RxFrameBufferAddress;
      CopyMem (Buffer, ReceiveBuffer, ReceiveBytes);

      //
      //  Display the header
      //
      DEBUG ((
        DEBUG_VERBOSE,
        "RX: %02x-%02x-%02x-%02x-%02x-%02x <-- %02x-%02x-%02x-%02x-%02x-%02x, %d bytes\n",
        ReceiveBuffer[0],
        ReceiveBuffer[1],
        ReceiveBuffer[2],
        ReceiveBuffer[3],
        ReceiveBuffer[4],
        ReceiveBuffer[5],
        ReceiveBuffer[6],
        ReceiveBuffer[7],
        ReceiveBuffer[8],
        ReceiveBuffer[9],
        ReceiveBuffer[10],
        ReceiveBuffer[11],
        ReceiveBytes 
       ));
    }
    
    //
    //  Return the received packet size
    //
    *BufferSize = ReceiveBytes;
  }

  //
  //  Hand this receive buffer back to the MAC
  //
  if (ReleasePacket) {
    PhysicalDescriptor = &EthernetContext->ReceiveDescriptors[Index] - 
                         EthernetContext->ReceiveDescriptors         +
                         EthernetContext->PhysicalReceiveDescriptors;
    PciIoMemWrite32 (EthernetContext, ETH_REG_RX_DESCR_SOFT_POINTER, (UINT32)(UINTN)PhysicalDescriptor);
    EthernetContext->ReceiveIndex = Index;
    if (IndexEnd == ((Index + 1) & IndexMask)) {
      //
      //  Clear the interrput when no more receive packets are available
      //
      EthernetContext->InterruptStatus &= ~EFI_SIMPLE_NETWORK_RECEIVE_INTERRUPT;
    } else {
      //
      //  Another receive packet is available
      //
      gBS->SignalEvent (EthernetContext->SimpleNetworkProtocol.WaitForPacket);
    }
  }

  //
  //  Return the operation status
  //
  return Status;
}

/**
  Allocate the necessary resources

  This routine must be called at TPL_NOTIFY.

  @param[in] EthernetContext  Address of an ETHERNET_CONTEXT structure

  @retval  EFI_SUCCESS           The resources were allocated successfully.
  @retval  EFI_NO_MAPPING        The common buffer was not properly mapped.
  @retval  EFI_OUT_OF_RESOURCES  The resources were not available.

**/
EFI_STATUS
EthernetResourceAllocate (
  IN ETHERNET_CONTEXT  *EthernetContext
 )
{
  EFI_STATUS               Status;
  UINTN                    PageCount;
  UINT8                    *Buffer;
  UINTN                    BufferSize;
  EFI_SIMPLE_NETWORK_MODE  *ModeData;

  //
  //  Allocate an array of receive buffer descriptors and an array of
  //  receive buffers.
  //
  //                                   .--- EthernetContext->ReceiveBufferDescriptors
  //                                  /
  //                                 /      EthernetContext->ReceiveBuffer
  //                                /              |
  //              +------------+<--'               V
  //     BASE     |  Receive   |           +----------------+
  //     ADDR --->|  Buffer    |---------->| Receive Buffer |
  //              | Descriptor |           +----------------+
  //              +------------+
  //              |  Receive   |           +----------------+
  //              |  Buffer    |---------->| Receive Buffer |
  //              | Descriptor |           +----------------+
  //              +------------+
  //     HARD     |  Receive   |           +----------------+
  //     PTR ---->|  Buffer    |---------->| Receive Buffer |
  //              | Descriptor |           +----------------+
  //              +------------+
  //                    *
  //                    *
  //                    *
  //              +------------+
  //      SOFT    |  Receive   |           +----------------+
  //      PTR --->|  Buffer    |---------->| Receive Buffer |
  //              | Descriptor |           +----------------+
  //              +------------+
  //
  //  The receive descriptor ring is empty when (ETH_REG_RX_DESCR_SOFT_POINTER
  //  + 1) modulo the ring size matches ETH_REG_RX_DESCR_HARD_POINTER.  The
  //  ring is full when ETH_REG_RX_DESCR_HARD_POINTER equals
  //  ETH_REG_RX_DESCR_SOFT_POINTER.  The controller fills the ring until
  //  the controller runs out of data or runs out of receive descritors.
  //  The driver increments the previous value of ETH_REG_RX_DESCR_SOFT_POINTER
  //  to get data from the next receive descriptor filled in by the controller.
  //  When the buffer is free, the value updated receive descriptor value
  //  is written back to the register to notify the controller that the
  //  receive buffer and descriptor are available for use.
  //

  //
  //  Allow the reset and shutdown routines to run
  //
  ModeData        = &EthernetContext->ModeData;
  ModeData->State = EfiSimpleNetworkInitialized;

  //
  //  Start the timer
  //
  Status = gBS->SetTimer (
                  EthernetContext->Timer,
                  TimerPeriodic,
                  ETHERNET_TIMER_TICK
                 );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "ERROR - Failed to start the timer, Status: %r\n", Status));
    return Status;
  }

  //
  //  Allocate the receive buffer
  //
  PageCount = RX_DESCRIPTOR_PAGES
            + RX_BUFFER_PAGES
            + TX_DESCRIPTOR_PAGES
            + TX_BUFFER_PAGES;
  DEBUG ((DEBUG_VERBOSE, "PageCount: 0x%08x pages\n", PageCount));
  Status = EthernetContext->PciIo->AllocateBuffer (
                                     EthernetContext->PciIo,
                                     0,
                                     EfiBootServicesData,
                                     PageCount,
                                     (VOID**)&Buffer,
                                     0
                                    );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "ERROR - Failed to allocate the receive buffers, Status: %r\n", Status));
    return Status;
  }

  DEBUG ((DEBUG_VERBOSE, "Mapping the common buffer\n"));
  BufferSize = EFI_PAGES_TO_SIZE (PageCount);
  Status = EthernetContext->PciIo->Map (
                                     EthernetContext->PciIo,
                                     EfiPciIoOperationBusMasterCommonBuffer,
                                     Buffer,
                                     &BufferSize,
                                     &EthernetContext->PhysicalReceiveDescriptors,
                                     &EthernetContext->DmaMapping
                                    );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "ERROR - Failed to map the common buffer, Status: %r\n", Status));
    return Status;
  }

  if (BufferSize != EFI_PAGES_TO_SIZE (PageCount)) {
    DEBUG ((DEBUG_ERROR, "ERROR - Failed to map the entire common buffer!\n"));
    return EFI_NO_MAPPING;
  }
  
  //
  //  Split the buffer
  //
  DEBUG ((DEBUG_VERBOSE, "Buffer: 0x%08x mapped to 0x%016Lx\n", Buffer, EthernetContext->PhysicalReceiveDescriptors));
  EthernetContext->PageCount                   = PageCount;
  EthernetContext->ReceiveDescriptors          = (ETH_RECEIVE_DESCRIPTOR *)Buffer;
  EthernetContext->ReceiveBuffer               = Buffer + EFI_PAGES_TO_SIZE (RX_DESCRIPTOR_PAGES);
  EthernetContext->PhysicalReceiveBuffer       = EthernetContext->PhysicalReceiveDescriptors + EFI_PAGES_TO_SIZE (RX_DESCRIPTOR_PAGES);
  EthernetContext->TransmitDescriptors         = (ETH_TRANSMIT_DESCRIPTOR *)(Buffer + EFI_PAGES_TO_SIZE (RX_DESCRIPTOR_PAGES + RX_BUFFER_PAGES));
  EthernetContext->PhysicalTransmitDescriptors = EthernetContext->PhysicalReceiveDescriptors + EFI_PAGES_TO_SIZE (RX_DESCRIPTOR_PAGES + RX_BUFFER_PAGES);
  EthernetContext->TransmitBuffer              = Buffer + EFI_PAGES_TO_SIZE (RX_DESCRIPTOR_PAGES + RX_BUFFER_PAGES + TX_DESCRIPTOR_PAGES);
  EthernetContext->PhysicalTransmitBuffer      = EthernetContext->PhysicalReceiveDescriptors + EFI_PAGES_TO_SIZE (RX_DESCRIPTOR_PAGES + RX_BUFFER_PAGES + TX_DESCRIPTOR_PAGES);

  //
  //  Initialize the context
  //
  ModeData->MCastFilterCount = 0;
  CopyMem (
    &ModeData->CurrentAddress, 
    &ModeData->PermanentAddress, 
    sizeof (ModeData->PermanentAddress)
   );
  CopyMem (
    &ModeData->MCastFilter[UNICAST_ADDRESS_INDEX], 
    &ModeData->PermanentAddress, 
    sizeof (ModeData->PermanentAddress)
   );

  return EFI_SUCCESS;
}


/**
  Free the resources allocated by EthernetResourceAllocate

  This routine must be called at TPL_NOTIFY.

  @param[in] EthernetContext  Address of an ETHERNET_CONTEXT structure

**/
VOID
EthernetResourceFree (
  IN ETHERNET_CONTEXT  *EthernetContext
 )
{
  EFI_STATUS  Status;

  //
  //  Free the allocated resources
  //
  if (EthernetContext->DmaMapping != NULL) {
    //
    //  Unmap the common buffer
    //
    DEBUG ((DEBUG_VERBOSE, "Unmapping the common buffer\n"));
    Status = EthernetContext->PciIo->Unmap (
                                       EthernetContext->PciIo,
                                       EthernetContext->DmaMapping
                                      );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "ERROR - Failed to unmap the common buffer, Status: %r\n", Status));
    }
    ASSERT_EFI_ERROR (Status);
  }

  if (EthernetContext->ReceiveDescriptors != NULL) {
    //
    //  Free the common buffer
    //
    DEBUG ((DEBUG_VERBOSE,"Freeing 0x%08x, 0x%08x pages\n", EthernetContext->ReceiveDescriptors, EthernetContext->PageCount));
    Status = EthernetContext->PciIo->FreeBuffer (
                                       EthernetContext->PciIo,
                                       EthernetContext->PageCount,
                                       (VOID**)EthernetContext->ReceiveDescriptors
                                      );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "ERROR - Failed to free buffer, Status: %r\n", Status));
    }
    ASSERT_EFI_ERROR (Status);
  }

  EthernetContext->DmaMapping         = NULL;
  EthernetContext->ReceiveDescriptors = NULL;
  EthernetContext->ReceiveBuffer      = NULL;
  EthernetContext->PageCount          = 0;
}


/**
  Set an Ethernet address

  This routine must be called at TPL_NOTIFY.

  @param[in] EthernetContext  Address of an ETHERNET_CONTEXT structure
  @param[in] Index            Index into the list of addresses

**/
VOID
EthernetSetAddress (
  IN ETHERNET_CONTEXT  *EthernetContext,
  IN UINTN             Index
 )
{
  UINT32 Data;
  EFI_SIMPLE_NETWORK_MODE * ModeData;
  UINTN RegisterAddress;
  UINT32 RegisterMask;
  UINTN            Count;

  //
  //  Gain access to the list of addresses
  //
  ModeData = &EthernetContext->ModeData;
  Index &= ETH_MAX_ADDRESSES - 1;
  RegisterAddress = (UNICAST_ADDRESS_INDEX == Index) ? 0 : (Index + 1);
  RegisterMask = (UINT32)(1 << RegisterAddress);
  RegisterAddress <<= 3;
  PciIoMemOr32 (EthernetContext, ETH_REG_ADDRESS_MASK, RegisterMask);
  Count = 0;
  do {
    gBS->Stall (10);
    Count++;
    Data = PciIoMemRead32 (EthernetContext, ETH_REG_ADDRESS_MASK);
  } while (0 != (Data & ETH_ADDRESS_MASK_BUSY) && Count < 20);
  if (Count >= 20) {
    DEBUG ((DEBUG_VERBOSE, "EthernetSetAddress: Can not set address\n"));
    return;
  }

  //
  //  Update the address
  //
  PciIoMemWrite32 (
    EthernetContext, 
    (ETH_REG_MAC_ADDRESS_1_A + RegisterAddress), 
    *(UINT32 *)&ModeData->MCastFilter [ Index ].Addr [ 0 ]
    );
  PciIoMemWrite32 (
    EthernetContext, 
    (ETH_REG_MAC_ADDRESS_1_B + RegisterAddress), 
    *(UINT16 *)&ModeData->MCastFilter [ Index ].Addr [ 4 ]
    );

  //
  //  Update the address in the MAC
  //
  PciIoMemAnd32 (EthernetContext, ETH_REG_ADDRESS_MASK, (UINT32)(~RegisterMask));
  Count = 0;
  do {
    gBS->Stall (10);
    Count++;
    Data = PciIoMemRead32 (EthernetContext, ETH_REG_ADDRESS_MASK);
  } while (0 != (Data & ETH_ADDRESS_MASK_BUSY) && Count < 20);
  if (Count >= 20) {
    DEBUG ((DEBUG_VERBOSE, "EthernetSetAddress: Can not set address\n"));
    return;
  }
}


/**
  Wait for packet event routine

  @param  Event           Event whose notification function is being invoked.
  @param  EthernetContext The pointer to the notification function's context,
                          which is implementation-dependent.

**/
VOID
EFIAPI
EthernetWaitForPacketNotify (
  IN  EFI_EVENT  Event,
  IN  VOID       *Context
  )
{
  EthernetInterrupts ((ETHERNET_CONTEXT  *)Context);
}

/**
  Periodic timer event routine

  @param  Event           Event whose notification function is being invoked.
  @param  EthernetContext The pointer to the notification function's context,
                          which is implementation-dependent.

**/
VOID
EFIAPI
EthernetTimer (
  IN  EFI_EVENT  Event,
  IN  VOID       *Context
  )
{
  ETHERNET_CONTEXT  *EthernetContext;
  UINT16 Data16;
  BOOLEAN Enable;
  UINTN Index;
  UINT32 InterruptStatus;
  LINK_STATE LinkState;
  BOOLEAN LinkUp;
  MAC_CONTROL_REGISTERS MacControl;
  EFI_PHYSICAL_ADDRESS PhysicalBuffer;
  UINT16 PhyData [ PHY_REGISTER_COUNT ];
  UINTN  PhyAddress;
  UINT32 PhyId;
  ETH_RECEIVE_DESCRIPTOR * ReceiveDescriptor;
  UINTN ReceiveMask;
  ETH_TRANSMIT_DESCRIPTOR * TransmitDescriptor;

  //
  // Get the context
  //
  EthernetContext = (ETHERNET_CONTEXT  *)Context;
  
  //
  //  Display the entry
  //
  if (EthernetContext->LinkTimerEventCounter > 0) {
    EthernetContext->LinkTimerEventCounter--;
  }

  //
  //  Update the state of the link
  //
  LinkState = EthernetContext->LinkState;

  switch (LinkState) {
  default:
  case LINK_STATE_UNKNOWN:
    DEBUG ((DEBUG_VERBOSE, "Link down\n"));
    LinkState = LINK_STATE_DOWN;

    //
    //  Fall through
    //      |
    //      |
    //      V

  case LINK_STATE_DOWN:

    DEBUG ((DEBUG_VERBOSE, "MAC and PHY reset\n"));

    //
    // Reset the PHY
    //
    EthernetPhyReset (EthernetContext);

    //
    //  Reset the MAC
    //
    EthernetMacReset (EthernetContext);

    EthernetContext->LinkTimerEventCounter = DivU64x64Remainder (EFI_TIMER_PERIOD_MILLISECONDS (10), ETHERNET_TIMER_TICK, NULL) + 1;

    LinkState = LINK_STATE_RESETTING;

    break;

  case LINK_STATE_RESETTING:
    //
    //  Generate a wide enough reset pulse
    //
    if (EthernetMacResetComplete (EthernetContext)) {
      //
      //  Allow the PHY to recover from the reset
      //
      EthernetContext->LinkTimerEventCounter = DivU64x64Remainder (EFI_TIMER_PERIOD_MILLISECONDS (40), ETHERNET_TIMER_TICK, NULL) + 1;
      LinkState = LINK_STATE_RESET;
    }
    break;

  case LINK_STATE_RESET:
    //
    //  Ensure enough recovery time after reset
    //
    if (EthernetMacResetDurationComplete (EthernetContext)) {
      DEBUG ((DEBUG_VERBOSE, "Auto-negotiation\n"));

      //
      //  Verify the descriptor counts are a power of 2
      //
      ASSERT (0 == (RX_BUFFERS & (RX_BUFFERS - 1)));
      ASSERT (0 == (TX_BUFFERS & (TX_BUFFERS - 1)));

      //
      //  Verify the buffer sizes
      //
      ASSERT (ETH_RECEIVE_BUFFER_SIZE == (1 << ((sizeof (ReceiveDescriptor->RxLength) << 3) - 2)));
      ASSERT (ETH_TRANSMIT_BUFFER_SIZE == (1 << ((sizeof (TransmitDescriptor->Length) << 3) - 2)));
      ASSERT (ETH_RECEIVE_BUFFER_SIZE == ETH_TRANSMIT_BUFFER_SIZE);

      //
      //  Determine the PHY address
      //
      DEBUG ((DEBUG_VERBOSE, "EthernetPhyGetAddress()\n"));

      if (!EthernetPhyGetAddress (EthernetContext, &PhyAddress, &PhyId)) {
        //
        // PHY Address could not be detected.  Start link detection state machine over
        //
        LinkState = LINK_STATE_DOWN;
      } else {
        EthernetContext->PhyAddress = PhyAddress;

        DEBUG ((DEBUG_VERBOSE, "Phy Address: %d\n", EthernetContext->PhyAddress));
        DEBUG ((DEBUG_VERBOSE, "    OID: 0x%06x\n", PhyId >> 10));
        DEBUG ((DEBUG_VERBOSE, "    Model: 0x%02x\n", (PhyId >> 4) & 0x3f));
        DEBUG ((DEBUG_VERBOSE, "    Revision: 0x%x\n", PhyId & 0xf));

        //
        //  Configure the PHY and start the auto-negotiation
        //
        Data16 = (UINT16)(ANA_100BASE_TX_FD
                         | ANA_100BASE_TX
                         | ANA_10BASE_T_FD
                         | ANA_10BASE_T
                         | ANA_SELECTOR_IEEE_802_3);
        EthernetPhyWrite (EthernetContext,
                           EthernetContext->PhyAddress,
                           PHY_AUTO_NEGOTIATION_ADVERTISEMENT,
                           Data16);
        Data16 = (UINT16)(MSC_1000BASE_T_FD
  //                       | MSC_MANUAL_CONFIGURATION_ENABLE
                         | MSC_1000BASE_T);
        EthernetPhyWrite (EthernetContext,
                           EthernetContext->PhyAddress,
                           PHY_MASTER_SLAVE_CONTROL,
                           Data16);
        Data16 = (UINT16)(CONTROL_SPEED_1000_MBPS
                         | CONTROL_AUTO_NEGOTIATION_ENABLE
                         | CONTROL_RESTART_AUTO_NEGOTIATION
                         | CONTROL_DUPLEX_MODE);
        EthernetPhyWrite (EthernetContext,
                           EthernetContext->PhyAddress,
                           PHY_CONTROL,
                           Data16);

        //
        //  Initialize the transmit DMA engine
        //
        TransmitDescriptor = EthernetContext->TransmitDescriptors;
        ZeroMem (TransmitDescriptor, EFI_PAGES_TO_SIZE (TX_DESCRIPTOR_PAGES));
        PciIoMemWrite32 (EthernetContext, ETH_REG_TX_DESCR_SIZE, TX_DESCRIPTOR_AREA_SIZE - sizeof (*TransmitDescriptor));
        PciIoMemWrite32 (EthernetContext, ETH_REG_TX_DESCR_BASE_ADDRESS, (UINT32)(UINTN)TransmitDescriptor);
        PciIoMemWrite32 (EthernetContext, ETH_REG_TX_DESCR_HARD_POINTER, (UINT32)(UINTN)TransmitDescriptor);
        PciIoMemWrite32 (EthernetContext, ETH_REG_TX_DESCR_SOFT_POINTER, (UINT32)(UINTN)TransmitDescriptor);
        EthernetContext->TransmitCompleteIndex = 0;
        EthernetContext->TransmitIndex = 0;
        EthernetContext->TransmitIndexMask = TX_BUFFERS - 1;

        //
        //  Configure the transmitter
        //
        PciIoMemWrite32 (
          EthernetContext, 
          ETH_REG_RX_FLOW_CONTROL, 
          0
          );
        PciIoMemWrite32 (
          EthernetContext, 
          ETH_REG_TX_MODE, 
          TX_MODE_LONG_PKT | TX_MODE_ST_AND_FD | TX_MODE_SHORT_PKT | TX_MODE_LTCOL_RETX
          );

        //
        //  Initialize the receive descriptors
        //
        ReceiveDescriptor = EthernetContext->ReceiveDescriptors;
        ZeroMem (ReceiveDescriptor, EFI_PAGES_TO_SIZE (RX_DESCRIPTOR_PAGES));
        PhysicalBuffer = EthernetContext->PhysicalReceiveBuffer;
        for (Index = 0; RX_BUFFERS > Index; Index++)
        {
          ReceiveDescriptor->RxFrameBufferAddress = (UINT32)(UINTN)PhysicalBuffer;
          PhysicalBuffer += ETH_RECEIVE_BUFFER_SIZE;
        }

        //
        //  Initialize the receive DMA engine
        //
        ASSERT ((0x10 <= RX_DESCRIPTOR_AREA_SIZE) && (0x10000 >= RX_DESCRIPTOR_AREA_SIZE));
        PciIoMemWrite32 (EthernetContext, ETH_REG_RX_DESCR_SIZE, RX_DESCRIPTOR_AREA_SIZE - sizeof (*ReceiveDescriptor));
        PciIoMemWrite32 (EthernetContext, ETH_REG_RX_DESCR_BASE_ADDRESS, (UINT32)(UINTN)EthernetContext->ReceiveDescriptors);
        PciIoMemWrite32 (EthernetContext, ETH_REG_RX_DESCR_HARD_POINTER, (UINT32)(UINTN)EthernetContext->ReceiveDescriptors);
        PciIoMemWrite32 (EthernetContext, ETH_REG_RX_DESCR_SOFT_POINTER, (UINT32)(UINTN)&EthernetContext->ReceiveDescriptors[EthernetContext->ReceiveIndex]);

        //
        //  Configure the receiver
        //
        ReceiveMask = EthernetContext->ModeData.ReceiveFilterSetting;
        Enable = (BOOLEAN)(0 != (ReceiveMask & EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS_MULTICAST));
        EthernetEnableAllMulticast (EthernetContext, Enable);
        Enable = (BOOLEAN)(0 != (ReceiveMask & EFI_SIMPLE_NETWORK_RECEIVE_BROADCAST));
        EthernetEnableBroadcast (EthernetContext, Enable);
        Enable = (BOOLEAN)(0 != (ReceiveMask & EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS));
        EthernetEnablePromiscuous (EthernetContext, Enable);
        ASSERT (ETH_MAX_ADDRESSES <= MAX_MCAST_FILTER_CNT);
        for (Index = 0; ETH_MAX_ADDRESSES > Index; Index++) {
          EthernetSetAddress (EthernetContext, Index);
        }

        //
        //  Disable the TCP/IP accelerators
        //
        PciIoMemWrite32 (EthernetContext, ETH_REG_TCP_IP_ACCELERATOR, 0);

        //
        //  Wait for the auto-negotiation to complete
        //
        EthernetContext->LinkTimerEventCounter = DivU64x64Remainder (LINK_TIMEOUT, ETHERNET_TIMER_TICK, NULL) + 1;

        LinkState = LINK_STATE_PHY_RESET;
      }
    }
    break;

  case LINK_STATE_PHY_RESET:
    //
    //  Determine if the link is up
    //
    PhyData [ PHY_STATUS ] = EthernetPhyRead (EthernetContext,
                                               EthernetContext->PhyAddress,
                                               PHY_STATUS);
    if (0 != (PhyData [ PHY_STATUS ] & STATUS_LINK_UP)) {
      //
      //  Save the auto-negotiation results
      //
      EthernetPhySaveResults (EthernetContext, PhyData [ PHY_STATUS ]);
      EthernetPhyDecode (&EthernetContext->PhyStatus,
                          &LinkUp,
                          &EthernetContext->LinkSpeed,
                          &EthernetContext->FullDuplex);

      //
      //  Display the critical PHY registers
      //
      DEBUG ((DEBUG_VERBOSE, "0x%04x: PHY_STATUS\n", EthernetContext->PhyStatus.Status));
      DEBUG ((DEBUG_VERBOSE, "0x%04x: PHY_AUTO_NEGOTIATION_LINK_PARTNER_AVILABILITY\n", EthernetContext->PhyStatus.AutoNegotiationStatus));
      DEBUG ((DEBUG_VERBOSE, "0x%04x: PHY_MASTER_SLAVE_STATUS\n", EthernetContext->PhyStatus.MasterSlaveStatus));

      LinkState = LINK_STATE_CONFIGURE_PHY;
    } else if (EthernetContext->LinkTimerEventCounter == 0) {
      //
      //  Link has timed out
      //
      DEBUG ((DEBUG_VERBOSE, "PHY configuration timeout\n"));
      DEBUG ((DEBUG_VERBOSE,
                "0x%04x: PHY_CONTROL\n",
                EthernetPhyRead (EthernetContext,
                                  EthernetContext->PhyAddress,
                                  PHY_CONTROL)));
      DEBUG ((DEBUG_VERBOSE, "0x%04x: PHY_STATUS\n", PhyData [ PHY_STATUS ]));
      DEBUG ((DEBUG_VERBOSE,
                "0x%04x: PHY_AUTO_NEGOTIATION_ADVERTISEMENT\n",
                EthernetPhyRead (EthernetContext,
                                  EthernetContext->PhyAddress,
                                  PHY_AUTO_NEGOTIATION_ADVERTISEMENT)));
      DEBUG ((DEBUG_VERBOSE,
                "0x%04x: PHY_AUTO_NEGOTIATION_LINK_PARTNER_AVILABILITY\n",
                EthernetPhyRead (EthernetContext,
                                  EthernetContext->PhyAddress,
                                  PHY_AUTO_NEGOTIATION_LINK_PARTNER_AVILABILITY)));
      DEBUG ((DEBUG_VERBOSE,
                "0x%04x: PHY_MASTER_SLAVE_CONTROL\n",
                EthernetPhyRead (EthernetContext,
                                  EthernetContext->PhyAddress,
                                  PHY_MASTER_SLAVE_CONTROL)));
      DEBUG ((DEBUG_VERBOSE,
                "0x%04x: PHY_MASTER_SLAVE_STATUS\n",
                EthernetPhyRead (EthernetContext,
                                  EthernetContext->PhyAddress,
                                  PHY_MASTER_SLAVE_STATUS)));

      LinkState = LINK_STATE_DOWN;
    }
    break;

  case LINK_STATE_CONFIGURE_PHY:
    //
    //  Determine if the link is up
    //
    PhyData [ PHY_STATUS ] = EthernetPhyRead (EthernetContext,
                                               EthernetContext->PhyAddress,
                                               PHY_STATUS);
    EthernetPhySaveResults (EthernetContext, PhyData [ PHY_STATUS ]);

    //
    //  Configure the MAC
    //
    DEBUG ((DEBUG_VERBOSE,
              "Configuring for: %d Mb/s %s Duplex\n",
              EthernetContext->LinkSpeed,
              EthernetContext->FullDuplex ? L"Full" : L"Half"));
    MacControl.Mode         = PciIoMemRead32 (EthernetContext, ETH_REG_MODE);
    MacControl.RgmiiControl = PciIoMemRead32 (EthernetContext, ETH_REG_RGMII_CONTROL);
    EthernetMacConfigure (&MacControl,
                           EthernetContext->LinkSpeed,
                           EthernetContext->FullDuplex);
    PciIoMemWrite32 (EthernetContext, ETH_REG_MODE, MacControl.Mode);
    PciIoMemWrite32 (EthernetContext, ETH_REG_RGMII_CONTROL, MacControl.RgmiiControl);

    //
    //  Configure the PHY
    //
    EthernetPhyConfigure (&EthernetContext->PhyControl,
                           EthernetContext->LinkSpeed,
                           EthernetContext->FullDuplex);
    EthernetPhyWrite (EthernetContext,
                       EthernetContext->PhyAddress,
                       PHY_CONTROL,
                       EthernetContext->PhyControl.Control);
    EthernetPhyWrite (EthernetContext,
                       EthernetContext->PhyAddress,
                       PHY_AUTO_NEGOTIATION_ADVERTISEMENT,
                       EthernetContext->PhyControl.AutoNegotiationAdvertisement);
    EthernetPhyWrite (EthernetContext,
                       EthernetContext->PhyAddress,
                       PHY_MASTER_SLAVE_CONTROL,
                       EthernetContext->PhyControl.MasterSlaveControl);
    PlatformSpecificPhyConfiguration (EthernetContext,
                                       EthernetContext->PhyAddress);

    //
    //  Wait for the auto-negotiation to complete
    //
    EthernetContext->LinkTimerEventCounter = DivU64x64Remainder (LINK_TIMEOUT, ETHERNET_TIMER_TICK, NULL) + 1;  

    LinkState = LINK_STATE_AUTO_NEGOTIATING;

    break;

  case LINK_STATE_AUTO_NEGOTIATING:
    //
    //  Determine if the link is up
    //
    PhyData [ PHY_STATUS ] = EthernetPhyRead (EthernetContext,
                                               EthernetContext->PhyAddress,
                                               PHY_STATUS);
    if (0 != (PhyData [ PHY_STATUS ] & STATUS_LINK_UP)) {
      //
      //  Display the critical PHY registers
      //
      DEBUG ((DEBUG_VERBOSE, "0x%04x: PHY_STATUS\n", PhyData [ PHY_STATUS ]));
      DEBUG ((DEBUG_VERBOSE,
                "0x%04x: PHY_AUTO_NEGOTIATION_LINK_PARTNER_AVILABILITY\n",
                EthernetPhyRead (EthernetContext,
                                  EthernetContext->PhyAddress,
                                  PHY_AUTO_NEGOTIATION_LINK_PARTNER_AVILABILITY)));
      DEBUG ((DEBUG_VERBOSE,
                "0x%04x: PHY_MASTER_SLAVE_STATUS\n",
                EthernetPhyRead (EthernetContext,
                                  EthernetContext->PhyAddress,
                                  PHY_MASTER_SLAVE_STATUS)));
      DEBUG ((DEBUG_VERBOSE,
                "Link up: %d Mb/s %s Duplex\n",
                EthernetContext->LinkSpeed,
                EthernetContext->FullDuplex ? L"Full" : L"Half"));

      //
      //  Enable the receiver
      //
      PciIoMemWrite32 (EthernetContext, ETH_REG_DMA_CONTROL, ETH_DMA_CONTROL_RX_DMA_EN);

      Enable = (BOOLEAN)(0 != EthernetContext->ModeData.ReceiveFilterSetting);
      EthernetEnableReceiver (EthernetContext, TRUE);
      LinkState = LINK_STATE_UP;

      DEBUG ((DEBUG_INFO, "LINK_STATE_UP\n"));
    } else if (EthernetContext->LinkTimerEventCounter == 0) {
      //
      //  Link has timed out
      //
      DEBUG ((DEBUG_VERBOSE, "Auto-negotation timeout\n"));
      DEBUG ((DEBUG_VERBOSE,
                "0x%04x: PHY_CONTROL\n",
                EthernetPhyRead (EthernetContext,
                                  EthernetContext->PhyAddress,
                                  PHY_CONTROL)));
      DEBUG ((DEBUG_VERBOSE, "0x%04x: PHY_STATUS\n", PhyData [ PHY_STATUS ]));
      DEBUG ((DEBUG_VERBOSE,
                "0x%04x: PHY_AUTO_NEGOTIATION_ADVERTISEMENT\n",
                EthernetPhyRead (EthernetContext,
                                  EthernetContext->PhyAddress,
                                  PHY_AUTO_NEGOTIATION_ADVERTISEMENT)));
      DEBUG ((DEBUG_VERBOSE,
                "0x%04x: PHY_AUTO_NEGOTIATION_LINK_PARTNER_AVILABILITY\n",
                EthernetPhyRead (EthernetContext,
                                  EthernetContext->PhyAddress,
                                  PHY_AUTO_NEGOTIATION_LINK_PARTNER_AVILABILITY)));
      DEBUG ((DEBUG_VERBOSE,
                "0x%04x: PHY_MASTER_SLAVE_CONTROL\n",
                EthernetPhyRead (EthernetContext,
                                  EthernetContext->PhyAddress,
                                  PHY_MASTER_SLAVE_CONTROL)));
      DEBUG ((DEBUG_VERBOSE,
                "0x%04x: PHY_MASTER_SLAVE_STATUS\n",
                EthernetPhyRead (EthernetContext,
                                  EthernetContext->PhyAddress,
                                  PHY_MASTER_SLAVE_STATUS)));
      LinkState = LINK_STATE_DOWN;
    }
    break;

  case LINK_STATE_UP:
    //
    //  Monitor the link status
    //
    PhyData[PHY_STATUS] = EthernetPhyRead (
                            EthernetContext,
                            EthernetContext->PhyAddress,
                            PHY_STATUS
                           );
    if ((PhyData[PHY_STATUS] & STATUS_LINK_UP) == 0) {
      LinkState = LINK_STATE_DOWN;
      DEBUG ((DEBUG_INFO, "LINK_STATE_DOWN\n"));
    } else {
      //
      //  Determine if any interrupts have occurred
      //
      InterruptStatus = EthernetInterrupts (EthernetContext);
    }
    break;
  }

  //
  //  Update the link state
  //
  EthernetContext->LinkState = LinkState;
}


/**
  Transmit an Ethernet frame

  This routine must be called at TPL_NOTIFY.

  @param[in] EthernetContext  Address of an ETHERNET_CONTEXT structure
  @param  BufferSize The size, in bytes, of the entire packet (media header and
                     data) to be transmitted through the network interface.
  @param  Buffer     A pointer to the packet (media header followed by data) to be
                     transmitted. This parameter cannot be NULL. If HeaderSize is zero,
                     then the media header in Buffer must already be filled in by the
                     caller. If HeaderSize is non-zero, then the media header will be
                     filled in by the Transmit() function.

  @retval EFI_SUCCESS           The packet was placed on the transmit queue.
  @retval EFI_NOT_READY         The network interface is too busy to accept this transmit request.                      

**/
EFI_STATUS
EthernetTransmitFrame (
  IN ETHERNET_CONTEXT  *EthernetContext,
  IN UINTN             BufferSize,
  IN UINT8             *Buffer
 )
{
  UINTN Index;
  UINTN IndexMask;
  UINTN NextIndex;
  EFI_PHYSICAL_ADDRESS PhysicalBuffer;
  EFI_PHYSICAL_ADDRESS PhysicalDescriptor;
  UINT8 * TransmitBuffer;
  ETH_TRANSMIT_DESCRIPTOR * TransmitDescriptor;

  if (EthernetContext->LinkState != LINK_STATE_UP) {
    //
    // Link is not up
    //
    return EFI_NOT_READY;
  }

  //
  //  Determine if a transmit descriptor is available
  //
  Index = EthernetContext->TransmitIndex;
  IndexMask = EthernetContext->TransmitIndexMask;
  NextIndex = (Index + 1) & IndexMask;
  if (NextIndex == EthernetContext->TransmitCompleteIndex) {
    //
    //  All buffer descriptors are in use
    //
    return EFI_NOT_READY;
  }
  //
  //  Display the header
  //
  DEBUG ((DEBUG_VERBOSE, "0x%p: TX Buffer\n", Buffer));
  DEBUG ((DEBUG_VERBOSE,
            "TX: %02x-%02x-%02x-%02x-%02x-%02x <-- %02x-%02x-%02x-%02x-%02x-%02x, %d bytes\n",
            Buffer [ 0 ],
            Buffer [ 1 ],
            Buffer [ 2 ],
            Buffer [ 3 ],
            Buffer [ 4 ],
            Buffer [ 5 ],
            Buffer [ 6 ],
            Buffer [ 7 ],
            Buffer [ 8 ],
            Buffer [ 9 ],
            Buffer [ 10 ],
            Buffer [ 11 ],
            BufferSize));

  //
  //  Copy the data into the transmit buffer
  //
  TransmitBuffer = &EthernetContext->TransmitBuffer [ Index * ETH_RECEIVE_BUFFER_SIZE ];
  PhysicalBuffer = (TransmitBuffer - EthernetContext->TransmitBuffer)
                 + EthernetContext->PhysicalTransmitBuffer;
  CopyMem (TransmitBuffer, Buffer, BufferSize);
  EthernetContext->UserBuffers [ Index ] = Buffer;

  //
  //  Build the transmit descriptor
  //
  TransmitDescriptor = &EthernetContext->TransmitDescriptors [ Index ];
  ZeroMem (TransmitDescriptor, sizeof (*TransmitDescriptor));
  TransmitDescriptor->TxFrameBufferAddress = (UINT32)(UINTN)PhysicalBuffer;
  TransmitDescriptor->Length = (UINT16)BufferSize;
  TransmitDescriptor->TxLength = ETH_TX_LENGTH (BufferSize);
  TransmitDescriptor->TxFrameControl = (UINT16)(TX_FRAME_CONTROL_TCP_ACC_OFF
      | ((60 > BufferSize) ? TX_FRAME_CONTROL_APAD : 0));

  //
  //  Hand the transmit descriptor to the TX DMA engine
  //
  PhysicalDescriptor = ((UINT8 *)&EthernetContext->TransmitDescriptors [ NextIndex ] - (UINT8 *)EthernetContext->TransmitDescriptors)
                     + EthernetContext->PhysicalTransmitDescriptors;
  PciIoMemWrite32 (EthernetContext, ETH_REG_TX_DESCR_SOFT_POINTER, (UINT32)(UINTN)PhysicalDescriptor);

  //
  //  Enable the transmit DMA engine
  //
  PciIoMemWrite32 (EthernetContext, ETH_REG_DMA_CONTROL, ETH_DMA_CONTROL_RX_DMA_EN | ETH_DMA_CONTROL_TX_DMA_EN);

  //
  //  Account for this descriptor use
  //
  EthernetContext->TransmitIndex = NextIndex;

  return EFI_SUCCESS;
}


/**
  Determine if a transmit is complete

  This routine must be called at TPL_NOTIFY.

  @param[in] EthernetContext  Address of an ETHERNET_CONTEXT structure

  @return  This routine returns the buffer address of the completed frame.

**/
UINT8 *
EthernetTransmitComplete (
  IN ETHERNET_CONTEXT  *EthernetContext
 )
{
  UINTN GmacStatus;
  UINT32 HardwarePointer;
  UINTN Index;
  UINTN IndexMask;
  UINTN NextIndex;
  EFI_STATUS Status;
  UINT8 * TransmitBuffer;
  ETH_TRANSMIT_DESCRIPTOR * TransmitDescriptor;
  UINTN TransmitIndex;

  //
  //  Assume no buffer is ready
  //
  TransmitBuffer = NULL;
  Status = EFI_NOT_READY;
  if ((EthernetContext->InterruptStatus & EFI_SIMPLE_NETWORK_TRANSMIT_INTERRUPT) != 0) {
    //
    //  Determine if a transmit descriptor is available
    //
    Index = EthernetContext->TransmitCompleteIndex;
    IndexMask = EthernetContext->TransmitIndexMask;
    HardwarePointer = PciIoMemRead32 (EthernetContext, ETH_REG_TX_DESCR_HARD_POINTER_HOLD);
    TransmitIndex = (ETH_TRANSMIT_DESCRIPTOR *)(UINTN)HardwarePointer - EthernetContext->TransmitDescriptors;
    TransmitDescriptor = &EthernetContext->TransmitDescriptors [ Index ];
    if ((TransmitIndex != Index) && (0 != TransmitDescriptor->GmacStatus)) {
      //
      //  At least one frame is complete
      //
      NextIndex = (Index + 1) & IndexMask;

      //
      //  Get the buffer address
      //
      TransmitBuffer = EthernetContext->UserBuffers [ Index ];
      DEBUG ((DEBUG_VERBOSE, "0x%p: Returning TX buffer\n", TransmitBuffer));

      //
      //  Update the statistics
      //
      GmacStatus = TransmitDescriptor->GmacStatus;
      EthernetContext->Statistics.TxTotalFrames += 1;
      EthernetContext->Statistics.TxTotalBytes += TransmitDescriptor->Length;
      if (0 != (GmacStatus & (TX_GMAC_STATUS_ABT
                               | TX_GMAC_STATUS_EXCOL
                               | TX_GMAC_STATUS_CRSER
                               | TX_GMAC_STATUS_TLNG
                               | TX_GMAC_STATUS_LTCOL
                               | TX_GMAC_STATUS_TFUNDFLW))) {
        if (0 != (GmacStatus & TX_GMAC_STATUS_EXCOL)) {
          EthernetContext->Statistics.Collisions += 16;
        } else {
          EthernetContext->Statistics.TxCrcErrorFrames += 1;
        }
      } else if (0 != (GmacStatus & TX_GMAC_STATUS_TSHRT)) {
        EthernetContext->Statistics.TxDroppedFrames += 1;
      } else {
        EthernetContext->Statistics.TxGoodFrames += 1;
        if ((64 - 4) > TransmitDescriptor->Length) {
          EthernetContext->Statistics.TxUndersizeFrames += 1;
        } else if ((1518 - 4) < TransmitDescriptor->Length) {
          EthernetContext->Statistics.TxOversizeFrames += 1;
        }
        if ((0xff == TransmitBuffer [ 0 ])
          && (0xff == TransmitBuffer [ 1 ])
          && (0xff == TransmitBuffer [ 2 ])
          && (0xff == TransmitBuffer [ 3 ])
          && (0xff == TransmitBuffer [ 4 ])
          && (0xff == TransmitBuffer [ 5 ])) {
          EthernetContext->Statistics.TxBroadcastFrames += 1;
        } else if (0 != (TransmitBuffer [ 0 ] & 1)) {
          EthernetContext->Statistics.TxMulticastFrames += 1;
        } else {
          EthernetContext->Statistics.TxUnicastFrames += 1;
        }
        if (0 != (GmacStatus & TX_GMAC_STATUS_MLTCOL)) {
          EthernetContext->Statistics.Collisions += 2;
        } else if (0 != (GmacStatus & TX_GMAC_STATUS_SNGCOL)) {
          EthernetContext->Statistics.Collisions += 1;
        }
      }

      //
      //  Release this descriptor to the transmit routine
      //
      EthernetContext->TransmitCompleteIndex = NextIndex;

      //
      //  When all of the buffers are returned, clear the transmit interrupt
      //
      if (TransmitIndex == NextIndex) {
        //
        //  No more completed buffers
        //
        EthernetContext->InterruptStatus &= ~EFI_SIMPLE_NETWORK_TRANSMIT_INTERRUPT;
      }
    }
  }

  //
  //  Return the transmit buffer
  //
  return TransmitBuffer;
}


/**
  Start the Ethernet controller

  This routine allocates the necessary resources for the driver.

  This routine is called by EthernetDriverStart to complete the driver
  initialization.

  @param[in] EthernetContext  Address of an ETHERNET_CONTEXT structure

  @retval EFI_SUCCESS         Driver API properly initialized
  
**/
EFI_STATUS
EthernetApiStart (
  IN ETHERNET_CONTEXT  *EthernetContext
 )
{
  EFI_STATUS                   Status;
  EFI_SIMPLE_NETWORK_MODE      *ModeData;
  EFI_SIMPLE_NETWORK_PROTOCOL  *SimpleNetworkProtocol;

  //
  // Retrieve original PCI attributes and save them in the private context data
  // structure.
  //
  Status = EthernetContext->PciIo->Attributes (
                                     EthernetContext->PciIo,
                                     EfiPciIoAttributeOperationGet,
                                     0,
                                     &EthernetContext->OriginalPciAttributes
                                     );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "ERROR - Failed to retrieve PCI attributes, Status: %r\n", Status));
    return Status;
  }

  //
  // Retrieve attributes that the PCI Controller supports
  //
  Status = EthernetContext->PciIo->Attributes (
                                     EthernetContext->PciIo,
                                     EfiPciIoAttributeOperationSupported,
                                     0,
                                     &EthernetContext->PciSupports
                                     );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "ERROR - Failed to retrieve PCI supported attributes, Status: %r\n", Status));
    return Status;
  }

  //
  // Enable Command register 
  //
  Status = EthernetContext->PciIo->Attributes (
                                     EthernetContext->PciIo,
                                     EfiPciIoAttributeOperationEnable,
                                     EthernetContext->PciSupports & EFI_PCI_DEVICE_ENABLE,
                                     NULL
                                     ); 
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "ERROR - Failed to enable PCI attributes, Status: %r\n", Status));
    return Status;
  }

  //
  //  Get the MAC address
  //
  Status = EthernetGetMacAddress (EthernetContext, &EthernetContext->ModeData.PermanentAddress);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "ERROR - Failed to read the MAC address, Status: %r\n", Status));
    return Status;
  }

  //
  //  Build the simple network protocol
  //
  SimpleNetworkProtocol = &EthernetContext->SimpleNetworkProtocol;
  ModeData = &EthernetContext->ModeData;
  CopyMem (
    SimpleNetworkProtocol,
    &gEthernetSimpleNetwork,
    sizeof (*SimpleNetworkProtocol)
    );
  SimpleNetworkProtocol->Mode = ModeData;
  Status = gBS->CreateEvent (
                  EVT_NOTIFY_WAIT,
                  TPL_NOTIFY,
                  EthernetWaitForPacketNotify,
                  EthernetContext,
                  &EthernetContext->SimpleNetworkProtocol.WaitForPacket
                 );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "ERROR - Failed to create wait event, Status: %r\n", Status));
    return Status;
  }

  //
  //  Initialize the mode data
  //
  ModeData->State = EfiSimpleNetworkStopped;
  ModeData->HwAddressSize = PXE_HWADDR_LEN_ETHER;
  ModeData->MediaHeaderSize = ETHERNET_HEADER_SIZE;
  ModeData->MaxPacketSize = PXE_MAX_TXRX_UNIT_ETHER;
  ModeData->NvRamSize = 0;
  ModeData->NvRamAccessSize = 0;
  ModeData->ReceiveFilterMask = EFI_SIMPLE_NETWORK_RECEIVE_UNICAST
                              | EFI_SIMPLE_NETWORK_RECEIVE_MULTICAST
                              | EFI_SIMPLE_NETWORK_RECEIVE_BROADCAST
                              | EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS
                              | EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS_MULTICAST;
  ModeData->ReceiveFilterSetting = 0;
  ModeData->MaxMCastFilterCount = ETH_MAX_MCAST_FILTER_CNT;
  ModeData->MCastFilterCount = 0;
  SetMem (&ModeData->BroadcastAddress,
           PXE_HWADDR_LEN_ETHER,
           0xff);
  ModeData->IfType = EfiNetworkInterfaceUndi;
  ModeData->MacAddressChangeable = TRUE;
  ModeData->MultipleTxSupported = TRUE;

  //
  // BUGBUG: The logic in this driver should be updated to support dynamic media detection.
  //
//    ModeData->MediaPresentSupported = TRUE;    
  ModeData->MediaPresentSupported = FALSE;

  ModeData->MediaPresent = FALSE;

  //
  //  Set the default MAC address
  //
  CopyMem (
    &ModeData->CurrentAddress,
    &ModeData->PermanentAddress,
    sizeof (ModeData->PermanentAddress)
    );
  DEBUG ((
    DEBUG_VERBOSE,
    "%02x-%02x-%02x-%02x-%02x-%02x PermanentAddress\n",
    ModeData->PermanentAddress.Addr[0],
    ModeData->PermanentAddress.Addr[1],
    ModeData->PermanentAddress.Addr[2],
    ModeData->PermanentAddress.Addr[3],
    ModeData->PermanentAddress.Addr[4],
    ModeData->PermanentAddress.Addr[5]
    ));

  //
  //  Get the interval timer to monitor the link
  //
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  EthernetTimer,
                  EthernetContext,
                  &EthernetContext->Timer
                 );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "ERROR - Failed to create timer event, Status: %r\n", Status));
    return Status;
  }

  return EFI_SUCCESS;
}


/**
  Stop the Ethernet controller

  This routine releases the resources allocated by EthernetApiStart.

  This routine is called by EthernetDriverStop to initiate the driver
  shutdown.

  @param[in] EthernetContext  Address of an ETHERNET_CONTEXT structure

**/
VOID
EthernetApiStop (
  IN ETHERNET_CONTEXT * EthernetContext
 )
{
  //
  //  Free the events
  //
  if (EthernetContext->Timer != NULL) {
    gBS->CloseEvent (EthernetContext->Timer);
  }
  EthernetContext->Timer = NULL;
  if (EthernetContext->SimpleNetworkProtocol.WaitForPacket != NULL) {
    gBS->CloseEvent (EthernetContext->SimpleNetworkProtocol.WaitForPacket);
  }
  EthernetContext->SimpleNetworkProtocol.WaitForPacket = NULL;

  //
  // Restore original PCI attributes
  //
  EthernetContext->PciIo->Attributes (
                            EthernetContext->PciIo,
                            EfiPciIoAttributeOperationSet,
                            EthernetContext->OriginalPciAttributes,
                            NULL
                            );
}

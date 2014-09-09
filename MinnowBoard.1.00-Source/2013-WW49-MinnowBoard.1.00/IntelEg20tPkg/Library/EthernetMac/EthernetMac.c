/** @file
  Implement the Ethernet MAC support routines.
  
  Copyright (c) 2013, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Uefi.h>

#include <IndustryStandard/Pci.h>

#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/E6xx.h>
#include <Library/EthernetMac.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Protocol/PciIo.h>

#include <Uefi/UefiPxe.h>

#include <Register/Eg20t.h>

EFI_GUID gEg20tEthernetGuid = { 0x9ad0b0fd, 0xf325, 0x425c, { 0x81, 0xdc, 0xa1, 0xc7, 0xa6, 0x27, 0x55, 0x1b } };

#define PCI_BRIDGE_ENABLE       ( EFI_PCI_COMMAND_MEMORY_SPACE | EFI_PCI_COMMAND_IO_SPACE )


/**
  Get the MAC address for the Ethernet controller

  @param[in]  PciIo       Pointer to an EFI_PCI_IO_PROTOCOL structure
  @param[out] MacAddress  Address of a buffer to receive the MAC address

  @retval EFI_SUCCESS     The function completed successfully.
  @retval EFI_NOT_FOUND   The variable was not found.

**/
EFI_STATUS
EthernetGetMacAddress (
  IN EFI_PCI_IO_PROTOCOL *PciIo,
  OUT EFI_MAC_ADDRESS * MacAddress
  )
{
  EFI_STATUS Status;

  //
  //  Get the MAC address
  //
  Status = PciIo->Mem.Read ( PciIo,
                             EfiPciWidthUint32,
                             1,
                             ETH_REG_MAC_ADDRESS_1_A,
                             2,
                             MacAddress );
  if ( !EFI_ERROR ( Status )) {
    ZeroMem ( &MacAddress->Addr [ PXE_HWADDR_LEN_ETHER ],
              sizeof ( *MacAddress ) - PXE_HWADDR_LEN_ETHER );
  }
  return Status;
}


EFI_STATUS
EthernetPciBridgeEnable (
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *PciRootBridgeIo,
  IN UINT8 BusNumber,
  IN UINT8 Device,
  IN UINT8 Function,
  OUT UINT32 * Command,
  OUT UINT8 * NextBus
  )
{
  UINT64 Address;
  UINT32 Value;
  UINT32 NewValue;
  EFI_STATUS Status;

  //
  //  Get the primary bus value
  //
  Address = EFI_PCI_ADDRESS ( BusNumber, 
                              Device, 
                              Function, 
                              0 );
  Status = PciRootBridgeIo->Pci.Read ( PciRootBridgeIo,
                                       EfiPciIoWidthUint8,
                                       Address | PCI_BRIDGE_SECONDARY_BUS_REGISTER_OFFSET,
                                       1,
                                       NextBus );
  if ( EFI_ERROR ( Status )) {
    DEBUG (( DEBUG_ERROR,
              "ERROR - Failed to read the secondary bus register, Status: %r\r\n",
              Status ));
  }
  else {
    //
    //  Get the command value
    //
    Status = PciRootBridgeIo->Pci.Read ( PciRootBridgeIo,
                                         EfiPciIoWidthUint32,
                                         Address | PCI_COMMAND_OFFSET,
                                         1,
                                         &Value );
    if ( EFI_ERROR ( Status )) {
      DEBUG (( DEBUG_ERROR,
                "ERROR - Failed to read the PCI command register, Status: %r\r\n",
                Status ));
    }
    else {
      //
      //  Save the command value
      //
      *Command = Value;

      //
      //  Enable the bridge
      //
      NewValue = Value | PCI_BRIDGE_ENABLE;
      if ( Value != NewValue ) {
        Status = PciRootBridgeIo->Pci.Write ( PciRootBridgeIo,
                                              EfiPciIoWidthUint32,
                                              Address | PCI_COMMAND_OFFSET,
                                              1,
                                              &NewValue );
        if ( EFI_ERROR ( Status )) {
          DEBUG (( DEBUG_ERROR,
                    "ERROR - Failed to write the PCI command register, Status: %r\r\n",
                    Status ));
        }
      }
    }
  }

  //
  //  Return the operation status
  //
  return Status;
}


EFI_STATUS
EthernetPciBridgeRestore (
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *PciRootBridgeIo,
  IN UINT8 BusNumber,
  IN UINT8 Device,
  IN UINT8 Function,
  IN UINT32 Command
  )
{
  UINT64 Address;
  EFI_STATUS Status;

  //
  //  Restore the bridge state
  //
  Status = EFI_SUCCESS;
  if ( Command != ( Command | PCI_BRIDGE_ENABLE )) {
    Address = EFI_PCI_ADDRESS ( BusNumber, 
                                Device, 
                                Function, 
                                PCI_COMMAND_OFFSET );
    Status = PciRootBridgeIo->Pci.Write ( PciRootBridgeIo,
                                          EfiPciIoWidthUint32,
                                          Address,
                                          1,
                                          &Command );
    if ( EFI_ERROR ( Status )) {
      DEBUG (( DEBUG_ERROR,
                "ERROR - Failed to write the PCI command register, Status: %r\r\n",
                Status ));
      ASSERT ( EFI_SUCCESS == Status );
    }
  }

  //
  //  Return the operation status
  //
  return Status;
}


/**
  Set the MAC address for the Ethernet controller

  @param[in] PciRootBridgeIo Pointer to an EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL structure
  @param[in] PciAddress      Address of the device configuration space
  @param[in] MacAddress      Address of a buffer containing the MAC address

  @retval EFI_SUCCESS           The function completed successfully.
  @retval EFI_INVALID_PARAMETER MacAddress is NULL
  @retval EFI_INVALID_PARAMETER MacAddress points to a multicast address

**/
EFI_STATUS
EthernetSetMacAddress (
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *PciRootBridgeIo,
  IN UINT64 PciAddress,
  IN EFI_MAC_ADDRESS * MacAddress
  )
{
  UINT64 BaseAddress;
  UINT32 Bridge0Command;
  UINT32 Bridge1Command;
  UINT8 Bus1;
  UINT8 Bus2;
  UINT32 Command;
  UINT32 Data;
  EFI_MAC_ADDRESS EthernetAddress;
  UINT32 Mask;
  EFI_STATUS Status;

  //
  //  Set the Ethernet MAC address only if it is not a multicast address
  //
  if (( NULL == MacAddress ) || ( 0 != ( MacAddress->Addr [ 0 ] & 1 ))) {
    DEBUG (( DEBUG_ERROR,
              "ERROR - Bad Ethernet MAC address: %02x-%02x-%02x-%02x-%02x-%02x\r\n",
              MacAddress->Addr [ 0 ],
              MacAddress->Addr [ 1 ],
              MacAddress->Addr [ 2 ],
              MacAddress->Addr [ 3 ],
              MacAddress->Addr [ 4 ],
              MacAddress->Addr [ 5 ]));
    Status = EFI_INVALID_PARAMETER;
  }
  else {
    //
    //  Enable the root bridge
    //
    Status = EthernetPciBridgeEnable ( PciRootBridgeIo,
                                       0,
                                       E6XX_PCIE_PORT_0,
                                       0,
                                       &Bridge0Command,
                                       &Bus1 );
    if ( !EFI_ERROR ( Status )) {
      //
      //  Enable the secondary bridge
      //
      Status = EthernetPciBridgeEnable ( PciRootBridgeIo,
                                         Bus1,
                                         0,
                                         0,
                                         &Bridge1Command,
                                         &Bus2 );
      if ( !EFI_ERROR ( Status )) {
        //
        //  Use break instead of goto
        //
        for ( ; ; ) {
          //
          //  Get the PCI controller status
          //
          Status = PciRootBridgeIo->Pci.Read ( PciRootBridgeIo,
                                               EfiPciIoWidthUint32,
                                               PciAddress | PCI_COMMAND_OFFSET,
                                               1,
                                               &Command );
          if ( EFI_ERROR ( Status )) {
            DEBUG (( DEBUG_ERROR,
                      "ERROR - Failed to read the PCI command register, Status: %r\r\n",
                      Status ));
            break;
          }

          //
          //  Enable the PCI controller
          //
          Mask = PCI_COMMAND_INTERRUPT_DISABLE
               | EFI_PCI_COMMAND_BUS_MASTER
               | EFI_PCI_COMMAND_MEMORY_SPACE;
          if ( Mask != ( Command & Mask )) {
            Command |= Mask;
            Status = PciRootBridgeIo->Pci.Write ( PciRootBridgeIo,
                                                  EfiPciIoWidthUint32,
                                                  PciAddress | PCI_COMMAND_OFFSET,
                                                  1,
                                                  &Command );
            if ( EFI_ERROR ( Status )) {
              DEBUG (( DEBUG_ERROR,
                        "ERROR - Failed to write the PCI command register, Status: %r\r\n",
                        Status ));
              break;
            }
          }

          //
          //  Read the base address register
          //
          BaseAddress = 0;
          Status = PciRootBridgeIo->Pci.Read ( PciRootBridgeIo,
                                               EfiPciIoWidthUint32,
                                               PciAddress | ( PCI_BASE_ADDRESSREG_OFFSET + 4 ),
                                               1,
                                               &BaseAddress );
          if ( EFI_ERROR ( Status )) {
            DEBUG (( DEBUG_ERROR,
                      "ERROR - Failed to read the base address register, Status: %r\r\n",
                      Status ));
            break;
          }

          //
          //  Enable the Ethernet clocks
          //
          Data = MODE_ETHER_MODE;
          Status = PciRootBridgeIo->Mem.Write ( PciRootBridgeIo,
                                                EfiPciWidthUint32,
                                                BaseAddress | ETH_REG_MODE,
                                                1,
                                                &Data );
          if ( EFI_ERROR ( Status )) {
            DEBUG (( DEBUG_ERROR,
                      "ERROR - Failed write to 0x%08x, Status: %r\r\n",
                      (UINT32)( BaseAddress | ETH_REG_MODE ),
                      Status ));
            break;
          }

          //
          //  Request access to MAC address 0
          //
          DEBUG (( DEBUG_VERBOSE, "Waiting to access the MAC address register\r\n" ));
          Data = 1;
          Status = PciRootBridgeIo->Mem.Write ( PciRootBridgeIo,
                                                EfiPciWidthUint32,
                                                BaseAddress | ETH_REG_ADDRESS_MASK,
                                                1,
                                                &Data );
          if ( EFI_ERROR ( Status )) {
            DEBUG (( DEBUG_ERROR,
                      "ERROR - Failed write to 0x%08x, Status: %r\r\n",
                      (UINT32)( BaseAddress | ETH_REG_ADDRESS_MASK ),
                      Status ));
            break;
          }
          do {
            Status = PciRootBridgeIo->Mem.Read ( PciRootBridgeIo,
                                                 EfiPciWidthUint32,
                                                 BaseAddress | ETH_REG_ADDRESS_MASK,
                                                 1,
                                                 &Data );
          } while (( !EFI_ERROR ( Status )) && ( 0 != ( Data & ETH_ADDRESS_MASK_BUSY )));
          if ( EFI_ERROR ( Status )) {
            DEBUG (( DEBUG_ERROR,
                      "ERROR - Failed read from 0x%08x, Status: %r\r\n",
                      (UINT32)( BaseAddress | ETH_REG_ADDRESS_MASK ),
                      Status ));
            break;
          }

          //
          //  Set the Ethernet MAC address
          //
          Status = PciRootBridgeIo->Mem.Write ( PciRootBridgeIo,
                                                EfiPciWidthUint32,
                                                BaseAddress | ETH_REG_MAC_ADDRESS_1_A,
                                                2,
                                                &MacAddress->Addr [ 0 ]);
          if ( EFI_ERROR ( Status )) {
            DEBUG (( DEBUG_ERROR,
                      "ERROR - Failed to write the MAC address, Status: %r\r\n",
                      Status ));
            break;
          }

          //
          //  Verify the MAC address
          //
          Status = PciRootBridgeIo->Mem.Read ( PciRootBridgeIo,
                                               EfiPciWidthUint32,
                                               BaseAddress | ETH_REG_MAC_ADDRESS_1_A,
                                               2,
                                               &EthernetAddress.Addr [ 0 ]);
          if ( EFI_ERROR ( Status )) {
            DEBUG (( DEBUG_ERROR,
                      "MAC Address: %02x-%02x-%02x-%02x-%02x-%02x\r\n",
                      EthernetAddress.Addr [ 0 ],
                      EthernetAddress.Addr [ 1 ],
                      EthernetAddress.Addr [ 2 ],
                      EthernetAddress.Addr [ 3 ],
                      EthernetAddress.Addr [ 4 ],
                      EthernetAddress.Addr [ 5 ]));
            DEBUG (( DEBUG_ERROR,
                      "ERROR - Failed to read the MAC address, Status: %r\r\n",
                      Status ));
            break;
          }
          if ( 0 != CompareMem ( &MacAddress->Addr [ 0 ], &EthernetAddress.Addr [ 0 ], 6 )) {
            DEBUG (( DEBUG_ERROR,
                      "ERROR - Ethernet address validation failed!\r\n" ));
            Status = EFI_DEVICE_ERROR;
            break;
          }

          //
          //  Release access to MAC address 0
          //
          DEBUG (( DEBUG_VERBOSE, "Waiting for the release of the MAC address register\r\n" ));
          Data = 0;
          Status = PciRootBridgeIo->Mem.Write ( PciRootBridgeIo,
                                                EfiPciWidthUint32,
                                                BaseAddress | ETH_REG_ADDRESS_MASK,
                                                1,
                                                &Data );
          if ( EFI_ERROR ( Status )) {
            DEBUG (( DEBUG_ERROR,
                      "ERROR - Failed write to 0x%08x, Status: %r\r\n",
                      (UINT32)( BaseAddress | ETH_REG_ADDRESS_MASK ),
                      Status ));
            break;
          }
          do {
            Status = PciRootBridgeIo->Mem.Read ( PciRootBridgeIo,
                                                 EfiPciWidthUint32,
                                                 BaseAddress | ETH_REG_ADDRESS_MASK,
                                                 1,
                                                 &Data );
          } while (( !EFI_ERROR ( Status )) && ( 0 != ( Data & ETH_ADDRESS_MASK_BUSY )));
          if ( EFI_ERROR ( Status )) {
            DEBUG (( DEBUG_ERROR,
                      "ERROR - Failed read from 0x%08x, Status: %r\r\n",
                      (UINT32)( BaseAddress | ETH_REG_ADDRESS_MASK ),
                      Status ));
            break;
          }

          //
          //  Display the Ethernet MAC address
          //
          DEBUG (( DEBUG_INFO, "--------------------\r\n" ));
          DEBUG (( DEBUG_INFO,
                    "EG20T MAC address: %02x-%02x-%02x-%02x-%02x-%02x\r\n",
                    MacAddress->Addr [ 0 ],
                    MacAddress->Addr [ 1 ],
                    MacAddress->Addr [ 2 ],
                    MacAddress->Addr [ 3 ],
                    MacAddress->Addr [ 4 ],
                    MacAddress->Addr [ 5 ]));
          DEBUG (( DEBUG_INFO, "--------------------\r\n" ));

          //
          //  Done
          //
          break;
        }

        //
        //  Restore the secondary bridge state
        //
        EthernetPciBridgeRestore ( PciRootBridgeIo,
                                   Bus1,
                                   0,
                                   0,
                                   Bridge1Command );
      }

      //
      //  Restore the root bridge state
      //
      EthernetPciBridgeRestore ( PciRootBridgeIo,
                                 0,
                                 E6XX_PCIE_PORT_0,
                                 0,
                                 Bridge0Command );
    }
  }

  //
  //  Return the operation status
  //
  return Status;
}


/**
  Configure the Ethernet MAC based upon the requested settings

  @param[in, out] MacControl  Pointer to a MAC_CONTROL_REGISTERS structure
  @param[in] LinkSpeed        Speed of the link
  @param[in] LinkFullDuplex   Duplex setting for the link

**/
VOID
EthernetMacConfigure (
  IN OUT MAC_CONTROL_REGISTERS * MacControl,
  IN UINTN LinkSpeed,
  IN BOOLEAN LinkFullDuplex
  )
{
  UINT32 Mode;
  UINT32 RgmiiControl;

  //
  //  Set the default values
  //
  Mode = MacControl->Mode
       & ( ~( MODE_DUPLEX_MODE | MODE_ETHER_MODE ));
  RgmiiControl = MacControl->RgmiiControl
               & ( ~ ( RGMII_CTL_RGMII_RATE
                     | RGMII_CTL_CRS_SEL ));

  //
  //  Determine the connection properties
  //
  if ( 1000 == LinkSpeed ) {
    //
    //  1000 Mb/s
    //
    Mode |= MODE_ETHER_MODE;
    RgmiiControl |= RGMII_CTL_RGMII_RATE_125_MHZ
                  | RGMII_CTL_RGMII_MODE;
    if ( LinkFullDuplex ) {
      Mode |= MODE_DUPLEX_MODE;
    }
  }
  else if ( 100 == LinkSpeed ) {
    //
    //  100 Mb/s
    //
    RgmiiControl |= RGMII_CTL_RGMII_RATE_25_MHZ
                  | RGMII_CTL_RGMII_MODE;
    if ( LinkFullDuplex ) {
      Mode |= MODE_DUPLEX_MODE;
    }
  }
  else {
    //
    //  10 Mb/s
    //
    RgmiiControl |= RGMII_CTL_RGMII_RATE_2_5_MHZ
                  | RGMII_CTL_RGMII_MODE
                  | RGMII_CTL_CRS_SEL;
    if ( LinkFullDuplex ) {
      Mode |= MODE_DUPLEX_MODE;
    }
  }

  //
  //  Update the MAC register values
  //
  MacControl->Mode = Mode;
  MacControl->RgmiiControl = RgmiiControl;
}

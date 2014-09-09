/** @file
  EG20T GPIO support library header file
  
  Copyright (c) 2013, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _EG20T_H_
#define _EG20T_H_

#include <IndustryStandard/Usb.h>

//----------------------------------------------------------------------
//  PCI support
//----------------------------------------------------------------------
#define EG20T_PCI_VENDOR_ID                            0x8086

#define EG20T_PCI_EXPRESS_BRIDGE_PCI_DEVICE_ID         0x8800
#define EG20T_PCI_EXPRESS_BRIDGE_PCI_DEVICE_NUMBER     0
#define EG20T_PCI_EXPRESS_BRIDGE_PCI_FUNCTION_NUMBER   0

#define EG20T_PACKET_HUB_PCI_DEVICE_ID                 0x8801
#define EG20T_PACKET_HUB_PCI_DEVICE_NUMBER             0
#define EG20T_PACKET_HUB_PCI_FUNCTION_NUMBER           0

#define EG20T_ETHERNET_PCI_DEVICE_ID                   0x8802
#define EG20T_ETHERNET_PCI_DEVICE_NUMBER               0
#define EG20T_ETHERNET_PCI_FUNCTION_NUMBER             1

#define EG20T_GPIO_PCI_DEVICE_ID                       0x8803
#define EG20T_GPIO_PCI_DEVICE_NUMBER                   0
#define EG20T_GPIO_PCI_FUNCTION_NUMBER                 2

#define EG20T_OHCI0_HOST1_PCI_DEVICE_ID                0x8804
#define EG20T_OHCI0_HOST1_PCI_DEVICE_NUMBER            2
#define EG20T_OHCI0_HOST1_PCI_FUNCTION_NUMBER          0

#define EG20T_OHCI1_HOST1_PCI_DEVICE_ID                0x8805
#define EG20T_OHCI1_HOST1_PCI_DEVICE_NUMBER            2
#define EG20T_OHCI1_HOST1_PCI_FUNCTION_NUMBER          1

#define EG20T_OHCI2_HOST1_PCI_DEVICE_ID                0x8806
#define EG20T_OHCI2_HOST1_PCI_DEVICE_NUMBER            2
#define EG20T_OHCI2_HOST1_PCI_FUNCTION_NUMBER          2

#define EG20T_EHCI_HOST1_PCI_DEVICE_ID                 0x8807
#define EG20T_EHCI_HOST1_PCI_DEVICE_NUMBER             2
#define EG20T_EHCI_HOST1_PCI_FUNCTION_NUMBER           3

#define EG20T_USB_DEVICE_PCI_DEVICE_ID                 0x8808
#define EG20T_USB_DEVICE_PCI_DEVICE_NUMBER             2
#define EG20T_USB_DEVICE_PCI_FUNCTION_NUMBER           4

#define EG20T_SDIO0_PCI_DEVICE_ID                      0x8809
#define EG20T_SDIO0_PCI_DEVICE_NUMBER                  4
#define EG20T_SDIO0_PCI_FUNCTION_NUMBER                0

#define EG20T_SDIO1_PCI_DEVICE_ID                      0x880A
#define EG20T_SDIO1_PCI_DEVICE_NUMBER                  4
#define EG20T_SDIO1_PCI_FUNCTION_NUMBER                1

#define EG20T_SATA_PCI_DEVICE_ID                       0x880B
#define EG20T_SATA_PCI_DEVICE_NUMBER                   6
#define EG20T_SATA_PCI_FUNCTION_NUMBER                 0

#define EG20T_OHCI0_HOST0_PCI_DEVICE_ID                0x880C
#define EG20T_OHCI0_HOST0_PCI_DEVICE_NUMBER            8
#define EG20T_OHCI0_HOST0_PCI_FUNCTION_NUMBER          0

#define EG20T_OHCI1_HOST0_PCI_DEVICE_ID                0x880D
#define EG20T_OHCI1_HOST0_PCI_DEVICE_NUMBER            8
#define EG20T_OHCI1_HOST0_PCI_FUNCTION_NUMBER          1

#define EG20T_OHCI2_HOST0_PCI_DEVICE_ID                0x880E
#define EG20T_OHCI2_HOST0_PCI_DEVICE_NUMBER            8
#define EG20T_OHCI2_HOST0_PCI_FUNCTION_NUMBER          2

#define EG20T_EHCI_HOST0_PCI_DEVICE_ID                 0x880F
#define EG20T_EHCI_HOST0_PCI_DEVICE_NUMBER             8
#define EG20T_EHCI_HOST0_PCI_FUNCTION_NUMBER           3

#define EG20T_DMA0_PCI_DEVICE_ID                       0x8810
#define EG20T_DMA0_PCI_DEVICE_NUMBER                   10
#define EG20T_DMA0_PCI_FUNCTION_NUMBER                 0

#define EG20T_UART0_PCI_DEVICE_ID                      0x8811
#define EG20T_UART0_PCI_DEVICE_NUMBER                  10
#define EG20T_UART0_PCI_FUNCTION_NUMBER                1

#define EG20T_UART1_PCI_DEVICE_ID                      0x8812
#define EG20T_UART1_PCI_DEVICE_NUMBER                  10
#define EG20T_UART1_PCI_FUNCTION_NUMBER                2

#define EG20T_UART2_PCI_DEVICE_ID                      0x8813
#define EG20T_UART2_PCI_DEVICE_NUMBER                  10
#define EG20T_UART2_PCI_FUNCTION_NUMBER                3

#define EG20T_UART3_PCI_DEVICE_ID                      0x8814
#define EG20T_UART3_PCI_DEVICE_NUMBER                  10
#define EG20T_UART3_PCI_FUNCTION_NUMBER                4

#define EG20T_DMA1_PCI_DEVICE_ID                       0x8815
#define EG20T_DMA1_PCI_DEVICE_NUMBER                   12
#define EG20T_DMA1_PCI_FUNCTION_NUMBER                 0

#define EG20T_SPI_PCI_DEVICE_ID                        0x8816
#define EG20T_SPI_PCI_DEVICE_NUMBER                    12
#define EG20T_SPI_PCI_FUNCTION_NUMBER                  1

#define EG20T_I2C_PCI_DEVICE_ID                        0x8817
#define EG20T_I2C_PCI_DEVICE_NUMBER                    12
#define EG20T_I2C_PCI_FUNCTION_NUMBER                  2

#define EG20T_CAN_PCI_DEVICE_ID                        0x8818
#define EG20T_CAN_PCI_DEVICE_NUMBER                    12
#define EG20T_CAN_PCI_FUNCTION_NUMBER                  3

#define EG20T_IEEE1588_PCI_DEVICE_ID                   0x8819
#define EG20T_IEEE1588_PCI_DEVICE_NUMBER               12
#define EG20T_IEEE1588_PCI_FUNCTION_NUMBER             4

///
/// PCI Command register bits
/// NOTE: Add to MdePkg/Include/IndustryStandard/Pci22.h
///
#define PCI_COMMAND_INTERRUPT_DISABLE   BIT10  // Disable device interrupts

//----------------------------------------------------------------------
//  Clocks
//----------------------------------------------------------------------

#define EG20T_CLKH            100000000 /// 100 MHz
#define EG20T_CLKL             50000000 ///  50 MHz

//----------------------------------------------------------------------
//  Packet Hub
//----------------------------------------------------------------------
#define EG20T_PACKET_HUB_CLKCFG_OFFSET       0x500

typedef struct {
  UINT32  PLL0PD:1;       // BIT 0
  UINT32  Reserved5:3;    // BIT 1..3
  UINT32  PLL1PD:1;       // BIT 4
  UINT32  Reserved4:3;    // BIT 5..7
  UINT32  PLL2PD:1;       // BIT 8
  UINT32  PLL2VCO:4;      // BIT 9..12
  UINT32  Reserved3:3;    // BIT 13..15
  UINT32  BAUDSEL:2;      // BIT 16..17
  UINT32  UARTCLKSEL:1;   // BIT 18
  UINT32  Reserved2:1;    // BIT 19
  UINT32  BAUDDIV:4;      // BIT 20..23
  UINT32  CANCLKSEL:2;    // BIT 24..25
  UINT32  Reserved1:1;    // BIT 26
  UINT32  CAN_PWRCHG:1;   // BIT 27
  UINT32  CANDIV:4;       // BIT 28..31
} EG20T_PACKET_HUB_CONTROL_CLKCFG_BITS;

typedef union {
  EG20T_PACKET_HUB_CONTROL_CLKCFG_BITS  Bits;
  UINT32                                Data;
} EG20T_PACKET_HUB_CONTROL_CLKCFG;

//----------------------------------------------------------------------
//  16550 UART
//----------------------------------------------------------------------

//
// Register offsets and bitfields
//
#define R_UART_RXBUF          0        // RO   Receive Buffer Register
#define R_UART_TXBUF          0        // WO   Transmit Holding Register
#define R_UART_BAUD_LOW       0        // R/W  Divisor Latch LSB
#define R_UART_BAUD_HIGH      1        // R/W  Divisor Latch MSB
#define R_UART_IER            1        // R/W  Interrupt Enable Register
#define R_UART_IIR            2        // RO   Interrupt Identification Register
#define R_UART_FCR            2        // WO   FIFO Cotrol Register
#define   B_UART_FCR_FIFOE    BIT0
#define   B_UART_FCR_FIFO64   BIT5
#define R_UART_LCR            3        // R/W  Line Control Register 
#define   B_UART_LCR_DLAB     BIT7
#define R_UART_MCR            4        // R/W  Modem Control Register 
#define   B_UART_MCR_RTS      BIT1
#define R_UART_LSR            5        // R/W  Line Status Register
#define   B_UART_LSR_RXRDY    BIT0
#define   B_UART_LSR_TXRDY    BIT5
#define   B_UART_LSR_TEMT     BIT6
#define R_UART_MSR            6        // R/W  Modem Status Register
#define   B_UART_MSR_CTS      BIT4
#define   B_UART_MSR_DSR      BIT5
#define R_UART_SCR            7        // R/W  Scratch Pad Register
#define R_UART_BRCSR          0x0E     // R/W  Baud Rate Reference Clock Select Register
#define R_UART_SRST           0x0F     // R/W  Soft Reset Register

#pragma pack(1)
//
//  Name:   SERIAL_PORT_IER_BITS
//  Purpose:  Define each bit in Interrupt Enable Register
//  Context:
//  Fields:
//     Ravie  Bit0: Receiver Data Available Interrupt Enable
//     Theie  Bit1: Transmistter Holding Register Empty Interrupt Enable
//     Rie      Bit2: Receiver Interrupt Enable
//     Mie      Bit3: Modem Interrupt Enable
//     Reserved Bit4-Bit7: Reserved
//
typedef struct {
  UINT8 Ravie : 1;
  UINT8 Theie : 1;
  UINT8 Rie : 1;
  UINT8 Mie : 1;
  UINT8 Reserved : 4;
} SERIAL_PORT_IER_BITS;

//
//  Name:   SERIAL_PORT_IER
//  Purpose:
//  Context:
//  Fields:
//      Bits    SERIAL_PORT_IER_BITS:  Bits of the IER
//      Data    UINT8: the value of the IER
//
typedef union {
  SERIAL_PORT_IER_BITS  Bits;
  UINT8                 Data;
} SERIAL_PORT_IER;

//
//  Name:   SERIAL_PORT_FCR_BITS
//  Purpose:  Define each bit in FIFO Control Register
//  Context:
//  Fields:
//      TrFIFOE    Bit0: Transmit and Receive FIFO Enable
//      ResetRF    Bit1: Reset Reciever FIFO
//      ResetTF    Bit2: Reset Transmistter FIFO
//      Dms        Bit3: DMA Mode Select
//      Reserved   Bit4: Reserved
//      Fifo256    Bit5: Enable larger FIFO
//      Rtb        Bit6-Bit7: Receive Trigger Bits
//
typedef struct {
  UINT8 TrFIFOE : 1;
  UINT8 ResetRF : 1;
  UINT8 ResetTF : 1;
  UINT8 Dms : 1;
  UINT8 Reserved : 1;
  UINT8 Fifo256 : 1;
  UINT8 Rtb : 2;
} SERIAL_PORT_FCR_BITS;

//
//  Name:   SERIAL_PORT_FCR
//  Purpose:
//  Context:
//  Fields:
//      Bits    SERIAL_PORT_FCR_BITS:  Bits of the FCR
//      Data    UINT8: the value of the FCR
//
typedef union {
  SERIAL_PORT_FCR_BITS  Bits;
  UINT8                 Data;
} SERIAL_PORT_FCR;

//
//  Name:   SERIAL_PORT_LCR_BITS
//  Purpose:  Define each bit in Line Control Register
//  Context:
//  Fields:
//      SerialDB  Bit0-Bit1: Number of Serial Data Bits
//      StopB     Bit2: Number of Stop Bits
//      ParEn     Bit3: Parity Enable
//      EvenPar   Bit4: Even Parity Select
//      SticPar   Bit5: Sticky Parity
//      BrCon     Bit6: Break Control
//      DLab      Bit7: Divisor Latch Access Bit
//
typedef struct {
  UINT8 SerialDB : 2;
  UINT8 StopB : 1;
  UINT8 ParEn : 1;
  UINT8 EvenPar : 1;
  UINT8 SticPar : 1;
  UINT8 BrCon : 1;
  UINT8 DLab : 1;
} SERIAL_PORT_LCR_BITS;

//
//  Name:   SERIAL_PORT_LCR
//  Purpose:
//  Context:
//  Fields:
//      Bits    SERIAL_PORT_LCR_BITS:  Bits of the LCR
//      Data    UINT8: the value of the LCR
//
typedef union {
  SERIAL_PORT_LCR_BITS  Bits;
  UINT8                 Data;
} SERIAL_PORT_LCR;

//
//  Name:   SERIAL_PORT_MCR_BITS
//  Purpose:  Define each bit in Modem Control Register
//  Context:
//  Fields:
//      DtrC     Bit0: Data Terminal Ready Control
//      Rts      Bit1: Request To Send Control
//      Out1     Bit2: Output1
//      Out2     Bit3: Output2, used to disable interrupt
//      Lme;     Bit4: Loopback Mode Enable
//      Reserved Bit5-Bit7: Reserved
//
typedef struct {
  UINT8 DtrC : 1;
  UINT8 Rts : 1;
  UINT8 Out1 : 1;
  UINT8 Out2 : 1;
  UINT8 Lme : 1;
  UINT8 Reserved : 3;
} SERIAL_PORT_MCR_BITS;

//
//  Name:   SERIAL_PORT_MCR
//  Purpose:
//  Context:
//  Fields:
//      Bits    SERIAL_PORT_MCR_BITS:  Bits of the MCR
//      Data    UINT8: the value of the MCR
//
typedef union {
  SERIAL_PORT_MCR_BITS  Bits;
  UINT8                 Data;
} SERIAL_PORT_MCR;

//
//  Name:   SERIAL_PORT_LSR_BITS
//  Purpose:  Define each bit in Line Status Register
//  Context:
//  Fields:
//      Dr    Bit0: Receiver Data Ready Status
//      Oe    Bit1: Overrun Error Status
//      Pe    Bit2: Parity Error Status
//      Fe    Bit3: Framing Error Status
//      Bi    Bit4: Break Interrupt Status
//      Thre  Bit5: Transmistter Holding Register Status
//      Temt  Bit6: Transmitter Empty Status
//      FIFOe Bit7: FIFO Error Status
//
typedef struct {
  UINT8 Dr : 1;
  UINT8 Oe : 1;
  UINT8 Pe : 1;
  UINT8 Fe : 1;
  UINT8 Bi : 1;
  UINT8 Thre : 1;
  UINT8 Temt : 1;
  UINT8 FIFOe : 1;
} SERIAL_PORT_LSR_BITS;

//
//  Name:   SERIAL_PORT_LSR
//  Purpose:
//  Context:
//  Fields:
//      Bits    SERIAL_PORT_LSR_BITS:  Bits of the LSR
//      Data    UINT8: the value of the LSR
//
typedef union {
  SERIAL_PORT_LSR_BITS  Bits;
  UINT8                 Data;
} SERIAL_PORT_LSR;

//
//  Name:   SERIAL_PORT_MSR_BITS
//  Purpose:  Define each bit in Modem Status Register
//  Context:
//  Fields:
//      DeltaCTS      Bit0: Delta Clear To Send Status
//      DeltaDSR        Bit1: Delta Data Set Ready Status
//      TrailingEdgeRI  Bit2: Trailing Edge of Ring Indicator Status
//      DeltaDCD        Bit3: Delta Data Carrier Detect Status
//      Cts             Bit4: Clear To Send Status
//      Dsr             Bit5: Data Set Ready Status
//      Ri              Bit6: Ring Indicator Status
//      Dcd             Bit7: Data Carrier Detect Status
//
typedef struct {
  UINT8 DeltaCTS : 1;
  UINT8 DeltaDSR : 1;
  UINT8 TrailingEdgeRI : 1;
  UINT8 DeltaDCD : 1;
  UINT8 Cts : 1;
  UINT8 Dsr : 1;
  UINT8 Ri : 1;
  UINT8 Dcd : 1;
} SERIAL_PORT_MSR_BITS;

//
//  Name:   SERIAL_PORT_MSR
//  Purpose:
//  Context:
//  Fields:
//      Bits    SERIAL_PORT_MSR_BITS:  Bits of the MSR
//      Data    UINT8: the value of the MSR
//
typedef union {
  SERIAL_PORT_MSR_BITS  Bits;
  UINT8                 Data;
} SERIAL_PORT_MSR;

#pragma pack()

//----------------------------------------------------------------------
//  Ethernet
//----------------------------------------------------------------------

//
//  Define the Ethernet registers
//  All the Ethernet registers are 32-bit
//

#define ETH_REG_INTERRUPT_STATUS            0x00  /// Interrupt status
#define ETH_REG_INTERRUPT_ENABLE            0x04  /// Interrupt enable
#define ETH_REG_MODE                        0x08  /// Mode of operation
#define ETH_REG_RESET                       0x0c  /// Reset
#define ETH_REG_TCP_IP_ACCELERATOR          0x10  /// TCP/IP accelerator control
#define ETH_REG_EXTERNAL_LIST               0x14  /// External list
#define ETH_REG_INTERRUPT_STATUS_HOLD       0x18  /// Interrupt status hold
#define ETH_REG_PHY_INTERRUPT_CONTROL       0x1c  /// PHY interrupt control
#define ETH_REG_MAC_RX_ENABLE               0x20  /// MAC RX enable
#define ETH_REG_RX_FLOW_CONTROL             0x24  /// RX flow control
#define ETH_REG_PAUSE_PACKET_REQUEST        0x28  /// Pause packet request
#define ETH_REG_RX_MODE                     0x2c  /// RX mode
#define ETH_REG_TX_MODE                     0x30  /// TX mode
#define ETH_REG_RX_FIFO_STATUS              0x34  /// RX FIFO status
#define ETH_REG_TX_FIFO_STATUS              0x38  /// TX FIFO status
#define ETH_REG_MAC_ADDRESS_1_A             0x60  /// MAC address 1
#define ETH_REG_MAC_ADDRESS_1_B             0x64
#define ETH_REG_MAC_ADDRESS_2_A             0x68  /// MAC address 2
#define ETH_REG_MAC_ADDRESS_2_B             0x6c
#define ETH_REG_MAC_ADDRESS_3_A             0x70  /// MAC address 3
#define ETH_REG_MAC_ADDRESS_3_B             0x74
#define ETH_REG_MAC_ADDRESS_4_A             0x78  /// MAC address 4
#define ETH_REG_MAC_ADDRESS_4_B             0x7c
#define ETH_REG_MAC_ADDRESS_5_A             0x80  /// MAC address 5
#define ETH_REG_MAC_ADDRESS_5_B             0x84
#define ETH_REG_MAC_ADDRESS_6_A             0x88  /// MAC address 6
#define ETH_REG_MAC_ADDRESS_6_B             0x8c
#define ETH_REG_MAC_ADDRESS_7_A             0x90  /// MAC address 7
#define ETH_REG_MAC_ADDRESS_7_B             0x94
#define ETH_REG_MAC_ADDRESS_8_A             0x98  /// MAC address 8
#define ETH_REG_MAC_ADDRESS_8_B             0x9c
#define ETH_REG_MAC_ADDRESS_9_A             0xa0  /// MAC address 9
#define ETH_REG_MAC_ADDRESS_9_B             0xa4
#define ETH_REG_MAC_ADDRESS_10_A            0xa8  /// MAC address 10
#define ETH_REG_MAC_ADDRESS_10_B            0xac
#define ETH_REG_MAC_ADDRESS_11_A            0xb0  /// MAC address 11
#define ETH_REG_MAC_ADDRESS_11_B            0xb4
#define ETH_REG_MAC_ADDRESS_12_A            0xb8  /// MAC address 12
#define ETH_REG_MAC_ADDRESS_12_B            0xbc
#define ETH_REG_MAC_ADDRESS_13_A            0xc0  /// MAC address 13
#define ETH_REG_MAC_ADDRESS_13_B            0xc4
#define ETH_REG_MAC_ADDRESS_14_A            0xc8  /// MAC address 14
#define ETH_REG_MAC_ADDRESS_14_B            0xcc
#define ETH_REG_MAC_ADDRESS_15_A            0xd0  /// MAC address 15
#define ETH_REG_MAC_ADDRESS_15_B            0xd4
#define ETH_REG_MAC_ADDRESS_16_A            0xd8  /// MAC address 16
#define ETH_REG_MAC_ADDRESS_16_B            0xdc
#define ETH_REG_ADDRESS_MASK                0xe0
#define ETH_REG_MIIM                        0xe4  /// PHY access control
#define ETH_REG_MAC_ADDRESS_1_LOAD          0xe8  /// MAC address1 load
#define ETH_REG_RGMII_STATUS                0xec  /// PHY status
#define ETH_REG_RGMII_CONTROL               0xf0  /// PHY access control
#define ETH_REG_DMA_CONTROL                 0x100 /// DMA control
#define ETH_REG_RX_DESCR_BASE_ADDRESS       0x110 /// RX descriptor base address
#define ETH_REG_RX_DESCR_SIZE               0x114 /// RX descriptor size
#define ETH_REG_RX_DESCR_HARD_POINTER       0x118 /// RX descriptor hard pointer
#define ETH_REG_RX_DESCR_HARD_POINTER_HOLD  0x11c /// RX descriptor hard pointer hold
#define ETH_REG_RX_DESCR_SOFT_POINTER       0x120 /// RX descriptor soft pointer
#define ETH_REG_TX_DESCR_BASE_ADDRESS       0x130 /// TX descriptor base address
#define ETH_REG_TX_DESCR_SIZE               0x134 /// TX descriptor size
#define ETH_REG_TX_DESCR_HARD_POINTER       0x138 /// TX descriptor hard pointer
#define ETH_REG_TX_DESCR_HARD_POINTER_HOLD  0x13c /// TX descriptor hard pointer hold
#define ETH_REG_TX_DESCR_SOFT_POINTER       0x140 /// TX descriptor soft pointer
#define ETH_REG_SOFT_RESET                  0x1fc /// Soft reset

#define ETH_MAX_ADDRESSES                   16

#define ETH_MAC_ADDRESS_LENGTH              6

//
//  0x00: Interrupt Status
//

#define ETH_INT_TCPIP_ERR       0x10000000  /// TCP/IP accelerator error
#define ETH_INT_WOL_DET         0x01000000  /// Wake-on-LAN event detection
#define ETH_INT_PHY_INT         0x00100000  /// PHY interrupt
#define ETH_INT_MIIM_CMPLT      0x00010000  /// MIIM operation completion
#define ETH_INT_PAUSE_CMPLT     0x00001000  /// Pause packet transmission complete
#define ETH_INT_TX_DMA_ERR      0x00000800  /// Transmit DMA error
#define ETH_INT_TX_FIFO_ERR     0x00000400  /// Transmit FIFO underflow
#define ETH_INT_TX_DMA_CMPLT    0x00000200  /// Transmit DMA complete
#define ETH_INT_TX_CMPLT        0x00000100  /// Transmit complete
#define ETH_INT_RX_DSC_EMP      0x00000020  /// Receive descriptor empty
#define ETH_INT_RX_DMA_ERR      0x00000010  /// Receive DMA error
#define ETH_INT_RX_FIFO_ERR     0x00000008  /// Receive FIFO overflow
#define ETH_INT_RX_FRAME_ERR    0x00000004  /// Receive frame error
#define ETH_INT_RX_VALID        0x00000002  /// Receive complete
#define ETH_INT_RX_DMA_CMPLT    0x00000001  /// Receive DMA complete

//
//  0x08: Mode
//

#define MODE_ETHER_MODE         0x80000000  /// 0:MII, 1:GMII/RGMII
#define MODE_DUPLEX_MODE        0x40000000  /// 0: Half, 1: Full
#define MODE_FR_BST             0x02000000  /// Frame bursting enabled

//
//  0x0c: ETH_REG_RESET
//

#define ETH_RESET_ALL_RST       0x80000000  /// Resest the entire MAC
#define ETH_RESET_TX_RST        0x00008000  /// Reset the transmitter
#define ETH_RESET_RX_RST        0x00004000  /// Reset the receiver

//
//  0x10: TCP/IP Accelerator Control
//

#define ACC_CTL_EX_LIST_EN      0x00000008  /// Enable external option list
#define ACC_CTL_RX_TCPIPACC_OFF 0x00000004  /// Disable the receive accelerator
#define ACC_CTL_TX_TCPIPACC_EN  0x00000002  /// Enable the transmit accelerator
#define ACC_CTL_RX_TCPIPACC_EN  0x00000001  /// Enable the receive accelerator

//
//  0x20: MAX RX Enable
//

#define MAC_RX_ENABLE           0x00000001  /// Enable the receive operation

//
//  0x24: RX Flow Control
//

#define RX_FLOW_CTL_FL_CTRL_EN  0x80000000  /// Pause packet RX/TX enabled

//
//  0x2c: RX Mode
//

#define RX_MODE_ADD_FIL_EN      0x80000000  /// Enable address filtering
#define RX_MODE_MLT_FIL_EN      0x40000000  /// Enable multicast filtering
#define RX_MODE_RH_ALM_EMP      0x0000c000  /// RX FIFO empty size
#define RX_MODE_RH_ALM_FULL     0x00003000  /// RX FIFO full size
#define RX_MODE_RH_RD_TRG       0x00000e00  /// RX FIFO read trigger size

#define RX_MODE_RH_ALM_EMP_4    0           /// 4 QWords
#define RX_MODE_RH_ALM_EMP_8    0x00004000  /// 8 QWords
#define RX_MODE_RH_ALM_EMP_16   0x00008000  /// 16 QWords
#define RX_MODE_RH_ALM_EMP_32   0x0000c000  /// 32 QWords

#define RX_MODE_RH_ALM_FULL_4   0           /// 4 QWords
#define RX_MODE_RH_ALM_FULL_8   0x00001000  /// 8 QWords
#define RX_MODE_RH_ALM_FULL_16  0x00002000  /// 16 QWords
#define RX_MODE_RH_ALM_FULL_32  0x00003000  /// 32 QWords

#define RX_MODE_RH_RD_TRG_4     0           /// 4 QWords
#define RX_MODE_RH_RD_TRG_8     0x00000200  /// 8 QWords
#define RX_MODE_RH_RD_TRG_16    0x00000400  /// 16 QWords
#define RX_MODE_RH_RD_TRG_32    0x00000600  /// 32 QWords
#define RX_MODE_RH_RD_TRG_64    0x00000800  /// 64 QWords
#define RX_MODE_RH_RD_TRG_128   0x00000a00  /// 128 QWords
#define RX_MODE_RH_RD_TRG_256   0x00000c00  /// 256 QWords
#define RX_MODE_RH_RD_TRG_512   0x00000e00  /// 512 QWords

//
//  0x30: TX Mode
//

#define TX_MODE_NO_RTRY         0x80000000  /// No resend at the time of collision
#define TX_MODE_LONG_PKT        0x40000000  /// Enable transmit of long frames
#define TX_MODE_ST_AND_FD       0x20000000  /// Transmit is started after writing
#define TX_MODE_SHORT_PKT       0x10000000  /// Enable transmit of short frames
#define TX_MODE_LTCOL_RETX      0x08000000  /// Resend in case of late collision
#define TX_MODE_TH_TX_STRT      0x0000c000  /// Start TX after N QWords
#define TX_MODE_TH_ALM_EMP      0x00003000  /// Interrupt when TX FIFO is almost empty
#define TX_MODE_TH_ALM_FULL     0x00000c00  /// Interrupt when TX FIFO is full

//
//  0xe0: Address Mask
//

#define ETH_ADDRESS_MASK_BUSY   0x80000000  /// MAC busy

//
//  0xe4: MIIM
//

#define MIIM_OPER             0x04000000
#define MIIM_PHY_ADDR         0x03e00000  /// PHY address
#define MIIM_REG_ADDR         0x001f0000  /// Register address
#define MIIM_DATA             0x0000ffff  /// Data read or written

#define MIIM_OPER_READ        0
#define MIIM_OPER_WRITE       MIIM_OPER

//
//  0xec: RGMII Status
//

#define RGMII_STS_LINK_UP     0x00000008  /// Link up
#define RGMII_STS_RXC_SPEED   0x00000006  /// RX clock speed
#define RGMII_STS_DUPLEX      0x00000001  /// Full duplex operation

#define RGMII_STS_RXC_SPEED_2_5_MHZ  0  /// 2.5 MHz
#define RGMII_STS_RXC_SPEED_25_MHZ   2  /// 25 MHz
#define RGMII_STS_RXC_SPEED_125_MHZ  4  /// 125 MHz

//
//  0xf0: RGMII Control
//
#define RGMII_CTL_CRS_SEL     0x00000010  /// Disable carrier sense
#define RGMII_CTL_RGMII_RATE  0x0000000c  /// Transmit clock rate
#define RGMII_CTL_RGMII_MODE  0x00000002  /// RGMII mode
#define RGMII_CTL_CHIP_TYPE   0x00000001  /// RGMII internal detection signal

#define RGMII_CTL_RGMII_RATE_125_MHZ   0  /// 125 MHz
#define RGMII_CTL_RGMII_RATE_25_MHZ    8  /// 25 MHz
#define RGMII_CTL_RGMII_RATE_2_5_MHZ 0xc  /// 2.5 MHz

///
/// 0x100: DMA Control
///

#define ETH_DMA_CONTROL_RX_DMA_EN 0x00000002  /// Enable receive DMA
#define ETH_DMA_CONTROL_TX_DMA_EN 0x00000001  /// Enable transmit DMA

///
/// 0x1fc:  Soft Reset
///

#define ETH_SOFT_RESET        0x00000001  /// Reset the MAC

///
/// Receive Descriptor
///
typedef struct {
  UINT32 RxFrameBufferAddress;
  UINT32 TcpIpAcceleratorStatus;
  UINT16 RxLength;
  UINT16 GmacStatus;
  UINT8 DmaStatus;
  UINT8 Reserved_0d [ 3 ];
} ETH_RECEIVE_DESCRIPTOR;

#define ETH_RECEIVE_BUFFER_SIZE   SIZE_16KB

///
/// Receive DMA Status
///

#define RX_DMA_STATUS_BUS_ERROR   0x01  /// Bus error during transfer

///
/// RX GMAC Status
///

#define RX_GMAC_STATUS_PAUSE      0x0200  /// Pause packet received
#define RX_GMAC_STATUS_MAR_BR     0x0100  /// Broadcast packet received
#define RX_GMAC_STATUS_MAR_MLT    0x0080  /// Multicast packet received
#define RX_GMAC_STATUS_MAR_IND    0x0040  /// Known address packet received
#define RX_GMAC_STATUS_MAR_NOTMT  0x0020  /// Packet not for this station
#define RX_GMAC_STATUS_TOO_LONG   0x0010  /// Longer than IEEE 802.3 max
#define RX_GMAC_STATUS_TOO_SHORT  0x0008  /// Shorter than IEEE 802.3 min
#define RX_GMAC_STATUS_NOT_OCTAL  0x0004  /// Not byte aligned
#define RX_GMAC_STATUS_NBL_ERR    0x0002  /// Encoding error during receive
#define RX_GMAC_STATUS_CRC_ERR    0x0001  /// CRC error

///
/// Convert RX length into bytes
///

#define ETH_RX_BYTES(ReceiveDescriptor)   ((INTN)( ReceiveDescriptor->RxLength - 3 ))

///
/// Transmit Descriptor
///
typedef struct {
  UINT32 TxFrameBufferAddress;
  UINT16 Length;
  UINT16 Reserved_06;
  UINT16 TxLength;
  UINT16 TxFrameControl;
  UINT8 DmaStatus;
  UINT8 Reserved_0e;
  UINT16 GmacStatus;
} ETH_TRANSMIT_DESCRIPTOR;

#define ETH_TRANSMIT_BUFFER_SIZE  SIZE_16KB

///
/// TX Frame Control
///

#define TX_FRAME_CONTROL_TCP_ACC_OFF  0x0008  /// Disable TCP/IP accelerator
#define TX_FRAME_CONTROL_ITAG         0x0004  /// Frame includes VLAN tag
#define TX_FRAME_CONTROL_ICRC         0x0002  /// Frame includes CRC
#define TX_FRAME_CONTROL_APAD         0x0001  /// Add frame padding

///
/// TX GMAC Status
///

#define TX_GMAC_STATUS_CMPLT          0x2000  /// Transmit complete
#define TX_GMAC_STATUS_ABT            0x1000  /// Transmit aborted
#define TX_GMAC_STATUS_EXCOL          0x0800  /// Excessive collisions
#define TX_GMAC_STATUS_SNGCOL         0x0400  /// Single collision
#define TX_GMAC_STATUS_MLTCOL         0x0200  /// Multiple collisions
#define TX_GMAC_STATUS_CRSER          0x0100  /// Carrier sense error
#define TX_GMAC_STATUS_TLNG           0x0080  /// Frame truncated at 1518 bytes
#define TX_GMAC_STATUS_TSHRT          0x0040  /// Frame not transmitted, too short
#define TX_GMAC_STATUS_LTCOL          0x0020  /// Late collision detected
#define TX_GMAC_STATUS_TFUNDFLW       0x0010  /// FIFO underflow
#define TX_GMAC_STATUS_RTYCNT         0x000f  /// Retry count

///
/// Convert TX length into bytes
///

#define ETH_TX_LENGTH(LengthInBytes)  ((UINT16)( LengthInBytes + 3 ))

//----------------------------------------------------------------------
//  GPIO support
//----------------------------------------------------------------------

/// Bitmask for valid GPIO ports
#define EG20T_GPIO_VALID_PORTS      0xfff

//
//  Define the GPIO registers
//  All GPIO registers are 32-bit
//

#define EG20T_REG_GPIO_IEN      0x00  /// RW Interrupt enable
#define EG20T_REG_GPIO_ISTATUS  0x04  /// RO Interrupt status
#define EG20T_REG_GPIO_IDISP    0x08  /// RO Interrupt source
#define EG20T_REG_GPIO_ICLR     0x0c  /// WO Interrupt clear
#define EG20T_REG_GPIO_IMASK    0x10  /// RW Interrupt mask
#define EG20T_REG_GPIO_IMASKCLR 0x14  /// WO Interrupt mask clear
#define EG20T_REG_GPIO_PO       0x18  /// RW Port output
#define EG20T_REG_GPIO_PI       0x1c  /// RO Port input
#define EG20T_REG_GPIO_PM       0x20  /// RW Port mode (1=output, 0=input)
#define EG20T_REG_GPIO_IM0      0x24  /// RW Interrupt mode 0
#define EG20T_REG_GPIO_IM1      0x28  /// RW Interrupt mode 1
#define EG20T_REG_GPIO_SRST     0x3c  /// RW Soft reset

#define EG20T_GPIO_DIRECTION_INPUT  0
#define EG20T_GPIO_DIRECTION_OUTPUT 1

//----------------------------------------------------------------------
//  I2C support
//----------------------------------------------------------------------

//
//  Define the I2C registers
//  All I2C registers are 16-bit
//

#define EG20T_REG_I2CSADR       0x00  /// Slave address register

#define I2CSADR_SLAVEADD            0x03ff

#define EG20T_REG_I2CCTL        0x04  /// Control register

#define I2CCTL_I2CCS                0x2000  //  Stop the I2C clock
#define I2CCTL_I2CDR_LDIE           0x1000  //  Enable/disable DR_LD interrupt
#define I2CCTL_I2CSTPIE             0x0800  //  Enable/disable STP interrupt
#define I2CCTL_I2CCFIE              0x0200  //  Enable/disable MCF interrupt
#define I2CCTL_I2CALIE              0x0100  //  Enable/disable MAL interrupt
#define I2CCTL_I2CMEN               0x0080  //  Enable/disable the I2C module
#define I2CCTL_I2CAASIE             0x0040  //  Enable/disable MAAS interrupt
#define I2CCTL_I2CMSTA              0x0020  //  Select MASTER/SLAVE operation
#define I2CCTL_I2CMTX               0x0010  //  Select transmit/receive
#define I2CCTL_I2CTXAK              0x0008  //  Transmit NACK/ACK during receive
#define I2CCTL_I2CRSTA              0x0004  //  Send repeated start bit
#define I2CCTL_I2CMD                0x0003  //  I2C operating mode
#define I2CCTL_I2CMD_STANDARD            0  //  Standard mode: 100 KHz
#define I2CCTL_I2CMD_FAST                1  //  Fast mode: 400 KHz

#define I2C_LOWEST_FAST_FREQUENCY   150000  //  Fast mode >= frequency in Hertz

#define EG20T_REG_I2CSR         0x08  /// Status register

#define I2CSR_I2CSTP                0x0200  //  Data reception is complete
#define I2CSR_I2CMCF                0x0080  //  Data transfer is complete
#define I2CSR_I2CMAAS               0x0040  //  Slave/Master mode
#define I2CSR_I2CMBB                0x0020  //  I2C bus busy/free
#define I2CSR_I2CMAL                0x0010  //  I2C bus arbitration lost
#define I2CSR_I2CDR_LD              0x0008  //  Load data register
#define I2CSR_I2CSRW                0x0004  //  Slave mode direction read/write
#define I2CSR_I2CMIF                0x0002  //  Interrupt request
#define I2CSR_I2CRXAK               0x0001  //  Received NACK/ACK status

#define EG20T_REG_I2CDR         0x0c  /// Data register

#define I2CDR_I2CDR                 0x00ff  //  Data bits

#define EG20T_REG_I2CMON        0x10  /// I2C bus level monitor register

#define I2CMON_MON_SDA              0x0002  //  SDA line level
#define I2CMON_MON_SCL              0x0001  //  SCL line level

#define EG20T_REG_I2CBC         0x14  /// I2C bus transfer rate counter

#define I2CBC_I2CBC                 0x00ff  //  Clock divider for I2C data rate

#define I2C_CLOCK_DIVIDER(Hertz)  ((UINT16)(( EG20T_CLKL + ( Hertz << 3 ) - 1 ) / ( Hertz << 3 )))
#define I2C_FREQUENCY(Divider)    ( EG20T_CLKL / ( Divider << 3 ))

//----------------------------------------------------------------------
//  SPI support
//----------------------------------------------------------------------

//
//  Define the SPI registers
//  All I2C registers are 32-bit
//

#define SPI_MAX_FREQUENCY     5000000     /// Must be <= 5 MHz

#define EG20T_REG_SPCR    0x00  /// SPI control register

#define SPCR_MOZ              0x08000000  /// Master output: 1=HI-Z, 0=Output
#define SPCR_SOZ              0x04000000  /// Slave output: 1=HI-Z, 0=Output
#define SPCR_SSZ              0x02000000  /// Slave select output: 1=HI-Z, 0=Output
#define SPCR_FICLR            0x01000000  /// FIFO clear
#define SPCR_RFIC             0x00f00000  /// N+1 RX frames generate interrupt
#define SPCR_TFIC             0x000f0000  /// N+1 free frames generate TX interrupt
#define SPCR_MDFIE            0x00001000  /// Enable mode fault interrupt
#define SPCR_ORIE             0x00000800  /// Enable verrun error interrupt
#define SPCR_FIE              0x00000400  /// Enable transfer end interrupt
#define SPCR_RFIE             0x00000200  /// Enable receive interrupt
#define SPCR_TFIE             0x00000100  /// Enable transmit interrupt
#define SPCR_CPOL             0x00000040  /// Clock polarity
#define SPCR_CPHA             0x00000020  /// Clock phase
#define SPCR_LSBF             0x00000010  /// 1=MSB first, 0=LSB first
#define SPCR_MODFEN           0x00000008  /// Enable mode fault
#define SPCR_MSTR             0x00000002  /// 1=Master, 0=Slave
#define SPCR_SPE              0x00000001  /// Enable SPI transfer

#define EG20T_REG_SPBRR   0x04  /// SPI baud rate register

#define SPBRR_DTL             0x01ff0000  /// SSN high interval between frames
#define SPBRR_LAG             0x0000c000  /// SCK to SSN delay interval
#define SPBRR_LEAD            0x00003000  /// SSN to SCK delay interval
#define SPBRR_SIZE            0x00000400  /// Transfer size 0=8-bits, 1=16-bits
#define SPBRR_SPBR            0x000003ff  /// Baudrate = CLKL / ( 2 * SPBR )

#define SPBRR_SIZE_8_BITS     0
#define SPBRR_SIZE_16_BITS    SPBRR_SIZE

#define SPBRR_DTL_NONE        0           /// Do not assert SSN between frames
#define SPBRR_LEAD_HALF_CLOCK 0x00004000  /// SSN leads by 1/2 clock
#define SPBRR_LAG_HALF_CLOCK  0x00001000  /// SSN lags by 1/2 clock

#define EG20T_REG_SPSR    0x08  /// SPI status register

#define SPSR_REF              0x00100000  /// Receive FIFO empty
#define SPSR_RFF              0x00080000  /// Receive FIFO full
#define SPSR_TFE              0x00040000  /// Transmit FIFO empty
#define SPSR_TFF              0x00020000  /// Transmit FIFO full
#define SPSR_WOF              0x00010000  /// Write overflow
#define SPSR_RFD              0x0000f800  /// Receive frames in RX FIFO
#define SPSR_TFD              0x000007C0  /// Transmit frames in TX FIFO
#define SPSR_SPIF             0x00000020  /// SPI transfer complete
#define SPSR_MDF              0x00000010  /// Mode fault
#define SPSR_ORF              0x00000008  /// Overrun error
#define SPSR_FI               0x00000004  /// Transfer complete
#define SPSR_RFI              0x00000002  /// Receive interrupt
#define SPSR_TFI              0x00000001  /// Transmit interrupt

#define EG20T_REG_SPDWR   0x0c  /// SPI write data register

#define SPDWR_VALID_DATA      0x0000ffff  /// Valid FIFO data

#define EG20T_REG_SPDRR   0x10  /// SPI read data register

#define SPDRR_VALID_DATA      0x0000ffff  /// Valid FIFO data

#define EG20T_REG_SSNXCR  0x18  /// SSN expanded control register

#define SSNXCR_SSNCEN         0x00000002  /// 1=SSN as GPIO, 0=SSN as SPI slave select
#define SSNXCR_SSNLVL         0x00000001  /// SSN GPIO output level

#define EG20T_REG_SRST    0x1c  /// Soft reset register

#define SRST_SRST             0x00000001  /// Reset the SPI controller

#define SPI_FIFO_DEPTH        16          /// Number of frames in the FIFO

#define SPI_CLOCK_DIVIDER(Hertz)  ((UINT16)(( EG20T_CLKL + ( Hertz << 1 ) - 1 ) / ( Hertz << 1 )))
#define SPI_FREQUENCY(Divider)    ( EG20T_CLKL / ( Divider << 1 ))

//----------------------------------------------------------------------
//  USB Device support
//----------------------------------------------------------------------

//
//  Define the USB device controller registers
//  All USB device registers are 32-bit
//
#define EG20T_REG_IN_ENDPOINT_0                       0
#define EG20T_REG_IN_ENDPOINT_1                    0x20
#define EG20T_REG_IN_ENDPOINT_2                    0x40
#define EG20T_REG_IN_ENDPOINT_3                    0x60

#define EG20T_REG_OUT_ENDPOINT_0                  0x200
#define EG20T_REG_OUT_ENDPOINT_1                  0x220
#define EG20T_REG_OUT_ENDPOINT_2                  0x240
#define EG20T_REG_OUT_ENDPOINT_3                  0x260

#define EG20T_REG_USBDEV_DEVICE_CONFIGURATION     0x400
#define EG20T_REG_USBDEV_DEVICE_CONTROL           0x404
#define EG20T_REG_USBDEV_DEVICE_STATUS            0x408
#define EG20T_REG_USBDEV_DEVICE_INTERRUPT         0x40c
#define EG20T_REG_USBDEV_DEVICE_INTERRUPT_MASK    0x410
#define EG20T_REG_USBDEV_ENDPOINT_INTERRUPT       0x414
#define EG20T_REG_USBDEV_ENDPOINT_INTERRUPT_MASK  0x418
#define EG20T_REG_USBDEV_SRST                     0x4fc

//
//  0: Endpoint Control
//

#define ENDPOINT_CONTROL                         0

#define ENDPOINT_CONTROL_MRX_FLUSH      0x00001000  /// Receive FIFO flush
#define ENDPOINT_CONTROL_CLOSE_DESC     0x00000800  /// Close descriptor channel for endpoint
#define ENDPOINT_CONTROL_SEND_NULL      0x00000400  /// Send zero-length packet
#define ENDPOINT_CONTROL_RRDY           0x00000200  /// Receive ready
#define ENDPOINT_CONTROL_CNAK           0x00000100  /// Clear NAK
#define ENDPOINT_CONTROL_SNAK           0x00000080  /// Set NAK
#define ENDPOINT_CONTROL_NAK            0x00000040  /// NAK condition detected
#define ENDPOINT_CONTROL_ET             0x00000030  /// Endpoint type
#define ENDPOINT_CONTROL_POLLDEMAND     0x00000008  /// Poll the endpoint
#define ENDPOINT_CONTROL_SN             0x00000004  /// Snoop mode
#define ENDPOINT_CONTROL_F              0x00000002  /// Flush the TxFIFO
#define ENDPOINT_CONTROL_S              0x00000001  /// Stall handshake

#define ENDPOINT_CONTROL_ET_CONTROL     0x00000000  /// Control endpoint
#define ENDPOINT_CONTROL_ET_ISOCHRONOUS 0x00000010  /// Isochronous endpoint
#define ENDPOINT_CONTROL_ET_BULK        0x00000020  /// Bulk endpoint
#define ENDPOINT_CONTROL_ET_INTERRUPT   0x00000030  /// Interrupt endpoint

//
//  4: Endpoint Status
//

#define ENDPOINT_STATUS                          4

#define ENDPOINT_STATUS_CDC             0x10000000  /// CDC clear
#define ENDPOINT_STATUS_XFERDONE        0x08000000  /// Transfer done, Transmit FIFO empty
#define ENDPOINT_STATUS_RSS             0x04000000  /// Received stall set
#define ENDPOINT_STATUS_RCS             0x02000000  /// Received clear stall
#define ENDPOINT_STATUS_TXEMPTY         0x01000000  /// Transmit FIFO empty
#define ENDPOINT_STATUS_ISO_INDONE      0x00800000  /// ISO IN done
#define ENDPOINT_STATUS_RXPKT_SIZE      0x007ff800  /// Receive packet size
#define ENDPOINT_STATUS_TDC             0x00000400  /// Transmit DMA complete
#define ENDPOINT_STATUS_HE              0x00000200  /// Host error
#define ENDPOINT_STATUS_MRXFIFO_EMPTY   0x00000100  /// Receive address FIFO empty
#define ENDPOINT_STATUS_BNA             0x00000080  /// Buffer not available
#define ENDPOINT_STATUS_IN              0x00000040  /// IN token received
#define ENDPOINT_STATUS_OUT             0x00000030  /// OUT packet received

#define ENDPOINT_STATUS_OUT_DATA        0x00000010  /// OUT packet received
#define ENDPOINT_STATUS_OUT_SETUP       0x00000020  /// SETUP packet received

//
//  8: Buffer Size/Frame Number
//

#define ENDPOINT_BSIRFN                          8

#define ENDPOINT_BSFN_ISO_PID           0x00030000  /// ISO IN/OUT PID
#define ENDPOINT_BSFN_BUFF_SIZE         0x0000ffff  /// Buffer size in 32-bit entries
#define ENDPOINT_BSFN_FRAME_NUMBER      0x0000ffff  /// Frame number

//
//  0x0c: Endpoint Buffer Size OUT/Maximum Packet Size
//

#define ENDPOINT_BSOMPS                       0x0c

#define ENDPOINT_BSOMPS_BUFF_SIZE       0xffff0000  /// Buffer size in 32-bit entries
#define ENDPOINT_BS0MPS_MAXPKT_SIZE     0x0000ffff  /// Maximum packet size in bytes

//
//  0x10: Endpoint SETUP buffer pointer
//

#define ENDPOINT_SETUP_BUFFER                 0x10

//
//  0x14: Endpoint Data Descriptor Pointer
//

#define ENDPOINT_DATA_DESCRIPTOR_POINTER      0x14

//
//  0x400: Device Configuration
//

#define USBDEV_CONFIG_LPM_EN            0x00200000  /// Link power mode enable
#define USBDEV_CONFIG_LPM_AUTO          0x00100000  /// Link power mode automatic
#define USBDEV_CONFIG_DDR               0x00080000  /// Double data rate
#define USBDEV_CONFIG_SET_DESC          0x00040000  /// Set descriptor is supported
#define USBDEV_CONFIG_CSR_PRG           0x00020000  /// Set configuration is supported
#define USBDEV_CONFIG_HALT_STATUS       0x00010000  /// Halt status, Stall if clear feature received
#define USBDEV_CONFIG_HS_TIMEOUT_CALIB  0x0000e000  /// PHY clocks for high-speed timeout
#define USBDEV_CONFIG_FS_TIMEOUT_CALIB  0x00001c00  /// PHY clocks for full-speed timeout
#define USBDEV_CONFIG_PHY_ERROR_DETECT  0x00000200  /// PHY error detect
#define USBDEV_CONFIG_STATUS_1          0x00000100  ///
#define USBDEV_CONFIG_STATUS            0x00000080  ///
#define USBDEV_CONFIG_DIR               0x00000040  /// PHY transfer direction
#define USBDEV_CONFIG_PI                0x00000020  /// PHY interface size: 0 = 16-bit, 1 = 8-bit
#define USBDEV_CONFIG_SS                0x00000010  /// Device supports sync frames
#define USBDEV_CONFIG_SP                0x00000008  /// Device is self powered
#define USBDEV_CONFIG_RWKP              0x00000004  /// Remove wake-up support
#define USBDEV_CONFIG_SPD               0x00000003  /// Device speed

#define USBDEV_CONFIG_SPD_HIGH                   0  /// Use high-speed
#define USBDEV_CONFIG_SPD_FULL                   1  /// Use full speed with 30/60 MHz clock
#define USBDEV_CONFIG_SPD_LOW                    2  /// Use low speed
#define USBDEV_CONFIG_SPD_FULL_48_MHZ            3  /// Use full speed with 48 MHz clock

//
//  0x404: Device Control
//

#define USBDEV_CONTROL_THLEN        0xff000000  /// Threshold length
#define USBDEV_CONTROL_BRLEN        0x00ff0000  /// Burst length
#define USBDEV_CONTROL_CSR_DONE     0x00002000  /// ACK set configuration/interface
#define USBDEV_CONTROL_DEVNAK       0x00001000  /// NAK all OUT endpoints
#define USBDEV_CONTROL_SCALE        0x00000800  /// Fast reset detect
#define USBDEV_CONTROL_SD           0x00000400  /// Soft disconnect
#define USBDEV_CONTROL_DMA_MODE     0x00000200  /// DMA mode
#define USBDEV_CONTROL_BREN         0x00000100  /// Enable burst operations
#define USBDEV_CONTROL_THE          0x00000080  /// Threshold enable
#define USBDEV_CONTROL_BF           0x00000040  /// Buffer fill
#define USBDEV_CONTROL_BE           0x00000020  /// Big endian support
#define USBDEV_CONTROL_DU           0x00000010  /// Descriptor update
#define USBDEV_CONTROL_TDE          0x00000008  /// Transmit DMA enable
#define USBDEV_CONTROL_RDE          0x00000004  /// Receive DMA enable
#define USBDEV_CONTROL_RES          0x00000001  /// Resume signalling

//
//  0x408: Device Status
//

#define USBDEV_STATUS_TS            0xfffc0000  /// Frame number of SOF
#define USBDEV_STATUS_RMTWKP_STATE  0x00020000  /// Remove wake-up
#define USBDEV_STATUS_PHY_ERROR     0x00010000  /// PHY error
#define USBDEV_STATUS_RXFIFO_EMPTY  0x00008000  /// RX FIFO empty
#define USBDEV_STATUS_ENUMSPD       0x00006000  /// Enumerated speed
#define USBDEV_STATUS_SUSP          0x00001000  /// Suspend detected
#define USBDEV_STATUS_ALT           0x00000f00  /// Alternate setting
#define USBDEV_STATUS_INTF          0x000000f0  /// Interface setting
#define USBDEV_STATUS_CFG           0x0000000f  /// Configuration setting

//
//  0x40c: Device Interrupt Status
//

#define USBDEV_DEVINT_RMTWKP_STATE  0x00000080  /// Remote wake-up received
#define USBDEV_DEVINT_ENUM          0x00000040  /// Speed enumeration complete
#define USBDEV_DEVINT_SOF           0x00000020  /// Start of frame detected
#define USBDEV_DEVINT_US            0x00000010  /// Suspended state detectd
#define USBDEV_DEVINT_UR            0x00000008  /// USB reset detected
#define USBDEV_DEVINT_ES            0x00000004  /// Idle state detected
#define USBDEV_DEVINT_SI            0x00000002  /// Set interface command received
#define USBDEV_DEVINT_SC            0x00000001  /// Set configuration command received

//
//  0x414:  Endpoint Interrupt Status
//

#define USBDEV_EPTINT_OUT           0xffff0000  /// Output endpoint interrupts
#define USBDEV_EPTINT_IN            0x0000ffff  /// Input endpoint interrupts

//
//  0x4fc: Soft Reset
//

#define USBDEV_SRST_PSRST           0x00000002  /// PHY soft reset
#define USBDEV_SRST_SRST            0x00000001  /// Soft reset

///
/// Receive descriptor
///
typedef struct {
  UINT32 Status;                /// Operation status
  UINT32 Reserved;
  UINT32 BufferPointer;         /// Physical address of the data buffer
  UINT32 NextDescriptor;        /// Physical address of next descriptor
} ENDPOINT_RECEIVE_DESCRIPTOR;

#define RXDESC_STS_BUFFER_STATUS    0xc0000000  /// Buffer status
#define RXDESC_STS_RX_STATUS        0x30000000  /// Receive status
#define RXDESC_STS_LAST             0x08000000  /// Last descriptor
#define RXDESC_STS_FRAME_NUMBER     0x07ff0000  /// Receive frame number
#define RXDESC_STS_RX_BYTES         0x0000ffff  /// Non-ISO frame bytes received
#define RXDESC_STS_ISO_PID          0x0000c000  /// Packet ID value
#define RXDESC_STS_ISO_RX_BYTES     0x00003fff  /// ISO frame bytes received

#define RXDESC_BUFSTS_HOST_READY    0           /// Host ready
#define RXDESC_BUFSTS_DMA_BUSY      0x40000000  /// DMA transfer in progress
#define RXDESC_BUFSTS_DMA_DONE      0x80000000  /// DMA done
#define RXDESC_BUFSTS_HOST_BUSY     0xc0000000  /// Host busy

#define RXDESC_RXSTS_SUCCESS        0
#define RXDESC_RXSTS_DESERR         0x10000000
#define RXDESC_RXSTS_RESERVED       0x20000000
#define RXDESC_RXSTS_BUFERR         0x30000000

///
/// Transmit descriptor
///
typedef struct {
  UINT32 Status;                /// Operation status
  UINT32 Reserved;
  UINT32 BufferPointer;         /// Physical address of the data buffer
  UINT32 NextDescriptor;        /// Physical address of next descriptor
} ENDPOINT_TRANSMIT_DESCRIPTOR;

#define TXDESC_STS_BUFFER_STATUS    0xc0000000  /// Buffer status
#define TXDESC_STS_TX_STATUS        0x30000000  /// Transmit status
#define TXDESC_STS_LAST             0x08000000  /// Last descriptor
#define TXDESC_STS_FRAME_NUMBER     0x07ff0000  /// Transmit by frame number
#define TXDESC_STS_TX_BYTES         0x0000ffff  /// Non-ISO frame bytes to transmit
#define TXDESC_STS_ISO_PID          0x0000c000  /// Number of packets per frame
#define TXDESC_STS_ISO_TX_BYTES     0x00003fff  /// ISO frame bytes to transmit

#define TXDESC_BUFSTS_HOST_READY    0           /// Host ready
#define TXDESC_BUFSTS_DMA_BUSY      0x40000000  /// DMA transfer in progress
#define TXDESC_BUFSTS_DMA_DONE      0x80000000  /// DMA done
#define TXDESC_BUFSTS_HOST_BUSY     0xc0000000  /// Host busy

#define TXDESC_TXSTS_SUCCESS        0
#define TXDESC_TXSTS_DESERR         0x10000000
#define TXDESC_TXSTS_RESERVED       0x20000000
#define TXDESC_TXSTS_BUFERR         0x30000000

///
/// Setup descriptor for Endpoint 0
///
typedef struct {
  UINT32 Status;                    /// Request status
  UINT32 Reserved;                  ///
  USB_DEVICE_REQUEST SetupMessage;  /// Setup message data
} ENDPOINT_SETUP_DESCRIPTOR;

#define SETUP_STATUS_BUFFER_STATUS  0xc0000000  /// Buffer status
#define SETUP_STATUS_RECEIVE_STATUS 0x03000000  /// Receive status
#define SETUP_STATUS_CONFIG_STATUS  0x0fff0000  /// Configuration status

#define SETUP_BUFSTS_HOST_READY     0           /// Host ready
#define SETUP_BUFSTS_DMA_BUSY       0x40000000  /// DMA transfer in progress
#define SETUP_BUFSTS_DMA_DONE       0x80000000  /// DMA done
#define SETUP_BUFSTS_HOST_BUSY      0xc0000000  /// Host busy

#define SETUP_RXSTS_SUCCESS         0
#define SETUP_RXSTS_DESERR          0x10000000
#define SETUP_RXSTS_RESERVED        0x20000000
#define SETUP_RXSTS_BUFERR          0x30000000

#define SETUP_CFGSTS_CONFIG_NUMBER  0x0f000000  /// Selected configuration
#define SETUP_CFGSTS_INTERFACE_NUM  0x00f00000  /// Selected interface
#define SETUP_CFGSTS_ALTERNATE_NUM  0x000f0000  /// Selected alternate setting

///
/// Maximum packet size for Endpoint 0
///
#define ENDPOINT_0_MAX_PACKET_SIZE    64

///
/// Maximum packet size for the other Endpoints
#define ENDPOINT_N_MAX_PACKET_SIZE    512

#endif  //  _EG20T_H_

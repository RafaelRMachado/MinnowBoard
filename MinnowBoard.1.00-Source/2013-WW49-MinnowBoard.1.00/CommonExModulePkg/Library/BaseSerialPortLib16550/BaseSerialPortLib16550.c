/** @file
  16550 UART Serial Port library functions

  Copyright (c) 2006 - 2013, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Base.h>
#include <IndustryStandard/Pci.h>
#include <Library/SerialPortLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Library/PciLib.h>
#include <Library/PlatformHookLib.h>
#include <Library/BaseLib.h>

//
// PCI Defintions.  Change to use PCI_BRIDGE_CONTROL_REGISTER structure
//
#define PCI_BRIDGE_IO_BASE_REGISTER_OFFSET      0x1c
#define PCI_BRIDGE_IO_LIMIT_REGISTER_OFFSET     0x1d
#define PCI_BRIDGE_MEMORY_BASE_REGISTER_OFFSET  0x20
#define PCI_BRIDGE_MEMORY_LIMIT_REGISTER_OFFSET 0x22
#define PCI_BRIDGE_IO_BASE_U16_REGISTER_OFFSET  0x30
#define PCI_BRIDGE_IO_LIMIT_U16_REGISTER_OFFSET 0x32

#define PCI_BRIDGE_32_BIT_IO_SPACE              0x01


//
// 16550 UART register offsets and bitfields
//
#define R_UART_RXBUF          0
#define R_UART_TXBUF          0
#define R_UART_BAUD_LOW       0
#define R_UART_BAUD_HIGH      1
#define R_UART_FCR            2
#define   B_UART_FCR_FIFOE    BIT0
#define   B_UART_FCR_FIFO64   BIT5
#define R_UART_LCR            3
#define   B_UART_LCR_DLAB     BIT7
#define R_UART_MCR            4
#define   B_UART_MCR_RTS      BIT1
#define R_UART_LSR            5
#define   B_UART_LSR_RXRDY    BIT0
#define   B_UART_LSR_TXRDY    BIT5
#define   B_UART_LSR_TEMT     BIT6
#define R_UART_MSR            6
#define   B_UART_MSR_CTS      BIT4
#define   B_UART_MSR_DSR      BIT5

typedef struct {
  UINT8 Device;
  UINT8 Function;
} PCI_UART_DEVICE_INFO;

/**
  Update the value of an 16-bit PCI configuration register in a PCI device.  If the  
  PCI Configuration register specified by PciAddress is already programmed with a 
  non-zero value, then return the current value.  Otherwise update the PCI configuration 
  register specified by PciAddress with the value specified by Value and return the
  value programmed into the PCI configuration register.  All values must be masked 
  using the bitmask specified by Mask.

  @param  PciAddress  PCI Library address of the PCI Configuration register to update.
  @param  Value       The value to program into the PCI Configuration Register.
  @param  Mask        Bitmask of the bits to check and update in the PCI configuration register.

**/
UINT16
SerialPortLibUpdatePciRegister16 (
  UINTN   PciAddress,
  UINT16  Value,
  UINT16  Mask
  )
{
  UINT16  CurrentValue;
  
  CurrentValue = PciRead16 (PciAddress) & Mask;
  if (CurrentValue != 0) {
    return CurrentValue;
  }
  return PciWrite16 (PciAddress, Value & Mask);
}

/**
  Update the value of an 32-bit PCI configuration register in a PCI device.  If the  
  PCI Configuration register specified by PciAddress is already programmed with a 
  non-zero value, then return the current value.  Otherwise update the PCI configuration 
  register specified by PciAddress with the value specified by Value and return the
  value programmed into the PCI configuration register.  All values must be masked 
  using the bitmask specified by Mask.

  @param  PciAddress  PCI Library address of the PCI Configuration register to update.
  @param  Value       The value to program into the PCI Configuration Register.
  @param  Mask        Bitmask of the bits to check and update in the PCI configuration register.

  @return  The Secondary bus number that is actually programed into the PCI to PCI Bridge device.

**/
UINT32
SerialPortLibUpdatePciRegister32 (
  UINTN   PciAddress,
  UINT32  Value,
  UINT32  Mask
  )
{
  UINT32  CurrentValue;
  
  CurrentValue = PciRead32 (PciAddress) & Mask;
  if (CurrentValue != 0) {
    return CurrentValue;
  }
  return PciWrite32 (PciAddress, Value & Mask);
}

/**
  Retrieve the I/O or MMIO base address register for the PCI UART device.  This 
  function enables the PCI to PCI Bridge in the 

  If the 
  base address register id not already programmed, then update the 

  @return  The base address register of the PCI UART device.

**/
UINTN
GetSerialRegisterBase (
  VOID
  )
{
  UINTN                 PciLibAddress;
  UINTN                 PrimaryBusNumber;
  UINTN                 BusNumber;
  UINTN                 SubordinateBusNumber;
  UINT32                IoBase;
  UINT32                IoLimit;
  UINT16                MemoryBase;
  UINT16                MemoryLimit;
  UINTN                 SerialRegisterBase;
  UINTN                 BarIndex;
  UINT32                RegisterBaseMask;
  PCI_UART_DEVICE_INFO  *DeviceInfo;

  //
  // Get PCI Device Info
  //
  DeviceInfo = (PCI_UART_DEVICE_INFO *) PcdGetPtr (PcdSerialPciDeviceInfo);
  
  //
  // If PCI Device Info is empty, then assume fixed address UART and return PcdSerialRegisterBase
  //  
  if (DeviceInfo->Device == 0xff) {
    return (UINTN)PcdGet64 (PcdSerialRegisterBase);
  }
  
  //
  // Enable I/O and MMIO in PCI Bridge
  // Assume Root Bus Numer is Zero. 
  //
  for (BusNumber = 0; (DeviceInfo + 1)->Device != 0xff; DeviceInfo++) {
    //
    // Compute PCI Lib Address to PCI to PCI Bridge
    //
    PciLibAddress = PCI_LIB_ADDRESS (BusNumber, DeviceInfo->Device, DeviceInfo->Function, 0);
    
    //
    // Retrieve and verify the bus numbers in the PCI to PCI Bridge
    //
    PrimaryBusNumber     = PciRead8 (PciLibAddress + PCI_BRIDGE_PRIMARY_BUS_REGISTER_OFFSET);
    BusNumber            = PciRead8 (PciLibAddress + PCI_BRIDGE_SECONDARY_BUS_REGISTER_OFFSET);
    SubordinateBusNumber = PciRead8 (PciLibAddress + PCI_BRIDGE_SUBORDINATE_BUS_REGISTER_OFFSET);
    if (BusNumber == 0 || BusNumber > SubordinateBusNumber) {
      return 0;
    }

    //
    // Retrieve and verify the I/O or MMIO decode window in the PCI to PCI Bridge
    //
    if (PcdGetBool (PcdSerialUseMmio)) {
      MemoryLimit = PciRead16 (PciLibAddress + PCI_BRIDGE_MEMORY_LIMIT_REGISTER_OFFSET) & 0xfff0;
      MemoryBase  = PciRead16 (PciLibAddress + PCI_BRIDGE_MEMORY_BASE_REGISTER_OFFSET)  & 0xfff0;
      if (MemoryLimit < MemoryBase) {
        return 0;
      }
    } else {
      IoLimit = PciRead8 (PciLibAddress + PCI_BRIDGE_IO_LIMIT_REGISTER_OFFSET);
      if ((IoLimit & PCI_BRIDGE_32_BIT_IO_SPACE ) == 0) {
        IoLimit = IoLimit >> 4;
      } else {
        IoLimit = (PciRead16 (PciLibAddress + PCI_BRIDGE_IO_LIMIT_U16_REGISTER_OFFSET) << 4) | (IoLimit >> 4);
      }
      IoBase = PciRead8 (PciLibAddress + PCI_BRIDGE_IO_BASE_REGISTER_OFFSET);
      if ((IoBase & PCI_BRIDGE_32_BIT_IO_SPACE ) == 0) {
        IoBase = IoBase >> 4;
      } else {
        IoBase = (PciRead16 (PciLibAddress + PCI_BRIDGE_IO_BASE_U16_REGISTER_OFFSET) << 4) | (IoBase >> 4);
      }
      if (IoLimit < IoBase) {
        return 0;
      }
    }
  }

  //
  // Compute PCI Lib Address to PCI UART
  //
  PciLibAddress = PCI_LIB_ADDRESS (BusNumber, DeviceInfo->Device, DeviceInfo->Function, 0);
  
  //
  // Find the first IO or MMIO BAR
  //
  RegisterBaseMask = 0xFFFFFFF0;
  for (BarIndex = 0; BarIndex < PCI_MAX_BAR; BarIndex ++) {
    SerialRegisterBase = PciRead32 (PciLibAddress + PCI_BASE_ADDRESSREG_OFFSET + BarIndex * 4);
    if (PcdGetBool (PcdSerialUseMmio) && ((SerialRegisterBase & BIT0) == 0)) {
      //
      // MMIO BAR is found
      //
      RegisterBaseMask = 0xFFFFFFF0;
      break;
    }

    if ((!PcdGetBool (PcdSerialUseMmio)) && ((SerialRegisterBase & BIT0) != 0)) {
      //
      // IO BAR is found
      //
      RegisterBaseMask = 0xFFFFFFF8;
      break;
    }
  }

  //
  // MMIO or IO BAR is not found.
  //
  if (BarIndex == PCI_MAX_BAR) {
    return 0;
  }

  //
  // Program UART BAR
  //  
  SerialRegisterBase = SerialPortLibUpdatePciRegister32 (
                         PciLibAddress + PCI_BASE_ADDRESSREG_OFFSET + BarIndex * 4,
                         (UINT32)PcdGet64 (PcdSerialRegisterBase), 
                         RegisterBaseMask
                         );
  
  //
  // Enable I/O and MMIO in PCI UART Device if they are not already enabled
  //
  SerialPortLibUpdatePciRegister16 (
    PciLibAddress + PCI_COMMAND_OFFSET,
    PcdGetBool (PcdSerialUseMmio) ? EFI_PCI_COMMAND_MEMORY_SPACE : EFI_PCI_COMMAND_IO_SPACE,
    PcdGetBool (PcdSerialUseMmio) ? EFI_PCI_COMMAND_MEMORY_SPACE : EFI_PCI_COMMAND_IO_SPACE
    );

  //
  // Get PCI Device Info
  //
  DeviceInfo = (PCI_UART_DEVICE_INFO *) PcdGetPtr (PcdSerialPciDeviceInfo);

  //
  // Enable I/O or MMIO in PCI Bridge
  // Assume Root Bus Numer is Zero. 
  //
  for (BusNumber = 0; (DeviceInfo + 1)->Device != 0xff; DeviceInfo++) {
    //
    // Compute PCI Lib Address to PCI to PCI Bridge
    //
    PciLibAddress = PCI_LIB_ADDRESS (BusNumber, DeviceInfo->Device, DeviceInfo->Function, 0);
    
    //
    // Enable the I/O or MMIO decode windows in the PCI to PCI Bridge
    //
    SerialPortLibUpdatePciRegister16 (
      PciLibAddress + PCI_COMMAND_OFFSET, 
      PcdGetBool (PcdSerialUseMmio) ? EFI_PCI_COMMAND_MEMORY_SPACE : EFI_PCI_COMMAND_IO_SPACE,
      PcdGetBool (PcdSerialUseMmio) ? EFI_PCI_COMMAND_MEMORY_SPACE : EFI_PCI_COMMAND_IO_SPACE
      );
      
    BusNumber = PciRead8 (PciLibAddress + PCI_BRIDGE_SECONDARY_BUS_REGISTER_OFFSET);
  }
    
  return SerialRegisterBase;
}

/**
  Read an 8-bit 16550 register.  If PcdSerialUseMmio is TRUE, then the value is read from 
  MMIO space.  If PcdSerialUseMmio is FALSE, then the value is read from I/O space.  The
  parameter Offset is added to the base address of the 16550 registers that is specified 
  by PcdSerialRegisterBase. 
  
  @param  Offset  The offset of the 16550 register to read.

  @return The value read from the 16550 register.

**/
UINT8
SerialPortReadRegister (
  UINTN  Base,
  UINTN  Offset
  )
{
  if (PcdGetBool (PcdSerialUseMmio)) {
    return MmioRead8 (Base + Offset);
  } else {
    return IoRead8 (Base + Offset);
  }
}

/**
  Write an 8-bit 16550 register.  If PcdSerialUseMmio is TRUE, then the value is written to
  MMIO space.  If PcdSerialUseMmio is FALSE, then the value is written to I/O space.  The
  parameter Offset is added to the base address of the 16550 registers that is specified 
  by PcdSerialRegisterBase. 
  
  @param  Offset  The offset of the 16550 register to write.
  @param  Value   The value to write to the 16550 register specified by Offset.

  @return The value written to the 16550 register.

**/
UINT8
SerialPortWriteRegister (
  UINTN  Base,
  UINTN  Offset,
  UINT8  Value
  )
{
  if (PcdGetBool (PcdSerialUseMmio)) {
    return MmioWrite8 (Base + Offset, Value);
  } else {
    return IoWrite8 (Base + Offset, Value);
  }
}

/**
  Return whether the hardware flow control signal allows writing.

  @retval TRUE  The serial port is writable.
  @retval FALSE The serial port is not writable.
**/
BOOLEAN
SerialPortWritable (
  UINTN  SerialRegisterBase
  )
{
  if (PcdGetBool (PcdSerialUseHardwareFlowControl)) {
    if (PcdGetBool (PcdSerialDetectCable)) {
      //
      // Wait for both DSR and CTS to be set
      //   DSR is set if a cable is connected.
      //   CTS is set if it is ok to transmit data
      //
      //   DSR  CTS  Description                               Action
      //   ===  ===  ========================================  ========
      //    0    0   No cable connected.                       Wait
      //    0    1   No cable connected.                       Wait
      //    1    0   Cable connected, but not clear to send.   Wait
      //    1    1   Cable connected, and clear to send.       Transmit
      //
      return (BOOLEAN) ((SerialPortReadRegister (SerialRegisterBase, R_UART_MSR) & (B_UART_MSR_DSR | B_UART_MSR_CTS)) == (B_UART_MSR_DSR | B_UART_MSR_CTS));
    } else {
      //
      // Wait for both DSR and CTS to be set OR for DSR to be clear.  
      //   DSR is set if a cable is connected.
      //   CTS is set if it is ok to transmit data
      //
      //   DSR  CTS  Description                               Action
      //   ===  ===  ========================================  ========
      //    0    0   No cable connected.                       Transmit
      //    0    1   No cable connected.                       Transmit
      //    1    0   Cable connected, but not clear to send.   Wait
      //    1    1   Cable connected, and clar to send.        Transmit
      //
      return (BOOLEAN) ((SerialPortReadRegister (SerialRegisterBase, R_UART_MSR) & (B_UART_MSR_DSR | B_UART_MSR_CTS)) != (B_UART_MSR_DSR));
    }
  }

  return TRUE;
}

/**
  Initialize the serial device hardware.
  
  If no initialization is required, then return RETURN_SUCCESS.
  If the serial device was successfully initialized, then return RETURN_SUCCESS.
  If the serial device could not be initialized, then return RETURN_DEVICE_ERROR.
  
  @retval RETURN_SUCCESS        The serial device was initialized.
  @retval RETURN_DEVICE_ERROR   The serial device could not be initialized.

**/
RETURN_STATUS
EFIAPI
SerialPortInitialize (
  VOID
  )
{
  RETURN_STATUS  Status;
  UINTN          SerialRegisterBase;
  UINT32         Divisor;
  UINT32         CurrentDivisor;  
  BOOLEAN        Initialized;

  //
  // Perform platform specific initialization required to enable use of the 16550 device
  // at the location specified by PcdSerialUseMmio and PcdSerialRegisterBase.
  //
  Status = PlatformHookSerialPortInitialize ();
  if (RETURN_ERROR (Status)) {
    return Status;
  }

  //
  // Calculate divisor for baud generator
  //    Ref_Clk_Rate / Baud_Rate / 16
  //
  Divisor = PcdGet32 (PcdSerialClockRate) / (PcdGet32 (PcdSerialBaudRate) * 16);
  if ((PcdGet32 (PcdSerialClockRate) % (PcdGet32 (PcdSerialBaudRate) * 16)) >= PcdGet32 (PcdSerialBaudRate) * 8) {
    Divisor++;
  }

  //
  // Get the base address of the serial port in either I/O or MMIO space
  //
  SerialRegisterBase = GetSerialRegisterBase ();
  if (SerialRegisterBase ==0) {
    return RETURN_DEVICE_ERROR;
  }

  //
  // Get the base address of the serial port in either I/O or MMIO space
  //
  SerialRegisterBase = GetSerialRegisterBase ();
  if (SerialRegisterBase ==0) {
    return RETURN_DEVICE_ERROR;
  }

  //
  // See if the serial port is already initialized
  //
  Initialized = TRUE;
  if ((SerialPortReadRegister (SerialRegisterBase, R_UART_FCR) & (B_UART_FCR_FIFOE | B_UART_FCR_FIFO64)) !=
      (PcdGet8 (PcdSerialFifoControl)      & (B_UART_FCR_FIFOE | B_UART_FCR_FIFO64))     ) {
    Initialized = FALSE;
  }
  if ((SerialPortReadRegister (SerialRegisterBase, R_UART_LCR) & 0x3F) != (PcdGet8 (PcdSerialLineControl) & 0x3F)) {
    Initialized = FALSE;
  }
  SerialPortWriteRegister (SerialRegisterBase, R_UART_LCR, (UINT8)(SerialPortReadRegister (SerialRegisterBase, R_UART_LCR) | B_UART_LCR_DLAB));
  CurrentDivisor =  SerialPortReadRegister (SerialRegisterBase, R_UART_BAUD_HIGH) << 8;
  CurrentDivisor |= SerialPortReadRegister (SerialRegisterBase, R_UART_BAUD_LOW);
  SerialPortWriteRegister (SerialRegisterBase, R_UART_LCR, (UINT8)(SerialPortReadRegister (SerialRegisterBase, R_UART_LCR) & ~B_UART_LCR_DLAB));
  if (CurrentDivisor != Divisor) {
    Initialized = FALSE;
  }
  if (Initialized) {
    return RETURN_SUCCESS;
  }

  //
  // Wait for the serial port to be ready.
  // Verify that both the transmit FIFO and the shift register are empty.
  //
  while ((SerialPortReadRegister (SerialRegisterBase, R_UART_LSR) & B_UART_LSR_TEMT) == 0);
  
  //
  // Configure baud rate
  //
  SerialPortWriteRegister (SerialRegisterBase, R_UART_LCR, B_UART_LCR_DLAB);
  SerialPortWriteRegister (SerialRegisterBase, R_UART_BAUD_HIGH, (UINT8) (Divisor >> 8));
  SerialPortWriteRegister (SerialRegisterBase, R_UART_BAUD_LOW, (UINT8) (Divisor & 0xff));

  //
  // Clear DLAB and configure Data Bits, Parity, and Stop Bits.
  // Strip reserved bits from PcdSerialLineControl
  //
  SerialPortWriteRegister (SerialRegisterBase, R_UART_LCR, (UINT8)(PcdGet8 (PcdSerialLineControl) & 0x3F));

  //
  // Enable and reset FIFOs
  // Strip reserved bits from PcdSerialFifoControl
  //
  SerialPortWriteRegister (SerialRegisterBase, R_UART_FCR, (UINT8)(PcdGet8 (PcdSerialFifoControl) & 0x27));

  //
  // Put Modem Control Register(MCR) into its reset state of 0x00.
  //  
  SerialPortWriteRegister (SerialRegisterBase, R_UART_MCR, 0x00);

  return RETURN_SUCCESS;
}

/**
  Write data from buffer to serial device. 

  Writes NumberOfBytes data bytes from Buffer to the serial device.  
  The number of bytes actually written to the serial device is returned.
  If the return value is less than NumberOfBytes, then the write operation failed.

  If Buffer is NULL, then ASSERT(). 

  If NumberOfBytes is zero, then return 0.

  @param  Buffer           Pointer to the data buffer to be written.
  @param  NumberOfBytes    Number of bytes to written to the serial device.

  @retval 0                NumberOfBytes is 0.
  @retval >0               The number of bytes written to the serial device.  
                           If this value is less than NumberOfBytes, then the read operation failed.

**/
UINTN
EFIAPI
SerialPortWrite (
  IN UINT8     *Buffer,
  IN UINTN     NumberOfBytes
  )
{
  UINTN  SerialRegisterBase;
  UINTN  Result;
  UINTN  Index;
  UINTN  FifoSize;

  if (Buffer == NULL) {
    return 0;
  }

  SerialRegisterBase = GetSerialRegisterBase ();
  if (SerialRegisterBase ==0) {
    return 0;
  }
  
  if (NumberOfBytes == 0) {
    //
    // Flush the hardware
    //

    //
    // Wait for both the transmit FIFO and shift register empty.
    //
    while ((SerialPortReadRegister (SerialRegisterBase, R_UART_LSR) & B_UART_LSR_TEMT) == 0);

    //
    // Wait for the hardware flow control signal
    //
    while (!SerialPortWritable (SerialRegisterBase));
    return 0;
  }

  //
  // Compute the maximum size of the Tx FIFO
  //
  FifoSize = 1;
  if ((PcdGet8 (PcdSerialFifoControl) & B_UART_FCR_FIFOE) != 0) {
    if ((PcdGet8 (PcdSerialFifoControl) & B_UART_FCR_FIFO64) == 0) {
      FifoSize = 16;
    } else {
      FifoSize = PcdGet32 (PcdSerialExtendedTxFifoSize);
    }
  }

  Result = NumberOfBytes;
  while (NumberOfBytes != 0) {
    //
    // Wait for the serial port to be ready, to make sure both the transmit FIFO
    // and shift register empty.
    //
    while ((SerialPortReadRegister (SerialRegisterBase, R_UART_LSR) & B_UART_LSR_TEMT) == 0);

    //
    // Fill then entire Tx FIFO
    //
    for (Index = 0; Index < FifoSize && NumberOfBytes != 0; Index++, NumberOfBytes--, Buffer++) {
      //
      // Wait for the hardware flow control signal
      //
      while (!SerialPortWritable (SerialRegisterBase));

      //
      // Write byte to the transmit buffer.
      //
      SerialPortWriteRegister (SerialRegisterBase, R_UART_TXBUF, *Buffer);
    }
  }
  return Result;
}

/**
  Reads data from a serial device into a buffer.

  @param  Buffer           Pointer to the data buffer to store the data read from the serial device.
  @param  NumberOfBytes    Number of bytes to read from the serial device.

  @retval 0                NumberOfBytes is 0.
  @retval >0               The number of bytes read from the serial device.  
                           If this value is less than NumberOfBytes, then the read operation failed.

**/
UINTN
EFIAPI
SerialPortRead (
  OUT UINT8     *Buffer,
  IN  UINTN     NumberOfBytes
  )
{
  UINTN  SerialRegisterBase;
  UINTN  Result;
  UINT8  Mcr;

  if (NULL == Buffer) {
    return 0;
  }

  SerialRegisterBase = GetSerialRegisterBase ();
  if (SerialRegisterBase ==0) {
    return 0;
  }

  Mcr = (UINT8)(SerialPortReadRegister (SerialRegisterBase, R_UART_MCR) & ~B_UART_MCR_RTS);
  
  for (Result = 0; NumberOfBytes-- != 0; Result++, Buffer++) {
    //
    // Wait for the serial port to have some data.
    //
    while ((SerialPortReadRegister (SerialRegisterBase, R_UART_LSR) & B_UART_LSR_RXRDY) == 0) {
      if (PcdGetBool (PcdSerialUseHardwareFlowControl)) {
        //
        // Set RTS to let the peer send some data
        //
        SerialPortWriteRegister (SerialRegisterBase, R_UART_MCR, (UINT8)(Mcr | B_UART_MCR_RTS));
      }
    }
    if (PcdGetBool (PcdSerialUseHardwareFlowControl)) {
      //
      // Clear RTS to prevent peer from sending data
      //
      SerialPortWriteRegister (SerialRegisterBase, R_UART_MCR, Mcr);
    }
    
    //
    // Read byte from the receive buffer.
    //
    *Buffer = SerialPortReadRegister (SerialRegisterBase, R_UART_RXBUF);
  }
  
  return Result;
}


/**
  Polls a serial device to see if there is any data waiting to be read.

  Polls aserial device to see if there is any data waiting to be read.
  If there is data waiting to be read from the serial device, then TRUE is returned.
  If there is no data waiting to be read from the serial device, then FALSE is returned.

  @retval TRUE             Data is waiting to be read from the serial device.
  @retval FALSE            There is no data waiting to be read from the serial device.

**/
BOOLEAN
EFIAPI
SerialPortPoll (
  VOID
  )
{
  UINTN  SerialRegisterBase;
  
  SerialRegisterBase = GetSerialRegisterBase ();
  if (SerialRegisterBase ==0) {
    return FALSE;
  }

  //
  // Read the serial port status
  //
  if ((SerialPortReadRegister (SerialRegisterBase, R_UART_LSR) & B_UART_LSR_RXRDY) != 0) {
    if (PcdGetBool (PcdSerialUseHardwareFlowControl)) {
      //
      // Clear RTS to prevent peer from sending data
      //
      SerialPortWriteRegister (SerialRegisterBase, R_UART_MCR, (UINT8)(SerialPortReadRegister (SerialRegisterBase, R_UART_MCR) & ~B_UART_MCR_RTS));
    }
    return TRUE;
  }    
  
  if (PcdGetBool (PcdSerialUseHardwareFlowControl)) {
    //
    // Set RTS to let the peer send some data
    //
    SerialPortWriteRegister (SerialRegisterBase, R_UART_MCR, (UINT8)(SerialPortReadRegister (SerialRegisterBase, R_UART_MCR) | B_UART_MCR_RTS));
  }
  
  return FALSE;
}

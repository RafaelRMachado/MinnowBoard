/** @file
  Smbus over I2C Driver
  
  Copyright (c) 2012, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "I2cPort.h"


/**

Routine Description:

  This function provides a standard way to execute a Data Read Command
  Protocol as defined in the SMBus Specification. The data can either be of
  the Length byte, word, or a block of data. The resulting transaction will be
  either the SMBus Slave Device accepts this transaction or this function
  returns with an error

  @param This         Address of an EFI_SMBUS_HC_PROTOCOL structure
  @param SlaveAddress Smbus Slave device the command is directed at
  @param HostCommand  Slave Device dependent
  @param ReadCommand  Which SMBus read protocol will be used
  @param PecCheck     Defines if Packet Error Code Checking is to be used
  @param Length       How many bytes to read. Must be 1 <= Length <= 32 depending on
                      ReadCommand
  @param Buffer       Address of a buffer containing the write data.  The
                      buffer will also be used to return the read data.

  @returns    This routine returns the operation status.
              Also Length will contain the actual number of bytes read
              and Buffer will contain the data read.

**/
EFI_STATUS
SmbusExecute (
  IN CONST EFI_SMBUS_HC_PROTOCOL        *This,
  IN       EFI_SMBUS_DEVICE_ADDRESS     SlaveAddress,
  IN       EFI_SMBUS_DEVICE_COMMAND     Command,
  IN       EFI_SMBUS_OPERATION          Operation,
  IN       BOOLEAN                      PecCheck,
  IN OUT  UINTN                         *Length,
  IN OUT  VOID                          *Buffer
  )
{
  UINTN RetryCount;
  UINTN SmBusAddress;
  EFI_STATUS Status;

  //
  //  Merge the length, command and PEC with the address to match the
  //  SMBus library implementation
  //
  SmBusAddress = SlaveAddress.SmbusDeviceAddress;
  SmBusAddress |= ( Command & 0xff ) << 8;
  if ( NULL != Length ) {
    SmBusAddress |= ( *Length & 0x3f ) << 16;
  }
  if ( PecCheck ) {
    SmBusAddress |= BIT22;
  }

  //
  //  Retry the SMBus operation as necessary
  //
  for ( RetryCount = 0; SMBUS_RETRY_MAX > RetryCount; RetryCount++ ) {

    //
    //  Perform the I2C operation
    //
    switch (Operation) {

    case EfiSmbusQuickWrite:
      Status = EFI_UNSUPPORTED;
      if ( !PecCheck ) {
        SmBusOnI2cQuickWrite ( This,
                               SmBusAddress,
                               &Status );
      }
      break;

    case EfiSmbusQuickRead:
      Status = EFI_UNSUPPORTED;
      if ( !PecCheck ) {
        SmBusOnI2cQuickRead ( This,
                              SmBusAddress,
                              &Status );
      }
      break;

    case EfiSmbusSendByte:
      HostCmdReg = CallBuffer[0];
      SlvAddrReg--;
      //
      // The "break;" command is not present here to allow code execution
      // do drop into the next case, which contains common code to this case.
      //

    case EfiSmbusReceiveByte:
      Status = EFI_BUFFER_TOO_SMALL;
      if ( 1 > *Length ) {
        *(UINT8 *)Buffer = SmBusOnI2cReceiveByte ( This,
                                                   
      SmbusOperation = SMBUS_V_SMB_CMD_BYTE;
      if (*Length < 1) {
        Status = EFI_BUFFER_TOO_SMALL;
      }
      *Length = 1;
      break;

    case EfiSmbusWriteByte:
      Private->SmbusIoWrite (Private, SMBUS_R_HD0, CallBuffer[0]);
      SlvAddrReg--;
      //
      // The "break;" command is not present here to allow code execution
      // do drop into the next case, which contains common code to this case.
      //

    case EfiSmbusReadByte:
      SmbusOperation = SMBUS_V_SMB_CMD_BYTE_DATA;
      if (*Length < 1) {
        Status = EFI_BUFFER_TOO_SMALL;
      }
      *Length = 1;
      break;

    case EfiSmbusReadWord:
      SmbusOperation = SMBUS_V_SMB_CMD_WORD_DATA;
      if (*Length < 2) {
        Status = EFI_BUFFER_TOO_SMALL;
      }
      *Length = 2;
      break;

    case EfiSmbusWriteWord:
      SmbusOperation = SMBUS_V_SMB_CMD_WORD_DATA;
      SlvAddrReg--;
      Private->SmbusIoWrite (Private, SMBUS_R_HD1, CallBuffer[1]);
      Private->SmbusIoWrite (Private, SMBUS_R_HD0, CallBuffer[0]);
      if (*Length < 2) {
        Status = EFI_BUFFER_TOO_SMALL;
      }
      *Length = 2;
      break;

    case EfiSmbusWriteBlock:
      Private->SmbusIoWrite (Private, SMBUS_R_HD0, *(UINT8 *) Length);
      SlvAddrReg--;
      BlockCount = (UINT8) (*Length);
      //
      // The "break;" command is not present here to allow code execution
      // do drop into the next case, which contains common code to this case.
      //

    case EfiSmbusReadBlock:
      SmbusOperation = SMBUS_V_SMB_CMD_BLOCK;
      if ((*Length < 1) || (*Length > 32)) {
        Status = EFI_INVALID_PARAMETER;
        break;
      }
      AuxcReg |= SMBUS_B_E32B;
      break;

    case EfiSmbusProcessCall:
      SmbusOperation = SMBUS_V_SMB_CMD_PROCESS_CALL;
      Private->SmbusIoWrite (Private, SMBUS_R_HD1, CallBuffer[1]);
      Private->SmbusIoWrite (Private, SMBUS_R_HD0, CallBuffer[0]);
      if (*Length < 2) {
        Status = EFI_BUFFER_TOO_SMALL;
      }
      *Length = 2;
      break;

    case EfiSmbusBWBRProcessCall:
      Status = EFI_UNSUPPORTED;
      break;

    default:
      Status = EFI_INVALID_PARAMETER;
      break;
    };

    if (EFI_ERROR(Status)) {
      break;
    }

    if (PecCheck == TRUE) {
      AuxcReg |= SMBUS_B_AAC;
    }

    //
    // Set Auxiliary Control register
    //
    Private->SmbusIoWrite (Private, SMBUS_R_AUXC, AuxcReg);

    //
    // Reset the pointer of the internal buffer
    //
    Private->SmbusIoRead (Private, SMBUS_R_HCTL);

    //
    // Now that the 32 byte buffer is turned on, we can write th block data
    // into it
    //
    if (Operation == EfiSmbusWriteBlock) {
      for (Index = 0; Index < BlockCount; Index++) {
        //
        // Write next byte
        //
        Private->SmbusIoWrite (Private, SMBUS_R_HBD, CallBuffer[Index]);
      }
    }

    //
    // Set SMBus slave address for the device to send/receive from
    //
    Private->SmbusIoWrite (Private, SMBUS_R_TSA, SlvAddrReg);

    //
    // Set Command register
    //
    Private->SmbusIoWrite (Private, SMBUS_R_HCMD, HostCmdReg);

    //
    // Set Control Register (Initiate Operation, Interrupt disabled)
    //
    Private->SmbusIoWrite (Private, SMBUS_R_HCTL,
                           (UINT8) (SmbusOperation + SMBUS_B_START));

    // Wait for IO to complete
    if (!(Private->IoDone (Private, &StsReg))) {
      Status = EFI_TIMEOUT;
      break;
    } else if (StsReg & SMBUS_B_DERR) {
      AuxStsReg = Private->SmbusIoRead (Private, SMBUS_R_AUXS);
      if (AuxStsReg & SMBUS_B_CRCE) {
        Status = EFI_CRC_ERROR;
      } else {
        Status = EFI_DEVICE_ERROR;
      }
      break;
    } else if (StsReg & SMBUS_B_BERR) {
      // Clear the Bus Error for another try
      Status = EFI_DEVICE_ERROR;
      Private->SmbusIoWrite (Private, SMBUS_R_HSTS, SMBUS_B_BERR);
      continue;
    }

    //
    // successfull completion
    // Operation Specifics (post-execution)
    //
    switch (Operation) {

    case EfiSmbusReadWord:
      //
      // The "break;" command is not present here to allow code execution
      // do drop into the next case, which contains common code to this case.
      //

    case EfiSmbusProcessCall:
      CallBuffer[1] = Private->SmbusIoRead (Private, SMBUS_R_HD1);
      //
      // The "break;" command is not present here to allow code execution
      // do drop into the next case, which contains common code to this case.
      //

    case EfiSmbusReadByte:
      CallBuffer[0] = Private->SmbusIoRead (Private, SMBUS_R_HD0);
      break;

    case EfiSmbusWriteBlock:
      Private->SmbusIoWrite (Private, SMBUS_R_HSTS, SMBUS_B_BYTE_DONE_STS);
      break;

    case EfiSmbusReadBlock:
      BufferTooSmall = FALSE;
      // Find out how many bytes will be in the block
      BlockCount = Private->SmbusIoRead (Private, SMBUS_R_HD0);
      if (*Length < BlockCount) {
        BufferTooSmall = TRUE;
      } else {
        for (Index = 0; Index < BlockCount; Index++) {
          //
          // Read the byte
          //
          CallBuffer[Index] = Private->SmbusIoRead (Private, SMBUS_R_HBD);
        }
      }

      *Length = BlockCount;
      if (BufferTooSmall) {
        Status = EFI_BUFFER_TOO_SMALL;
      }
      break;

    default:
      break;
    };

    if ((StsReg & SMBUS_B_BERR) && (Status != EFI_BUFFER_TOO_SMALL)) {
      // Clear the Bus Error for another try
      Status = EFI_DEVICE_ERROR;
      Private->SmbusIoWrite (Private, SMBUS_R_HSTS, SMBUS_B_BERR);
      continue;
    } else {
      break;
    }
  }


  InitializePrivate();

  return SmbusExec (
           mSmbusContext,
           SlaveAddress,
           Command,
           Operation,
           PecCheck,
           Length,
           Buffer
           );
}


/**
  Start the I2C port driver

  This routine allocates the necessary resources for the driver.

  This routine is called by I2cPortDriverStart to complete the driver
  initialization.

  @param[in] I2cPort          Address of an I2C_PORT_CONTEXT structure

  @retval EFI_SUCCESS         Driver API properly initialized
  
**/
EFI_STATUS
I2cPortApiStart (
  IN I2C_PORT_CONTEXT *I2cPort
  );

/**
  Stop the I2C driver

  This routine releases the resources allocated by I2cApiStart.

  This routine is called by I2cPortDriverStop to initiate the driver
  shutdown.

  @param[in] I2cPort          Address of an I2C_PORT_CONTEXT structure

**/
VOID
I2cPortApiStop (
  IN I2C_PORT_CONTEXT *I2cPort
  );

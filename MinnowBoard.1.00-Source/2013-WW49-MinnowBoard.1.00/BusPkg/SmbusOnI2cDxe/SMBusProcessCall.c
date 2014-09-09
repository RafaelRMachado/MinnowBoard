/** @file
  Implement the SMBus process call operations layered on the I2C port driver.
  
  Copyright (c) 2012, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <SmbusOnI2c.h>

/**
  Executes an SMBUS block process call command.

  Executes an SMBUS block process call command on the SMBUS device specified by SmBusAddress.
  The SMBUS slave address, SMBUS command, and SMBUS length fields of SmBusAddress are required.
  Bytes are written to the SMBUS from WriteBuffer.  Bytes are then read from the SMBUS into ReadBuffer.
  If Status is not NULL, then the status of the executed command is returned in Status.
  It is the caller's responsibility to make sure ReadBuffer is large enough for the total number of bytes read.
  SMBUS supports a maximum transfer size of 32 bytes, so Buffer does not need to be any larger than 32 bytes.
  If Length in SmBusAddress is zero or greater than 32, then ASSERT().
  If WriteBuffer is NULL, then ASSERT().
  If ReadBuffer is NULL, then ASSERT().
  If any reserved bits of SmBusAddress are set, then ASSERT().

  @param  This          Address of an EFI_SMBUS_PROTOCOL structure.
  @param  SmBusAddress  Address that encodes the SMBUS Slave Address,
                        SMBUS Command, SMBUS Data Length, and PEC.
  @param  WriteBuffer   Pointer to the buffer of bytes to write to the SMBUS.
  @param  ReadBuffer    Pointer to the buffer of bytes to read from the SMBUS.
  @param  Status        Return status for the executed command.
                        This is an optional parameter and may be NULL.
                        RETURN_TIMEOUT A timeout occurred while executing the SMBUS command.
                        RETURN_DEVICE_ERROR  The request was not completed because a failure
                        reflected in the Host Status Register bit.  Device errors are a result
                        of a transaction collision, illegal command field, unclaimed cycle
                        (host initiated), or bus errors (collisions).
                        RETURN_CRC_ERROR  The checksum is not correct (PEC is incorrect)
                        RETURN_UNSUPPORTED  The SMBus operation is not supported.

  @return The number of bytes written.

**/
UINTN
EFIAPI
SmbusOnI2cBlockProcessCall (
  IN  EFI_SMBUS_HC_PROTOCOL *This,
  IN  UINTN SmBusAddress,
  IN  VOID *WriteBuffer,
  OUT VOID *ReadBuffer,
  OUT RETURN_STATUS *Status OPTIONAL
  )
{
  UINT8 Crc;
  UINT8 DataReceived [ 1 + 32 + 1 ];
  UINT8 DataTransmit [ 1 + 1 + 32 ];
  EFI_I2C_MASTER_PROTOCOL * I2cMaster;
  RETURN_STATUS I2cStatus;
  UINTN Index;
  UINTN Length;
  UINTN SlaveAddress;
  SMBUS_CONTEXT * SMBus;
  SMBUS_TRANSACTION SmbusTransaction;
  BOOLEAN UsingPec;

  //
  //  Validate the parameters
  //
  ASSERT ( NULL != WriteBuffer );
  ASSERT ( NULL != ReadBuffer );
  ASSERT ( 1 <= SMBUS_LIB_LENGTH ( SmBusAddress ));
  ASSERT ( 32 >= SMBUS_LIB_LENGTH ( SmBusAddress ));
  ASSERT ( 0 == SMBUS_LIB_RESERVED ( SmBusAddress ));

  //
  //  Get the context
  //
  SMBus = SMBUS_CONTEXT_FROM_PROTOCOL ( This );

  //
  //  Get the slave address
  //
  SlaveAddress = SMBUS_LIB_SLAVE_ADDRESS ( SmBusAddress );

  //
  //  Get the buffer length
  //
  Length = SMBUS_LIB_LENGTH ( SmBusAddress );

  //
  //  Build the request
  //
  SmbusTransaction.Request.Timeout = 0;
  SmbusTransaction.Request.OperationCount = 2;
  SmbusTransaction.Request.Operation [ 0 ].Flags = I2C_FLAG_SMBUS_OPERATION
                                                 | I2C_FLAG_SMBUS_BLOCK
                                                 | I2C_FLAG_SMBUS_PROCESS_CALL;
  SmbusTransaction.Request.Operation [ 0 ].Buffer = &DataTransmit [ 0 ];
  SmbusTransaction.Request.Operation [ 0 ].LengthInBytes = (UINT32)( Length + 2 );
  SmbusTransaction.Request.Operation [ 0 ].Flags = I2C_FLAG_READ;
  SmbusTransaction.Request.Operation [ 1 ].Buffer = &DataReceived [ 0 ];
  SmbusTransaction.Request.Operation [ 1 ].LengthInBytes = 1 + 32;  //  Maximum number of bytes

  //
  //  Get the data byte
  //
  DataTransmit [ 0 ] = (UINT8)SMBUS_LIB_COMMAND ( SmBusAddress );
  DataTransmit [ 1 ] = (UINT8)Length;
  CopyMem ( &DataTransmit [ 2 ], WriteBuffer, Length );

  //
  //  Determine if PEC is in use
  //
  UsingPec = SMBUS_LIB_PEC ( SmBusAddress );
  if ( UsingPec ) {
    SmbusTransaction.Request.Operation [ 0 ].Flags |= I2C_FLAG_SMBUS_PEC;
    SmbusTransaction.Request.Operation [ 1 ].LengthInBytes += 1;
  }

  //
  //  Initiate a synchronous request.
  //
  I2cMaster = SMBus->I2cMaster;
  I2cStatus = I2cMaster->StartRequest ( I2cMaster,
                                        SlaveAddress,
                                        NULL,
                                        &SmbusTransaction.Request,
                                        NULL );

  //
  //  Get the receive length
  //
  Length = SmbusTransaction.Request.Operation [ 1 ].LengthInBytes;

  //
  //  Validate the PEC if necessary
  //
  if ( UsingPec && ( !EFI_ERROR ( I2cStatus ))) {
    //
    //  Compute the PEC value
    //
    Crc = SMBUS_PEC_READ_INIT;
    Crc = Crc8 ( Crc, (UINT8)(( SlaveAddress << 1 ) | 0 ));
    for ( Index = 0; 1 + Length + 1 > Index; Index++ ) {
      Crc = Crc8 ( Crc, DataReceived [ Index ]);
    }
    if ( SMBUS_PEC_VALID != Crc ) {
      //
      //  CRC error detected
      //
      I2cStatus = RETURN_CRC_ERROR;
    }
    else
    {
      //
      //  Copy the bytes into the receive buffer
      //
      CopyMem ( ReadBuffer, &DataReceived [ 1 ], Length );
    }
  }

  //
  //  Return the operation status
  //
  if ( NULL != Status ) {
    *Status = I2cStatus;
  }

  //
  //  Return the data value
  //
  return Length;
}


/**
  Executes an SMBUS process call command.

  Executes an SMBUS process call command on the SMBUS device specified by SmBusAddress.
  The 16-bit value specified by Value is written.
  Only the SMBUS slave address and SMBUS command fields of SmBusAddress are required.
  The 16-bit value returned by the process call command is returned.
  If Status is not NULL, then the status of the executed command is returned in Status.
  If Length in SmBusAddress is not zero, then ASSERT().
  If any reserved bits of SmBusAddress are set, then ASSERT().

  @param  This          Address of an EFI_SMBUS_PROTOCOL structure.
  @param  SmBusAddress  Address that encodes the SMBUS Slave Address,
                        SMBUS Command, SMBUS Data Length, and PEC.
  @param  Value         The 16-bit value to write.
  @param  Status        Return status for the executed command.
                        This is an optional parameter and may be NULL.
                        RETURN_SUCCESS The SMBUS command was executed.
                        RETURN_TIMEOUT A timeout occurred while executing the SMBUS command.
                        RETURN_DEVICE_ERROR  The request was not completed because a failure
                        reflected in the Host Status Register bit.  Device errors are a result
                        of a transaction collision, illegal command field, unclaimed cycle
                        (host initiated), or bus errors (collisions).
                        RETURN_CRC_ERROR  The checksum is not correct (PEC is incorrect)
                        RETURN_UNSUPPORTED  The SMBus operation is not supported.

  @return The 16-bit value returned by the process call command.

**/
UINT16
EFIAPI
SmbusOnI2cProcessCall (
  IN  EFI_SMBUS_HC_PROTOCOL *This,
  IN  UINTN SmBusAddress,
  IN  UINT16 Value,
  OUT RETURN_STATUS *Status OPTIONAL
  )
{
  UINT8 Crc;
  union {
    UINT8 U8 [ 3 ];
    UINT16 U16;
  } DataReceived;
  UINT8 DataTransmit [ 3 ];
  EFI_I2C_MASTER_PROTOCOL * I2cMaster;
  RETURN_STATUS I2cStatus;
  UINTN SlaveAddress;
  SMBUS_CONTEXT * SMBus;
  SMBUS_TRANSACTION SmbusTransaction;
  BOOLEAN UsingPec;

  //
  //  Validate the parameters
  //
  ASSERT ( 0 == SMBUS_LIB_LENGTH ( SmBusAddress ));
  ASSERT ( 0 == SMBUS_LIB_RESERVED ( SmBusAddress ));

  //
  //  Get the context
  //
  SMBus = SMBUS_CONTEXT_FROM_PROTOCOL ( This );

  //
  //  Get the slave address
  //
  SlaveAddress = SMBUS_LIB_SLAVE_ADDRESS ( SmBusAddress );

  //
  //  Get the data byte
  //
  DataTransmit [ 0 ] = (UINT8)SMBUS_LIB_COMMAND ( SmBusAddress );
  DataTransmit [ 1 ] = (UINT8)Value;
  DataTransmit [ 2 ] = (UINT8)( Value >> 8 );

  //
  //  Build the request
  //
  SmbusTransaction.Request.Timeout = 0;
  SmbusTransaction.Request.OperationCount = 2;
  SmbusTransaction.Request.Operation [ 0 ].Flags = I2C_FLAG_SMBUS_OPERATION
                                                 | I2C_FLAG_SMBUS_PROCESS_CALL;
  SmbusTransaction.Request.Operation [ 0 ].Buffer = &DataTransmit [ 0 ];
  SmbusTransaction.Request.Operation [ 0 ].LengthInBytes = 3;
  SmbusTransaction.Request.Operation [ 1 ].Flags = I2C_FLAG_READ;
  SmbusTransaction.Request.Operation [ 1 ].Buffer = &DataReceived.U8 [ 0 ];
  SmbusTransaction.Request.Operation [ 1 ].LengthInBytes = 2;

  //
  //  Determine if PEC is in use
  //
  UsingPec = SMBUS_LIB_PEC ( SmBusAddress );
  if ( UsingPec ) {
    SmbusTransaction.Request.Operation [ 0 ].Flags |= I2C_FLAG_SMBUS_PEC;
    SmbusTransaction.Request.Operation [ 1 ].LengthInBytes += 1;
  }

  //
  //  Initiate a synchronous request.
  //
  I2cMaster = SMBus->I2cMaster;
  I2cStatus = I2cMaster->StartRequest ( I2cMaster,
                                        SlaveAddress,
                                        NULL,
                                        &SmbusTransaction.Request,
                                        NULL );

  //
  //  Validate the packet error code if necessary
  //
  if ( UsingPec && ( !EFI_ERROR ( I2cStatus )))  {
    //
    //  Validate the PEC value
    //
    Crc = SMBUS_PEC_READ_INIT;
    Crc = Crc8 ( Crc, (UINT8)(( SlaveAddress << 1 ) | 1 ));
    Crc = Crc8 ( Crc, DataReceived.U8 [ 0 ]);
    Crc = Crc8 ( Crc, DataReceived.U8 [ 1 ]);
    Crc = Crc8 ( Crc, DataReceived.U8 [ 2 ]);

    //
    //  CRC error detected
    //
    if ( SMBUS_PEC_VALID != Crc ) {
      I2cStatus = RETURN_CRC_ERROR;
    }
  }

  //
  //  Return the operation status
  //
  if ( NULL != Status ) {
    *Status = I2cStatus;
  }

  //
  //  Return the data value
  //
  return DataReceived.U16;
}

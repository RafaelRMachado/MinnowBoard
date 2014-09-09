/** @file
  Implement the SMBus write operations layered on the I2C port driver.
  
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
  Executes an SMBUS quick write command.

  Executes an SMBUS quick write command on the SMBUS device specified by SmBusAddress.
  Only the SMBUS slave address field of SmBusAddress is required.
  If Status is not NULL, then the status of the executed command is returned in Status.
  If PEC is set in SmBusAddress, then ASSERT().
  If Command in SmBusAddress is not zero, then ASSERT().
  If Length in SmBusAddress is not zero, then ASSERT().
  If any reserved bits of SmBusAddress are set, then ASSERT().

  @param  This          Address of an EFI_SMBUS_PROTOCOL structure.
  @param  SmBusAddress  Address that encodes the SMBUS Slave Address,
                        SMBUS Command, SMBUS Data Length, and PEC.
  @param  Status        Return status for the executed command.
                        This is an optional parameter and may be NULL.
                        RETURN_SUCCESS The SMBUS command was executed.
                        RETURN_TIMEOUT A timeout occurred while executing the SMBUS command.
                        RETURN_DEVICE_ERROR  The request was not completed because a failure
                        reflected in the Host Status Register bit.  Device errors are a result
                        of a transaction collision, illegal command field, unclaimed cycle
                        (host initiated), or bus errors (collisions).
                        RETURN_UNSUPPORTED  The SMBus operation is not supported.

**/
VOID
EFIAPI
SmBusOnI2cQuickWrite (
  IN  EFI_SMBUS_HC_PROTOCOL *This,
  IN  UINTN SmBusAddress,
  OUT RETURN_STATUS *Status OPTIONAL
  )
{
  EFI_I2C_REQUEST_PACKET I2cRequest;
  EFI_I2C_MASTER_PROTOCOL * I2cMaster;
  RETURN_STATUS I2cStatus;
  UINTN SlaveAddress;
  SMBUS_CONTEXT * SMBus;

  //
  //  Validate the parameters
  //
  ASSERT ( !SMBUS_LIB_PEC ( SmBusAddress ));
  ASSERT ( 0 == SMBUS_LIB_COMMAND ( SmBusAddress ));
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
  //  Build the request
  //
  I2cRequest.Timeout = 0;
  I2cRequest.OperationCount = 1;
  I2cRequest.Operation [ 0 ].Flags = I2C_FLAG_SMBUS_OPERATION;
  I2cRequest.Operation [ 0 ].Buffer = NULL;
  I2cRequest.Operation [ 0 ].LengthInBytes = 0;

  //
  //  Initiate a synchronous request.
  //
  I2cMaster = SMBus->I2cMaster;
  I2cStatus = I2cMaster->StartRequest ( I2cMaster,
                                        SlaveAddress,
                                        NULL,
                                        &I2cRequest,
                                        NULL );

  //
  //  Return the operation status
  //
  if ( NULL != Status ) {
    *Status = I2cStatus;
  }
}


/**
  Executes an SMBUS send byte command.

  Executes an SMBUS send byte command on the SMBUS device specified by SmBusAddress.
  The byte specified by Value is sent.
  Only the SMBUS slave address field of SmBusAddress is required.  Value is returned.
  If Status is not NULL, then the status of the executed command is returned in Status.
  If Command in SmBusAddress is not zero, then ASSERT().
  If Length in SmBusAddress is not zero, then ASSERT().
  If any reserved bits of SmBusAddress are set, then ASSERT().

  @param  This          Address of an EFI_SMBUS_PROTOCOL structure.
  @param  SmBusAddress  Address that encodes the SMBUS Slave Address,
                        SMBUS Command, SMBUS Data Length, and PEC.
  @param  Value         The 8-bit value to send.
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

  @return The parameter of Value.

**/
UINT8
EFIAPI
SmBusOnI2cSendByte (
  IN  EFI_SMBUS_HC_PROTOCOL *This,
  IN  UINTN SmBusAddress,
  IN  UINT8 Value,
  OUT RETURN_STATUS *Status OPTIONAL
  )
{
  UINT8 Crc;
  UINT8 Data [ 2 ];
  EFI_I2C_REQUEST_PACKET I2cRequest;
  EFI_I2C_MASTER_PROTOCOL * I2cMaster;
  RETURN_STATUS I2cStatus;
  UINTN SlaveAddress;
  SMBUS_CONTEXT * SMBus;

  //
  //  Validate the parameters
  //
  ASSERT ( 0 == SMBUS_LIB_COMMAND ( SmBusAddress ));
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
  Data [ 0 ] = Value;

  //
  //  Build the request
  //
  I2cRequest.Timeout = 0;
  I2cRequest.OperationCount = 1;
  I2cRequest.Operation [ 0 ].Flags = I2C_FLAG_SMBUS_OPERATION;
  I2cRequest.Operation [ 0 ].Buffer = &Data [ 0 ];
  I2cRequest.Operation [ 0 ].LengthInBytes = 1;

  //
  //  Determine if PEC is in use
  //
  if ( SMBUS_LIB_PEC ( SmBusAddress )) {
    I2cRequest.Operation [ 0 ].Flags |= I2C_FLAG_SMBUS_PEC;
    I2cRequest.Operation [ 0 ].LengthInBytes += 1;

    //
    //  Compute the PEC value
    //
    Crc = SMBUS_PEC_WRITE_INIT;
    Crc = Crc8 ( Crc, (UINT8)(( SlaveAddress << 1 ) | 0 ));
    Crc = Crc8 ( Crc, Value );
    Data [ 1 ] = Crc;
  }

  //
  //  Initiate a synchronous request.
  //
  I2cMaster = SMBus->I2cMaster;
  I2cStatus = I2cMaster->StartRequest ( I2cMaster,
                                        SlaveAddress,
                                        NULL,
                                        &I2cRequest,
                                        NULL );

  //
  //  Return the operation status
  //
  if ( NULL != Status ) {
    *Status = I2cStatus;
  }
}


/**
  Executes an SMBUS write data byte command.

  Executes an SMBUS write data byte command on the SMBUS device specified by SmBusAddress.
  The 8-bit value specified by Value is written.
  Only the SMBUS slave address and SMBUS command fields of SmBusAddress are required.
  Value is returned.
  If Status is not NULL, then the status of the executed command is returned in Status.
  If Length in SmBusAddress is not zero, then ASSERT().
  If any reserved bits of SmBusAddress are set, then ASSERT().

  @param  This          Address of an EFI_SMBUS_PROTOCOL structure.
  @param  SmBusAddress  Address that encodes the SMBUS Slave Address,
                        SMBUS Command, SMBUS Data Length, and PEC.
  @param  Value         The 8-bit value to write.
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

  @return The parameter of Value.

**/
UINT8
EFIAPI
SmBusOnI2cWriteDataByte (
  IN  EFI_SMBUS_HC_PROTOCOL *This,
  IN  UINTN SmBusAddress,
  IN  UINT8 Value,
  OUT RETURN_STATUS *Status OPTIONAL
  )
{
  UINT8 Crc;
  UINT8 Data [ 3 ];
  EFI_I2C_REQUEST_PACKET I2cRequest;
  EFI_I2C_MASTER_PROTOCOL * I2cMaster;
  RETURN_STATUS I2cStatus;
  UINTN SlaveAddress;
  SMBUS_CONTEXT * SMBus;

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
  Data [ 0 ] = (UINT8)SMBUS_LIB_COMMAND ( SmBusAddress );
  Data [ 1 ] = Value;

  //
  //  Build the request
  //
  I2cRequest.Timeout = 0;
  I2cRequest.OperationCount = 1;
  I2cRequest.Operation [ 0 ].Flags = I2C_FLAG_SMBUS_OPERATION;
  I2cRequest.Operation [ 0 ].Buffer = &Data [ 0 ];
  I2cRequest.Operation [ 0 ].LengthInBytes = 2;

  //
  //  Determine if PEC is in use
  //
  if ( SMBUS_LIB_PEC ( SmBusAddress )) {
    I2cRequest.Operation [ 0 ].Flags |= I2C_FLAG_SMBUS_PEC;
    I2cRequest.Operation [ 0 ].LengthInBytes += 1;

    //
    //  Compute the PEC value
    //
    Crc = SMBUS_PEC_WRITE_INIT;
    Crc = Crc8 ( Crc, (UINT8)(( SlaveAddress << 1 ) | 0 ));
    Crc = Crc8 ( Crc, Data [ 0 ]);
    Crc = Crc8 ( Crc, Data [ 1 ]);
    Data [ 2 ] = Crc;
  }

  //
  //  Initiate a synchronous request.
  //
  I2cMaster = SMBus->I2cMaster;
  I2cStatus = I2cMaster->StartRequest ( I2cMaster,
                                        SlaveAddress,
                                        NULL,
                                        &I2cRequest,
                                        NULL );

  //
  //  Return the operation status
  //
  if ( NULL != Status ) {
    *Status = I2cStatus;
  }

  //
  //  Return the data value
  //
  return Value;
}


/**
  Executes an SMBUS write data word command.

  Executes an SMBUS write data word command on the SMBUS device specified by SmBusAddress.
  The 16-bit value specified by Value is written.
  Only the SMBUS slave address and SMBUS command fields of SmBusAddress are required.
  Value is returned.
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

  @return The parameter of Value.

**/
UINT16
EFIAPI
SmBusOnI2cWriteDataWord (
  IN  EFI_SMBUS_HC_PROTOCOL *This,
  IN  UINTN SmBusAddress,
  IN  UINT16 Value,
  OUT RETURN_STATUS *Status OPTIONAL
  )
{
  UINT8 Crc;
  UINT8 Data [ 4 ];
  EFI_I2C_REQUEST_PACKET I2cRequest;
  EFI_I2C_MASTER_PROTOCOL * I2cMaster;
  RETURN_STATUS I2cStatus;
  UINTN SlaveAddress;
  SMBUS_CONTEXT * SMBus;

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
  Data [ 0 ] = (UINT8)SMBUS_LIB_COMMAND ( SmBusAddress );
  Data [ 1 ] = (UINT8)Value;
  Data [ 2 ] = (UINT8)( Value >> 8 );

  //
  //  Build the request
  //
  I2cRequest.Timeout = 0;
  I2cRequest.OperationCount = 1;
  I2cRequest.Operation [ 0 ].Flags = I2C_FLAG_SMBUS_OPERATION;
  I2cRequest.Operation [ 0 ].Buffer = &Data [ 0 ];
  I2cRequest.Operation [ 0 ].LengthInBytes = 3;

  //
  //  Determine if PEC is in use
  //
  if ( SMBUS_LIB_PEC ( SmBusAddress )) {
    I2cRequest.Operation [ 0 ].Flags |= I2C_FLAG_SMBUS_PEC;
    I2cRequest.Operation [ 0 ].LengthInBytes += 1;

    //
    //  Compute the PEC value
    //
    Crc = SMBUS_PEC_WRITE_INIT;
    Crc = Crc8 ( Crc, (UINT8)(( SlaveAddress << 1 ) | 0 ));
    Crc = Crc8 ( Crc, Data [ 0 ]);
    Crc = Crc8 ( Crc, Data [ 1 ]);
    Crc = Crc8 ( Crc, Data [ 2 ]);
    Data [ 3 ] = Crc;
  }

  //
  //  Initiate a synchronous request.
  //
  I2cMaster = SMBus->I2cMaster;
  I2cStatus = I2cMaster->StartRequest ( I2cMaster,
                                        SlaveAddress,
                                        NULL,
                                        &I2cRequest,
                                        NULL );

  //
  //  Return the operation status
  //
  if ( NULL != Status ) {
    *Status = I2cStatus;
  }

  //
  //  Return the data value
  //
  return Value;
}


/**
  Executes an SMBUS write block command.

  Executes an SMBUS write block command on the SMBUS device specified by SmBusAddress.
  The SMBUS slave address, SMBUS command, and SMBUS length fields of SmBusAddress are required.
  Bytes are written to the SMBUS from Buffer.
  The number of bytes written is returned, and will never return a value larger than 32-bytes.
  If Status is not NULL, then the status of the executed command is returned in Status.  
  If Length in SmBusAddress is zero or greater than 32, then ASSERT().
  If Buffer is NULL, then ASSERT().
  If any reserved bits of SmBusAddress are set, then ASSERT().

  @param  This          Address of an EFI_SMBUS_PROTOCOL structure.
  @param  SmBusAddress  Address that encodes the SMBUS Slave Address,
                        SMBUS Command, SMBUS Data Length, and PEC.
  @param  Buffer        Pointer to the buffer to store the bytes read from the SMBUS.
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
SmBusOnI2cWriteBlock (
  IN  EFI_SMBUS_HC_PROTOCOL *This,
  IN  UINTN SmBusAddress,
  OUT VOID *Buffer,
  OUT RETURN_STATUS *Status OPTIONAL
  )
{
  UINT8 Crc;
  UINT8 Data [ 1 + 1 + 32 + 1 ];
  EFI_I2C_REQUEST_PACKET I2cRequest;
  EFI_I2C_MASTER_PROTOCOL * I2cMaster;
  RETURN_STATUS I2cStatus;
  UINTN Index;
  UINTN Length;
  UINTN SlaveAddress;
  SMBUS_CONTEXT * SMBus;

  //
  //  Validate the parameters
  //
  ASSERT ( NULL != Buffer );
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
  I2cRequest.Timeout = 0;
  I2cRequest.OperationCount = 1;
  I2cRequest.Operation [ 0 ].Flags = I2C_FLAG_SMBUS_OPERATION
                                   | I2C_FLAG_SMBUS_BLOCK;
  I2cRequest.Operation [ 0 ].Buffer = &Data [ 0 ];
  I2cRequest.Operation [ 0 ].LengthInBytes = (UINT32)( Length + 1 );

  //
  //  Get the data byte
  //
  Data [ 0 ] = (UINT8)SMBUS_LIB_COMMAND ( SmBusAddress );
  Data [ 1 ] = (UINT8)Length;
  CopyMem ( &Data [ 2 ], Buffer, Length );

  //
  //  Determine if PEC is in use
  //
  if ( SMBUS_LIB_PEC ( SmBusAddress )) {
    I2cRequest.Operation [ 0 ].Flags |= I2C_FLAG_SMBUS_PEC;

    //
    //  Compute the PEC value
    //
    Crc = SMBUS_PEC_WRITE_INIT;
    Crc = Crc8 ( Crc, (UINT8)(( SlaveAddress << 1 ) | 0 ));
    for ( Index = 0; I2cRequest.Operation [ 0 ].LengthInBytes > Index; Index++ ) {
      Crc = Crc8 ( Crc, Data [ Index ]);
    }
    Data [ I2cRequest.Operation [ 0 ].LengthInBytes++ ] = Crc;
  }

  //
  //  Initiate a synchronous request.
  //
  I2cMaster = SMBus->I2cMaster;
  I2cStatus = I2cMaster->StartRequest ( I2cMaster,
                                        SlaveAddress,
                                        NULL,
                                        &I2cRequest,
                                        NULL );

  //
  //  Return the operation status
  //
  if ( NULL != Status ) {
    *Status = I2cStatus;
  }

  //
  //  Return the data value
  //
  return I2cRequest.Operation [ 0 ].LengthInBytes;
}

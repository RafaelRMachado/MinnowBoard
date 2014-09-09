/** @file
  Implement the SMBus read operations layered on the I2C port driver.
  
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
  Executes an SMBUS quick read command.

  Executes an SMBUS quick read command on the SMBUS device specified by SmBusAddress.
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
                        RETURN_SUCCESS  The SMBUS command was executed.
                        RETURN_TIMEOUT  A timeout occurred while executing the SMBUS command.
                        RETURN_DEVICE_ERROR The request was not completed because a failure
                        reflected in the Host Status Register bit.  Device errors are a result
                        of a transaction collision, illegal command field, unclaimed cycle
                        (host initiated), or bus errors (collisions).
                        RETURN_UNSUPPORTED  The SMBus operation is not supported.

**/
VOID
EFIAPI
SmBusOnI2cQuickRead (
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
  //  Build a ping (read) request
  //
  I2cRequest.Timeout = 0;
  I2cRequest.OperationCount = 1;
  I2cRequest.Operation [ 0 ].Flags = I2C_FLAG_SMBUS_OPERATION
                                   | I2C_FLAG_READ;
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
  Executes an SMBUS read data byte command.

  Executes an SMBUS read data byte command on the SMBUS device specified by SmBusAddress.
  Only the SMBUS slave address and SMBUS command fields of SmBusAddress are required.
  The 8-bit value read from the SMBUS is returned.
  If Status is not NULL, then the status of the executed command is returned in Status.
  If Length in SmBusAddress is not zero, then ASSERT().
  If any reserved bits of SmBusAddress are set, then ASSERT().

  @param  This          Address of an EFI_SMBUS_PROTOCOL structure.
  @param  SmBusAddress    Address that encodes the SMBUS Slave Address,
                          SMBUS Command, SMBUS Data Length, and PEC.
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

  @return The byte read from the SMBUS.

**/
UINT8
EFIAPI
SmBusOnI2cReadDataByte (
  IN  EFI_SMBUS_HC_PROTOCOL *This,
  IN  UINTN SmBusAddress,
  OUT RETURN_STATUS *Status OPTIONAL
  )
{
  UINT8 Command;
  UINT8 Crc;
  UINT8 Data [ 2 ];
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
  //  Get the command
  //
  Command = (UINT8)SMBUS_LIB_COMMAND ( SmBusAddress );

  //
  //  Build the request
  //
  SmbusTransaction.Request.Timeout = 0;
  SmbusTransaction.Request.OperationCount = 2;
  SmbusTransaction.Request.Operation [ 0 ].Flags = I2C_FLAG_SMBUS_OPERATION;
  SmbusTransaction.Request.Operation [ 0 ].Buffer = &Command;
  SmbusTransaction.Request.Operation [ 0 ].LengthInBytes = sizeof ( Command );
  SmbusTransaction.Request.Operation [ 1 ].Flags = I2C_FLAG_READ;
  SmbusTransaction.Request.Operation [ 1 ].Buffer = &Data [ 0 ];
  SmbusTransaction.Request.Operation [ 1 ].LengthInBytes = 1;

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
  if ( UsingPec && ( !EFI_ERROR ( I2cStatus ))) {
    //
    //  Validate the PEC value
    //
    Crc = SMBUS_PEC_READ_INIT;
    Crc = Crc8 ( Crc, (UINT8)(( SlaveAddress << 1 ) | 1 ));
    Crc = Crc8 ( Crc, Data [ 0 ]);
    Crc = Crc8 ( Crc, Data [ 1 ]);

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
  //  Return the data byte
  //
  return Data [ 0 ];
}


/**
  Executes an SMBUS read data word command.

  Executes an SMBUS read data word command on the SMBUS device specified by SmBusAddress.
  Only the SMBUS slave address and SMBUS command fields of SmBusAddress are required.
  The 16-bit value read from the SMBUS is returned.
  If Status is not NULL, then the status of the executed command is returned in Status.
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
                        RETURN_CRC_ERROR  The checksum is not correct (PEC is incorrect)
                        RETURN_UNSUPPORTED  The SMBus operation is not supported.

  @return The byte read from the SMBUS.

**/
UINT16
EFIAPI
SmBusOnI2cReadDataWord (
  IN  EFI_SMBUS_HC_PROTOCOL *This,
  IN  UINTN SmBusAddress,
  OUT RETURN_STATUS *Status OPTIONAL
  )
{
  UINT8 Command;
  UINT8 Crc;
  union {
    UINT8 U8 [ 3 ];
    UINT16 U16;
  } Data;
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
  //  Get the command
  //
  Command = (UINT8)SMBUS_LIB_COMMAND ( SmBusAddress );

  //
  //  Build the request
  //
  SmbusTransaction.Request.Timeout = 0;
  SmbusTransaction.Request.OperationCount = 2;
  SmbusTransaction.Request.Operation [ 0 ].Flags = I2C_FLAG_SMBUS_OPERATION;
  SmbusTransaction.Request.Operation [ 0 ].Buffer = &Command;
  SmbusTransaction.Request.Operation [ 0 ].LengthInBytes = sizeof ( Command );
  SmbusTransaction.Request.Operation [ 1 ].Flags = I2C_FLAG_READ;
  SmbusTransaction.Request.Operation [ 1 ].Buffer = &Data.U8 [ 0 ];
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
    Crc = Crc8 ( Crc, Data.U8 [ 0 ]);
    Crc = Crc8 ( Crc, Data.U8 [ 1 ]);
    Crc = Crc8 ( Crc, Data.U8 [ 2 ]);

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
  return Data.U16;
}


/**
  Executes an SMBUS read block command.

  Executes an SMBUS read block command on the SMBUS device specified by SmBusAddress.
  Only the SMBUS slave address and SMBUS command fields of SmBusAddress are required.
  Bytes are read from the SMBUS and stored in Buffer.
  The number of bytes read is returned, and will never return a value larger than 32-bytes.
  If Status is not NULL, then the status of the executed command is returned in Status.
  It is the caller's responsibility to make sure Buffer is large enough for the total number of bytes read.
  SMBUS supports a maximum transfer size of 32 bytes, so Buffer does not need to be any larger than 32 bytes.
  If Length in SmBusAddress is not zero, then ASSERT().
  If Buffer is NULL, then ASSERT().
  If any reserved bits of SmBusAddress are set, then ASSERT().

  @param  This          Address of an EFI_SMBUS_PROTOCOL structure.
  @param  SmBusAddress  Address that encodes the SMBUS Slave Address,
                        SMBUS Command, SMBUS Data Length, and PEC.
  @param  Buffer        Pointer to the buffer to store the bytes read from the SMBUS.
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

  @return The number of bytes read.

**/
UINTN
EFIAPI
SmBusOnI2cReadBlock (
  IN  EFI_SMBUS_HC_PROTOCOL *This,
  IN  UINTN SmBusAddress,
  OUT VOID *Buffer,
  OUT RETURN_STATUS *Status OPTIONAL
  )
{
  UINT8 Command;
  UINT8 Crc;
  UINT8 Data [ 1 + 32 + 1 ];
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
  ASSERT ( NULL != Buffer );
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
  //  Get the command
  //
  Command = (UINT8)SMBUS_LIB_COMMAND ( SmBusAddress );

  //
  //  Build the request
  //
  SmbusTransaction.Request.Timeout = 0;
  SmbusTransaction.Request.OperationCount = 2;
  SmbusTransaction.Request.Operation [ 0 ].Flags = I2C_FLAG_SMBUS_OPERATION
                                                 | I2C_FLAG_SMBUS_BLOCK;
  SmbusTransaction.Request.Operation [ 0 ].Buffer = &Command;
  SmbusTransaction.Request.Operation [ 0 ].LengthInBytes = sizeof ( Command );
  SmbusTransaction.Request.Operation [ 1 ].Flags = I2C_FLAG_READ;
  SmbusTransaction.Request.Operation [ 1 ].Buffer = Buffer;
  SmbusTransaction.Request.Operation [ 1 ].LengthInBytes = 1 + 32;  //  Maximum number of bytes

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
  if ( UsingPec && ( !EFI_ERROR ( Status ))) {
    //
    //  Compute the PEC value
    //
    Crc = SMBUS_PEC_READ_INIT;
    Crc = Crc8 ( Crc, (UINT8)(( SlaveAddress << 1 ) | 0 ));
    for ( Index = 0; 1 + Length + 1 > Index; Index++ ) {
      Crc = Crc8 ( Crc, Data [ Index ]);
    }
    if ( SMBUS_PEC_VALID != Crc ) {
      //
      //  CRC error detected
      //
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
  //  Return the byte count
  //
  return Length;
}



/**
  Executes an SMBUS receive byte command.

  Executes an SMBUS receive byte command on the SMBUS device specified by SmBusAddress.
  Only the SMBUS slave address field of SmBusAddress is required.
  The byte received from the SMBUS is returned.
  If Status is not NULL, then the status of the executed command is returned in Status.
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
                        RETURN_CRC_ERROR  The checksum is not correct (PEC is incorrect)
                        RETURN_UNSUPPORTED  The SMBus operation is not supported.

  @return The byte received from the SMBUS.

**/
UINT8
EFIAPI
SmBusOnI2cReceiveByte (
  IN  EFI_SMBUS_HC_PROTOCOL *This,
  IN  UINTN SmBusAddress,
  OUT RETURN_STATUS *Status OPTIONAL
  )
{
  UINT8 Crc;
  UINT8 Data [ 2 ];
  EFI_I2C_MASTER_PROTOCOL * I2cMaster;
  RETURN_STATUS I2cStatus;
  UINTN SlaveAddress;
  SMBUS_CONTEXT * SMBus;
  SMBUS_TRANSACTION SmbusTransaction;
  BOOLEAN UsingPec;

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
  //  Build the request
  //
  SmbusTransaction.Request.Timeout = 0;
  SmbusTransaction.Request.OperationCount = 2;
  SmbusTransaction.Request.Operation [ 0 ].Flags = I2C_FLAG_SMBUS_OPERATION;
  SmbusTransaction.Request.Operation [ 0 ].Buffer = NULL;
  SmbusTransaction.Request.Operation [ 0 ].LengthInBytes = 0;
  SmbusTransaction.Request.Operation [ 1 ].Flags = I2C_FLAG_READ;
  SmbusTransaction.Request.Operation [ 1 ].Buffer = &Data [ 0 ];
  SmbusTransaction.Request.Operation [ 1 ].LengthInBytes = 1;

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
  if ( UsingPec && ( !EFI_ERROR ( I2cStatus ))) {
    //
    //  Validate the PEC value
    //
    Crc = SMBUS_PEC_READ_INIT;
    Crc = Crc8 ( Crc, (UINT8)(( SlaveAddress << 1 ) | 1 ));
    Crc = Crc8 ( Crc, Data [ 0 ]);
    Crc = Crc8 ( Crc, Data [ 1 ]);
    if ( SMBUS_PEC_VALID != Crc ) {
      //
      //  CRC error detected
      //
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
  //  Return the data byte
  //
  return Data [ 0 ];
}

/** @file
  Implement the I2C host protocol.
  
  Copyright (c) 2012, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "I2cHost.h"


/**
  Handle the I2C bus configuration available event

  This routine is called at TPL_I2C_SYNC.

  @param[in] Event    Address of an EFI_EVENT handle
  @param[in] Context  Address of an I2C_HOST_CONTEXT structure

**/
VOID
EFIAPI
I2cHostI2cBusConfigurationAvailable (
  IN EFI_EVENT Event,
  IN VOID *Context
  )
{
  I2C_HOST_CONTEXT *I2cHost;
  CONST EFI_I2C_MASTER_PROTOCOL * I2cMaster;
  I2C_REQUEST *I2cRequest;

  //
  //  Display entry
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cHostI2cBusConfigurationAvailable entered\r\n" ));

  //
  //  Mark this I2C bus configuration management operation as complete
  //
  I2cHost = (I2C_HOST_CONTEXT *)Context;
  I2cHost->I2cBusConfigurationManagementPending = FALSE;

  //
  //  Determine if a request is pending
  //
  I2cRequest = I2cHost->RequestListHead;
  if ( NULL != I2cRequest ) {
    //
    //  Display the request state
    //
    if ( I2cHost->I2cBusConfiguration != I2cRequest->I2cBusConfiguration ) {
      DEBUG (( DEBUG_I2C_OPERATION,
                "0x%016Lx: I2cHost, I2C bus configuration %d for request packet 0x%016Lx, Status: %r\r\n",
                (UINT64)(UINTN)I2cHost,
                I2cRequest->I2cBusConfiguration,
                (UINT64)(UINTN)&I2cRequest->RequestPacket,
                I2cHost->Status ));
    }

    //
    //  Determine if the driver is shutting down
    //
    if ( I2cHost->ShuttingDown ) {
      //
      //  Abort this request
      //
      I2cHostRequestCompleteError ( I2cHost, EFI_ABORTED  );
    }
    else {
      //
      //  Validate the completion status
      //
      if ( EFI_ERROR ( I2cHost->Status )) {
        I2cHostRequestCompleteError ( I2cHost, I2cHost->Status );

        //
        //  Unknown I2C bus configuration
        //  Force next operation to enable the I2C bus configuration
        //
        I2cHost->I2cBusConfiguration = (UINTN)-1;
      }
      else {
        //
        //  Update the I2C bus configuration
        //
        I2cHost->I2cBusConfiguration = I2cRequest->I2cBusConfiguration;

        //
        //  Clear the event
        //
        gBS->CheckEvent ( I2cHost->I2cEvent );

        //
        //  Display the request state
        //
        DEBUG (( DEBUG_I2C_OPERATION,
                  "0x%016Lx: I2cHost starting I2C for request packet 0x%016Lx\r\n",
                  (UINT64)(UINTN)I2cHost,
                  (UINT64)(UINTN)&I2cRequest->RequestPacket ));

        //
        //  Start an I2C operation on the host
        //
        I2cMaster = I2cHost->I2cMaster;
        I2cMaster->StartRequest ( I2cMaster,
                                  I2cRequest->SlaveAddress,
                                  I2cHost->I2cEvent,
                                  &I2cRequest->RequestPacket,
                                  &I2cHost->Status );
      }
    }
  }

  //
  //  Display exit
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cHostI2cBusConfigurationAvailable exiting\r\n" ));
}


/**
  Complete the current request

  This routine is called at TPL_I2C_SYNC.

  @param[in] I2cHost  Address of an I2C_HOST_CONTEXT structure.
  @param[in] Status   Status of the I<sub>2</sub>C operation.

  @return This routine returns the input status value.

**/
EFI_STATUS
I2cHostRequestComplete (
  I2C_HOST_CONTEXT *I2cHost,
  EFI_STATUS Status
  )
{
  I2C_REQUEST *I2cRequest;

  //
  //  Display entry
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cHostRequestComplete entered\r\n" ));

  //
  //  Remove the current I2C request from the list
  //
  I2cRequest = I2cHost->RequestListHead;
  I2cHost->RequestListHead = I2cRequest->Next;
  if ( NULL == I2cHost->RequestListHead ) {
    I2cHost->RequestListTail = NULL;
  }

  //
  //  Display the request state
  //
  DEBUG (( DEBUG_I2C_OPERATION,
            "0x%016Lx: I2cHost removed I2C request packet 0x%016Lx from queue\r\n",
            (UINT64)(UINTN)I2cHost,
            (UINT64)(UINTN)&I2cRequest->RequestPacket ));

  //
  //  Display the request state
  //
  DEBUG (( DEBUG_I2C_OPERATION,
            "0x%016Lx: I2cHost, I2C request packet 0x%016Lx completion, Status: %r\r\n",
            (UINT64)(UINTN)I2cHost,
            (UINT64)(UINTN)&I2cRequest->RequestPacket,
            Status ));

  //
  //  Save the status for the user
  //
  if ( NULL != I2cRequest->Status ) {
    *I2cRequest->Status = Status;
  }

  //
  //  Notify the user of the I2C request completion
  //
  if ( NULL != I2cRequest->Event ) {
    gBS->SignalEvent ( I2cRequest->Event );
  }

  //
  //  Done with this request
  //
  FreePool ( I2cRequest );

  //
  //  Start the next request
  //
  I2cHostRequestEnable ( I2cHost );

  //
  //  Display exit
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cHostRequestComplete exiting, Status: %r\r\n", Status ));
  return Status;
}


/**
  Complete the current request with an error

  @param[in] I2cHost  Address of an I2C_HOST_CONTEXT structure.
  @param[in] Status   Status of the I<sub>2</sub>C operation.

  @return This routine returns the input status value.

**/
EFI_STATUS
I2cHostRequestCompleteError (
  I2C_HOST_CONTEXT *I2cHost,
  EFI_STATUS Status
  )
{
  EFI_TPL TplPrevious;

  //
  //  Display entry
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cHostRequestCompleteError entered\r\n" ));

  //
  //  Synchronize with the other threads
  //
  TplPrevious = gBS->RaiseTPL ( TPL_I2C_SYNC );

  //
  //  Complete the request
  //
  I2cHostRequestComplete ( I2cHost, Status );

  //
  //  Release the thread synchronization
  //
  gBS->RestoreTPL ( TplPrevious );

  //
  //  Display exit
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cHostRequestCompleteError exiting, Status: %r\r\n", Status ));
  return Status;
}


/**
  Handle the bus available event

  This routine is called at TPL_I2C_SYNC.

  @param[in] Event    Address of an EFI_EVENT handle
  @param[in] Context  Address of an I2C_HOST_CONTEXT structure

**/
VOID
EFIAPI
I2cHostRequestCompleteEvent (
  IN EFI_EVENT Event,
  IN VOID *Context
  )
{
  I2C_HOST_CONTEXT *I2cHost;

  //
  //  Display entry
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cHostRequestCompleteEvent entered\r\n" ));

  //
  //  Handle the completion event
  //
  I2cHost = (I2C_HOST_CONTEXT *)Context;
  I2cHostRequestComplete ( I2cHost, I2cHost->Status );

  //
  //  Display exit
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cHostRequestCompleteEvent exiting\r\n" ));
}


/**
  Enable access to the I2C bus configuration

  @param[in] I2cHost    Address of an I2C_HOST_CONTEXT structure

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_ABORTED           The request did not complete because the driver
                                was shutdown.
  @retval EFI_BAD_BUFFER_SIZE   The WriteBytes or ReadBytes buffer size is too large.
  @retval EFI_DEVICE_ERROR      There was an I2C error (NACK) during the operation.
                                This could indicate the slave device is not present.
  @retval EFI_INVALID_PARAMETER RequestPacket is NULL
  @retval EFI_NO_MAPPING        Invalid I2cBusConfiguration value
  @retval EFI_NO_RESPONSE       The I2C device is not responding to the
                                slave address.  EFI_DEVICE_ERROR may also be
                                returned if the controller can not distinguish
                                when the NACK occurred.
  @retval EFI_NOT_FOUND         I2C slave address exceeds maximum address
  @retval EFI_NOT_READY         I2C bus is busy or operation pending, wait for
                                the event and then read status.
  @retval EFI_OUT_OF_RESOURCES  Insufficient memory for I2C operation
  @retval EFI_TIMEOUT           The transaction did not complete within an internally
                                specified timeout period.

**/
EFI_STATUS
I2cHostRequestEnable (
  I2C_HOST_CONTEXT *I2cHost
  )
{
  UINTN I2cBusConfiguration;
  CONST EFI_I2C_BUS_CONFIGURATION_MANAGEMENT_PROTOCOL *I2cBusConfigurationManagement;
  I2C_REQUEST *I2cRequest;
  EFI_STATUS Status;
  EFI_TPL TplPrevious;

  //
  //  Display entry
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cHostRequestEnable entered\r\n" ));

  //
  //  Assume pending request
  //
  Status = EFI_NOT_READY;

  //
  //  Determine if the I2C request requires access to another
  //  I2C bus configuration
  //
  I2cRequest = I2cHost->RequestListHead;
  I2cBusConfiguration = I2cRequest->I2cBusConfiguration;
  if ( I2cHost->I2cBusConfiguration != I2cBusConfiguration ) {
    //
    //  Display the request state
    //
    DEBUG (( DEBUG_I2C_OPERATION,
              "0x%016Lx: I2cHost configuring access to I2C bus configuration %d for request packet 0x%016Lx\r\n",
              (UINT64)(UINTN)I2cHost,
              I2cRequest->I2cBusConfiguration,
              (UINT64)(UINTN)&I2cRequest->RequestPacket ));

    //
    //  This case only occurs when I2cBus is non-NULL!
    //
    I2cBusConfigurationManagement = I2cHost->I2cBusConfigurationManagement;

    //
    //  Clear the event
    //
    gBS->CheckEvent ( I2cHost->I2cBusConfigurationEvent );

    //
    //  Another I2C bus configuration is required for this request
    //  Enable access to the required I2C bus configuration
    //
    I2cHost->I2cBusConfigurationManagementPending = TRUE;
    Status = I2cBusConfigurationManagement->EnableI2cBusConfiguration (
                I2cBusConfigurationManagement,
                I2cBusConfiguration,
                I2cHost->I2cBusConfigurationEvent,
                &I2cHost->Status );

    //
    //  N.B. This routine is not allowed to access the
    //  I2cRequest after this point!  It is possible for the
    //  port driver to complete this request at any time including
    //  immediately.
    //

    if ( EFI_SUCCESS == Status ) {
      //
      //  Display the request state
      //
      DEBUG (( DEBUG_INFO,
                "INFO - Platform driver should be returning EFI_NOT_READY, Status: %r\r\n",
                Status ));

      //
      //  The platform code should always return not ready!
      //  The I2cHostI2cBusConfigurationAvailable will continue to
      //  drive the state machine to complete the I2C request.
      //
      Status = EFI_NOT_READY;
    }
  }
  else
  {
    //
    //  Synchronize with the other threads
    //
    TplPrevious = gBS->RaiseTPL ( TPL_I2C_SYNC );

    //
    //  Same I2C bus configuration
    //
    I2cHost->Status = EFI_SUCCESS;
    I2cHostI2cBusConfigurationAvailable ( I2cHost->I2cBusConfigurationEvent,
                                          I2cHost );

    //
    //  Release the thread synchronization
    //
    gBS->RestoreTPL ( TplPrevious );
  }

  //
  //  Display exit
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cHostRequestEnable exiting, Status: %r\r\n", Status ));
  return Status;
}


/**
  Queue an I2C transaction for execution on the I2C controller.

  This routine must be called at or below TPL_NOTIFY.  For
  synchronous requests this routine must be called at or below
  TPL_CALLBACK.
  
  The I2C host protocol uses the concept of I2C bus configurations
  to describe the I2C bus.  An I2C bus configuration is defined as
  a unique setting of the multiplexers and switches in the I2C bus
  which enable access to one or more I2C devices.  When using a
  switch to divide a bus, due to bus frequency differences, the
  I2C bus configuration management protocol defines an I2C bus
  configuration for the I2C devices on each side of the switch.
  When using a multiplexer, the I2C bus configuration management
  defines an I2C bus configuration for each of the selector values
  required to control the multiplexer.  See Figure 1 in the I2C -bus
  specification and user manual for a complex I2C bus configuration.

  The I2C host protocol processes all transactions in FIFO order.
  Prior to performing the transaction, the I2C host protocol calls
  EnableI2cBusConfiguration to reconfigure the switches and
  multiplexers in the I2C bus enabling access to the specified I2C
  device.  The EnableI2cBusConfiguration also selects the I2C bus
  frequency for the I2C device.  After the I2C bus is configured,
  the I2C host protocol calls the I2C master protocol to start the
  I2C transaction.

  If the I2C host protocol has pending I2C transactions queued when
  the driver binding Stop() routine is called then the I2C host
  protocol completes all of the pending I2C transactions by returning
  EFI_ABORTED status.  This notifies the upper layers allowing them
  to take corrective action or prepare to stop.

  When Event is NULL, QueueRequest() operates synchronously and
  returns the I2C completion status as its return value.

  When Event is not NULL, QueueRequest() synchronously returns
  EFI_SUCCESS indicating that the asynchronously I2C transaction was
  queued.  The values above are returned in the buffer pointed to by
  I2cStatus upon the completion of the I2C transaction when I2cStatus
  is not NULL.

  @param[in] This             Pointer to an EFI_I2C_HOST_PROTOCOL structure.
  @param[in] I2cBusConfiguration  I2C bus configuration to access the I2C
                                  device
  @param[in] SlaveAddress     Address of the device on the I2C bus.  Set
                              the I2C_ADDRESSING_10_BIT when using 10-bit
                              addresses, clear this bit for 7-bit addressing.
                              Bits 0-6 are used for 7-bit I2C slave addresses
                              and bits 0-9 are used for 10-bit I2C slave
                              addresses.
  @param[in] Event            Event to signal for asynchronous transactions,
                              NULL for synchronous transactions
  @param[in] RequestPacket    Pointer to an EFI_I2C_REQUEST_PACKET structure
                              describing the I2C transaction
  @param[out] I2cStatus       Optional buffer to receive the I2C transaction
                              completion status

  @return  When Event is NULL, QueueRequest operates synchrouously and returns the
  I2C completion status as its return value.  In this case it is recommended to use
  NULL for I2cStatus.  The values returned from QueueRequest are:

  @retval EFI_SUCCESS           The asynchronous transaction was successfully
                                queued when Event is not NULL.
  @retval EFI_SUCCESS           The transaction completed successfully when
                                Event is NULL.
  @retval EFI_ABORTED           The request did not complete because the
                                driver binding Stop() routine was called.
  @retval EFI_BAD_BUFFER_SIZE   The RequestPacket->LengthInBytes value is
                                too large.
  @retval EFI_DEVICE_ERROR      There was an I2C error (NACK) during the
                                transaction.
  @retval EFI_INVALID_PARAMETER RequestPacket is NULL
  @retval EFI_NOT_FOUND         Reserved bit set in the SlaveAddress parameter
  @retval EFI_NO_MAPPING        Invalid I2cBusConfiguration value
  @retval EFI_NO_RESPONSE       The I2C device is not responding to the slave
                                address.  EFI_DEVICE_ERROR will be returned
                                if the controller cannot distinguish when the
                                NACK occurred.
  @retval EFI_OUT_OF_RESOURCES  Insufficient memory for I2C transaction
  @retval EFI_UNSUPPORTED       The controller does not support the requested
                                transaction.

**/
EFI_STATUS
EFIAPI
I2cHostRequestQueue (
  IN CONST EFI_I2C_HOST_PROTOCOL *This,
  IN UINTN I2cBusConfiguration,
  IN UINTN SlaveAddress,
  IN EFI_EVENT Event OPTIONAL,
  IN EFI_I2C_REQUEST_PACKET *RequestPacket,
  OUT EFI_STATUS *I2cStatus OPTIONAL
  )
{
  I2C_HOST_CONTEXT *I2cHost;
  I2C_REQUEST *I2cRequest;
  I2C_REQUEST *Previous;
  BOOLEAN StartRequest;
  EFI_STATUS Status;
  EFI_EVENT SyncEvent;
  EFI_TPL TplPrevious;

  //
  //  Display entry
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cHostRequestQueue entered\r\n" ));

  //
  //  Allocate the event if necessary
  //
  SyncEvent = NULL;
  Status = EFI_SUCCESS;
  if ( NULL == Event ) {
    Status = gBS->CreateEvent ( 0,
                                TPL_I2C_SYNC,
                                NULL,
                                NULL,
                                &SyncEvent );
  }
  if ( !EFI_ERROR ( Status )) {
    //
    //  Assume pending request
    //
    Status = EFI_NOT_READY;

    //
    //  Validate the request packet
    //
    if ( NULL == RequestPacket ) {
      DEBUG (( DEBUG_WARN,
                "WARNING - RequestPacket is NULL\r\n" ));
      Status = EFI_INVALID_PARAMETER;
    }
    else {
      //
      //  Validate the TPL
      //
      TplPrevious = gBS->RaiseTPL ( TPL_HIGH_LEVEL );
      gBS->RestoreTPL ( TplPrevious );
      if (( TPL_I2C_SYNC < TplPrevious )
        || (( NULL == Event ) && ( TPL_CALLBACK < TplPrevious ))) {
        DEBUG (( DEBUG_ERROR,
                  "ERROR - TPL %d is too high!\r\n",
                  TplPrevious  ));
        Status = EFI_INVALID_PARAMETER;
      }
      else {
        //
        //  Allocate the request structure
        //
        I2cHost = I2C_HOST_CONTEXT_FROM_PROTOCOL ( This );
        I2cRequest = AllocateZeroPool ( sizeof ( *I2cRequest ));
        if ( NULL == I2cRequest ) {
          DEBUG (( DEBUG_WARN,
                    "WARNING - Failed to allocate I2C_REQUEST!\r\n" ));
          Status = EFI_OUT_OF_RESOURCES;
        }
        else {
          //
          //  Initialize the request
          //
          I2cRequest->I2cBusConfiguration = I2cBusConfiguration;
          I2cRequest->SlaveAddress = SlaveAddress;
          I2cRequest->Event = ( NULL == Event ) ? SyncEvent : Event;
          I2cRequest->Status = I2cStatus;
          CopyMem ( &I2cRequest->RequestPacket, RequestPacket, sizeof ( *RequestPacket ));

          //
          //  Display the request state
          //
          DEBUG (( DEBUG_I2C_OPERATION,
                    "0x%016Lx: I2cHost queuing I2C request packet 0x%016Lx\r\n",
                    (UINT64)(UINTN)I2cHost,
                    (UINT64)(UINTN)&I2cRequest->RequestPacket ));

          //
          //  Synchronize with the other threads
          //
          gBS->RaiseTPL ( TPL_I2C_SYNC );

          //
          //  Place the request at the end of the pending list
          //
          Previous = I2cHost->RequestListTail;
          StartRequest = (BOOLEAN)( NULL == Previous );
          if ( !StartRequest ) {
            //
            //  Another request is pending
            //  Place this request at the end of the list
            //
            Previous->Next = I2cRequest;
          }
          else {
            //
            //  This is the first request
            //
            I2cHost->RequestListHead = I2cRequest;
          }
          I2cHost->RequestListTail = I2cRequest;

          //
          //  Release the thread synchronization
          //
          gBS->RestoreTPL ( TplPrevious );

          //
          //  N.B. This routine is not allowed to access the
          //  I2cRequest after this point!  It is possible for the
          //  port driver to complete this request at any time including
          //  immediately.
          //
          //  Start processing this request
          //
          if ( StartRequest ) {
            //
            //  Enable access to the I2C bus configuration
            //
            Status = I2cHostRequestEnable ( I2cHost );
          }

          //
          //  Check for a synchronous operation
          //
          if (( NULL == Event )
            && ( EFI_NOT_READY == Status )) {
            //
            //  Display the request state
            //
            DEBUG (( DEBUG_I2C_OPERATION,
                      "0x%016Lx: I2cHost waiting for synchronous I2C request packet 0x%016Lx\r\n",
                      (UINT64)(UINTN)I2cHost,
                      (UINT64)(UINTN)RequestPacket ));

            //
            //  Wait for the operation completion
            //
            do {
              Status = gBS->CheckEvent ( SyncEvent );
            } while ( EFI_NOT_READY == Status );

            //
            //  Get the operation status
            //
            Status = I2cHost->Status;

            //
            //  Display the request state
            //
            DEBUG (( DEBUG_I2C_OPERATION,
                      "0x%016Lx: I2cHost, synchronous I2C request packet 0x%016Lx complete, Status: %r\r\n",
                      (UINT64)(UINTN)I2cHost,
                      (UINT64)(UINTN)RequestPacket,
                      Status ));
          }
        }
      }
    }
  }

  //
  //  Close the event if necessary
  //
  if ( NULL != SyncEvent ) {
    gBS->CloseEvent ( SyncEvent );
  }

  //
  //  Display exit
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cHostRequestQueue exiting, Status: %r\r\n", Status ));
  return Status;
}


/**
  Start the I2C driver

  This routine allocates the necessary resources for the driver.

  This routine is called by I2cHostDriverStart to complete the driver
  initialization.

  @param [in] I2cHost         Address of an I2C_HOST_CONTEXT structure

  @retval EFI_SUCCESS         Driver API properly initialized
  
**/
EFI_STATUS
I2cHostApiStart (
  IN I2C_HOST_CONTEXT *I2cHost
  )
{
  CONST EFI_I2C_BUS_CONFIGURATION_MANAGEMENT_PROTOCOL *I2cBusConfigurationManagement;
  CONST EFI_I2C_MASTER_PROTOCOL *I2cMaster;
  EFI_STATUS Status;

  //
  //  Display entry
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cHostApiStart entered\r\n" ));

  //
  //  Assume success
  //
  Status = EFI_SUCCESS;

  //
  //  Reset the controller
  //
  I2cMaster = I2cHost->I2cMaster;
  I2cMaster->Reset ( I2cMaster );

  //
  //  Get the I2C event
  //
  Status = gBS->CreateEvent ( EVT_NOTIFY_SIGNAL,
                              TPL_I2C_SYNC,
                              &I2cHostRequestCompleteEvent,
                              I2cHost,
                              &I2cHost->I2cEvent );
  if ( EFI_ERROR ( Status )) {
    DEBUG (( DEBUG_ERROR,
              "ERROR - Failed to allocate the I2C event, Status: %r\r\n",
              Status ));
  }
  else {
    //
    //  Determine the number of I2C bus configurations
    //
    I2cBusConfigurationManagement = I2cHost->I2cBusConfigurationManagement;

    //
    //  Get the bus management event
    //
    Status = gBS->CreateEvent ( EVT_NOTIFY_SIGNAL,
                                TPL_I2C_SYNC,
                                I2cHostI2cBusConfigurationAvailable,
                                I2cHost,
                                &I2cHost->I2cBusConfigurationEvent );
    if ( !EFI_ERROR ( Status )) {
      //
      //  Enable the primary I2C bus configuration
      //
      Status = I2cBusConfigurationManagement->EnableI2cBusConfiguration (
                        I2cBusConfigurationManagement,
                        I2cHost->I2cBusConfiguration,
                        NULL,
                        NULL );
    }
  }

  //
  //  Build the I2C host protocol
  //
  I2cHost->HostApi.QueueRequest = I2cHostRequestQueue;
  I2cHost->HostApi.I2cControllerCapabilities = I2cMaster->I2cControllerCapabilities;

  //
  //  Display exit
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cHostApiStart exiting, Status: %r\r\n", Status ));

  //
  //  Return the startup status
  //
  return Status;
}


/**
  Stop the I2C driver

  This routine releases the resources allocated by I2cApiStart.

  This routine is called by I2cHostDriverStop to initiate the driver
  shutdown.

  @param [in] I2cHost         Address of an I2C_HOST_CONTEXT structure

**/
VOID
I2cHostApiStop (
  IN I2C_HOST_CONTEXT *I2cHost
  )
{
  I2C_REQUEST *I2cRequest;
  I2C_REQUEST *I2cRequestList;
  EFI_STATUS Status;
  EFI_TPL TplPrevious;

  //
  //  Display entry
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cHostApiStop entered\r\n" ));

  //
  //  Synchronize with the other threads
  //
  TplPrevious = gBS->RaiseTPL ( TPL_I2C_SYNC );

  //
  //  Flag the driver shutdown
  //
  I2cHost->ShuttingDown = TRUE;

  //
  //  Remove all but the active request from the pending list
  //
  I2cRequestList = I2cHost->RequestListHead;
  if (( NULL != I2cRequestList )
    && ( !I2cHost->I2cBusConfigurationManagementPending )) {
    I2cRequestList = I2cRequestList->Next;
    I2cHost->RequestListHead->Next = NULL;
    I2cHost->RequestListTail = I2cHost->RequestListHead;
  }
  
  //
  //  Release the thread synchronization
  //
  gBS->RestoreTPL ( TplPrevious );

  //
  //  Abort any pending requests
  //
  while ( NULL != I2cRequestList ) {
    I2cRequest = I2cRequestList;
    I2cRequestList = I2cRequest->Next;

    //
    //  Abort this request
    //
    if ( NULL != I2cRequest->Status ) {
      *I2cRequest->Status = EFI_ABORTED;
    }
    if ( NULL != I2cRequest->Event ) {
      gBS->SignalEvent ( I2cRequest->Event );
    }

    //
    //  Done with this request
    //
    FreePool ( I2cRequest );
  }

  //
  //  Wait for the bus management to complete
  //
  while ( I2cHost->I2cBusConfigurationManagementPending ) {
  };

  //
  //  Release the events
  //
  if ( NULL != I2cHost->I2cBusConfigurationEvent ) {
    Status = gBS->CloseEvent ( I2cHost->I2cBusConfigurationEvent );
    ASSERT ( EFI_SUCCESS == Status );
    I2cHost->I2cBusConfigurationEvent = NULL;
  }

  if ( NULL != I2cHost->I2cEvent ) {
    Status = gBS->CloseEvent ( I2cHost->I2cEvent );
    ASSERT ( EFI_SUCCESS == Status );
    I2cHost->I2cEvent = NULL;
  }

  //
  //  Display exit
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cHostApiStop exiting\r\n" ));
}

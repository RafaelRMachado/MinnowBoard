/** @file
  Application to manipulate the EG20T GPIO ports.
  
  Copyright (c) 2012-2013, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "GpioEg20t.h"


EFI_HANDLE gGpioEg20tHiiHandle = NULL;

STATIC CONST GPIO_COMMAND mCommands [ ] = {
  { 0, 0, L"addr", L"                  Display the GPIO base address" },
  { 0, 1, L"config", L"  [port]        Display GPIO port configurations" },
  { 2, 2, L"dir", L"     <port> <0:1>  Set GPIO port direction (output:input)" },
  { 1, 1, L"iclr", L"    <port>        Interrupt clear" },
  { 2, 2, L"ienbl", L"   <port> <0:1>  Interrupt (disable:enable)" },
  { 2, 2, L"imask", L"   <port> <0:1>  Interrupt mask (clear:set)" },
  { 2, 2, L"imode", L"   <port> <0-4>  Interrupt mode (0=FE, 1=RE, 2=L, 3=H, 4=both edges)" },
  { 2, 2, L"output", L"  <port> <0:1>  Set GPIO port output value" },
  { 1, 1, L"reset", L"   <0:1>         Reset the controller (clear:set)" }
};
STATIC CONST CHAR16 * mPinName [ ] = {
  L"[AB20]",  //  0
  L"[AB19]",  //  1
  L"[AB18]",  //  2
  L"[AA18]",  //  3
  L"[Y18]",   //  4
  L"[AB17]",  //  5
  L"[AA17]",  //  6
  L"[Y17]",   //  7
  L"[A9]",    //  8
  L"[B9]",    //  9
  L"[A8]",    //  10
  L"[B8]"     //  11
};
STATIC CONST UINT64 mMaxPortNumber = DIM ( mPinName );
STATIC CONST CHAR16 * mInterruptMode [ ] = {
  L"FE",  //  0
  L"RE",  //  1
  L"L ",  //  2
  L" H",  //  3
  L"BE",  //  4
  L"??",  //  5
  L"??",  //  6
  L"??"   //  7
};


// {44222F54-47AD-4063-917E-CA883E5976F7}
STATIC CONST GUID mGpioEg20tGuid = 
{ 0x44222f54, 0x47ad, 0x4063, { 0x91, 0x7e, 0xca, 0x88, 0x3e, 0x59, 0x76, 0xf7 } };

VOID
GpioEg20tGetHiiHandle (
  VOID
  )
{
  if ( NULL == gGpioEg20tHiiHandle ) {
    gGpioEg20tHiiHandle = HiiAddPackages ( &mGpioEg20tGuid,
                                           gImageHandle,
                                           STRING_ARRAY_NAME,
                                           NULL );
  }
}

EFI_STATUS
GpioEg20tConfig (
  IN UINT64 PortNumber,
  IN CONST CHAR16 * PinName
  )
{
  BOOLEAN InterruptEnable;
  BOOLEAN InterruptMask;
  CONST CHAR16 * InterruptMode;
  BOOLEAN InterruptPending;
  BOOLEAN InterruptRequest;
  UINTN Level;
  BOOLEAN Output;
  BOOLEAN OutputValue;
  EFI_STATUS Status;
  UINTN Value;

  for ( ; ; ) {
    //
    //  Get the port direction
    //
    Status = Eg20tGpioPinDirection ( PortNumber, &Value );
    if ( EFI_ERROR ( Status )) {
      break;
    }
    Output = (BOOLEAN)Value;

    //
    //  Get the port level
    //
    Status = Eg20tGpioPinPortInput ( PortNumber, &Level );
    if ( EFI_ERROR ( Status )) {
      break;
    }

    //
    //  Get the port output level
    //
    Status = Eg20tGpioPinPortOutput ( PortNumber, &Value );
    if ( EFI_ERROR ( Status )) {
      break;
    }
    OutputValue = (BOOLEAN)Value;

    //
    //  Get the interrupt enable/disable
    //
    Status = Eg20tGpioPinInterruptEnable ( PortNumber, &Value );
    if ( EFI_ERROR ( Status )) {
      break;
    }
    InterruptEnable = (BOOLEAN)Value;

    //
    //  Get the interrupt mode
    //
    Status = Eg20tGpioPinInterruptMode ( PortNumber, &Value );
    if ( EFI_ERROR ( Status )) {
      break;
    }
    InterruptMode = mInterruptMode [ Value ];

    //
    //  Get the interupt mask
    //
    Status = Eg20tGpioPinInterruptMask ( PortNumber, &Value );
    if ( EFI_ERROR ( Status )) {
      break;
    }
    InterruptMask = (BOOLEAN)Value;

    //
    //  Get the interrupt pending status
    //
    Status = Eg20tGpioPinInterruptPending ( PortNumber, &Value );
    if ( EFI_ERROR ( Status )) {
      break;
    }
    InterruptPending = (BOOLEAN)Value;

    //
    //  Get the interrupt request
    //
    Status = Eg20tGpioPinInterruptRequest ( PortNumber, &Value );
    if ( EFI_ERROR ( Status )) {
      break;
    }
    InterruptRequest = (BOOLEAN)Value;

    //
    //  Display the pin configuration
    //
    Print ( L"GPIO%-2Ld %-6s: %s  %d  %d   %d   %2s  %d   %d   %d\r\n",
            PortNumber,
            PinName,
            Output ? L"Out" : L"In ",
            Level,
            OutputValue,
            InterruptEnable,
            InterruptMode,
            InterruptMask,
            InterruptPending,
            InterruptRequest );
    break;
  }

  //
  //  Return the operation status
  //
  return Status;
}


EFI_STATUS
GpioEg20tMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{
  CONST CHAR16 * Command;
  UINTN CommandIndex;
  BOOLEAN DisplayHelp;
  UINTN GpioBaseAddress;
  UINT64 PortNumber;
  UINTN Reset;
  EFI_STATUS Status;
  UINT64 Value;

  //
  //  Get the HII handle
  //
  GpioEg20tGetHiiHandle ( );
  ASSERT ( NULL != gGpioEg20tHiiHandle );

  //
  //  Assume success
  //
  Status = EFI_SUCCESS;
  DisplayHelp = (BOOLEAN)( 2 > Argc );

  //
  //  Determinte the command index
  //
  Command = NULL;
  PortNumber = (UINT64)-1;
  Value = 0;
  CommandIndex = DIM ( mCommands );
  if ( !DisplayHelp ) {
    for ( CommandIndex = 0; DIM ( mCommands ) > CommandIndex; CommandIndex += 1 ) {
      Command = mCommands [ CommandIndex ].Command;
      if ( 0 == StrCmp ( Argv [ 1 ], Command )) {
        break;
      }
    }
    if ( DIM ( mCommands ) <= CommandIndex ) {
      ShellPrintHiiEx ( -1,
                        -1,
                        NULL,
                        STRING_TOKEN (STR_UNSUPPORTED_COMMAND),
                        gGpioEg20tHiiHandle );
      DisplayHelp = TRUE;
      Status = EFI_UNSUPPORTED;
    }
    else {
      //
      //  Validate the parameter count
      //
      if ( Argc > ( 2 + mCommands [ CommandIndex ].MaxParameters )) {
        ShellPrintHiiEx ( -1,
                          -1,
                          NULL,
                          STRING_TOKEN (STR_TOO_MANY_PARAMETERS),
                          gGpioEg20tHiiHandle );
        DisplayHelp = TRUE;
        Status = EFI_INVALID_PARAMETER;
      }
      else if ( Argc < ( 2 + mCommands [ CommandIndex ].RequiredParameters )) {
        ShellPrintHiiEx ( -1,
                          -1,
                          NULL,
                          STRING_TOKEN (STR_TOO_FEW_PARAMETERS),
                          gGpioEg20tHiiHandle );
        DisplayHelp = TRUE;
        Status = EFI_INVALID_PARAMETER;
      }
    }
  }

  //
  //  Get the port number
  //
  if ( 3 <= Argc ) {
    Status = ShellConvertStringToUint64 ( Argv [ 2 ], &PortNumber, FALSE, FALSE );
    if ( EFI_ERROR ( Status )) {
      ShellPrintHiiEx ( -1,
                        -1,
                        NULL,
                        STRING_TOKEN (STR_BAD_PORT_NUMBER),
                        gGpioEg20tHiiHandle,
                        Status );
      DisplayHelp = TRUE;
    }
    else {
      if ( mMaxPortNumber <= PortNumber ) {
        ShellPrintHiiEx ( -1,
                          -1,
                          NULL,
                          STRING_TOKEN (STR_INVALID_PORT),
                          gGpioEg20tHiiHandle,
                          mMaxPortNumber );
        Status = EFI_INVALID_PARAMETER;
      }
    }
  }
//Print ( L"PortNumber: %d\r\n", PortNumber );

  //
  //  Get the new value
  //
  if ( 4 <= Argc ) {
    Status = ShellConvertStringToUint64 ( Argv [ 3 ], &Value, FALSE, FALSE );
    if ( EFI_ERROR ( Status )) {
      ShellPrintHiiEx ( -1,
                        -1,
                        NULL,
                        STRING_TOKEN (STR_BAD_VALUE),
                        gGpioEg20tHiiHandle,
                        Status );
      DisplayHelp = TRUE;
    }
    else if ((( 1 < Value ) && ( 0 != StrCmp ( L"imode", Command )))
            || (( 4 < Value ) && ( 0 == StrCmp ( L"imode", Command )))) {
      ShellPrintHiiEx ( -1,
                        -1,
                        NULL,
                        STRING_TOKEN (STR_UNSUPPORTED_VALUE),
                        gGpioEg20tHiiHandle );
      DisplayHelp = TRUE;
      Status = EFI_INVALID_PARAMETER;
    }
  }
//Print ( L"Value: %d\r\n", Value );

  //
  //  Display the help text
  //
  if ( DisplayHelp ) {
    ShellPrintHelp ( Argv [ 0 ], NULL, FALSE );
/*
    ShellPrintHiiEx ( -1,
                      -1,
                      NULL,
                      STRING_TOKEN (STR_GET_HELP_GPIO_EG20T),
                      gGpioEg20tHiiHandle );
*/
  }
  
  //
  //  Exit for help or errors
  //
  if (( !DisplayHelp ) && ( !EFI_ERROR ( Status ))) {
    //
    //  Process the command
    //
    if ( 0 == StrCmp ( L"addr", Command )) {
      Status = Eg20tGpioBaseAddress ( &GpioBaseAddress );
      if ( !EFI_ERROR ( Status )) {
        Print ( L"GPIO base address: 0x%016Lx\r\n", (UINT64)GpioBaseAddress );
      }
    }
    else if ( 0 == StrCmp ( L"config", Command )) {
      Print ( L"               Dir In Out  En  Md Msk Pnd Req\n" );
      if ( -1 == PortNumber ) {
        //
        //  Display the configuration for all of the GPIO ports
        //
        for ( PortNumber = 0; mMaxPortNumber > PortNumber; PortNumber += 1 ) {
          Status = GpioEg20tConfig ( PortNumber, mPinName [ PortNumber ]);
          if ( EFI_ERROR ( Status )) {
            break;
          }
        }
      }
      else {
        //
        //  Display the configuration for the specific GPIO port
        //
        Status = GpioEg20tConfig ( PortNumber, mPinName [ PortNumber ]);
      }
      //
      //  Determine if the controller is reset
      //
      if ( !EFI_ERROR ( Status )) {
        Status = Eg20tGpioSoftResetStatus ( & Reset );
        if ( Reset ) {
          Print ( L"Controller is being reset!\r\n" );
        }
      }
    }
    else if ( 0 == StrCmp ( L"dir", Command )) {
      Status = ( 0 != Value ) ? Eg20tGpioDirectionOutput ( PortNumber )
                              : Eg20tGpioDirectionInput ( PortNumber );
    }
    else if ( 0 == StrCmp ( L"iclr", Command )) {
      Status = Eg20tGpioInterruptClear ( PortNumber );
    }
    else if ( 0 == StrCmp ( L"ienbl", Command )) {
      Status = ( 0 != Value ) ? Eg20tGpioInterruptEnable ( PortNumber )
                              : Eg20tGpioInterruptDisable ( PortNumber );
    }
    else if ( 0 == StrCmp ( L"imask", Command )) {
      Status = ( 0 != Value ) ? Eg20tGpioInterruptMaskSet ( PortNumber )
                              : Eg20tGpioInterruptMaskClear ( PortNumber );
    }
    else if ( 0 == StrCmp ( L"imode", Command )) {
      Status = Eg20tGpioInterruptMode ( PortNumber,
                                        Value );
    }
    else if ( 0 == StrCmp ( L"output", Command )) {
      Status = ( 0 != Value ) ? Eg20tGpioOutputSet ( PortNumber )
                              : Eg20tGpioOutputClear ( PortNumber );
    }
    else if ( 0 == StrCmp ( L"reset", Command )) {
      Status = Eg20tGpioSoftReset ( PortNumber );
    }
  }

  //
  //  Return the command status
  //
  return Status;
}

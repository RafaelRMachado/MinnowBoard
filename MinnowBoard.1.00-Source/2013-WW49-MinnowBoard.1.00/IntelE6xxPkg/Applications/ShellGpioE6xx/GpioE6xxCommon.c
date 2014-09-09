/** @file
  E6xx GPIO application
  
  Copyright (c) 2012-2013, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "GpioE6xx.h"


STATIC CONST GPIO_COMMAND mCommands [ ] = {
  { 1, 1, L"addr" },
  { 1, 2, L"config" },
  { 3, 3, L"dir" },
  { 3, 3, L"gpe" },
  { 3, 3, L"neg" },
  { 3, 3, L"output" },
  { 3, 3, L"port" },
  { 3, 3, L"pos" },
  { 3, 3, L"smi" }
};
STATIC CONST CHAR16 * mPinNameCore [ ] = {
  L"[E6]",
  L"[C4]",
  L"[A6]",
  L"[D5]",
  L"[B5]"
};
STATIC CONST CHAR16 * mPinNameResume [ ] = {
  L"[L10]",
  L"[N4]",
  L"[P1]",
  L"[M3]",
  L"[K11]",
  L"[M1]",
  L"[L2]",
  L"[J4]",
  L"[H1]"
};
STATIC CONST UINT64 mMaxPorts [ ] = {
  DIM ( mPinNameCore ),
  DIM ( mPinNameResume )
};
STATIC CONST CHAR16 ** mPinNames [ ] = {
  &mPinNameCore [ 0 ],
  &mPinNameResume [ 0 ]
};
STATIC CONST CHAR16 * mPortType [ ] = {
  L"",
  L"_SUS"
};

EFI_HANDLE gGpioE6xxHiiHandle = NULL;

// {E31E3C4B-0D56-4b7e-8689-FF1D40FE9CE6}
STATIC CONST GUID mGpioE6xxGuid = 
{ 0xe31e3c4b, 0xd56, 0x4b7e, { 0x86, 0x89, 0xff, 0x1d, 0x40, 0xfe, 0x9c, 0xe6 } };

VOID
GpioE6xxGetHiiHandle (
  VOID
  )
{
  if ( NULL == gGpioE6xxHiiHandle ) {
    gGpioE6xxHiiHandle = HiiAddPackages ( &mGpioE6xxGuid,
                                          gImageHandle,
                                          STRING_ARRAY_NAME,
                                          NULL );
  }
}

EFI_STATUS
GpioE6xxConfig (
  IN UINT64 ControllerNumber,
  IN UINT64 PortNumber,
  IN CONST CHAR16 * PortType,
  IN CONST CHAR16 * PinName
  )
{
  BOOLEAN EdgeNegative;
  BOOLEAN EdgePositive;
  BOOLEAN Enable;
  BOOLEAN Gpe;
  BOOLEAN Input;
  UINTN Level;
  BOOLEAN Smi;
  EFI_STATUS Status;
  BOOLEAN Trigger;
  UINTN Value;

  for ( ; ; ) {
    //
    //  Get the port direction
    //
    Status = E6xxGpioPinEnable ( ControllerNumber, PortNumber, &Value );
    if ( EFI_ERROR ( Status )) {
      break;
    }
    Enable = (BOOLEAN)Value;

    //
    //  Get the port direction
    //
    Status = E6xxGpioPinDirection ( ControllerNumber, PortNumber, &Value );
    if ( EFI_ERROR ( Status )) {
      break;
    }
    Input = (BOOLEAN)Value;

    //
    //  Get the port level
    //
    Status = E6xxGpioPinLevel ( ControllerNumber, PortNumber, &Level );
    if ( EFI_ERROR ( Status )) {
      break;
    }

    //
    //  Get the negative trigger level
    //
    Status = E6xxGpioPinNegativeEdge ( ControllerNumber, PortNumber, &Value );
    if ( EFI_ERROR ( Status )) {
      break;
    }
    EdgeNegative = (BOOLEAN)Value;

    //
    //  Get the positive trigger level
    //
    Status = E6xxGpioPinPositiveEdge ( ControllerNumber, PortNumber, &Value );
    if ( EFI_ERROR ( Status )) {
      break;
    }
    EdgePositive = (BOOLEAN)Value;

    //
    //  Get the port GPE
    //
    Status = E6xxGpioPinGpe ( ControllerNumber, PortNumber, &Value );
    if ( EFI_ERROR ( Status )) {
      break;
    }
    Gpe = (BOOLEAN)Value;

    //
    //  Get the port SMI
    //
    Status = E6xxGpioPinSmi ( ControllerNumber, PortNumber, &Value );
    if ( EFI_ERROR ( Status )) {
      break;
    }
    Smi = (BOOLEAN)Value;

    //
    //  Get the port trigger status
    //
    Status = E6xxGpioPinTriggerStatus ( ControllerNumber, PortNumber, &Value);
    if ( EFI_ERROR ( Status )) {
      break;
    }
    Trigger = (BOOLEAN)Value;

    //
    //  Display the pin configuration
    //
    Print ( L"GPIO%s%Ld %-5s:  %s %d",
            PortType,
            PortNumber,
            PinName,
            Input ? L"Input " : L"Output",
            Level );
    Print ( L" %s", Enable ? L"Enabled " : L"Disabled" );
    if ( Gpe ) {
      Print ( L" GPE" );
    }
    if ( Smi ) {
      Print ( L" SMI" );
    }
    if ( EdgePositive ) {
      Print ( L" Pos" );
    }
    if ( EdgeNegative ) {
      Print ( L" Neg" );
    }
    if ( Trigger ) {
      Print ( L" Int" );
    }
    Print ( L"\r\n" );
    break;
  }

  //
  //  Return the operation status
  //
  return Status;
}


EFI_STATUS
GpioE6xxMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{
  CONST CHAR16 * Command;
  UINTN CommandIndex;
  UINT64 ControllerNumber;
  BOOLEAN DisplayHelp;
  UINTN GpioBaseAddress;
  UINT64 MaxPortNumber;
  UINT64 PortNumber;
  EFI_STATUS Status;
  UINT64 Value;

  //
  //  Validate the arrays
  //
  ASSERT ( DIM ( mMaxPorts ) == DIM ( mPortType ));
  ASSERT ( DIM ( mMaxPorts ) == DIM ( mPinNames ));

  //
  //  Get the HII handle
  //
  GpioE6xxGetHiiHandle ( );
  ASSERT ( NULL != gGpioE6xxHiiHandle );

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
                        gGpioE6xxHiiHandle );
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
                          gGpioE6xxHiiHandle );
        DisplayHelp = TRUE;
        Status = EFI_INVALID_PARAMETER;
      }
      else if ( Argc < ( 2 + mCommands [ CommandIndex ].RequiredParameters )) {
        ShellPrintHiiEx ( -1,
                          -1,
                          NULL,
                          STRING_TOKEN (STR_TOO_FEW_PARAMETERS),
                          gGpioE6xxHiiHandle );
        DisplayHelp = TRUE;
        Status = EFI_INVALID_PARAMETER;
      }
    }
  }

  //
  //  Get the controller number
  //
  MaxPortNumber = 0;
  ControllerNumber = DIM ( mMaxPorts );
  if ( 3 <= Argc ) {
    Status = ShellConvertStringToUint64 ( Argv [ 2 ], &ControllerNumber, FALSE, FALSE );
    if ( EFI_ERROR ( Status )) {
      ShellPrintHiiEx ( -1,
                        -1,
                        NULL,
                        STRING_TOKEN (STR_BAD_CONTROLLER_NUMBER),
                        gGpioE6xxHiiHandle,
                        Status );
      DisplayHelp = TRUE;
    }
    else {
      if ( DIM ( mMaxPorts ) <= ControllerNumber )
      {
        ShellPrintHiiEx ( -1,
                          -1,
                          NULL,
                          STRING_TOKEN (STR_INVALID_CONTROLLER),
                          gGpioE6xxHiiHandle,
                          DIM ( mMaxPorts ));
        Status = EFI_INVALID_PARAMETER;
      }
      else {
        MaxPortNumber = mMaxPorts [ ControllerNumber ];
      }
    }
  }
//Print ( L"ControllerNumber: %d\r\n", ControllerNumber );
//Print ( L"MaxPortNumber: %d\r\n", MaxPortNumber );

  //
  //  Get the port number
  //
  if ( 4 <= Argc ) {
    Status = ShellConvertStringToUint64 ( Argv [ 3 ], &PortNumber, FALSE, FALSE );
    if ( EFI_ERROR ( Status )) {
      ShellPrintHiiEx ( -1,
                        -1,
                        NULL,
                        STRING_TOKEN (STR_BAD_PORT_NUMBER),
                        gGpioE6xxHiiHandle,
                        Status );
      DisplayHelp = TRUE;
    }
    else {
      if ( MaxPortNumber <= PortNumber ) {
        ShellPrintHiiEx ( -1,
                          -1,
                          NULL,
                          STRING_TOKEN (STR_INVALID_PORT),
                          gGpioE6xxHiiHandle,
                          MaxPortNumber );
        Status = EFI_INVALID_PARAMETER;
      }
    }
  }
//Print ( L"PortNumber: %d\r\n", PortNumber );

  //
  //  Get the new value
  //
  if ( 5 <= Argc ) {
    Status = ShellConvertStringToUint64 ( Argv [ 4 ], &Value, FALSE, FALSE );
    if ( EFI_ERROR ( Status )) {
      ShellPrintHiiEx ( -1,
                        -1,
                        NULL,
                        STRING_TOKEN (STR_BAD_VALUE),
                        gGpioE6xxHiiHandle,
                        Status );
      DisplayHelp = TRUE;
    }
    else if ( 1 < Value ) {
      ShellPrintHiiEx ( -1,
                        -1,
                        NULL,
                        STRING_TOKEN (STR_UNSUPPORTED_VALUE),
                        gGpioE6xxHiiHandle );
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
                      STRING_TOKEN (STR_GET_HELP_GPIO_E6XX),
                      gGpioE6xxHiiHandle );
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
      Status = E6xxGpioBaseAddress ( ControllerNumber, &GpioBaseAddress );
      if ( !EFI_ERROR ( Status )) {
        Print ( L"GPIO base address: 0x%016Lx\r\n", (UINT64)GpioBaseAddress );
      }
    }
    else if ( 0 == StrCmp ( L"config", Command )) {
      if ( -1 == PortNumber ) {
        //
        //  Display the configuration for all of the GPIO ports
        //
        for ( PortNumber = 0; mMaxPorts [ ControllerNumber ] > PortNumber; PortNumber += 1 ) {
          Status = GpioE6xxConfig ( ControllerNumber,
                                    PortNumber,
                                    mPortType [ ControllerNumber ],
                                    ( mPinNames [ ControllerNumber ]) [ PortNumber ]);
          if ( EFI_ERROR ( Status )) {
            break;
          }
        }
      }
      else {
        //
        //  Display the configuration for the specific GPIO port
        //
        Status = GpioE6xxConfig ( ControllerNumber,
                                  PortNumber,
                                  mPortType [ ControllerNumber ],
                                  ( mPinNames [ ControllerNumber ]) [ PortNumber ]);
      }
    }
    else if ( 0 == StrCmp ( L"dir", Command )) {
      Status = ( 0 != Value ) ? E6xxGpioDirectionInput ( ControllerNumber,
                                                         PortNumber )
                              : E6xxGpioDirectionOutput ( ControllerNumber,
                                                          PortNumber );
    }
    else if ( 0 == StrCmp ( L"gpe", Command )) {
      Status = ( 0 != Value ) ? E6xxGpioGpeEnable ( ControllerNumber,
                                                    PortNumber )
                              : E6xxGpioGpeDisable ( ControllerNumber,
                                                     PortNumber );
    }
    else if ( 0 == StrCmp ( L"neg", Command )) {
      Status = ( 0 != Value ) ? E6xxGpioNegativeEdgeEnable ( ControllerNumber,
                                                             PortNumber )
                              : E6xxGpioNegativeEdgeDisable ( ControllerNumber,
                                                              PortNumber );
    }
    else if ( 0 == StrCmp ( L"output", Command )) {
      Status = ( 0 != Value ) ? E6xxGpioOutputSet ( ControllerNumber,
                                                    PortNumber )
                              : E6xxGpioOutputClear ( ControllerNumber,
                                                      PortNumber );
    }
    else if ( 0 == StrCmp ( L"port", Command )) {
      Status = ( 0 != Value ) ? E6xxGpioPortEnable ( ControllerNumber,
                                                     PortNumber )
                              : E6xxGpioPortDisable ( ControllerNumber,
                                                      PortNumber );
    }
    else if ( 0 == StrCmp ( L"pos", Command )) {
      Status = ( 0 != Value ) ? E6xxGpioPositiveEdgeEnable ( ControllerNumber,
                                                             PortNumber )
                              : E6xxGpioPositiveEdgeDisable ( ControllerNumber,
                                                              PortNumber );
    }
    else if ( 0 == StrCmp ( L"smi", Command )) {
      Status = ( 0 != Value ) ? E6xxGpioSmiEnable ( ControllerNumber,
                                                    PortNumber )
                              : E6xxGpioSmiDisable ( ControllerNumber,
                                                     PortNumber );
    }
  }

  //
  //  Return the command status
  //
  return (INTN)Status;
}

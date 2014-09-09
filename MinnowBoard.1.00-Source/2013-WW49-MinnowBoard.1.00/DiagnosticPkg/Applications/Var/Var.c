/** @file
  Variable test program
  
  Copyright (c) 2012-2013, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Uefi.h>
#include <Library/AsciiDump.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/HiiLib.h>
#include <Library/ShellCommandLib.h>
#include <Library/ShellCEntryLib.h>
#include <Library/ShellLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Protocol/EfiShellParameters.h>

#define DIM(x)      ( sizeof ( x ) / sizeof ( x [ 0 ]))

STATIC EFI_HII_HANDLE mHiiHandle;
STATIC CONST EFI_GUID mAppGuid = { 0x734d2deb, 0x4f53, 0x487f, { 0xa2, 0xb2, 0x81, 0x71, 0x3a, 0x20, 0x46, 0x1f } };
STATIC EFI_GUID mVendorGuid = { 0x916cd177, 0x477f, 0x4cd0, { 0x92, 0x47, 0x2b, 0xaf, 0x37, 0xce, 0x1e, 0x5a } };
STATIC CHAR16 mVariableNameBuffer [ 16384 ];
STATIC UINT8 mBuffer [ 16384 ];

VOID
GetHiiHandle (
  VOID
  )
{
  if ( NULL == mHiiHandle ) {
    mHiiHandle = HiiAddPackages ( &mAppGuid,
                                  gImageHandle,
                                  STRING_ARRAY_NAME,
                                  NULL );
  }
}


INTN
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{
  UINTN Attributes;
  UINTN ArgIndex;
  UINTN BufferLengthInBytes;
  BOOLEAN Delete;
  BOOLEAN DisplayAll;
  BOOLEAN DisplayHelp;
  EFI_GUID GuidBuffer;
  EFI_STATUS Status;
  EFI_GUID * VendorGuid;
  CHAR16 * VariableName;
  CHAR16 * VariableData;

  //
  // initialize the shell lib (we must be in non-auto-init...)
  //
  for ( ; ; ) {
    Status = ShellInitialize ( );
    if ( EFI_ERROR ( Status )) {
      break;
    }

    //
    //  Get the HII handle
    //
    GetHiiHandle ( );
    if ( NULL == mHiiHandle ) {
      Status = EFI_NO_MAPPING;
      ASSERT ( NULL != mHiiHandle );
      break;
    }

    //
    //  Display help by default
    //
    DisplayHelp = TRUE;

    //
    //  Get the command attributes
    //
    ArgIndex = 1;
    Attributes = EFI_VARIABLE_BOOTSERVICE_ACCESS;
    Delete = FALSE;
    DisplayAll = FALSE;
    VendorGuid = &mVendorGuid;
    VariableName = NULL;
    VariableData = NULL;
    while ( ArgIndex < Argc ) {
      //
      //  -all: Display all variables
      //
      if ( 0 == StrCmp ( L"-all", Argv [ ArgIndex ])) {
        ArgIndex += 1;
        DisplayAll = TRUE;
        continue;
      }

      //
      //  -nv: Store in non-volatile storage
      //
      if ( 0 == StrCmp ( L"-nv", Argv [ ArgIndex ])) {
        ArgIndex += 1;
        Attributes |= EFI_VARIABLE_NON_VOLATILE;
        continue;
      }

      //
      //  -rt: Runtime access
      //
      if ( 0 == StrCmp ( L"-rt", Argv [ ArgIndex ])) {
        ArgIndex += 1;
        Attributes |= EFI_VARIABLE_RUNTIME_ACCESS;
        continue;
      }

      //
      //  -delete: Delete a variable
      //
      if ( 0 == StrCmp ( L"-delete", Argv [ ArgIndex ])) {
        Delete = TRUE;
        ArgIndex += 1;
        Attributes = 0;
        break;
      }

      //
      //  No more qualifiers
      //
      if ( '-' == *Argv [ ArgIndex ]) {
        ShellPrintHiiEx ( -1,
                          -1,
                          NULL,
                          STRING_TOKEN (STR_UNSUPPORTED_QUALIFIER),
                          mHiiHandle,
                          Argv [ ArgIndex ]);
        Status = EFI_INVALID_PARAMETER;
        break;
      }

      break;
    }
    if ( !EFI_ERROR ( Status )) {
      //
      //  Verify the name and value
      //
      VariableName = Argv [ ArgIndex ];
      if ( Delete && ( 1 == ( Argc - ArgIndex ))) {
        DisplayHelp = FALSE;
      }
      else if (( !Delete ) && ( 2 == ( Argc - ArgIndex ))) {
        DisplayHelp = FALSE;
        VariableData = Argv [ ArgIndex + 1 ];
      }
      else if ( 2 <= ( Argc - ArgIndex )) {
        ShellPrintHiiEx ( -1,
                          -1,
                          NULL,
                          STRING_TOKEN (STR_TOO_MANY_PARAMETERS),
                          mHiiHandle );
        Status = EFI_INVALID_PARAMETER;
      }
      else {
        ShellPrintHiiEx ( -1,
                          -1,
                          NULL,
                          STRING_TOKEN (STR_TOO_FEW_PARAMETERS),
                          mHiiHandle );
        Status = EFI_INVALID_PARAMETER;
      }
    }
    if ( DisplayHelp ) {
      ShellPrintHiiEx ( -1,
                        -1,
                        NULL,
                        STRING_TOKEN(STR_GET_HELP_VAR),
                        mHiiHandle );
    }
    else if ( !EFI_ERROR ( Status )) {
      BufferLengthInBytes = 0;
      if ( NULL != VariableData ) {
        BufferLengthInBytes = ( StrLen ( VariableData ) + 1 ) << 1;
      }
      Status = gRT->SetVariable ( VariableName,
                                  VendorGuid,
                                  Attributes,
                                  BufferLengthInBytes,
                                  VariableData );
      if ( EFI_ERROR ( Status )) {
        ShellPrintHiiEx ( -1,
                          -1,
                          NULL,
                          STRING_TOKEN (STR_SET_VARIABLE_FAILED),
                          mHiiHandle,
                          Status );
      }
      else {
        //
        //  Display the variables
        //
        VariableName = &mVariableNameBuffer [ 0 ];
        mVariableNameBuffer [ 0 ] = 0;
        do {
          BufferLengthInBytes = sizeof ( mVariableNameBuffer );
          Status = gRT->GetNextVariableName ( &BufferLengthInBytes,
                                              VariableName,
                                              &GuidBuffer );
          if ( !EFI_ERROR ( Status )) {
            if ( DisplayAll || ( 0 == CompareMem ( &mVendorGuid,
                                                   &GuidBuffer,
                                                   sizeof ( GuidBuffer )))) {
              Print ( L"\r\n%08x-%04x-%04x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x (0x%08x): %s\r\n",
                      GuidBuffer.Data1,
                      GuidBuffer.Data2,
                      GuidBuffer.Data3,
                      GuidBuffer.Data4 [ 0 ],
                      GuidBuffer.Data4 [ 1 ],
                      GuidBuffer.Data4 [ 2 ],
                      GuidBuffer.Data4 [ 3 ],
                      GuidBuffer.Data4 [ 4 ],
                      GuidBuffer.Data4 [ 5 ],
                      GuidBuffer.Data4 [ 6 ],
                      GuidBuffer.Data4 [ 7 ],
                      Attributes,
                      VariableName );
              BufferLengthInBytes = sizeof ( mBuffer );
              Status = gRT->GetVariable ( VariableName,
                                          &GuidBuffer,
                                          &Attributes,
                                          &BufferLengthInBytes,
                                          &mBuffer );
              if ( !EFI_ERROR ( Status )) {
                AsciiDump ( NULL, mBuffer, BufferLengthInBytes );
              }
            }
          }
        } while ( !EFI_ERROR ( Status ));
      }
    }
    break;
  }

  //
  //  Return the command status
  //
  return Status;
}

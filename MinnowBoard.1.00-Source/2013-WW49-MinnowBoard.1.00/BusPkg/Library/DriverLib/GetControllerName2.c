/** @file
  Common GetControllerName2 implementation

  Copyright (c) 2011 - 2013, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Library/DriverLib.h>


/**
  Retrieves a string that is the user readable name of
  the controller that is being managed by an EFI Driver.

  @param  This             A pointer to the
                           EFI_COMPONENT_NAME2_PROTOCOL instance.

  @param  ControllerHandle The handle of a controller that the
                           driver specified by This is managing.
                           This handle specifies the controller
                           whose name is to be returned.

  @param  ChildHandle      The handle of the child controller to
                           retrieve the name of.  This is an
                           optional parameter that may be NULL.
                           It will be NULL for device drivers.
                           It will also be NULL for bus
                           drivers that wish to retrieve the
                           name of the bus controller.  It will
                           not be NULL for a bus driver that
                           wishes to retrieve the name of a
                           child controller.

  @param  Language         A pointer to a Null-terminated ASCII
                           string array indicating the language.
                           This is the language of the driver
                           name that the caller is requesting,
                           and it must match one of the
                           languages specified in
                           SupportedLanguages. The number of
                           languages supported by a driver is up
                           to the driver writer. Language is
                           specified in RFC 4646 language code
                           format.

  @param  ControllerName   A pointer to the string to return.
                           This string is the name of the controller
                           specified by ControllerHandle and ChildHandle
                           in the language specified by Language
                           from the point of view of the driver
                           specified by This.

  @retval EFI_SUCCESS           The string for the user
                                readable name in the language
                                specified by Language for the
                                driver specified by This was
                                returned in DriverName.

  @retval EFI_INVALID_PARAMETER ControllerHandle is NULL.

  @retval EFI_INVALID_PARAMETER ChildHandle is not NULL and it
                                is not a valid EFI_HANDLE.

  @retval EFI_INVALID_PARAMETER Language is NULL.

  @retval EFI_INVALID_PARAMETER ControllerName is NULL.

  @retval EFI_UNSUPPORTED       The driver specified by This is
                                not currently managing the
                                controller specified by
                                ControllerHandle and
                                ChildHandle.

  @retval EFI_UNSUPPORTED       The driver specified by This
                                does not support the language
                                specified by Language.

**/
EFI_STATUS
DlGetControllerName2 (
  IN EFI_COMPONENT_NAME2_PROTOCOL *This,
  IN  EFI_HANDLE                  ControllerHandle,
  IN  EFI_HANDLE                  ChildHandle        OPTIONAL,
  IN  CHAR8                       *Language,
  OUT CHAR16                      **ControllerName
  )
{
  EFI_STATUS Status;

  //
  //  Get the controller name
  //
  Status = LookupUnicodeString2 (
             Language,
             This->SupportedLanguages,
             mDriverLib.ControllerNameStringTable,
             ControllerName,
             FALSE
             );

  //
  //  Return the lookup status
  //
  return Status;
}

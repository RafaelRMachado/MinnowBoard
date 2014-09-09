/** @file
  Locate an ACPI device path node at the end of a device path

  Various drivers and applications use this API to locate the
  device that they are interested in manipulating.

  Copyright (c) 2012-2013, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Library/DriverLib.h>
#include <Library/DevicePathLib.h>

/**
  Locate a matching ACPI device path node

  This routine walks the device path attached to the ControllerHandle
  and determines if the last (non-end) device path node is an
  ACPI_HID_DEVICE_PATH node and if the CID or _CIDSTR values
  match the specified values.

  @param [in] CompatibleIdentification  The value to match against the CID
                                        value in the ACPI_HID_DEVICE_PATH
                                        node.  This value must be zero when
                                        the CompatibleIdentification
                                        value is not NULL.
  @param [in] CompatibleIdentificationString  This value is specified as NULL
                                              when the CompatibleIdentification
                                              value is non-zero.  When the
                                              CompatibleIdentification value is
                                              zero (0), this value should point
                                              to a zero terminated charater
                                              string value.

  @return           When the ACPI device path node is found, this routine
                    returns the pointer to the ACPI_HID_DEVICE_PATH node.
                    Otherwise when the device path is not found this routine
                    returns NULL.

**/
CONST ACPI_HID_DEVICE_PATH *
EFIAPI
DlAcpiFindDeviceWithMatchingCid (
  EFI_HANDLE ControllerHandle,
  UINTN CompatibleIdentification,
  CONST CHAR8 * CompatibleIdentificationString OPTIONAL
  )
{
  ACPI_EXTENDED_HID_DEVICE_PATH *AcpiPath;
  CHAR8 *CidStr;
  EFI_DEVICE_PATH_PROTOCOL *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL *EndPath;

  //
  //  Locate the last node in the device path
  //
  EndPath = NULL;
  DevicePath = DevicePathFromHandle ( ControllerHandle );
  if ( NULL == DevicePath ) {
    DEBUG (( DEBUG_INFO, "INFO - No device path for this controller!\r\n" ));
  }
  else {
    EndPath = DevicePath;
    while ( !IsDevicePathEnd ( EndPath )) {
      //
      //  Skip this portion of the device path
      //
      DevicePath = EndPath;
      if ( sizeof ( *DevicePath ) > DevicePathNodeLength ( EndPath )) {
        //
        //  Invalid device path node
        //
        DEBUG (( DEBUG_ERROR,
                  "ERROR - Invalid device path found at 0x%016Lx\r\n",
                  (UINT64)(UINTN) EndPath ));
        return NULL;
      }
      EndPath = NextDevicePathNode ( EndPath );
    }

    //
    //  Determine if this device is supported
    //
    EndPath = NULL;
    AcpiPath = (ACPI_EXTENDED_HID_DEVICE_PATH *)DevicePath;
    if ( ACPI_DEVICE_PATH != DevicePathType ( DevicePath )) {
        DEBUG (( DEBUG_INFO, "INFO - Not an ACPI device path node!\r\n" ));
    }
    else {
      if ( ACPI_EXTENDED_DP != DevicePathSubType ( DevicePath )) {
        DEBUG (( DEBUG_INFO, "INFO - ACPI device path node does not include the CID field!\r\n" ));
      }
      else {
        if ( AcpiPath->CID != CompatibleIdentification ) {
          DEBUG (( DEBUG_INFO,
                    "INFO - The CID value (%d) does not match %d!\r\n",
                    AcpiPath->CID,
                    CompatibleIdentification ));
        }
        else {
          if ( 0 != AcpiPath->CID ) {
            DEBUG (( DEBUG_INFO, "INFO - Found device using CID value\r\n" ));
            EndPath = DevicePath;
          }
          else {
            if ( NULL != CompatibleIdentificationString ) {
              //
              //  Skip over the HID
              //
              CidStr = (CHAR8*)( AcpiPath + 1 );
              CidStr += AsciiStrLen ( CidStr ) + 1;
              
              //
              //  Skip over the UID
              //
              CidStr += AsciiStrLen ( CidStr ) + 1;

              //
              //  Validate the CID
              //
              if ( 0 == AsciiStrCmp ( CidStr, CompatibleIdentificationString )) {
                DEBUG (( DEBUG_INFO, "INFO - Found device using _CIDSTR value\r\n" ));
                EndPath = DevicePath;
              }
              else {
                DEBUG (( DEBUG_INFO,
                          "INFO - _CIDSTR value (%a) does not match %a!\r\n",
                          CidStr,
                          CompatibleIdentificationString ));
              }
            }
            else {
              DEBUG (( DEBUG_ERROR, "ERROR - CompatibleIdentificationString must be non-NULL for comparison!\r\n" ));
            }
          }
        }
      }
    }
  }

  //
  //  The ACPI device path node that was found
  //
  return (ACPI_HID_DEVICE_PATH *)EndPath;
}

//
// This file contains an 'Intel Peripheral Driver' and is
// licensed for Intel CPUs and chipsets under the terms of your
// license agreement with Intel or your vendor.  This file may
// be modified by the user, subject to additional terms of the
// license agreement
//
/** @file

Copyright (c) 2006 - 2013, Intel Corporation. All rights reserved.<BR>
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  Spi.h

Abstract:

  This file defines the EFI SPI Protocol which implements the
  Intel(R) ICH SPI Host Controller Compatibility Interface.

**/
#ifndef _EFI_SPI_H_
#define _EFI_SPI_H_

#include <Protocol/SpiFlashPart.h>

//
// Define the SPI protocol GUID
//
#define EFI_SPI_PROTOCOL_GUID \
  {0x8035CD68, 0x9363, 0x11DF, {0x88, 0x50, 0xAB, 0x1C, 0x04, 0xCE, 0xFC, 0xE0}}

//
// Extern the GUID for protocol users.
//
extern EFI_GUID gEfiSpiProtocolGuid;

//
// Number of prefix opcode bytes
//
#define SPI_NUM_PREFIX_OPCODE 2

//
// Forward reference for ANSI C compatibility
//
typedef struct _EFI_SPI_PROTOCOL  EFI_SPI_PROTOCOL;

typedef enum {
  EnumSpiRegionAll,
  EnumSpiRegionBios,
  EnumSpiRegionMe,
  EnumSpiRegionGbE,
  EnumSpiRegionDescriptor,
  EnumSpiRegionPlatformData,
  EnumSpiRegionMax
} SPI_REGION_TYPE;

//
// Defines a Protected Range Register in the SPI controller used to lock regions.
//
typedef struct {
  UINT32    RangeBase;
  UINT32    RangeLimit;
  BOOLEAN   ReadProtect;
  BOOLEAN   WriteProtect;
} SPI_PRR_ENTRY;

//
// Defines the number of protected range registers in the SPI controller.  For
// this chipset registers 0 - 4 are defined.
//
#define SPI_PRR_COUNT     5

//
// Defines a set of all the Protected Range Registers and the state of the global
// lock bit.
//
typedef struct {
  BOOLEAN       FlashConfigLock;
  SPI_PRR_ENTRY PrrEntries[SPI_PRR_COUNT];
} SPI_PRR_DATA;

//
// Protocol member functions
//

/**
  Control write access to the SPI part

  @param This             Pointer to the EFI_SPI_PROTOCOL instance.
  @param Enable           TRUE enables writes to the SPI device,
                          FALSE protects the SPI device

  @return EFI_SUCCESS            Flash is write enabled
  @return EFI_INVALID_PARAMETER  This is NULL

**/
typedef
EFI_STATUS
(EFIAPI *EFI_SPI_ENABLE_WRITE) (
  IN EFI_SPI_PROTOCOL  *This,
  IN BOOLEAN           Enable
  );

/**
  Location and size of the SPI flash

  @param This             Pointer to the EFI_SPI_PROTOCOL instance.
  @param FlashOffset      The offset of the start of the BIOS image relative to the flash device.
                          Please note this is a Flash Linear Address, NOT a memory space address.
                          This value is platform specific and depends on the system flash map.
                          This value is only used on non Descriptor mode.
  @param FlashSize        The the BIOS Image size in flash. This value is platform specific
                          and depends on the system flash map. Please note BIOS Image size may
                          be smaller than BIOS Region size (in Descriptor Mode) or the flash size
                          (in Non Descriptor Mode), and in this case, BIOS Image is supposed to be
                          placed at the top end of the BIOS Region (in Descriptor Mode) or the flash
                          (in Non Descriptor Mode)

  @return EFI_SUCCESS            Flash location set successfully
  @return EFI_INVALID_PARAMETER  This is NULL

**/
typedef
EFI_STATUS
(EFIAPI *EFI_SPI_FLASH_LOCATION) (
  IN EFI_SPI_PROTOCOL  *This,
  IN UINTN             FlashOffset,
  IN UINTN             FlashSize
  );

/**
  Load an opcode description into the host controller

  @param  This                    Pointer to the EFI_SPI_PROTOCOL instance.
  @param  Opcode                  Pointer to a SPI_FLASH_PART_OPCODE_ENTRY describing the opcode
  @param  ControllerOpcodeIndex   Index into the controller's opcode table

  @retval  EFI_SUCCESS            Opcode loaded successfully
  @retval  EFI_INVALID_PARAMETER  This is NULL
  @retval  EFI_INVALID_PARAMETER  Opcode is NULL
  @retval  EFI_INVALID_PARAMETER  ControllerOpcodeIndex does not reference
                                  a valid slot in the SPI controller

**/
typedef
EFI_STATUS
(EFIAPI *EFI_SPI_LOAD_OPCODE) (
  IN EFI_SPI_PROTOCOL                   *This,
  IN CONST SPI_FLASH_PART_OPCODE_ENTRY  *Opcode,
  IN UINTN                              ControllerOpcodeIndex
  );

/**
  Load the prefix bytes into the SPI controller

  @param  This                    Pointer to the EFI
  @param  Prefix                  Opcode prefixes for the SPI controller

  @retval  EFI_SUCCESS            Opcode loaded successfully
  @retval  EFI_INVALID_PARAMETER  This is NULL

**/
typedef
EFI_STATUS
(EFIAPI *EFI_SPI_LOAD_PREFIX_BYTES) (
  IN EFI_SPI_PROTOCOL  *This,
  IN UINT8             Prefix [SPI_NUM_PREFIX_OPCODE]
  );

/**
  Lock the SPI Static Configuration Interface.
  Once locked, the interface is no longer open for configuration changes.
  The lock state automatically clears on next system reset.

  @param[in] This     Pointer to the EFI_SPI_PROTOCOL instance.

  @retval EFI_SUCCESS       Lock operation succeed.
  @retval EFI_DEVICE_ERROR  Device error, operation failed.
  @retval EFI_ACCESS_DENIED The interface has already been locked.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_SPI_LOCK) (
  IN EFI_SPI_PROTOCOL     *This
  );

/**
  Execute SPI commands from the host controller.

  @param[in] This               Pointer to the EFI_SPI_PROTOCOL instance.
  @param[in] ReadStatusIndex    Read status opcode index
  @param[in] OpcodeIndex        Index of the command in the OpCode Menu.
  @param[in] PrefixOpcodeIndex  Index of the first command to run when in an atomic cycle sequence.
  @param[in] DataCycle          TRUE if the SPI cycle contains data
  @param[in] Atomic             TRUE if the SPI cycle is atomic and interleave cycles are not allowed.
  @param[in] ShiftOut           If DataByteCount is not zero, TRUE to shift data out and FALSE to shift data in.
  @param[in] Address            In Descriptor Mode, for Descriptor Region, GbE Region, ME Region and Platform
                                Region, this value specifies the offset from the Region Base; for BIOS Region,
                                this value specifies the offset from the start of the BIOS Image. In Non
                                Descriptor Mode, this value specifies the offset from the start of the BIOS Image.
                                Please note BIOS Image size may be smaller than BIOS Region size (in Descriptor
                                Mode) or the flash size (in Non Descriptor Mode), and in this case, BIOS Image is
                                supposed to be placed at the top end of the BIOS Region (in Descriptor Mode) or
                                the flash (in Non Descriptor Mode)
  @param[in] DataByteCount      Number of bytes in the data portion of the SPI cycle.
  @param[in,out] Buffer         Pointer to caller-allocated buffer containing the dada received or sent during the SPI cycle.
  @param[in] SpiRegionType      SPI Region type. Values EnumSpiRegionBios, EnumSpiRegionGbE, EnumSpiRegionMe,
                                EnumSpiRegionDescriptor, and EnumSpiRegionPlatformData are only applicable in
                                Descriptor mode. Value EnumSpiRegionAll is applicable to both Descriptor Mode
                                and Non Descriptor Mode, which indicates "SpiRegionOffset" is actually relative
                                to base of the 1st flash device (i.e., it is a Flash Linear Address).

  @retval EFI_SUCCESS           Command succeed.
  @retval EFI_INVALID_PARAMETER The parameters specified are not valid.
  @retval EFI_UNSUPPORTED       Command not supported.
  @retval EFI_DEVICE_ERROR      Device error, command aborts abnormally.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_SPI_EXECUTE) (
  IN     EFI_SPI_PROTOCOL   *This,
  IN     UINT8              ReadStatusIndex,
  IN     UINT8              OpcodeIndex,
  IN     UINT8              PrefixOpcodeIndex,
  IN     BOOLEAN            DataCycle,
  IN     BOOLEAN            Atomic,
  IN     BOOLEAN            ShiftOut,
  IN     UINTN              Address,
  IN     UINT32             DataByteCount,
  IN OUT UINT8              *Buffer,
  IN     SPI_REGION_TYPE    SpiRegionType
  );

/**
  Gets the current state of the Protected Range Registers as well as the state
  of the Flash Configuration Lock Down bit.

  @param[in]      This    Pointer to the EFI_SPI_PROTOCOL instance.
  @param[in,out]  PrrData Pointer to a SPI_PRR_DATA structure.

  @retval EFI_SUCCESS     PRR data returned.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_GET_PRR) (
  IN      EFI_SPI_PROTOCOL  *This,
  IN OUT  SPI_PRR_DATA      *PrrData
  );

/**
  Sets the state of the Protected Range Registers.  The state of the Flash
  Configuration Lock Down bit is ignored.  To lock the configuration register
  use the lock interface.

  @param[in]      This    Pointer to the EFI_SPI_PROTOCOL instance.
  @param[in,out]  PrrData Pointer to a SPI_PRR_DATA structure.

  @retval EFI_SUCCESS         PRR data set.
  @retval EFI_ACCESS_DENIED   PRR data not set due to configuration being locked.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_SET_PRR) (
  IN      EFI_SPI_PROTOCOL  *This,
  IN OUT  SPI_PRR_DATA      *PrrData
  );

//
// Protocol definition
//
struct _EFI_SPI_PROTOCOL {
  EFI_SPI_ENABLE_WRITE       EnableWrite;
  EFI_SPI_FLASH_LOCATION     FlashLocation;
  EFI_SPI_LOAD_OPCODE        LoadOpcode;
  EFI_SPI_LOAD_PREFIX_BYTES  LoadPrefixByte;
  EFI_SPI_LOCK               Lock;
  EFI_SPI_EXECUTE            Execute;
  EFI_GET_PRR                GetPrr;
  EFI_SET_PRR                SetPrr;
};

#endif

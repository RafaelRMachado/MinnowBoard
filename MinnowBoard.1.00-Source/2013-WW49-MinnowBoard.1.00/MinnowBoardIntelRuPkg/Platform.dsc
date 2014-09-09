#
# This file contains 'Framework Code' and is licensed as such
# under the terms of your license agreement with Intel or your
# vendor.  This file may not be modified, except as allowed by
# additional terms of your license agreement.
#
## @file
# MinnowBoard platform.
#
# This package provides platform specific modules.
# Copyright (c) 2010-2013, Intel Corporation.
#
# All rights reserved.
#    This software and associated documentation (if any) is furnished
#    under a license and may only be used or copied in accordance
#    with the terms of the license. Except as permitted by such
#    license, no part of this software or documentation may be
#    reproduced, stored in a retrieval system, or transmitted in any
#    form or by any means without the express written consent of
#    Intel Corporation.
#
##

################################################################################
#
# Defines Section - statements that will be processed to create a Makefile.
#
################################################################################
[Defines]
  PLATFORM_NAME                  = MinnowBoardIntelRuPkg
  PLATFORM_GUID                  = 1BEDB57A-7904-406e-8486-C89FC7FB39EE
  PLATFORM_VERSION               = 0.2
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/MinnowBoardIntelRuPkg
  SUPPORTED_ARCHITECTURES        = IA32
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = MinnowBoardIntelRuPkg/Platform.fdf
  FIX_LOAD_TOP_MEMORY_ADDRESS    = 0
  VPD_TOOL_GUID                  = 8C3D856A-9BE6-468E-850A-24F7A8D38E08

  DEFINE EFI_BINARY_DIRECTORY    = $(OUTPUT_DIRECTORY)/$(TARGET)_$(TOOL_CHAIN_TAG)

  # These macros are required for the MkBinPkg tool.
  DEFINE BUILD_NUMBER       = 1212190900
  DEFINE EMU_VARIABLE       = FALSE ## EMU Variable
  DEFINE FAST_BOOT          = TRUE  ## Use optimized boot path after first boot
  DEFINE INCLUDE_DP         = FALSE ## Display Performance
  DEFINE LOGGING            = FALSE ## DEBUG Message
  DEFINE S3_ENABLE          = TRUE  ## S3
  DEFINE SECURE_BOOT_ENABLE = TRUE
  DEFINE SMM_ENABLE         = TRUE  ## SMM Enable must be TRUE if S3 is enabled
  DEFINE SMM_VARIABLE       = FALSE ## SMM Variable SMM and EMU Variable services cannot both be active
  DEFINE STREAMLINE_BOOT    = FALSE ## Speed up booting by reducing drivers
  DEFINE SYMBOLIC_DEBUG     = FALSE ## Source Level Debug
  DEFINE TPM_ENABLE         = FALSE
  DEFINE UID_ENABLE         = FALSE
  DEFINE UNDER_DEVELOPMENT  = FALSE ## Load under development components

  !if $(S3_ENABLE) == TRUE
    DEFINE SMM_ENABLE = TRUE
  !endif

  !if $(SMM_VARIABLE) == TRUE
    DEFINE SMM_ENABLE = TRUE
    DEFINE EMU_VARIABLE = FALSE
  !endif

[BuildOptions]
  GCC:*_*_*_ASLPP_FLAGS          == -x c -E -include AutoGen.h
  GCC:*_UNIXGCC_*_CC_FLAGS       = -DMDEPKG_NDEBUG
  GCC:RELEASE_GCC44_*_CC_FLAGS   = -DMDEPKG_NDEBUG
  INTEL:RELEASE_*_*_CC_FLAGS     = /D MDEPKG_NDEBUG
  MSFT:*_*_*_CC_FLAGS            = /wd4200
  MSFT:RELEASE_*_*_CC_FLAGS      = /D MDEPKG_NDEBUG

################################################################################
#
# SKU Identification section - list of all SKU IDs supported by this
#                              Platform.
#
################################################################################

[SkuIds]
  0|DEFAULT              # The entry: 0|DEFAULT is reserved and always required.

################################################################################
#
# Select the board
#
################################################################################

[Defines]
  DEFINE BOARD_SELECT_INFORCE     = FALSE
  DEFINE TEST_LURE                = FALSE
  DEFINE TIP_BUILD                = FALSE

################################################################################
#
# Select the Shell
#
################################################################################

[Defines]
  DEFINE SHELL_BUILT      = TRUE  ## Place the built shell in the payload
  DEFINE SHELL_PREBUILT   = FALSE ## Place the prebuilt shell in the payload
  DEFINE SHELL_MIN        = FALSE ## Place the minimum version 2 shell in the payload
  DEFINE SHELL_FULL       = FALSE ## Place the full version 2 shell in the payload

################################################################################
#
# Select the SPI flash size and emulation
#
################################################################################

[Defines]
  #
  #  Select the flash part being used by the target system
  #
  DEFINE FLASH_4_MB       = TRUE  ## Using 4MB SPI Flash
  DEFINE FLASH_8_MB       = FALSE ## Using 8MB SPI Flash

  #
  #  Fake a smaller flash part if required
  #
  DEFINE FLASH_8_FOR_4    = FALSE ## Using 8MB SPI Flash as a replacement for 4MB SPI Flash

################################################################################
#
# Peripheral support
#
#  Note that the network only builds successfully when DxeNetLib and DxeNetLib.h
#  come from https://edk2.svn.sourceforge.net/svnroot/edk2/trunk/edk2/MdeModulePkg
#
################################################################################

[Defines]
  DEFINE ENABLE_ETHERNET          = TRUE      ## External Ethernet support
  DEFINE ENABLE_ETHERNET_AX88772      = FALSE     ## External USB/Ethernet adapter such as SMC Networks SMC2209
  DEFINE ENABLE_ETHERNET_AX88772B     = FALSE     ## External USB/Ethernet adapter
  DEFINE ENABLE_ETHERNET_EG20T        = TRUE      ## EG20T Ethernet adapter
  DEFINE ENABLE_NETWORKING        = TRUE      ## Enable network support, IP4 only when the following two are FALSE
  DEFINE    DUAL_NETWORK_ENABLE       = TRUE      ## IPv6 and IPv4 support
  DEFINE    ENABLE_IP6_ONLY           = FALSE     ## IPv6 support only
  DEFINE ENABLE_SNP               = FALSE

  !if $(STREAMLINE_BOOT) == TRUE

    DEFINE ENABLE_GRAPHICS        = FALSE
    DEFINE ENABLE_SD_MMC          = FALSE
    DEFINE ENABLE_USB_HOST        = FALSE

  !else   ##  STREAMLINE_BOOT

    DEFINE ENABLE_GRAPHICS        = TRUE
    DEFINE ENABLE_SD_MMC          = TRUE
    DEFINE ENABLE_USB_HOST        = TRUE

  !endif  ##  STREAMLINE_BOOT

################################################################################
#
#  Diagnostic features
#
################################################################################

  #
  # Infinite loop in SecCore to wait for ITP to set AL=0
  #
[PcdsFeatureFlag]
  gCrownBayTokenSpaceGuid.PcdFeatureSecCoreWaitForItp|FALSE

  #
  #  Test the debug UART
  #
[Defines]
  DEFINE DIAG_DEBUG_UART_OUTPUT = FALSE ## Loop forever outputting "This is a test.\r\n"
  DEFINE DIAG_OUTPUT_RESET_LOOP = FALSE ## Loop forever counting down to reset

  #
  #  Diagnostic shell commands
  #
[Defines]
  DEFINE E6XX_GPIO_CMD          = TRUE  ## Include the E6xxGpio command
  DEFINE EG20T_GPIO_CMD         = TRUE  ## Include the Eg20tGpio command
  DEFINE SHELL_DEBUG_CMDS       = TRUE  ## Include the shell's debug commands

  #
  #  Boot Phases: LED and serial output
  #
  DEFINE DIAG_BOOT_PHASES       = TRUE   ## Use LEDs and serial to indicate boot phases
  DEFINE DIAG_STARTUP_NOISE     = TRUE   ## Add PEIM to display boot phases via debug serial

[PcdsDynamicExDefault]
  !if $(DIAG_BOOT_PHASES) == TRUE
    gMinnowBoardPkgTokenSpaceGuid.PcdDiagBootPhasesLedBlinkRate|100
  !else   ##  DIAG_BOOT_PHASES
    gMinnowBoardPkgTokenSpaceGuid.PcdDiagBootPhasesLedBlinkRate|0
  !endif  ##  DIAG_BOOT_PHASES

[PcdsFeatureFlag]
  !if $(DIAG_BOOT_PHASES) == TRUE
    gMinnowBoardPkgTokenSpaceGuid.PcdDiagBootPhasesSerial|TRUE
  !endif  ##  DIAG_BOOT_PHASES

################################################################################
#
# Select the debug UART
#
################################################################################

[Defines]
  DEFINE DEBUG_UART_NUMBER    = 0       ## 0=UART0, 1=UART1, 2=UART2

  DEFINE DEBUG_UART_BAUDRATE  = 115200
  DEFINE DEBUG_UART_DATABITS  = 8
  DEFINE DEBUG_UART_PARITY    = 1
  DEFINE DEBUG_UART_STOPBITS  = 1

  #
  #  Set the default debug output
  #
[PcdsFixedAtBuild]
  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x803000c7

  #
  #  Select the UART I/O address
  #
  !if $(DEBUG_UART_NUMBER) == 2
    DEFINE DEBUG_UART_ADDRESS   = 0xA0038000
  !elseif $(DEBUG_UART_NUMBER) == 1
    DEFINE DEBUG_UART_ADDRESS   = 0xA0039000
  !else   ##  DEBUG_UART_NUMBER
    DEFINE DEBUG_UART_ADDRESS   = 0xA003A000
  !endif  ##  DEBUG_UART_NUMBER

  #
  #  Select the EG20T UART clock input frequency
  #
  !if $(BOARD_SELECT_INFORCE) == TRUE
    DEFINE DEBUG_UART_CLK     = 1843200   ##  Input frequency to the UART baud rate divider
  !else
    DEFINE DEBUG_UART_CLK     = 50000000  ##  Input frequency to the UART baud rate divider
  !endif

################################################################################
#
# Library Class section - list of all Library Classes needed by this Platform.
#
################################################################################
[LibraryClasses]
  #
  # Entry point
  #
  PeiCoreEntryPoint|MdePkg/Library/PeiCoreEntryPoint/PeiCoreEntryPoint.inf
  PeimEntryPoint|MdePkg/Library/PeimEntryPoint/PeimEntryPoint.inf
  DxeCoreEntryPoint|MdePkg/Library/DxeCoreEntryPoint/DxeCoreEntryPoint.inf
  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf

  #
  # Basic
  #
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  SynchronizationLib|MdePkg/Library/BaseSynchronizationLib/BaseSynchronizationLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLibRepStr/BaseMemoryLibRepStr.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  CpuLib|MdePkg/Library/BaseCpuLib/BaseCpuLib.inf
  IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
  PciLib|MdePkg/Library/BasePciLibPciExpress/BasePciLibPciExpress.inf
  PciCf8Lib|MdePkg/Library/BasePciCf8Lib/BasePciCf8Lib.inf
  PciExpressLib|MdePkg/Library/BasePciExpressLib/BasePciExpressLib.inf
  CacheMaintenanceLib|MdePkg/Library/BaseCacheMaintenanceLib/BaseCacheMaintenanceLib.inf
  PeCoffLib|MdePkg/Library/BasePeCoffLib/BasePeCoffLib.inf
  PeCoffGetEntryPointLib|MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf

  #
  # UEFI & PI
  #
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  UefiRuntimeLib|MdePkg/Library/UefiRuntimeLib/UefiRuntimeLib.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  UefiHiiServicesLib|MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
  HiiLib|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  UefiDecompressLib|IntelFrameworkModulePkg/Library/BaseUefiTianoCustomDecompressLib/BaseUefiTianoCustomDecompressLib.inf
  PeiServicesTablePointerLib|MdePkg/Library/PeiServicesTablePointerLibIdt/PeiServicesTablePointerLibIdt.inf
  PeiServicesLib|MdePkg/Library/PeiServicesLib/PeiServicesLib.inf
  DxeServicesLib|MdePkg/Library/DxeServicesLib/DxeServicesLib.inf
  DxeServicesTableLib|MdePkg/Library/DxeServicesTableLib/DxeServicesTableLib.inf
  UefiCpuLib|UefiCpuPkg/Library/BaseUefiCpuLib/BaseUefiCpuLib.inf

  #
  # Generic Modules
  #
  UefiUsbLib|MdePkg/Library/UefiUsbLib/UefiUsbLib.inf
  UefiScsiLib|MdePkg/Library/UefiScsiLib/UefiScsiLib.inf
  OemHookStatusCodeLib|MdeModulePkg/Library/OemHookStatusCodeLibNull/OemHookStatusCodeLibNull.inf
  CapsuleLib|IntelFrameworkModulePkg/Library/DxeCapsuleLib/DxeCapsuleLib.inf
  SecurityManagementLib|MdeModulePkg/Library/DxeSecurityManagementLib/DxeSecurityManagementLib.inf

  #
  # CPU
  #
  MtrrLib|UefiCpuPkg/Library/MtrrLib/MtrrLib.inf
  LocalApicLib|UefiCpuPkg/Library/BaseXApicLib/BaseXApicLib.inf
  TimerLib|CommonExModulePkg/Library/AcpiTimerLib/DxeAcpiTimerLib.inf

  #
  # Crypto
  #
  BaseCryptLib|CryptoPkg/Library/BaseCryptLib/BaseCryptLib.inf
  OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLib.inf
  IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf

  #
  # Peripheral & Flash
  #
  SerialPortLib|CommonExModulePkg/Library/BaseSerialPortLib16550/BaseSerialPortLib16550.inf

  #
  # Platform
  #
  FlashDeviceLib|IntelE6xxRuPkg/Library/FlashDeviceLib/FlashDeviceLib.inf
  PlatformHookLib|CommonExModulePkg/Library/PlatformHookLib/PlatformHookLib.inf
  RecoveryOemHookLib|MinnowBoardIntelRuPkg/Library/RecoveryOemHookLib/RecoveryOemHookLib.inf

  #
  # Measured Boot Lib
  #
  TpmCommLib|SecurityPkg/Library/TpmCommLib/TpmCommLib.inf
  TcgPhysicalPresenceLib|SecurityPkg/Library/DxeTcgPhysicalPresenceLib/DxeTcgPhysicalPresenceLib.inf
  PlatformSecureLib|SecurityPkg/Library/PlatformSecureLibNull/PlatformSecureLibNull.inf

  #
  # Misc
  #
  !if $(LOGGING)
    DebugLib|IntelFrameworkModulePkg/Library/PeiDxeDebugLibReportStatusCode/PeiDxeDebugLibReportStatusCode.inf
  !else
    DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  !endif

  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  PerformanceLib|MdeModulePkg/Library/DxePerformanceLib/DxePerformanceLib.inf

  !if $(SYMBOLIC_DEBUG) == TRUE
    PeCoffExtraActionLib|SourceLevelDebugPkg/Library/PeCoffExtraActionLibDebug/PeCoffExtraActionLibDebug.inf
  !else
    PeCoffExtraActionLib|MdePkg/Library/BasePeCoffExtraActionLibNull/BasePeCoffExtraActionLibNull.inf
  !endif

  DebugCommunicationLib|SourceLevelDebugPkg/Library/DebugCommunicationLibSerialPort/DebugCommunicationLibSerialPort.inf
  DebugAgentLib|MdeModulePkg/Library/DebugAgentLibNull/DebugAgentLibNull.inf

  #
  # Shell Lib
  #
  SortLib|ShellPkg/Library/UefiSortLib/UefiSortLib.inf
  !if $(TIP_BUILD) == TRUE
    ShellLib|ShellPkg/Library/UefiShellLib/UefiShellLib.inf
  !else   ##  TIP_BUILD
    ShellLib|MinnowBoardPkg/Override/ShellPkg/Library/UefiShellLib/UefiShellLib.inf
  !endif  ##  TIP_BUILD
  FileHandleLib|ShellPkg/Library/UefiFileHandleLib/UefiFileHandleLib.inf
  ShellCEntryLib|ShellPkg/Library/UefiShellCEntryLib/UefiShellCEntryLib.inf

[LibraryClasses.common.SEC]
  #
  # SEC phase
  #
  !if $(TIP_BUILD) == TRUE
    CpuExceptionHandlerLib|UefiCpuPkg/Library/CpuExceptionHandlerLib/SecPeiCpuExceptionHandlerLib.inf
  !endif  ##  TIP_BUILD
  PciLib|MdePkg/Library/BasePciLibCf8/BasePciLibCf8.inf
  DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  HobLib|MdePkg/Library/PeiHobLib/PeiHobLib.inf
  MemoryAllocationLib|MdePkg/Library/PeiMemoryAllocationLib/PeiMemoryAllocationLib.inf
  ReportStatusCodeLib|MdePkg/Library/BaseReportStatusCodeLibNull/BaseReportStatusCodeLibNull.inf
  TimerLib|CommonExModulePkg/Library/AcpiTimerLib/BaseAcpiTimerLib.inf

[LibraryClasses.common.PEI_CORE]
  #
  # PEI phase common
  #
  !if $(TIP_BUILD) == TRUE
    CpuExceptionHandlerLib|UefiCpuPkg/Library/CpuExceptionHandlerLib/SecPeiCpuExceptionHandlerLib.inf
  !endif  ##  TIP_BUILD
  PcdLib|MdePkg/Library/PeiPcdLib/PeiPcdLib.inf
  HobLib|MdePkg/Library/PeiHobLib/PeiHobLib.inf
  MemoryAllocationLib|MdePkg/Library/PeiMemoryAllocationLib/PeiMemoryAllocationLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/PeiReportStatusCodeLib/PeiReportStatusCodeLib.inf
  PerformanceLib|MdeModulePkg/Library/PeiPerformanceLib/PeiPerformanceLib.inf
  TimerLib|CommonExModulePkg/Library/AcpiTimerLib/BaseAcpiTimerLib.inf

[LibraryClasses.common.PEIM]
  #
  # PEI phase common
  #
  !if $(TIP_BUILD) == TRUE
    CpuExceptionHandlerLib|UefiCpuPkg/Library/CpuExceptionHandlerLib/SecPeiCpuExceptionHandlerLib.inf
  !endif  ##  TIP_BUILD
  PcdLib|MdePkg/Library/PeiPcdLib/PeiPcdLib.inf
  HobLib|MdePkg/Library/PeiHobLib/PeiHobLib.inf
  MemoryAllocationLib|MdePkg/Library/PeiMemoryAllocationLib/PeiMemoryAllocationLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/PeiReportStatusCodeLib/PeiReportStatusCodeLib.inf
  ExtractGuidedSectionLib|MdePkg/Library/PeiExtractGuidedSectionLib/PeiExtractGuidedSectionLib.inf
  PerformanceLib|MdeModulePkg/Library/PeiPerformanceLib/PeiPerformanceLib.inf
  BaseCryptLib|CryptoPkg/Library/BaseCryptLib/PeiCryptLib.inf
  TimerLib|CommonExModulePkg/Library/AcpiTimerLib/BaseAcpiTimerLib.inf

[LibraryClasses.common.DXE_CORE]
  !if $(TIP_BUILD) == TRUE
    CpuExceptionHandlerLib|UefiCpuPkg/Library/CpuExceptionHandlerLib/DxeSmmCpuExceptionHandlerLib.inf
  !endif  ##  TIP_BUILD
  HobLib|MdePkg/Library/DxeCoreHobLib/DxeCoreHobLib.inf
  MemoryAllocationLib|MdeModulePkg/Library/DxeCoreMemoryAllocationLib/DxeCoreMemoryAllocationLib.inf
  ExtractGuidedSectionLib|MdePkg/Library/DxeExtractGuidedSectionLib/DxeExtractGuidedSectionLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/DxeReportStatusCodeLib/DxeReportStatusCodeLib.inf
  PerformanceLib|MdeModulePkg/Library/DxeCorePerformanceLib/DxeCorePerformanceLib.inf

[LibraryClasses.common.DXE_DRIVER]
  !if $(TIP_BUILD) == TRUE
    CpuExceptionHandlerLib|UefiCpuPkg/Library/CpuExceptionHandlerLib/DxeSmmCpuExceptionHandlerLib.inf
  !endif  ##  TIP_BUILD
  PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/DxeReportStatusCodeLib/DxeReportStatusCodeLib.inf
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf

[LibraryClasses.common.DXE_RUNTIME_DRIVER]
  PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/RuntimeDxeReportStatusCodeLib/RuntimeDxeReportStatusCodeLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf
  BaseCryptLib|CryptoPkg/Library/BaseCryptLib/RuntimeCryptLib.inf

[LibraryClasses.common.UEFI_DRIVER,LibraryClasses.common.UEFI_APPLICATION]
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/DxeReportStatusCodeLib/DxeReportStatusCodeLib.inf
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf

################################################################################
#
# Pcd Section - list of all EDK II PCD Entries defined by this Platform
#
################################################################################
[PcdsFeatureFlag]
  gEfiMdeModulePkgTokenSpaceGuid.PcdUnicodeCollationSupport|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdSupportHiiImageProtocol|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutUgaSupport|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdDevicePathSupportDevicePathToText|TRUE
  gEfiMdeModulePkgTokenSpaceGuid.PcdDevicePathSupportDevicePathFromText|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdRecoveryOnIdeDisk|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdRecoveryOnFatFloppyDisk|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdRecoveryOnDataCD|FALSE

!if $(LOGGING)
  gEfiMdeModulePkgTokenSpaceGuid.PcdStatusCodeUseSerial|TRUE
!else
  gEfiMdeModulePkgTokenSpaceGuid.PcdStatusCodeUseSerial|FALSE
!endif

[PcdsFeatureFlag]
  gMinnowBoardPkgTokenSpaceGuid.PcdFeatureBdsConnectAll|TRUE
  gMinnowBoardPkgTokenSpaceGuid.PcdDiagBootPhasesSerial|TRUE
  gEfiMdeModulePkgTokenSpaceGuid.PcdDxeIplSwitchToLongMode|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdPciBusHotplugDeviceSupport|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdInstallAcpiSdtProtocol|TRUE
  gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdPlatformCsmSupport|FALSE

  gEfiCpuTokenSpaceGuid.PcdCpuPrescottFamilyFlag|FALSE
  gEfiCpuTokenSpaceGuid.PcdCpuCedarMillFamilyFlag|FALSE
  gEfiCpuTokenSpaceGuid.PcdCpuConroeFamilyFlag|FALSE
  gEfiCpuTokenSpaceGuid.PcdCpuNehalemFamilyFlag|FALSE
  gEfiCpuTokenSpaceGuid.PcdCpuSandyBridgeFamilyFlag|FALSE
  gEfiCpuTokenSpaceGuid.PcdCpuIvyBridgeFamilyFlag|FALSE
  gEfiCpuTokenSpaceGuid.PcdCpuTunnelCreekFamilyFlag|TRUE

  # Leave those power management related feature initialization to power management driver
  gEfiCpuTokenSpaceGuid.PcdCpuThermalManagementFlag|FALSE
  gEfiCpuTokenSpaceGuid.PcdCpuEnhancedCStateFlag|FALSE
  gEfiCpuTokenSpaceGuid.PcdCpuEistFlag|TRUE
  gEfiCpuTokenSpaceGuid.PcdCpuMonitorMwaitFlag|FALSE
  gEfiCpuTokenSpaceGuid.PcdCpuPeciFlag|FALSE
  gEfiCpuTokenSpaceGuid.PcdCpuHardwarePrefetcherFlag|FALSE
  gEfiCpuTokenSpaceGuid.PcdCpuAdjacentCacheLinePrefetchFlag|FALSE
  gEfiCpuTokenSpaceGuid.PcdCpuDcuPrefetcherFlag|FALSE
  gEfiCpuTokenSpaceGuid.PcdCpuIpPrefetcherFlag|FALSE
  gEfiCpuTokenSpaceGuid.PcdCpuMlcStreamerPrefetcherFlag|FALSE
  gEfiCpuTokenSpaceGuid.PcdCpuMlcSpatialPrefetcherFlag|FALSE
  gEfiCpuTokenSpaceGuid.PcdCpuGateA20MDisableFlag|FALSE
  gEfiCpuTokenSpaceGuid.PcdCpuThreeStrikeCounterFlag|FALSE
  gEfiCpuTokenSpaceGuid.PcdCpuEnergyPerformanceBiasFlag|FALSE
  gEfiCpuTokenSpaceGuid.PcdCpuTStateFlag|FALSE
  gEfiCpuTokenSpaceGuid.PcdCpuAesFlag|FALSE
  gEfiCpuTokenSpaceGuid.PcdCpuDcaFlag|FALSE
  gEfiCpuTokenSpaceGuid.PcdCpuCStateFlag|FALSE
  gEfiCpuTokenSpaceGuid.PcdCpuX2ApicFlag|FALSE
  gEfiCpuTokenSpaceGuid.PcdCpuApicTprUpdateMessageFlag|FALSE
  gEfiCpuTokenSpaceGuid.PcdCpuDcuModeSelectionFlag|FALSE

  gEfiMdePkgTokenSpaceGuid.PcdDriverDiagnosticsDisable|TRUE
  gEfiMdePkgTokenSpaceGuid.PcdComponentNameDisable|TRUE
  gEfiMdePkgTokenSpaceGuid.PcdUgaConsumeSupport|FALSE
  gEfiMdePkgTokenSpaceGuid.PcdUefiVariableDefaultLangDeprecate|TRUE

[PcdsFixedAtBuild]

  !if $(LOGGING)
    gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x27
    gEfiMdePkgTokenSpaceGuid.PcdReportStatusCodePropertyMask|0x7
  !else
    gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x0
    gEfiMdePkgTokenSpaceGuid.PcdPostCodePropertyMask|0x0
    gEfiMdePkgTokenSpaceGuid.PcdReportStatusCodePropertyMask|0x3
  !endif

  # Front Side Bus speed
  gEfiMdePkgTokenSpaceGuid.PcdFSBClock|100000000

  # turn off performance measurement support
  gEfiMdePkgTokenSpaceGuid.PcdPerformanceLibraryPropertyMask|0x1
  gEfiMdeModulePkgTokenSpaceGuid.PcdMaxPeiPerformanceLogEntries|40

  gEfiMdeModulePkgTokenSpaceGuid.PcdLoadModuleAtFixAddressEnable|0
  gEfiCpuTokenSpaceGuid.PcdTemporaryRamSize|0x4000

  gCrownBayTokenSpaceGuid.PcdLocalApicAddress                 | 0xFEE00000
  gCrownBayTokenSpaceGuid.PcdIoApicSettingGlobalInterruptBase | 0x0

  gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdLegacyBiosCacheLegacyRegion|FALSE

  # it must be FixedAtBuild, because it is used in AcpiTable.
  gEfiMdePkgTokenSpaceGuid.PcdPciExpressBaseAddress|0xE0000000

  # UefiShell File Guid 7C04A583-9E3E-4f1c-AD65-E05268D0B4D1
  gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdShellFile|{0x83, 0xA5, 0x04, 0x7C, 0x3E, 0x9E, 0x1C, 0x4F, 0xAD, 0x65, 0xE0, 0x52, 0x68, 0xD0, 0xB4, 0xD1}

  gEfiMdeModulePkgTokenSpaceGuid.PcdVpdBaseAddress|0x0

  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwSpareBase|0x0
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwSpareBase64|0x0
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwSpareSize|0x0

  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwWorkingBase|0x0
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwWorkingBase64|0x0
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwWorkingSize|0x0

  #
  # The following should be patchable in module
  #
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageVariableBase|0x0
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageVariableBase64|0x0
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageVariableSize|0x0
  gEfiMdeModulePkgTokenSpaceGuid.PcdMaxHardwareErrorVariableSize|0x2000
  gEfiMdeModulePkgTokenSpaceGuid.PcdMaxVariableSize|0x2000
  gEfiMdeModulePkgTokenSpaceGuid.PcdVariableStoreSize|0xa000

  gEfiSecurityPkgTokenSpaceGuid.PcdFixedMediaImageVerificationPolicy|0x05
  gEfiSecurityPkgTokenSpaceGuid.PcdFixedUsbCredentialProviderTokenFileName|L"Token.bin"
  gEfiSecurityPkgTokenSpaceGuid.PcdOptionRomImageVerificationPolicy|0x00
  gEfiSecurityPkgTokenSpaceGuid.PcdRemovableMediaImageVerificationPolicy|0x05

[PcdsPatchableInModule]
  gEfiMdePkgTokenSpaceGuid.PcdUefiVariableDefaultPlatformLang|"en-US"
  gEfiMdePkgTokenSpaceGuid.PcdUefiVariableDefaultPlatformLangCodes|"en;fr;en-US;fr-FR"

  gEfiMdeModulePkgTokenSpaceGuid.PcdSerialLineControl|0x03
  gEfiMdeModulePkgTokenSpaceGuid.PcdSerialBaudRate|$(DEBUG_UART_BAUDRATE)
  gEfiMdeModulePkgTokenSpaceGuid.PcdSerialFifoControl|0x27
  gEfiMdeModulePkgTokenSpaceGuid.PcdSerialDetectCable|FALSE
  gEfiMdePkgTokenSpaceGuid.PcdDefaultTerminalType|0
  gUefiCpuPkgTokenSpaceGuid.PcdCpuLocalApicBaseAddress|0xFEE00000


  gCrownBayTokenSpaceGuid.PcdPPMFunctionEnables | 0x00FF
  gMinnowBoardPkgTokenSpaceGuid.PcdBootOrderPolicyEnable |TRUE
  gMinnowBoardPkgTokenSpaceGuid.PcdBootOrderPayload | 0
  gMinnowBoardPkgTokenSpaceGuid.PcdBootOrderHD      | 1
  gMinnowBoardPkgTokenSpaceGuid.PcdBootOrderDvdCd   | 2
  gMinnowBoardPkgTokenSpaceGuid.PcdBootOrderUsb     | 3
  gMinnowBoardPkgTokenSpaceGuid.PcdBootOrderNetwork | 4

  gEfiSecurityPkgTokenSpaceGuid.PcdPhysicalPresenceLifetimeLock|FALSE
  gEfiSecurityPkgTokenSpaceGuid.PcdPhysicalPresenceCmdEnable|TRUE
  gEfiSecurityPkgTokenSpaceGuid.PcdPhysicalPresenceHwEnable|TRUE

  #
  # ACPI PCI Routing
  #
  gCrownBayTokenSpaceGuid.PcdPciDeviceInfoNumber|0x0D

  # PCI device 1
  gCrownBayTokenSpaceGuid.PcdPciDevice1Name|"Host Bridge"
  gCrownBayTokenSpaceGuid.PcdPciDevice1BridgeAddress|0
  gCrownBayTokenSpaceGuid.PcdPciDevice1DeviceAddress|0
  gCrownBayTokenSpaceGuid.PcdPciDevice1INTA|0xFFFF
  gCrownBayTokenSpaceGuid.PcdPciDevice1INTB|0xFFFF
  gCrownBayTokenSpaceGuid.PcdPciDevice1INTC|0xFFFF
  gCrownBayTokenSpaceGuid.PcdPciDevice1INTD|0xFFFF
  gCrownBayTokenSpaceGuid.PcdPciDevice1GpePin|0xFF
  gCrownBayTokenSpaceGuid.PcdPciDevice1SxNum|0xFF

  # PCI device 2
  gCrownBayTokenSpaceGuid.PcdPciDevice2Name|"Integrated Graphics and Video Device"
  gCrownBayTokenSpaceGuid.PcdPciDevice2BridgeAddress|0
  gCrownBayTokenSpaceGuid.PcdPciDevice2DeviceAddress|0x00000200
  gCrownBayTokenSpaceGuid.PcdPciDevice2INTA|0x1001
  gCrownBayTokenSpaceGuid.PcdPciDevice2INTB|0xFFFF
  gCrownBayTokenSpaceGuid.PcdPciDevice2INTC|0xFFFF
  gCrownBayTokenSpaceGuid.PcdPciDevice2INTD|0xFFFF
  gCrownBayTokenSpaceGuid.PcdPciDevice2GpePin|0xFF
  gCrownBayTokenSpaceGuid.PcdPciDevice2SxNum|0xFF

  # PCI device 3
  gCrownBayTokenSpaceGuid.PcdPciDevice3Name|"SDVO Display Unit"
  gCrownBayTokenSpaceGuid.PcdPciDevice3BridgeAddress|0
  gCrownBayTokenSpaceGuid.PcdPciDevice3DeviceAddress|0x00000300
  gCrownBayTokenSpaceGuid.PcdPciDevice3INTA|0x1001
  gCrownBayTokenSpaceGuid.PcdPciDevice3INTB|0xFFFF
  gCrownBayTokenSpaceGuid.PcdPciDevice3INTC|0xFFFF
  gCrownBayTokenSpaceGuid.PcdPciDevice3INTD|0xFFFF
  gCrownBayTokenSpaceGuid.PcdPciDevice3GpePin|0xFF
  gCrownBayTokenSpaceGuid.PcdPciDevice3SxNum|0xFF

  # PCI device 4
  gCrownBayTokenSpaceGuid.PcdPciDevice4Name|"PCI Express Port 0"
  gCrownBayTokenSpaceGuid.PcdPciDevice4BridgeAddress|0
  gCrownBayTokenSpaceGuid.PcdPciDevice4DeviceAddress|0x00001700
  gCrownBayTokenSpaceGuid.PcdPciDevice4INTA|0x1304
  gCrownBayTokenSpaceGuid.PcdPciDevice4INTB|0xFFFF
  gCrownBayTokenSpaceGuid.PcdPciDevice4INTC|0xFFFF
  gCrownBayTokenSpaceGuid.PcdPciDevice4INTD|0xFFFF
  gCrownBayTokenSpaceGuid.PcdPciDevice4GpePin|0x06
  gCrownBayTokenSpaceGuid.PcdPciDevice4SxNum|0x03

  # PCI device 5
  gCrownBayTokenSpaceGuid.PcdPciDevice5Name|"PCI Express Port 1"
  gCrownBayTokenSpaceGuid.PcdPciDevice5BridgeAddress|0
  gCrownBayTokenSpaceGuid.PcdPciDevice5DeviceAddress|0x00001800
  gCrownBayTokenSpaceGuid.PcdPciDevice5INTA|0x1405
  gCrownBayTokenSpaceGuid.PcdPciDevice5INTB|0xFFFF
  gCrownBayTokenSpaceGuid.PcdPciDevice5INTC|0xFFFF
  gCrownBayTokenSpaceGuid.PcdPciDevice5INTD|0xFFFF
  gCrownBayTokenSpaceGuid.PcdPciDevice5GpePin|0x07
  gCrownBayTokenSpaceGuid.PcdPciDevice5SxNum|0x03

  # PCI device 6
  gCrownBayTokenSpaceGuid.PcdPciDevice6Name|"PCI Express Port 2"
  gCrownBayTokenSpaceGuid.PcdPciDevice6BridgeAddress|0
  gCrownBayTokenSpaceGuid.PcdPciDevice6DeviceAddress|0x00001900
  gCrownBayTokenSpaceGuid.PcdPciDevice6INTA|0x1506
  gCrownBayTokenSpaceGuid.PcdPciDevice6INTB|0xFFFF
  gCrownBayTokenSpaceGuid.PcdPciDevice6INTC|0xFFFF
  gCrownBayTokenSpaceGuid.PcdPciDevice6INTD|0xFFFF
  gCrownBayTokenSpaceGuid.PcdPciDevice6GpePin|0x08
  gCrownBayTokenSpaceGuid.PcdPciDevice6SxNum|0x03

  # PCI device 7
  gCrownBayTokenSpaceGuid.PcdPciDevice7Name|"PCI Express Port 3"
  gCrownBayTokenSpaceGuid.PcdPciDevice7BridgeAddress|0
  gCrownBayTokenSpaceGuid.PcdPciDevice7DeviceAddress|0x00001A00
  gCrownBayTokenSpaceGuid.PcdPciDevice7INTA|0x1607
  gCrownBayTokenSpaceGuid.PcdPciDevice7INTB|0xFFFF
  gCrownBayTokenSpaceGuid.PcdPciDevice7INTC|0xFFFF
  gCrownBayTokenSpaceGuid.PcdPciDevice7INTD|0xFFFF
  gCrownBayTokenSpaceGuid.PcdPciDevice7GpePin|0x09
  gCrownBayTokenSpaceGuid.PcdPciDevice7SxNum|0x03

  # PCI device 8
  gCrownBayTokenSpaceGuid.PcdPciDevice8Name|"Intel High Definition Audio Controller"
  gCrownBayTokenSpaceGuid.PcdPciDevice8BridgeAddress|0
  gCrownBayTokenSpaceGuid.PcdPciDevice8DeviceAddress|0x00001B00
  gCrownBayTokenSpaceGuid.PcdPciDevice8INTA|0x1708
  gCrownBayTokenSpaceGuid.PcdPciDevice8INTB|0xFFFF
  gCrownBayTokenSpaceGuid.PcdPciDevice8INTC|0xFFFF
  gCrownBayTokenSpaceGuid.PcdPciDevice8INTD|0xFFFF
  gCrownBayTokenSpaceGuid.PcdPciDevice8GpePin|0x19
  gCrownBayTokenSpaceGuid.PcdPciDevice8SxNum|0x03

  # PCI device 9
  gCrownBayTokenSpaceGuid.PcdPciDevice9Name|"LPC interface"
  gCrownBayTokenSpaceGuid.PcdPciDevice9BridgeAddress|0
  gCrownBayTokenSpaceGuid.PcdPciDevice9DeviceAddress|0x00001F00
  gCrownBayTokenSpaceGuid.PcdPciDevice9INTA|0x1001
  gCrownBayTokenSpaceGuid.PcdPciDevice9INTB|0xFFFF
  gCrownBayTokenSpaceGuid.PcdPciDevice9INTC|0xFFFF
  gCrownBayTokenSpaceGuid.PcdPciDevice9INTD|0xFFFF
  gCrownBayTokenSpaceGuid.PcdPciDevice9GpePin|0xFF
  gCrownBayTokenSpaceGuid.PcdPciDevice9SxNum|0xFF

  # PCI device 10
  gCrownBayTokenSpaceGuid.PcdPciDevice10Name|"PCI Device"
  gCrownBayTokenSpaceGuid.PcdPciDevice10BridgeAddress|0x00001700
  gCrownBayTokenSpaceGuid.PcdPciDevice10DeviceAddress|0x00010000
  gCrownBayTokenSpaceGuid.PcdPciDevice10INTA|0x1001
  gCrownBayTokenSpaceGuid.PcdPciDevice10INTB|0x1102
  gCrownBayTokenSpaceGuid.PcdPciDevice10INTC|0x1203
  gCrownBayTokenSpaceGuid.PcdPciDevice10INTD|0x1304
  gCrownBayTokenSpaceGuid.PcdPciDevice10GpePin|0xFF
  gCrownBayTokenSpaceGuid.PcdPciDevice10SxNum|0xFF

  # PCI device 10
  gCrownBayTokenSpaceGuid.PcdPciDevice10Name|"PCI Device"
  gCrownBayTokenSpaceGuid.PcdPciDevice10BridgeAddress|0x00001700
  gCrownBayTokenSpaceGuid.PcdPciDevice10DeviceAddress|0x00010000
  gCrownBayTokenSpaceGuid.PcdPciDevice10INTA|0x1001
  gCrownBayTokenSpaceGuid.PcdPciDevice10INTB|0x1102
  gCrownBayTokenSpaceGuid.PcdPciDevice10INTC|0x1203
  gCrownBayTokenSpaceGuid.PcdPciDevice10INTD|0x1304
  gCrownBayTokenSpaceGuid.PcdPciDevice10GpePin|0xFF
  gCrownBayTokenSpaceGuid.PcdPciDevice10SxNum|0xFF

  # PCI device 11
  gCrownBayTokenSpaceGuid.PcdPciDevice11Name|"PCI Device"
  gCrownBayTokenSpaceGuid.PcdPciDevice11BridgeAddress|0x00001800
  gCrownBayTokenSpaceGuid.PcdPciDevice11DeviceAddress|0x00030000
  gCrownBayTokenSpaceGuid.PcdPciDevice11INTA|0x1102
  gCrownBayTokenSpaceGuid.PcdPciDevice11INTB|0x1203
  gCrownBayTokenSpaceGuid.PcdPciDevice11INTC|0x1304
  gCrownBayTokenSpaceGuid.PcdPciDevice11INTD|0x1001
  gCrownBayTokenSpaceGuid.PcdPciDevice11GpePin|0xFF
  gCrownBayTokenSpaceGuid.PcdPciDevice11SxNum|0xFF

  # PCI device 12
  gCrownBayTokenSpaceGuid.PcdPciDevice12Name|"PCI Device"
  gCrownBayTokenSpaceGuid.PcdPciDevice12BridgeAddress|0x00001900
  gCrownBayTokenSpaceGuid.PcdPciDevice12DeviceAddress|0x00050000
  gCrownBayTokenSpaceGuid.PcdPciDevice12INTA|0x1203
  gCrownBayTokenSpaceGuid.PcdPciDevice12INTB|0x1304
  gCrownBayTokenSpaceGuid.PcdPciDevice12INTC|0x1001
  gCrownBayTokenSpaceGuid.PcdPciDevice12INTD|0x1102
  gCrownBayTokenSpaceGuid.PcdPciDevice12GpePin|0xFF
  gCrownBayTokenSpaceGuid.PcdPciDevice12SxNum|0xFF

  # PCI device 13
  gCrownBayTokenSpaceGuid.PcdPciDevice13Name|"PCI Device"
  gCrownBayTokenSpaceGuid.PcdPciDevice13BridgeAddress|0x00001A00
  gCrownBayTokenSpaceGuid.PcdPciDevice13DeviceAddress|0x00070000
  gCrownBayTokenSpaceGuid.PcdPciDevice13INTA|0x1304
  gCrownBayTokenSpaceGuid.PcdPciDevice13INTB|0x1001
  gCrownBayTokenSpaceGuid.PcdPciDevice13INTC|0x1102
  gCrownBayTokenSpaceGuid.PcdPciDevice13INTD|0x1203
  gCrownBayTokenSpaceGuid.PcdPciDevice13GpePin|0xFF
  gCrownBayTokenSpaceGuid.PcdPciDevice13SxNum|0xFF

  #
  # ACPI MADT
  #

  gCrownBayTokenSpaceGuid.PcdLocalApicAddressOverride                 | 0x00
  gCrownBayTokenSpaceGuid.PcdLocalApicSettingLocalApicLint            | 0x01
  gCrownBayTokenSpaceGuid.PcdLocalApicSettingTrigerMode               | 0x01
  gCrownBayTokenSpaceGuid.PcdLocalApicSettingPolarity                 | 0x01
  gCrownBayTokenSpaceGuid.PcdLocalApicSettingAddressOverrideEnable    | FALSE
  gCrownBayTokenSpaceGuid.PcdLocalApicSettingNmiEnabelApicIdMask      | 0x03

  gCrownBayTokenSpaceGuid.PcdIoApicSettingPolarity            | 0x0
  gCrownBayTokenSpaceGuid.PcdIoApicSettingTrigerMode          | 0x0
  gCrownBayTokenSpaceGuid.PcdIoApicSettingIoApicId            | 0x02
  gCrownBayTokenSpaceGuid.PcdIoApicSettingNmiEnable           | FALSE
  gCrownBayTokenSpaceGuid.PcdIoApicSettingNmiSource           | 0x0

  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable0Enable     | TRUE
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable0TrigerMode | 0x00
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable0Polarity   | 0x00
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable0SourceIrq  | 0x00
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable0GlobalIrq  | 0x02

  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable1Enable     | TRUE
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable1TrigerMode | 0x03
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable1Polarity   | 0x01
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable1SourceIrq  | 0x09
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable1GlobalIrq  | 0x09

  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable2Enable     | FALSE
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable2TrigerMode | 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable2Polarity   | 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable2SourceIrq  | 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable2GlobalIrq  | 0x0

  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable3Enable     | FALSE
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable3TrigerMode | 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable3Polarity   | 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable3SourceIrq  | 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable3GlobalIrq  | 0x0

  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable4Enable     | FALSE
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable4TrigerMode | 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable4Polarity   | 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable4SourceIrq  | 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable4GlobalIrq  | 0x0

  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable5Enable     | FALSE
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable5TrigerMode | 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable5Polarity   | 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable5SourceIrq  | 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable5GlobalIrq  | 0x0

  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable6Enable     | FALSE
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable6TrigerMode | 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable6Polarity   | 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable6SourceIrq  | 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable6GlobalIrq  | 0x0

  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable7Enable     | FALSE
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable7TrigerMode | 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable7Polarity   | 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable7SourceIrq  | 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable7GlobalIrq  | 0x0

  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable8Enable     | FALSE
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable8TrigerMode | 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable8Polarity   | 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable8SourceIrq  | 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable8GlobalIrq  | 0x0

  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable9Enable     | FALSE
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable9TrigerMode | 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable9Polarity   | 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable9SourceIrq  | 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable9GlobalIrq  | 0x0

  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable10Enable    | FALSE
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable10TrigerMode| 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable10Polarity  | 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable10SourceIrq | 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable10GlobalIrq | 0x0

  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable11Enable    | FALSE
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable11TrigerMode| 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable11Polarity  | 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable11SourceIrq | 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable11GlobalIrq | 0x0

  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable12Enable    | FALSE
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable12TrigerMode| 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable12Polarity  | 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable12SourceIrq | 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable12GlobalIrq | 0x0

  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable13Enable    | 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable13TrigerMode| 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable13Polarity  | 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable13SourceIrq | 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable13GlobalIrq | 0x0

  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable14Enable    | FALSE
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable14TrigerMode| 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable14Polarity  | 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable14SourceIrq | 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable14GlobalIrq | 0x0

  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable15Enable    | FALSE
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable15TrigerMode| 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable15Polarity  | 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable15SourceIrq | 0x0
  gCrownBayTokenSpaceGuid.PcdInterruptOverrideSettingTable15GlobalIrq | 0x0

  gAcpiTimerTokenSpaceGuid.PcdAcpiIoPciBusNumber           |0
  gAcpiTimerTokenSpaceGuid.PcdAcpiIoPciDeviceNumber        |31
  gAcpiTimerTokenSpaceGuid.PcdAcpiIoPciFunctionNumber      |0
  gAcpiTimerTokenSpaceGuid.PcdAcpiIoPciEnableRegisterOffset|0x4b
  gAcpiTimerTokenSpaceGuid.PcdAcpiIoBarEnableMask          |0x80
  gAcpiTimerTokenSpaceGuid.PcdAcpiIoPciBarRegisterOffset   |0x48
  gAcpiTimerTokenSpaceGuid.PcdAcpiIoPortBaseAddress        |0x1000
  gAcpiTimerTokenSpaceGuid.PcdAcpiPm1TmrOffset             |0x0008

[PcdsDynamicExHii.common.DEFAULT]
  gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdPlatformBootTimeOut|L"Timeout"|gEfiGlobalVariableGuid|0x0|1
  #gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdHardwareErrorRecordLevel|L"HwErrRecSupport"|gEfiGlobalVariableGuid|0x0|1
  gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdBootState|L"BootState"|gCrownBayTokenSpaceGuid|0x0|TRUE

[PcdsDynamicExDefault.common.DEFAULT]
  #gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdPlatformBootTimeOut|0x0001
  #test to change the bds show up time period
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutRow|25
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutColumn|80
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoHorizontalResolution|1024
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoVerticalResolution|768

  !if $(FAST_BOOT) == TRUE
    gMinnowBoardPkgTokenSpaceGuid.PcdEnableFastBoot|TRUE
  !else   ##  STREAMLINE_BOOT
    gMinnowBoardPkgTokenSpaceGuid.PcdEnableFastBoot|FALSE
  !endif  ##  STREAMLINE_BOOT

#----------------------------------------------------------------------
#  IA32FamilyCpuPkg PCDs
#----------------------------------------------------------------------

[PcdsFixedAtBuild]
  gEfiCpuTokenSpaceGuid.PcdCpuMicrocodePatchAddress|0x0
  gEfiCpuTokenSpaceGuid.PcdCpuMicrocodePatchRegionSize|0x0

[PcdsDynamicExDefault.common.DEFAULT]
  gEfiCpuTokenSpaceGuid.PcdCpuApInitTimeOutInMicroSeconds|50000
  gEfiCpuTokenSpaceGuid.PcdCpuApStackSize|0x8000
  gEfiCpuTokenSpaceGuid.PcdCpuProcessorFeatureUserConfiguration|0xFFFF06FF
  gEfiCpuTokenSpaceGuid.PcdCpuProcessorFeatureUserConfigurationEx1|0
  gEfiCpuTokenSpaceGuid.PcdPlatformHighPowerLoadLineSupport|TRUE
  gEfiCpuTokenSpaceGuid.PcdPlatformDynamicVidSupport|TRUE
  gEfiCpuTokenSpaceGuid.PcdPlatformType|1
  gEfiCpuTokenSpaceGuid.PcdPlatformCpuMaxCoreFrequency|3800
  gEfiCpuTokenSpaceGuid.PcdPlatformCpuMaxFsbFrequency|1066
  gEfiCpuTokenSpaceGuid.PcdCpuIEDEnabled|FALSE
  gEfiCpuTokenSpaceGuid.PcdCpuIEDRamSize|0x20000
  gEfiCpuTokenSpaceGuid.PcdCpuEnergyPolicy|0x0
  gEfiCpuTokenSpaceGuid.PcdCpuAcpiLvl2Addr|0x0
  gEfiCpuTokenSpaceGuid.PcdCpuPackageCStateLimit|0x0
  gEfiCpuTokenSpaceGuid.PcdCpuClockModulationDutyCycle|0x0
  gEfiCpuTokenSpaceGuid.PcdCpuHwCoordination|FALSE
  gEfiCpuTokenSpaceGuid.PcdCpuDcuMode|0x0

  gEfiCpuTokenSpaceGuid.PcdCpuTccActivationOffset|0

[PcdsDynamicExDefault.common.DEFAULT]
  gEfiCpuTokenSpaceGuid.PcdCpuProcessorFeatureCapability|0
  gEfiCpuTokenSpaceGuid.PcdCpuProcessorFeatureSetting|0
  gEfiCpuTokenSpaceGuid.PcdCpuProcessorFeatureCapabilityEx1|0
  gEfiCpuTokenSpaceGuid.PcdCpuProcessorFeatureSettingEx1|0
  gEfiCpuTokenSpaceGuid.PcdCpuConfigContextBuffer|0x0
  gEfiCpuTokenSpaceGuid.PcdCpuCallbackSignal|0x0
  gEfiCpuTokenSpaceGuid.PcdPlatformCpuFrequencyLists|0x0
  gEfiCpuTokenSpaceGuid.PcdPlatformCpuSocketCount|0x0
  gEfiCpuTokenSpaceGuid.PcdPlatformCpuSocketNames|0x0
  gEfiCpuTokenSpaceGuid.PcdPlatformCpuAssetTags|0x0
  gEfiCpuTokenSpaceGuid.PcdIsPowerOnReset|FALSE
  gEfiCpuTokenSpaceGuid.PcdCpuPageTableAddress|0x0
  gEfiCpuTokenSpaceGuid.PcdCpuMtrrTableAddress|0x0
  gEfiCpuTokenSpaceGuid.PcdCpuSocketId|{0x0}

  gEfiCpuTokenSpaceGuid.PcdCpuHotPlugDataAddress|0x0

#----------------------------------------------------------------------
#  IntelEg20tRuPkg PCDs
#----------------------------------------------------------------------

[PcdsFixedAtBuild]
  gIntelEg20tRuTokenSpaceGuid.PcdPeiP2PMemoryBaseAddress|0xA0000000

[PcdsDynamicExDefault.common.DEFAULT]
  gIntelEg20tRuTokenSpaceGuid.PcdPeiTcUsbControllerMemoryBaseAddress|0xA0010000

#----------------------------------------------------------------------
#  IntelE6xxRuPkg PCDs
#----------------------------------------------------------------------

[PcdsPatchableInModule]
  !if ($(SECURE_BOOT_ENABLE) == TRUE) || ($(SMM_VARIABLE) == TRUE)
    gIntelE6xxRuTokenSpaceGuid.PcdSmramAllocSize               |0x00400000
  !else
    gIntelE6xxRuTokenSpaceGuid.PcdSmramAllocSize               |0x00200000
  !endif

[PcdsDynamicExDefault.common.DEFAULT]
  gIntelE6xxRuTokenSpaceGuid.PcdE6xxAzaliaConfig|0x02
  gIntelE6xxRuTokenSpaceGuid.PcdE6xxDeviceEnables|0x6F
  gIntelE6xxRuTokenSpaceGuid.PcdE6xxGpioLevel|0x00000000
  gIntelE6xxRuTokenSpaceGuid.PcdE6xxPcieRootPort1Configuration|0x00010000
  gIntelE6xxRuTokenSpaceGuid.PcdE6xxPcieRootPort2Configuration|0x00020100
  gIntelE6xxRuTokenSpaceGuid.PcdE6xxPcieRootPort3Configuration|0x00030100
  gIntelE6xxRuTokenSpaceGuid.PcdE6xxPcieRootPort4Configuration|0x00040100
  gIntelE6xxRuTokenSpaceGuid.PcdE6xxPciHostBridgeIoBase|0x2000
  gIntelE6xxRuTokenSpaceGuid.PcdE6xxPciHostBridgeIoSize|0xE000
  gIntelE6xxRuTokenSpaceGuid.PcdIgdPreAllocSize|0x02

###################################################################################################
#
# Components Section - list of the modules and components that will be processed by compilation
#                      tools and the EDK II tools to generate PE32/PE32+/Coff image files.
#
# Note: The EDK II DSC file is not used to specify how compiled binary images get placed
#       into firmware volume images. This section is just a list of modules to compile from
#       source into UEFI-compliant binaries.
#       It is the FDF file that contains information on combining binary files into firmware
#       volume images, whose concept is beyond UEFI and is described in PI specification.
#       Binary modules do not need to be listed in this section, as they should be
#       specified in the FDF file. For example: Shell binary (Shell_Full.efi), FAT binary (Fat.efi),
#       Logo (Logo.bmp), and etc.
#       There may also be modules listed in this section that are not required in the FDF file,
#       When a module listed here is excluded from FDF file, then UEFI-compliant binary will be
#       generated for it, but the binary will not be put into any firmware volume.
#
###################################################################################################

[Components.IA32]
  #
  # SEC Core
  #
  MinnowBoardIntelRuBinPkg/Binaries/$(TARGET)/SecCore.inf

  #
  # PEI Core
  #
  MdeModulePkg/Core/Pei/PeiMain.inf  {
    <PcdsFixedAtBuild>
      !if $(LOGGING)
        gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x27
      !endif  ##  LOGGING
  }
  MinnowBoardIntelRuBinPkg/Binaries/$(TARGET)/DebugAgentPei.inf

  #
  # PEIM
  #
  MdeModulePkg/Universal/PCD/Pei/Pcd.inf {
    <LibraryClasses>
      PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  }

  MdeModulePkg/Universal/ReportStatusCodeRouter/Pei/ReportStatusCodeRouterPei.inf

  MinnowBoardIntelRuBinPkg/Binaries/$(TARGET)/CpuPei.inf
  MdeModulePkg/Universal/Variable/Pei/VariablePei.inf
  MdeModulePkg/Universal/CapsulePei/CapsulePei.inf
  UefiCpuPkg/CpuIoPei/CpuIoPei.inf
  MdeModulePkg/Universal/PcatSingleSegmentPciCfg2Pei/PcatSingleSegmentPciCfg2Pei.inf

  IntelE6xxRuBinPkg/Binaries/$(TARGET)/MemoryInitPei.inf
  MinnowBoardIntelRuBinPkg/Binaries/$(TARGET)/PlatformPeim.inf

  MdeModulePkg/Core/DxeIplPeim/DxeIpl.inf {
    <LibraryClasses>
      NULL|IntelFrameworkModulePkg/Library/LzmaCustomDecompressLib/LzmaCustomDecompressLib.inf
  }

  #
  # USB Support
  #
  MdeModulePkg/Bus/Pci/EhciPei/EhciPei.inf
  IntelEg20tRuBinPkg/Binaries/$(TARGET)/UsbPei.inf
  IntelEg20tRuBinPkg/Binaries/$(TARGET)/OhciPei.inf
  MdeModulePkg/Bus/Usb/UsbBusPei/UsbBusPei.inf
  MdeModulePkg/Bus/Usb/UsbBotPei/UsbBotPei.inf

  #
  # File system support
  #
  FatPkg/FatPei/FatPei.inf
  MdeModulePkg/Universal/Disk/CdExpressPei/CdExpressPei.inf

  #
  # Measured boot and authenticated variable
  #
  SecurityPkg/VariableAuthenticated/Pei/VariablePei.inf
  SecurityPkg/Tcg/PhysicalPresencePei/PhysicalPresencePei.inf
  SecurityPkg/Tcg/TcgPei/TcgPei.inf

  #
  # DXE Core
  #
  MinnowBoardIntelRuBinPkg/Binaries/$(TARGET)/DebugAgentDxe.inf

  #
  # Components that produce the architectural protocols
  #
  MdeModulePkg/Universal/SecurityStubDxe/SecurityStubDxe.inf {
    <LibraryClasses>
      !if $(SECURE_BOOT_ENABLE) == TRUE
        NULL|SecurityPkg/Library/DxeImageVerificationLib/DxeImageVerificationLib.inf
      !endif
      !if $(UID_ENABLE) == TRUE
        NULL|SecurityPkg/Library/DxeDeferImageLoadLib/DxeDeferImageLoadLib.inf
      !endif
      !if $(TPM_ENABLE) == TRUE
        NULL|SecurityPkg/Library/DxeTpmMeasureBootLib/DxeTpmMeasureBootLib.inf
      !endif
      NULL|SecurityPkg/Library/DxeImageAuthenticationStatusLib/DxeImageAuthenticationStatusLib.inf
  }

  MinnowBoardIntelRuBinPkg/Binaries/$(TARGET)/CpuArchDxe.inf
  MinnowBoardIntelRuBinPkg/Binaries/$(TARGET)/CpuMpDxe.inf
  MdeModulePkg/Universal/Metronome/Metronome.inf
  MinnowBoardPkg/LoadFileOnFv2/LoadFileOnFv2.inf
  MinnowBoardPkg/Bds/Bds.inf
  MinnowBoardPkg/BlinkLed/BlinkLed.inf
  MdeModulePkg/Universal/WatchdogTimerDxe/WatchdogTimer.inf
  MdeModulePkg/Core/RuntimeDxe/RuntimeDxe.inf
  MdeModulePkg/Universal/CapsuleRuntimeDxe/CapsuleRuntimeDxe.inf
  MdeModulePkg/Universal/MonotonicCounterRuntimeDxe/MonotonicCounterRuntimeDxe.inf
  IntelE6xxRuBinPkg/Binaries/$(TARGET)/TCResetSystemRuntimeDxe.inf
  PcAtChipsetPkg/PcatRealTimeClockRuntimeDxe/PcatRealTimeClockRuntimeDxe.inf

  # SPI flash
  IntelE6xxPkg/SpiFlashParts/GenericSpiFlash/GenericSpiFlash.inf

  # Smm solution for variable
  IntelE6xxPkg/SpiDeviceSyncDxe/SpiDeviceSyncDxe.inf
  IntelE6xxRuBinPkg/Binaries/$(TARGET)/SpiSmm.inf
  !if $(SMM_VARIABLE) == TRUE
    IntelE6xxRuPkg/SpiDeviceDxe/SpiDeviceSmm.inf
    IntelE6xxRuPkg/SpiDeviceDxe/SpiDeviceSmmDxe.inf
    MinnowBoardIntelRuPkg/FvbRuntimeDxe/FvbSmm.inf
    MinnowBoardIntelRuPkg/FvbRuntimeDxe/FvbSmmDxe.inf
    MdeModulePkg/Universal/FaultTolerantWriteDxe/FaultTolerantWriteSmm.inf
    MdeModulePkg/Universal/Variable/RuntimeDxe/VariableSmmRuntimeDxe.inf
    MdeModulePkg/Universal/Variable/RuntimeDxe/VariableSmm.inf
  !else
    IntelE6xxRuPkg/SpiDeviceDxe/SpiDeviceDxe.inf
  !endif

  # Runtime solution for variable
  IntelE6xxRuBinPkg/Binaries/$(TARGET)/SpiRuntimeDxe.inf {
    <LibraryClasses>
      PciExpressLib|MdePkg/Library/DxeRuntimePciExpressLib/DxeRuntimePciExpressLib.inf
  }
  MinnowBoardIntelRuPkg/FvbRuntimeDxe/FvbRuntimeDxe.inf
  MdeModulePkg/Universal/FaultTolerantWriteDxe/FaultTolerantWriteDxe.inf
  !if $(EMU_VARIABLE)
    MdeModulePkg/Universal/Variable/EmuRuntimeDxe/EmuVariableRuntimeDxe.inf
  !else
    MdeModulePkg/Universal/Variable/RuntimeDxe/VariableRuntimeDxe.inf
  !endif

  #
  # Following are the DXE drivers (alphabetical order)
  #
  IntelE6xxRuBinPkg/Binaries/MicrocodeUpdate.inf
  MdeModulePkg/Universal/DevicePathDxe/DevicePathDxe.inf
  MdeModulePkg/Universal/PCD/Dxe/Pcd.inf {
    <LibraryClasses>
      PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  }

  UefiCpuPkg/CpuIo2Dxe/CpuIo2Dxe.inf {
    <LibraryClasses>
      IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
  }

  IntelE6xxRuBinPkg/Binaries/$(TARGET)/E6xxInitDxe.inf
  IntelE6xxPkg/HdAudioSyncDxe/HdAudioSyncDxe.inf
  IntelEg20tRuBinPkg/Binaries/$(TARGET)/Eg20tInitDxe.inf
  MinnowBoardIntelRuBinPkg/Binaries/$(TARGET)/SetupDxe.inf
  MinnowBoardIntelRuBinPkg/Binaries/$(TARGET)/UiApp.inf

  MdeModulePkg/Universal/ReportStatusCodeRouter/RuntimeDxe/ReportStatusCodeRouterRuntimeDxe.inf
  MdeModulePkg/Universal/MemoryTest/NullMemoryTestDxe/NullMemoryTestDxe.inf

  #
  # ACPI
  #
  MdeModulePkg/Universal/Acpi/AcpiTableDxe/AcpiTableDxe.inf
  MinnowBoardIntelRuBinPkg/Binaries/$(TARGET)/AcpiTablesDxe.inf
  MinnowBoardIntelRuBinPkg/Binaries/$(TARGET)/AcpiPlatform.inf
  MinnowBoardIntelRuPkg/AcpiSupportOnAcpiTableAndAcpiSdtThunk/AcpiSupportOnAcpiTableAndAcpiSdtThunk.inf

  #
  # SMBIOS
  #
  MdeModulePkg/Universal/SmbiosDxe/SmbiosDxe.inf
  MinnowBoardIntelRuPkg/SmbiosMiscDxe/SmbiosMiscDxe.inf

  #
  # PCI
  #
  MinnowBoardPkg/IncompatiblePciDeviceSupportDxe/IncompatiblePciDeviceSupportDxe.inf
  MdeModulePkg/Bus/Pci/PciBusDxe/PciBusDxe.inf

  #
  # USB
  #
  MdeModulePkg/Bus/Pci/EhciDxe/EhciDxe.inf {
    <BuildOptions>
      GCC:*_UNIXGCC_*_CC_FLAGS       = -DMDEPKG_NDEBUG
      GCC:RELEASE_GCC44_*_CC_FLAGS   = -DMDEPKG_NDEBUG
      INTEL:RELEASE_*_*_CC_FLAGS     = /D MDEPKG_NDEBUG
      MSFT:RELEASE_*_*_CC_FLAGS      = /D MDEPKG_NDEBUG
  }
  IntelEg20tRuBinPkg/Binaries/$(TARGET)/OhciDxe.inf
  MdeModulePkg/Bus/Usb/UsbBusDxe/UsbBusDxe.inf
  MdeModulePkg/Bus/Usb/UsbKbDxe/UsbKbDxe.inf
  MdeModulePkg/Bus/Usb/UsbMassStorageDxe/UsbMassStorageDxe.inf
  MdeModulePkg/Bus/Usb/UsbMouseDxe/UsbMouseDxe.inf

  #
  # SDIO
  #
  IntelEg20tRuBinPkg/Binaries/$(TARGET)/SDController.inf
  IntelEg20tRuBinPkg/Binaries/$(TARGET)/SDMediaDevice.inf

  #
  # IDE
  #
  MdeModulePkg/Universal/Disk/DiskIoDxe/DiskIoDxe.inf
  MdeModulePkg/Universal/Disk/PartitionDxe/PartitionDxe.inf
  MdeModulePkg/Bus/Ata/AtaBusDxe/AtaBusDxe.inf
  MdeModulePkg/Bus/Ata/AtaAtapiPassThru/AtaAtapiPassThru.inf
  MdeModulePkg/Bus/Scsi/ScsiBusDxe/ScsiBusDxe.inf
  MdeModulePkg/Bus/Scsi/ScsiDiskDxe/ScsiDiskDxe.inf
  IntelEg20tRuBinPkg/Binaries/$(TARGET)/SataControllerDxe.inf

  #
  # ISA
  #
  IntelEg20tRuBinPkg/Binaries/$(TARGET)/IohSerialDxe.inf

  #
  # Console
  #
  MdeModulePkg/Universal/Console/ConPlatformDxe/ConPlatformDxe.inf
  MdeModulePkg/Universal/Console/ConSplitterDxe/ConSplitterDxe.inf {
    <LibraryClasses>
      PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
  }
  MdeModulePkg/Universal/Console/TerminalDxe/TerminalDxe.inf
  MdeModulePkg/Universal/HiiDatabaseDxe/HiiDatabaseDxe.inf
  MdeModulePkg/Universal/SetupBrowserDxe/SetupBrowserDxe.inf
  MdeModulePkg/Universal/Disk/UnicodeCollation/EnglishDxe/EnglishDxe.inf
  FatPkg/EnhancedFatDxe/Fat.inf

  #
  # Legacy Modules
  #
  PcAtChipsetPkg/8259InterruptControllerDxe/8259.inf

  #
  # User identification modules
  #
  !if $(UID_ENABLE)
    SecurityPkg/UserIdentification/UserIdentifyManagerDxe/UserIdentifyManagerDxe.inf
    SecurityPkg/UserIdentification/UserProfileManagerDxe/UserProfileManagerDxe.inf
    SecurityPkg/UserIdentification/PwdCredentialProviderDxe/PwdCredentialProviderDxe.inf
    SecurityPkg/UserIdentification/UsbCredentialProviderDxe/UsbCredentialProviderDxe.inf
  !endif

  #
  # Authenticated variable modules
  #
  SecurityPkg/VariableAuthenticated/RuntimeDxe/VariableRuntimeDxe.inf
  SecurityPkg/VariableAuthenticated/RuntimeDxe/VariableSmmRuntimeDxe.inf
  SecurityPkg/VariableAuthenticated/RuntimeDxe/VariableSmm.inf
  SecurityPkg/VariableAuthenticated/SecureBootConfigDxe/SecureBootConfigDxe.inf

#----------------------------------------------------------------------
#  Dependencies on:
#       SerialPortLib
#----------------------------------------------------------------------

[Components.IA32]

  #
  # PEI Core
  #
  MinnowBoardIntelRuBinPkg/Binaries/$(TARGET)/SerialPortPei.inf

  #
  # PEIM
  #
  MinnowBoardIntelRuBinPkg/Binaries/$(TARGET)/StatusCodeHandlerPei.inf
  MinnowBoardIntelRuBinPkg/Binaries/$(TARGET)/StartupNoisePeim.inf

  #
  # DXE Core
  #
  MinnowBoardIntelRuBinPkg/Binaries/$(TARGET)/DxeCore.inf

  #
  # DXE Drivers
  #
  MinnowBoardIntelRuBinPkg/Binaries/$(TARGET)/StatusCodeHandlerRuntimeDxe.inf

  #
  # PCI
  #
  MinnowBoardIntelRuBinPkg/Binaries/$(TARGET)/PciPlatformDxe.inf

  #------------------------------
  #  Diagnostic Support
  #------------------------------

  MinnowBoardIntelRuBinPkg/Binaries/$(TARGET)/DiagOutputResetPeim.inf
  MinnowBoardIntelRuBinPkg/Binaries/$(TARGET)/DiagUartOutputPeim.inf

#----------------------------------------------------------------------
#  Debug UART Support
#----------------------------------------------------------------------

[PcdsFixedAtBuild]
  gEfiMdePkgTokenSpaceGuid.PcdUartDefaultBaudRate|$(DEBUG_UART_BAUDRATE)
  gEfiMdePkgTokenSpaceGuid.PcdUartDefaultDataBits|$(DEBUG_UART_DATABITS)
  gEfiMdePkgTokenSpaceGuid.PcdUartDefaultParity|$(DEBUG_UART_PARITY)
  gEfiMdePkgTokenSpaceGuid.PcdUartDefaultStopBits|$(DEBUG_UART_STOPBITS)

  #
  #  PcdSerialRegisterBase needs to be the same as PcdIohUartIoBase!
  #
  gEfiMdeModulePkgTokenSpaceGuid.PcdSerialRegisterBase|$(DEBUG_UART_ADDRESS)
  gEfiMdeModulePkgTokenSpaceGuid.PcdSerialUseMmio|TRUE

#
#  The following PCDs must be fixed at build because the dynamic PCD support
#  is not available at the time when the debug serial driver loads
#
[PcdsFixedAtBuild]
  #
  # EG20T is connected to E6xx Root Port 0 at PCI Bus 0, Device 0x17, Function 0
  #
  gIntelEg20tRuTokenSpaceGuid.PcdEg20tParentPciBus     |0x00
  gIntelEg20tRuTokenSpaceGuid.PcdEg20tParentPciDevice  |0x17
  gIntelEg20tRuTokenSpaceGuid.PcdEg20tParentPciFunction|0x00

  gIntelEg20tRuTokenSpaceGuid.PcdIohUartPortNumber|$(DEBUG_UART_NUMBER)
  gIntelEg20tRuTokenSpaceGuid.PcdIohUartClk|$(DEBUG_UART_CLK)

  #
  # BRCSR - Baud Rate Clock Select Register
  #   0 = PLLDIVCLK - Must use BAUDSEL.  Use PLL2VCO and BAUDDIV if PLL2VC0 != 0
  #   1 = UART_CLK - Ignore BAUDSEL, PLL2VCO and BAUDDIV
  #
  gIntelEg20tRuTokenSpaceGuid.PcdIohUartBrcsr      | 0x00

  #
  # BAUDSEL
  #   00 = UART_CLK pin
  #   01 = 48 MHz
  #   10 = 25 MHz
  #   11 = 25 MHz
  #
  gIntelEg20tRuTokenSpaceGuid.PcdIohClkCfgBaudSel  | 0x00

  # PLL2VCO
  #   0000 - Disable PLL2VCO and BAUDDIV
  #   0110 - x6
  #   0111 - x7
  #   1000 - x8
  #   1001 - x9
  #   1010 - x10
  #   1011 - x11
  #
  gIntelEg20tRuTokenSpaceGuid.PcdIohClkCfgPll2Vc0  | 0x00

  #
  # BAUDDIV
  #   0000 = Divide by 16
  #   0001 = Divide by 1
  #   0010 = Divide by 2
  #   0011 = Divide by 3
  #   0100 = Divide by 4
  #   0101 = Divide by 5
  #   0110 = Divide by 6
  #   0111 = Divide by 7
  #   1000 = Divide by 8
  #   1001 = Divide by 9
  #   1010 = Divide by 10
  #   1011 = Divide by 11
  #   1100 = Divide by 12
  #   1101 = Divide by 13
  #   1110 = Divide by 14
  #   1111 = Divide by 15
  #
  gIntelEg20tRuTokenSpaceGuid.PcdIohClkCfgBaudDiv  | 0x0d

  #
  # Input clock frequency to the UART baudrate divider based upon
  # the inputs above
  #
  gUart16550PciTokenSpaceGuid.PcdSerialClockRate|50000000

  #
  # Enable Serial I/O Protocol only on UART 0
  #  BIT 0 - Enable/Disable Serial I/O on EG20T UART 0
  #  BIT 1 - Enable/Disable Serial I/O on EG20T UART 1
  #  BIT 2 - Enable/Disable Serial I/O on EG20T UART 2
  #  BIT 3 - Enable/Disable Serial I/O on EG20T UART 3
  #  BIT 4 - Reserved.  Must be 0.
  #  BIT 5 - Reserved.  Must be 0.
  #  BIT 6 - Reserved.  Must be 0.
  #  BIT 7 - Reserved.  Must be 0.
  #
  gIntelEg20tRuTokenSpaceGuid.PcdEg20tUartSerialIoEnableMask|0x07

  #
  # Based on configuration above, the number of bytes in the UART 0 Tx FIFO is:
  #
  gUart16550PciTokenSpaceGuid.PcdSerialExtendedTxFifoSize|256

  #
  # UART #0 in EG20T is access through PPB at Bus 0/Dev 0x17/Func 0, followed by a
  # PPB at Device 0/Function 0, followed by the UART at Device 0xA/Function 0x01.
  #
  gUart16550PciTokenSpaceGuid.PcdSerialPciDeviceInfo|{0x17, 0x00, 0x00, 0x00, 0x0A, 0x01, 0xFF}

#----------------------------------------------------------------------
#  Diagnostic Support
#----------------------------------------------------------------------

[LibraryClasses]
  WaitForItpLib|DiagnosticPkg/Library/WaitForItpLib/WaitForItpLib.inf

[Components.common]
  DiagnosticPkg/Applications/ReadPerf/ReadPerf.inf
  DiagnosticPkg/Applications/Var/Var.inf

#----------------------------------------------------------------------
#  Display Performance Application
#----------------------------------------------------------------------

[Components]
  PerformancePkg/Dp_App/Dp.inf

#----------------------------------------------------------------------
#  Ethernet Support
#----------------------------------------------------------------------

  #
  # Rev A Board
  #
[LibraryClasses]
  PhyConfigLib|MinnowBoardPkg/Library/PhyConfigLib/PhyConfigLib.inf
  PhyResetLib|MinnowBoardPkg/Library/PhyResetLib/PhyResetLib.inf
  EthernetMac|IntelEg20tPkg/Library/EthernetMac/EthernetMac.inf
  EthernetPhy|IntelEg20tPkg/Library/EthernetPhy/EthernetPhy.inf

[PcdsFixedAtBuild]
  gIntelEg20tPkgTokenSpaceGuid.PcdEthernetPhyResetGpio|8
  gIntelEg20tPkgTokenSpaceGuid.PcdEthernetPhyResetGpioController|1

[Components]
  IntelEg20tPkg/Applications/Eg20tMacPhy/Eg20tMacPhy.inf
  IntelEg20tPkg/EthernetDxe/Eg20tEthernetDxe.inf {
    <PcdsFixedAtBuild>
      gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000000
    <PcdsFeatureFlag>
      gCrownBayTokenSpaceGuid.PcdFeatureEthernetDisplayRegisters|FALSE
  }
  !if $(TIP_BUILD) == TRUE
    OptionRomPkg/Bus/Usb/UsbNetworking/Ax88772/Ax88772.inf
    OptionRomPkg/Bus/Usb/UsbNetworking/Ax88772b/Ax88772b.inf
  !endif  ##  TIP_BUILD

#----------------------------------------------------------------------
#  FirmwareUpdate Application
#----------------------------------------------------------------------

[PcdsFixedAtBuild]
  gCrownBayTokenSpaceGuid.PcdFlashBlocksPerDot|4

[Components]
  MinnowBoardIntelRuBinPkg/Binaries/$(TARGET)/FirmwareUpdate.inf

#----------------------------------------------------------------------
#  GPIO Support
#----------------------------------------------------------------------

[LibraryClasses]
  E6xxGpioLib|IntelE6xxPkg/Library/E6xxGpioLib/E6xxGpioLib.inf
  Eg20tGpioLib|IntelEg20tPkg/Library/Eg20tGpioLib/Eg20tGpioLib.inf
  LedLib|MinnowBoardPkg/Library/LedLib/LedLib.inf

[Components]
  IntelE6xxPkg/Applications/ShellGpioE6xx/GpioE6xx.inf
  IntelEg20tPkg/Applications/ShellGpioEg20t/GpioEg20t.inf

  #
  # Rev A Board
  #
[PcdsDynamicExDefault.common.DEFAULT]
  gIntelE6xxRuTokenSpaceGuid.PcdE6xxGpioUseSelect|0x1F
  gIntelE6xxRuTokenSpaceGuid.PcdE6xxGpioIoSelect|0x0F
  gIntelE6xxRuTokenSpaceGuid.PcdE6xxGpioLevel|0x00
  gIntelE6xxRuTokenSpaceGuid.PcdE6xxGpioUseSelectRsm|0x1FF
  gIntelE6xxRuTokenSpaceGuid.PcdE6xxGpioIoSelectRsm|0x080
  gIntelE6xxRuTokenSpaceGuid.PcdE6xxGpioLevelRsm|0x160

#----------------------------------------------------------------------
#  Graphics Support
#----------------------------------------------------------------------

[Components.IA32]
  !if $(ENABLE_GRAPHICS)
    MdeModulePkg/Universal/Console/GraphicsConsoleDxe/GraphicsConsoleDxe.inf  {
      <LibraryClasses>
        PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
    }
    # uegd driver is a binary only driver listed in the FDF file only.
  !endif  ##  ENABLE_GRAPHICS

#----------------------------------------------------------------------
#  HD Audio Support
#----------------------------------------------------------------------

[Components]
  MinnowBoardPkg/HdAudio/MinnowBoard/HdAudioConnections.inf
  !if $(TEST_LURE) == TRUE
    MinnowBoardPkg/HdAudio/TestLure/HdAudioConnections.inf
  !endif  ##  TEST_LURE

#----------------------------------------------------------------------
#  I2C Support
#----------------------------------------------------------------------

[LibraryClasses]
  AsciiDump|BusPkg/Library/AsciiDump/AsciiDump.inf
  DriverLib|BusPkg/Library/DriverLib/DriverLib.inf

#----------------------------------------------------------------------
#  Networking Support
#----------------------------------------------------------------------

[LibraryClasses]
  !if $(ENABLE_NETWORKING) == TRUE
    DpcLib|MdeModulePkg/Library/DxeDpcLib/DxeDpcLib.inf
    NetLib|MdeModulePkg/Library/DxeNetLib/DxeNetLib.inf
    IpIoLib|MdeModulePkg/Library/DxeIpIoLib/DxeIpIoLib.inf
    TcpIoLib|MdeModulePkg/Library/DxeTcpIoLib/DxeTcpIoLib.inf
    UdpIoLib|MdeModulePkg/Library/DxeUdpIoLib/DxeUdpIoLib.inf
    UefiScsiLib|MdePkg/Library/UefiScsiLib/UefiScsiLib.inf
  !endif  ## ENABLE_NETWORKING

[Components]
  !if $(ENABLE_NETWORKING) == TRUE
    MdeModulePkg/Universal/Network/ArpDxe/ArpDxe.inf
    MdeModulePkg/Universal/Network/Dhcp4Dxe/Dhcp4Dxe.inf
    MdeModulePkg/Universal/Network/DpcDxe/DpcDxe.inf
    MdeModulePkg/Universal/Network/Ip4ConfigDxe/Ip4ConfigDxe.inf
    MdeModulePkg/Universal/Network/Ip4Dxe/Ip4Dxe.inf
    MdeModulePkg/Universal/Network/IScsiDxe/IScsiDxe.inf
    MdeModulePkg/Universal/Network/MnpDxe/MnpDxe.inf
    MdeModulePkg/Universal/Network/Mtftp4Dxe/Mtftp4Dxe.inf
    MdeModulePkg/Universal/Network/SnpDxe/SnpDxe.inf
    MdeModulePkg/Universal/Network/Tcp4Dxe/Tcp4Dxe.inf
    MdeModulePkg/Universal/Network/Udp4Dxe/Udp4Dxe.inf
    MdeModulePkg/Universal/Network/UefiPxeBcDxe/UefiPxeBcDxe.inf
    MdeModulePkg/Universal/Network/VlanConfigDxe/VlanConfigDxe.inf

    NetworkPkg/IScsiDxe/IScsiDxe.inf
    NetworkPkg/Ip6Dxe/Ip6Dxe.inf
    NetworkPkg/TcpDxe/TcpDxe.inf
    NetworkPkg/Udp6Dxe/Udp6Dxe.inf
    NetworkPkg/Dhcp6Dxe/Dhcp6Dxe.inf
    NetworkPkg/Mtftp6Dxe/Mtftp6Dxe.inf
    NetworkPkg/UefiPxeBcDxe/UefiPxeBcDxe.inf

    NetworkPkg/Application/IfConfig6/IfConfig6.inf
    NetworkPkg/Application/Ping6/Ping6.inf
  !endif  ## ENABLE_NETWORKING

#----------------------------------------------------------------------
#  Platform Configuration
#----------------------------------------------------------------------

[PcdsPatchableInModule]
  gIntelE6xxRuTokenSpaceGuid.PcdE6xxMemoryConfiguration|0x5c
  gCrownBayTokenSpaceGuid.PcdPciSubsystemVendorId|0x1cc8
  gCrownBayTokenSpaceGuid.PcdPciSubsystemId|1

[PcdsDynamicExVpd.common.DEFAULT]
  gIntelEg20tRuTokenSpaceGuid.Eg20tGigabitEthernetMac   |0x0000|8|{0xff, 0xff, 0xff, 0xff, 0xff, 0xff}
  gIntelEg20tRuTokenSpaceGuid.Eg20tPciSubsystemVendorId |0x0008|0x1CC8
  gIntelEg20tRuTokenSpaceGuid.Eg20tPciSubsystemDeviceId |0x000a|0x0001

#----------------------------------------------------------------------
#  Platform Firmware
#----------------------------------------------------------------------

[PcdsFixedAtBuild]
  #  Selectable boot supported
  #  Boot from CD supported
  #  BIOS shadowing is allowed
  #  BIOS is Upgradable (Flash)
  #  PCI supported
  gCrownBayTokenSpaceGuid.PcdSMBIOSBiosChar|0x000000019880

  #  ACPI supported
  gCrownBayTokenSpaceGuid.PcdSMBIOSBiosCharEx1|0x01

  #  UEFI
  gCrownBayTokenSpaceGuid.PcdSMBIOSBiosCharEx2|0x08

  #
  #  SMBIOS BIOS (Type 0) Information
  #
[PcdsDynamicExVpd.common.DEFAULT]
  gEfiMdeModulePkgTokenSpaceGuid.PcdFirmwareRevision  |0x000c|$(BUILD_NUMBER)
  gCrownBayTokenSpaceGuid.PcdSMBIOSBiosReleaseDate    |0x0010|16|"11/22/2013"
  gCrownBayTokenSpaceGuid.PcdSMBIOSSystemUuid         |0x0020|16|{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}
  gEfiMdeModulePkgTokenSpaceGuid.PcdFirmwareVendor    |0x0040|64|L"Firmware Version 1.00"
  gCrownBayTokenSpaceGuid.PcdSMBIOSBiosVersion        |0x0080|32|"MinnowBoard Firmware 1.00"
  gCrownBayTokenSpaceGuid.PcdSMBIOSBiosVendor         |0x00a0|32|"Intel Corp."

#----------------------------------------------------------------------
# Shell Application.
#----------------------------------------------------------------------

[PcdsFixedAtBuild]
  gEfiShellPkgTokenSpaceGuid.PcdShellLibAutoInitialize|FALSE
  gEfiMdePkgTokenSpaceGuid.PcdUefiLibMaxPrintBufferSize|16000

  #------------------------------
  #  Select the commands
  #------------------------------

  #
  #  Bit 3 Network: ifconfig, ping
  #  Bit 2 Install: bcfg
  #  Bit 1 Debug:   comp, dblk, dmem, dmpstore, edit, eficompress, efidecompress,
  #                 hexedit, loadpcirom, memmap, mm, mode, pci, sermode, setsize,
  #                 setvar, smbiosview
  #                 mem
  #  Bit 0 Driver:  connect, devices, devtree, dh, disconnect, drivers, drvcfg,
  #                 drvdiag, openinfo, reconnect, unload
  #
  gEfiShellPkgTokenSpaceGuid.PcdShellProfileMask|0xf

  #
  #  3: alias, cls, echo, getmtc, help, pause, touch, type, ver
  #  2: attrib, cd, cp, load, ls, map, mkdir, mv, parse, reset, rm, set, vol
  #     cd.., cd\, copy, del, dir, md, ren
  #     date, time, timezone
  #  1: else, endfor, endif, exit, for, goto, if, shift, stall
  #
  gEfiShellPkgTokenSpaceGuid.PcdShellSupportLevel|3

  #------------------------------
  #  Build the shell
  #------------------------------

[Components.IA32]
  ShellPkg/Application/Shell/Shell.inf {

    #------------------------------
    #  Basic commands
    #------------------------------

    <LibraryClasses>
      NULL|ShellPkg/Library/UefiShellLevel1CommandsLib/UefiShellLevel1CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellLevel2CommandsLib/UefiShellLevel2CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellLevel3CommandsLib/UefiShellLevel3CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellDriver1CommandsLib/UefiShellDriver1CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellInstall1CommandsLib/UefiShellInstall1CommandsLib.inf
      !if $(SHELL_DEBUG_CMDS) == TRUE
        NULL|ShellPkg/Library/UefiShellDebug1CommandsLib/UefiShellDebug1CommandsLib.inf
      !endif  ## SHELL_DEBUG_CMDS


    #------------------------------
    #  Diagnostic commands
    #------------------------------

    <LibraryClasses>
      !if $(E6XX_GPIO_CMD) == TRUE
        NULL|IntelE6xxPkg/Applications/ShellGpioE6xx/ShellGpioE6xx.inf
      !endif  ## E6XX_GPIO_CMD

      !if $(EG20T_GPIO_CMD) == TRUE
        NULL|IntelEg20tPkg/Applications/ShellGpioEg20t/ShellGpioEg20t.inf
      !endif  ## EG20T_GPIO_CMD

      NULL|MinnowBoardPkg/Library/BlinkOff/BlinkOff.inf
      NULL|MinnowBoardPkg/Library/BlinkOn/BlinkOn.inf

    #------------------------------
    #  Networking commands
    #------------------------------

    <LibraryClasses>
      !if $(ENABLE_NETWORKING) == TRUE

        NULL|ShellPkg/Library/UefiShellNetwork1CommandsLib/UefiShellNetwork1CommandsLib.inf

        !if $(ENABLE_IP6_ONLY) == TRUE

          NULL|MinnowBoardPkg/Override/NetworkPkg/Application/IfConfig6/IfConfig6Lib.inf

        !elseif $(DUAL_NETWORK_ENABLE) == TRUE

          NULL|MinnowBoardPkg/Override/NetworkPkg/Application/IfConfig6/IfConfig6Lib.inf

        !endif  # IP6_NETWORK_ENABLE
      !endif  ## ENABLE_NETWORKING

    #------------------------------
    #  Performance command
    #------------------------------

    <LibraryClasses>
      !if $(INCLUDE_DP) == TRUE
        !if $(TIP_BUILD) == TRUE
          NULL|ShellPkg/Library/UefiDpLib/UefiDpLib.inf
        !else   ##  TIP_BUILD
          NULL|MinnowBoardPkg/Override/ShellPkg/Library/UefiDpLib/UefiDpLib.inf
        !endif  ##  TIP_BUILD
      !endif  ## INCLUDE_DP

    #------------------------------
    #  Support libraries
    #------------------------------

    <LibraryClasses>
      !if $(TIP_BUILD) == TRUE
        ShellLib|ShellPkg/Library/UefiShellLib/UefiShellLib.inf
        ShellCommandLib|ShellPkg/Library/UefiShellCommandLib/UefiShellCommandLib.inf
      !else   ##  TIP_BUILD
        ShellLib|MinnowBoardPkg/Override/ShellPkg/Library/UefiShellLib/UefiShellLib.inf
        ShellCommandLib|MinnowBoardPkg/Override/ShellPkg/Library/UefiShellCommandLib/UefiShellCommandLib.inf
      !endif  ##  TIP_BUILD
      FileHandleLib|ShellPkg/Library/UefiFileHandleLib/UefiFileHandleLib.inf
      ShellCEntryLib|ShellPkg/Library/UefiShellCEntryLib/UefiShellCEntryLib.inf
      SortLib|ShellPkg/Library/UefiSortLib/UefiSortLib.inf
      HandleParsingLib|ShellPkg/Library/UefiHandleParsingLib/UefiHandleParsingLib.inf
      PathLib|ShellPkg/Library/BasePathLib/BasePathLib.inf
      NetLib|MdeModulePkg/Library/DxeNetLib/DxeNetLib.inf
      DebugLib|MdePkg/Library/UefiDebugLibConOut/UefiDebugLibConOut.inf
      PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
  }

#----------------------------------------------------------------------
#  SMM Support
#----------------------------------------------------------------------

[PcdsFeatureFlag]
  gEfiCpuTokenSpaceGuid.PcdCpuSmmEnableBspElection|FALSE

[PcdsDynamicExDefault.common.DEFAULT]
  gEfiCpuTokenSpaceGuid.PcdCpuSmmCodeAccessCheckEnable|TRUE
  gEfiCpuTokenSpaceGuid.PcdCpuSmmMsrSaveStateEnable|FALSE
  gEfiCpuTokenSpaceGuid.PcdCpuSmmSmrr2Base|0
  gEfiCpuTokenSpaceGuid.PcdCpuSmmSmrr2Size|0
  gEfiCpuTokenSpaceGuid.PcdCpuSmmSmrr2CacheType|5
  gEfiCpuTokenSpaceGuid.PcdCpuSmmUseDelayIndication|TRUE
  gEfiCpuTokenSpaceGuid.PcdCpuSmmUseBlockIndication|TRUE
  gEfiCpuTokenSpaceGuid.PcdCpuSmmUseSmmEnableIndication|TRUE

[PcdsDynamicExDefault.common.DEFAULT]
  gIntelE6xxRuTokenSpaceGuid.PcdSmmActivationData|0x55
  gIntelE6xxRuTokenSpaceGuid.PcdSmmDataPort|0xb3

[LibraryClasses.common.PEIM]
  LockBoxLib|MdeModulePkg/Library/SmmLockBoxLib/SmmLockBoxPeiLib.inf

[LibraryClasses.common.DXE_CORE]
  !if $(TIP_BUILD) == TRUE
    CpuExceptionHandlerLib|UefiCpuPkg/Library/CpuExceptionHandlerLib/DxeSmmCpuExceptionHandlerLib.inf
  !endif  ##  TIP_BUILD

[LibraryClasses.common.DXE_DRIVER]
  !if $(TIP_BUILD) == TRUE
    CpuExceptionHandlerLib|UefiCpuPkg/Library/CpuExceptionHandlerLib/DxeSmmCpuExceptionHandlerLib.inf
  !endif  ##  TIP_BUILD
  LockBoxLib|MdeModulePkg/Library/SmmLockBoxLib/SmmLockBoxDxeLib.inf
  TpmMeasurementLib|SecurityPkg/Library/DxeTpmMeasurementLib/DxeTpmMeasurementLib.inf

[LibraryClasses.common.DXE_RUNTIME_DRIVER]
  LockBoxLib|MdeModulePkg/Library/SmmLockBoxLib/SmmLockBoxDxeLib.inf

[LibraryClasses.common.UEFI_DRIVER,LibraryClasses.common.UEFI_APPLICATION]
  LockBoxLib|MdeModulePkg/Library/SmmLockBoxLib/SmmLockBoxDxeLib.inf

[LibraryClasses.common.SMM_CORE]
  !if $(TIP_BUILD) == TRUE
    CpuExceptionHandlerLib|UefiCpuPkg/Library/CpuExceptionHandlerLib/DxeSmmCpuExceptionHandlerLib.inf
  !endif  ##  TIP_BUILD
  MemoryAllocationLib|MdeModulePkg/Library/PiSmmCoreMemoryAllocationLib/PiSmmCoreMemoryAllocationLib.inf
  PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
  PerformanceLib|MdeModulePkg/Library/SmmCorePerformanceLib/SmmCorePerformanceLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/SmmReportStatusCodeLib/SmmReportStatusCodeLib.inf
  SmmCorePlatformHookLib|MdeModulePkg/Library/SmmCorePlatformHookLibNull/SmmCorePlatformHookLibNull.inf
  SmmServicesTableLib|MdeModulePkg/Library/PiSmmCoreSmmServicesTableLib/PiSmmCoreSmmServicesTableLib.inf

[LibraryClasses.common.DXE_SMM_DRIVER]
  BaseCryptLib|CryptoPkg/Library/BaseCryptLib/SmmCryptLib.inf
  !if $(TIP_BUILD) == TRUE
    CpuExceptionHandlerLib|UefiCpuPkg/Library/CpuExceptionHandlerLib/DxeSmmCpuExceptionHandlerLib.inf
  !endif  ##  TIP_BUILD
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf
  LockBoxLib|MdeModulePkg/Library/SmmLockBoxLib/SmmLockBoxSmmLib.inf
  MemoryAllocationLib|MdePkg/Library/SmmMemoryAllocationLib/SmmMemoryAllocationLib.inf
  PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
  PerformanceLib|MdeModulePkg/Library/DxeSmmPerformanceLib/DxeSmmPerformanceLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/SmmReportStatusCodeLib/SmmReportStatusCodeLib.inf
  SmmServicesTableLib|MdePkg/Library/SmmServicesTableLib/SmmServicesTableLib.inf
  TpmMeasurementLib|SecurityPkg/Library/DxeTpmMeasurementLib/DxeTpmMeasurementLib.inf

[Components.IA32]
  MinnowBoardIntelRuBinPkg/Binaries/$(TARGET)/PiSmmCommunicationSmm.inf
  MinnowBoardIntelRuBinPkg/Binaries/$(TARGET)/SmmPlatformHandler.inf

  MdeModulePkg/Universal/ReportStatusCodeRouter/Smm/ReportStatusCodeRouterSmm.inf
  MdeModulePkg/Universal/StatusCodeHandler/Smm/StatusCodeHandlerSmm.inf {
    <LibraryClasses>
      !if $(LOGGING)
        DebugLib|MdePkg/Library/BaseDebugLibSerialPort/BaseDebugLibSerialPort.inf
      !else
        SerialPortLib|MdePkg/Library/BaseSerialPortLibNull/BaseSerialPortLibNull.inf
      !endif
  }

  MinnowBoardIntelRuBinPkg/Binaries/$(TARGET)/PowerManagementAcpiTablesDxe.inf

  MdeModulePkg/Core/PiSmmCore/PiSmmIpl.inf
  MdeModulePkg/Core/PiSmmCore/PiSmmCore.inf {
    <LibraryClasses>
      PeCoffExtraActionLib|MdePkg/Library/BasePeCoffExtraActionLibNull/BasePeCoffExtraActionLibNull.inf
  }
  MinnowBoardIntelRuBinPkg/Binaries/$(TARGET)/PiSmmCpuDxeSmm.inf
  MdeModulePkg/Universal/LockBox/SmmLockBox/SmmLockBox.inf
  UefiCpuPkg/CpuIo2Smm/CpuIo2Smm.inf {
    <LibraryClasses>
      IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
  }
  IntelE6xxRuBinPkg/Binaries/$(TARGET)/SmmControlDxe.inf
  IntelE6xxRuBinPkg/Binaries/$(TARGET)/TCSmmDispatcher.inf
  MinnowBoardIntelRuBinPkg/Binaries/$(TARGET)/SmmPowerManagement.inf
  IntelE6xxRuBinPkg/Binaries/$(TARGET)/SmmAccessDxe.inf

#------------------------------
# S3 Support
#------------------------------

[PcdsDynamicExDefault.common.DEFAULT]
  gEfiCpuTokenSpaceGuid.PcdCpuS3DataAddress|0x0
  gEfiMdeModulePkgTokenSpaceGuid.PcdS3BootScriptTablePrivateDataPtr|0x0

[Components.IA32]
    IntelE6xxRuBinPkg/Binaries/$(TARGET)/SmmAccessPei.inf
    IntelE6xxRuBinPkg/Binaries/$(TARGET)/SmmControlPei.inf
    MinnowBoardIntelRuBinPkg/Binaries/$(TARGET)/PiSmmCommunicationPei.inf
    UefiCpuPkg/Universal/Acpi/S3Resume2Pei/S3Resume2Pei.inf

    #
    # ACPI
    #
    MinnowBoardIntelRuBinPkg/Binaries/$(TARGET)/S3SaveStateDxe.inf
    MinnowBoardIntelRuBinPkg/Binaries/$(TARGET)/BootScriptExecutorDxe.inf
    IntelFrameworkModulePkg/Universal/Acpi/AcpiS3SaveDxe/AcpiS3SaveDxe.inf

[LibraryClasses]
  S3BootScriptLib|MdeModulePkg/Library/PiDxeS3BootScriptLib/DxeS3BootScriptLib.inf
  S3IoLib|MdePkg/Library/BaseS3IoLib/BaseS3IoLib.inf
  S3PciLib|MdePkg/Library/BaseS3PciLib/BaseS3PciLib.inf

#----------------------------------------------------------------------
#  TPM
#----------------------------------------------------------------------

[PcdsFixedAtBuild]
  gEfiSecurityPkgTokenSpaceGuid.PcdHideTpmSupport|FALSE
  gEfiSecurityPkgTokenSpaceGuid.PcdTpmPlatformClass|0

[PcdsPatchableInModule]
  gEfiSecurityPkgTokenSpaceGuid.PcdTpmPhysicalPresence|TRUE

[PcdsDynamicExDefault.common.DEFAULT]
  gEfiSecurityPkgTokenSpaceGuid.PcdHideTpm|FALSE

#
# Measured boot modules
#
[Components.IA32]
  !if $(TPM_ENABLE)
    SecurityPkg/Tcg/TcgDxe/TcgDxe.inf {
      <LibraryClasses>
        PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
    }
    SecurityPkg/Tcg/MemoryOverwriteControl/TcgMor.inf
    SecurityPkg/Tcg/TcgSmm/TcgSmm.inf
    SecurityPkg/Tcg/TcgConfigDxe/TcgConfigDxe.inf
  !endif  ## TPM_ENABLE


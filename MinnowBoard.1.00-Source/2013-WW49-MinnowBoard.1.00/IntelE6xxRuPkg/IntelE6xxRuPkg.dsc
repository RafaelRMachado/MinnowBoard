#
# This file contains 'Framework Code' and is licensed as such 
# under the terms of your license agreement with Intel or your
# vendor.  This file may not be modified, except as allowed by
# additional terms of your license agreement.                 
#
## @file
# INTEL Tunnel Creek Processor Module Package Reference Implementations
#
# This DSC file is used for Package Level build.
#
# This Module provides FRAMEWORK reference implementation for INTEL Tunnel Creek.
# Copyright (c) 2010-2012, Intel Corporation.

# All rights reserved.
#   This software and associated documentation (if any) is furnished
#   under a license and may only be used or copied in accordance
#   with the terms of the license. Except as permitted by such
#   license, no part of this software or documentation may be
#   reproduced, stored in a retrieval system, or transmitted in any
#   form or by any means without the express written consent of
#   Intel Corporation.
#
##

################################################################################
#
# Defines Section - statements that will be processed to create a Makefile.
#
################################################################################
[Defines]
  PLATFORM_NAME                  = IntelE6xxRuPkg
  PLATFORM_GUID                  = 5F9864F4-EAFB-4ded-A41A-CA501EE50502
  PLATFORM_VERSION               = 0.2
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/IntelE6xxRuPkg
  SUPPORTED_ARCHITECTURES        = IA32
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT

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
# Library Class section - list of all Library Classes needed by this Platform.
#
################################################################################
[LibraryClasses.IA32.PEIM]
  TimerLib|MdePkg/Library/SecPeiDxeTimerLibCpu/SecPeiDxeTimerLibCpu.inf
  PeiServicesTablePointerLib|MdePkg/Library/PeiServicesTablePointerLib/PeiServicesTablePointerLib.inf

[LibraryClasses.common.PEIM]
  DebugLib|IntelFrameworkModulePkg/Library/PeiDxeDebugLibReportStatusCode/PeiDxeDebugLibReportStatusCode.inf
  IoLib|MdePkg/Library/PeiIoLibCpuIo/PeiIoLibCpuIo.inf
  PcdLib|MdePkg/Library/PeiPcdLib/PeiPcdLib.inf
  HobLib|MdePkg/Library/PeiHobLib/PeiHobLib.inf
  PeimEntryPoint|MdePkg/Library/PeimEntryPoint/PeimEntryPoint.inf
  PeiServicesLib|MdePkg/Library/PeiServicesLib/PeiServicesLib.inf
  BaseMemoryLib|MdePkg/Library/PeiMemoryLib/PeiMemoryLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/PeiReportStatusCodeLib/PeiReportStatusCodeLib.inf
  SmbusLib|IntelFrameworkPkg/Library/PeiSmbusLibSmbusPpi/PeiSmbusLibSmbusPpi.inf
  MemoryAllocationLib|MdePkg/Library/PeiMemoryAllocationLib/PeiMemoryAllocationLib.inf

################################################################################
#
# Pcd Section - list of all EDK II PCD Entries defined by this Platform
#
################################################################################

[PcdsFixedAtBuild]
  gIntelE6xxRuTokenSpaceGuid.PcdTCRcbaMmioBaseAddress|0xFED1C000
  gIntelE6xxRuTokenSpaceGuid.PcdTCAcpiIoPortBaseAddress|0x1000
  gIntelE6xxRuTokenSpaceGuid.PcdTCGpioIoPortBaseAddress|0x1080
  gIntelE6xxRuTokenSpaceGuid.PcdTCGpeIoPortBaseAddress|0x10C0
  gIntelE6xxRuTokenSpaceGuid.PcdTCSmbusIoPortBaseAddress|0x1040
  gIntelE6xxRuTokenSpaceGuid.PcdIoApicBaseAddress|0xFEC00000
  gIntelE6xxRuTokenSpaceGuid.PcdIoApicSpaceSize|0x1000
  gIntelE6xxRuTokenSpaceGuid.PcdTCRcbaMmioSpaceSize|0x4000
  gIntelE6xxRuTokenSpaceGuid.PcdPciExpressSpaceSize|0x10000000
  gIntelE6xxRuTokenSpaceGuid.PcdHpetBaseAddress|0xFED00000
  gIntelE6xxRuTokenSpaceGuid.PcdHpetSpaceSize|0x400
  gIntelE6xxRuTokenSpaceGuid.PcdTpmBaseAddress|0xFED40000
  gIntelE6xxRuTokenSpaceGuid.PcdTpmSpaceSize|0xC000
  gIntelE6xxRuTokenSpaceGuid.PcdTCPmbaIoPortBaseAddress|0x410
  gIntelE6xxRuTokenSpaceGuid.PcdTCPciHostBridgeIoBase|0x2000
  gIntelE6xxRuTokenSpaceGuid.PcdTCPciHostBridgeIoSize|0xE000
  gIntelE6xxRuTokenSpaceGuid.PcdTCPcieMemory32NonPrefetchableSize|0x0A00000
  gIntelE6xxRuTokenSpaceGuid.PcdTCPcieMemory32PrefetchableSize|0x0A00000
  gIntelE6xxRuTokenSpaceGuid.PcdTCPcieMemory64PrefetchableSize|0
  gIntelE6xxRuTokenSpaceGuid.PcdTCPcieIoSize|0x1000
  gIntelE6xxRuTokenSpaceGuid.PcdTCPcieBusSize|0
  gIntelE6xxRuTokenSpaceGuid.PcdTCD31IP|0x00000000
  gIntelE6xxRuTokenSpaceGuid.PcdTCD27IP|0x00000001
  gIntelE6xxRuTokenSpaceGuid.PcdTCD26IP|0x00000001
  gIntelE6xxRuTokenSpaceGuid.PcdTCD25IP|0x00000001
  gIntelE6xxRuTokenSpaceGuid.PcdTCD24IP|0x00000001
  gIntelE6xxRuTokenSpaceGuid.PcdTCD23IP|0x00000001
  gIntelE6xxRuTokenSpaceGuid.PcdTCD03IP|0x00000001
  gIntelE6xxRuTokenSpaceGuid.PcdTCD02IP|0x00000001
  gIntelE6xxRuTokenSpaceGuid.PcdTCD31IR|0x3210
  gIntelE6xxRuTokenSpaceGuid.PcdTCD27IR|0x3210
  gIntelE6xxRuTokenSpaceGuid.PcdTCD26IR|0x3210
  gIntelE6xxRuTokenSpaceGuid.PcdTCD25IR|0x3210
  gIntelE6xxRuTokenSpaceGuid.PcdTCD24IR|0x3210
  gIntelE6xxRuTokenSpaceGuid.PcdTCD23IR|0x3210
  gIntelE6xxRuTokenSpaceGuid.PcdTCD03IR|0x3210
  gIntelE6xxRuTokenSpaceGuid.PcdTCD02IR|0x3210
  gIntelE6xxRuTokenSpaceGuid.PcdTCGpioUseSelect|0x000001F
  gIntelE6xxRuTokenSpaceGuid.PcdTCGpioIoSelect|0x0000001F
  gIntelE6xxRuTokenSpaceGuid.PcdTCGpioLevel|0x00000000
  gIntelE6xxRuTokenSpaceGuid.PcdTCGpioUseSelectRsm|0x000001F8
  gIntelE6xxRuTokenSpaceGuid.PcdTCGpioIoSelectRsm|0x000001E7
  gIntelE6xxRuTokenSpaceGuid.PcdTCGpioLevelRsm|0x00000018
  gIntelE6xxRuTokenSpaceGuid.PcdTCOSPMIoPortBaseAddress|0x1100
  gIntelE6xxRuTokenSpaceGuid.PcdTCPSMIIoPortBaseAddress|0x1140
  gIntelE6xxRuTokenSpaceGuid.PcdTCAPMIoPortBaseAddress|0x1180
  gIntelE6xxRuTokenSpaceGuid.PcdTCPMIoPortBaseAddress|0x1010
  gIntelE6xxRuTokenSpaceGuid.PcdIgdPreAllocSize|0x02
  gIntelE6xxRuTokenSpaceGuid.PcdSmmActivationPort|0xb2
  gIntelE6xxRuTokenSpaceGuid.PcdSmmDataPort|0xb3
  gIntelE6xxRuTokenSpaceGuid.PcdSmmActivationData|0x55

[PcdsFeatureFlag]
  gIntelE6xxRuTokenSpaceGuid.PcdTCSmbusIoPortBaseAddressFixed|FALSE
  gIntelE6xxRuTokenSpaceGuid.PcdUseAcpiTimerInSmartTimer|TRUE
  gIntelE6xxRuTokenSpaceGuid.PcdTCPciexpressHotplugEnabled|TRUE

################################################################################
#
# Pcd Dynamic Section - list of all EDK II PCD Entries defined by this Platform
#
################################################################################

[PcdsDynamicDefault.common.DEFAULT]
  gIntelE6xxRuTokenSpaceGuid.PcdPlatformSmbusAddrNum|0x0
  gIntelE6xxRuTokenSpaceGuid.PcdPlatformSmbusAddrTable|0x0
  gIntelE6xxRuTokenSpaceGuid.PcdTCDeviceEnables|0x6F
  gIntelE6xxRuTokenSpaceGuid.PcdTCAzaliaConfig|0x02
  gIntelE6xxRuTokenSpaceGuid.PcdTCPcieRootPort1Configuration |0x00010000
  gIntelE6xxRuTokenSpaceGuid.PcdTCPcieRootPort2Configuration |0x00020100
  gIntelE6xxRuTokenSpaceGuid.PcdTCPcieRootPort3Configuration |0x00030100
  gIntelE6xxRuTokenSpaceGuid.PcdTCPcieRootPort4Configuration |0x00040100

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

[Components]
  
#
# Copyright (c) 2006 - 2010, Intel Corporation. All rights reserved.<BR>
# This program and the accompanying materials
# are licensed and made available under the terms and conditions of the BSD License
# which accompanies this distribution.  The full text of the license may be found at
# http://opensource.org/licenses/bsd-license.php
# 
# THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
# WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
# In *inux environment, the build tools's source is required and need to be compiled
# firstly, please reference https://edk2.tianocore.org/unix-getting-started.html to 
# to get how to setup build tool.
#
# After build tool is downloaded and compiled, a soft symbol linker need to be created
# at <workspace>/Conf. For example: ln -s /work/BaseTools /work/edk2/Conf/BaseToolsSource.
#
# Setup the environment for unix-like systems running a bash-like shell.
# This file must be "sourced" not merely executed. For example: ". edksetup.sh"
#
# CYGWIN users: Your path and filename related environment variables should be
# set up in the unix style.  This script will make the necessary conversions to
# windows style.
#
# Please reference edk2 user manual for more detail descriptions at https://edk2.tianocore.org/files/documents/64/494/EDKII_UserManual.pdf
#

if [ \
     "$1" = "-?" -o \
     "$1" = "-h" -o \
     "$1" = "--help" \
   ]
then
  echo BaseTools Usage: \'. edksetup.sh\'
  echo
  echo Please note: This script must be \'sourced\' so the environment can be changed.
  echo \(Either \'. edksetup.sh\' or \'source edksetup.sh\'\)
  return
fi

###
### Begin Copying Override Files into location
###

#OverridePath="./MinnowBoardPkg/Overrides/"
#OverrideArray=( $(find ${OverridePath} -type f -not -name ".*") ) #find paths to all override files, reject hidden files, store paths in array
#ArrayCount=${#OverrideArray[@]}

#for (( i = 0; i < $ArrayCount; i++))
#do
#   Destination=( $(echo ${OverrideArray[i]} | sed "s:${OverridePath}:./:g") ) #strip the override path from the destination path
#   cp -f ${OverrideArray[i]} $Destination
#done

#unset -v OverridePath OverrideArray ArrayCount

###
### End Of Override Copy
###

###
### setup date+time variables
###
MINNOW_BUILD_NUMBER=$(($(date +%y) % 32))$(date +%m%d%H%M)
MINNOW_BUILD_DATE=$(date +%m)'/'$(date +%d)'/'$(date +%Y)
###
### end setup of date+time variables
### 
   
if [ -z "$WORKSPACE" ]
then
  . BaseTools/BuildEnv $*
else
  . $WORKSPACE/BaseTools/BuildEnv $*
fi



/** @file
  Read performance test for Block I/O devices
  
  Copyright (c) 2011 - 2012, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/ShellCEntryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Protocol/BlockIo.h>
#include <Protocol/BlockIo2.h>

#define MAX_MBYTES                512
#define BUFFER_LENGTH_IN_BYTES    ( 1024 * 1024 )
#define MAX_READS                 (( MAX_MBYTES * 1024 * 1024 ) / BUFFER_LENGTH_IN_BYTES )
#define TEST_COUNT                8
#define TEST_TIME_IN_SECONDS      15
#define WARM_UP_READS             2

#define KIBYTE                    1024
#define MIBYTE                    ( KIBYTE * KIBYTE )
#define GIBYTE                    ( KIBYTE * MIBYTE )

#define DIM(x)                    ( sizeof ( x ) / sizeof ( x[ 0 ]))

volatile BOOLEAN bTestRunning;
UINT32 FinalReadCount;
EFI_BLOCK_IO2_TOKEN mTokens[ 4 ];
static UINT8 mBuffer[ BUFFER_LENGTH_IN_BYTES ];
volatile UINT32 ReadCount;
UINT32 TotalReadCount;


/**
  Save the test results

  This routine is called when the test completion timer fires.
**/
VOID
EFIAPI
TestComplete (
  IN EFI_EVENT Event,
  IN VOID * Context
  )
{
  //
  //  Save the final read count
  //
  FinalReadCount = ReadCount;

  //
  //  Terminate the test
  //
  bTestRunning = FALSE;
}


/**
  Read data as fast as possible from a block I/O device

  The ShellCEntryLib library instance wrappers the actual UEFI application
  entry point and calls this ShellAppMain function.

  @param  ImageHandle  The image handle of the UEFI Application.
  @param  SystemTable  A pointer to the EFI System Table.

  @retval  0               The application exited normally.
  @retval  Other           An error occurred.

**/
INTN
EFIAPI 
ShellAppMain (
  IN UINTN Argc, 
  IN CHAR16 **Argv
  )
{
  BOOLEAN bBlockIo2;
  UINT32 BlockIoCount;
  UINT32 BlockIo2Count;
  UINT32 BlocksPerRead;
  UINT32 BlockSize;
  BOOLEAN bReadComplete;
  UINT32 BytesPerSecond;
  UINT32 BytesToRead;
  UINT64 DataRead;
  EFI_LBA Lba;
  EFI_LBA LbaMax;
  EFI_HANDLE Handle;
  UINTN HandleCount;
  UINTN Index;
  UINT64 MaxMBytes;
  UINT64 MaxReads;
  UINT64 MediaBytes;
  UINT32 MediaId;
  EFI_BLOCK_IO_PROTOCOL * pBlockIo;
  EFI_BLOCK_IO2_PROTOCOL * pBlockIo2;
  EFI_HANDLE * pBlockIoArray;
  EFI_HANDLE * pBlockIo2Array;
  EFI_HANDLE * pHandle;
  EFI_HANDLE * pHandleEnd;
  EFI_BLOCK_IO_MEDIA * pMedia;
  EFI_BLOCK_IO2_TOKEN * pToken;
  EFI_BLOCK_IO2_TOKEN * pTokenEnd;
  CHAR8 * pUnits;
  UINT32 Seconds;
  EFI_STATUS Status;
  UINT64 Timeout;
  EFI_EVENT TimeoutEvent;
  UINT32 TestCount;
  UINT32 WarmUpBlocks;

  //
  //  Display the help text
  //
  if ( 1 == Argc ) {
    Print ( L"%s   [/2]   <handle>\r\n", Argv[ 0 ]);
    Print ( L"\r\n" );
    Print ( L"/2 - Use Block I/O 2 protocol\r\n" );

    //
    //  Determine which handles have the block I/O protocol on them
    //
    Print ( L"Block I/O Handles\r\n" );
    Status = gBS->LocateHandleBuffer ( ByProtocol,
                                       &gEfiBlockIoProtocolGuid,
                                       NULL,
                                       &HandleCount,
                                       &pBlockIoArray );
    if ( !EFI_ERROR ( Status )) {
      pHandle = pBlockIoArray;
      BlockIoCount = (UINT32)HandleCount;
      pHandleEnd = &pHandle[ BlockIoCount ];
      while ( pHandleEnd > pHandle ) {
        Handle = *pHandle++;
        Status = gBS->OpenProtocol ( Handle,
                                     &gEfiBlockIoProtocolGuid,
                                     (VOID **)&pBlockIo,
                                     NULL,
                                     NULL,
                                     EFI_OPEN_PROTOCOL_GET_PROTOCOL );
        if ( !EFI_ERROR ( Status )) {
          if (( NULL != pBlockIo )
            && ( NULL != pBlockIo->Media )
            && ( pBlockIo->Media->MediaPresent )) {
            //
            //  Display the handle and device size
            //
            pUnits = "Bytes";
            MediaBytes = MultU64x32 ( pBlockIo->Media->LastBlock + 1,
                                      pBlockIo->Media->BlockSize );
            if ( KIBYTE <= MediaBytes ) {
              pUnits = "KiBytes";
              if ( MIBYTE <= MediaBytes ) {
                pUnits = "MiBytes";
                MediaBytes = DivU64x32 ( MediaBytes, 1024 );
                if ( MIBYTE <= MediaBytes ) {
                  pUnits = "GiBytes";
                  MediaBytes = DivU64x32 ( MediaBytes, 1024 );
                  if ( MIBYTE <= MediaBytes ) {
                    pUnits = "TiBytes";
                    MediaBytes = DivU64x32 ( MediaBytes, 1024 );
                    if ( MIBYTE <= MediaBytes ) {
                      pUnits = "PiBytes";
                      MediaBytes = DivU64x32 ( MediaBytes, 1024 );
                    }
                  }
                }
              }
              BytesToRead = (UINT32)MediaBytes;
              Print ( L"    0x%016Lx: %d.%03d %a\r\n",
                      (UINT64)(UINTN)Handle,
                      BytesToRead / 1024,
                      ( BytesToRead % 1024 ) * 1000 / 1024,
                      pUnits );
            }
            else {
              BytesToRead = (UINT32)MediaBytes;
              Print ( L"    0x%016Lx: %d %a\r\n",
                      (UINT64)(UINTN)Handle,
                      BytesToRead,
                      pUnits );
            }
          }
        }
      }

      //
      //  Free the handle buffer
      //
      gBS->FreePool ( pBlockIoArray );
    }
    else {
      Print ( L"    No block I/O handles found!\r\n" );
    }

    //
    //  Determine which handles have the block I/O 2 protocol on them
    //
    Print ( L"Block I/O 2 Handles\r\n" );
    Status = gBS->LocateHandleBuffer ( ByProtocol,
                                       &gEfiBlockIo2ProtocolGuid,
                                       NULL,
                                       &HandleCount,
                                       &pBlockIo2Array );
    if ( !EFI_ERROR ( Status )) {
      pHandle = pBlockIo2Array;
      BlockIo2Count = (UINT32)HandleCount;
      pHandleEnd = &pHandle[ BlockIo2Count ];
      while ( pHandleEnd > pHandle ) {
        Handle = *pHandle++;
        Status = gBS->OpenProtocol ( Handle,
                                     &gEfiBlockIoProtocolGuid,
                                     (VOID **)&pBlockIo,
                                     NULL,
                                     NULL,
                                     EFI_OPEN_PROTOCOL_GET_PROTOCOL );
        if ( !EFI_ERROR ( Status )) {
          if (( NULL != pBlockIo )
            && ( NULL != pBlockIo->Media )
            && ( pBlockIo->Media->MediaPresent )) {
            //
            //  Display the handle and device size
            //
            pUnits = "Bytes";
            MediaBytes = MultU64x32 ( pBlockIo->Media->LastBlock + 1,
                                      pBlockIo->Media->BlockSize );
            if ( KIBYTE <= MediaBytes ) {
              pUnits = "KiBytes";
              if ( MIBYTE <= MediaBytes ) {
                pUnits = "MiBytes";
                MediaBytes = DivU64x32 ( MediaBytes, 1024 );
                if ( MIBYTE <= MediaBytes ) {
                  pUnits = "GiBytes";
                  MediaBytes = DivU64x32 ( MediaBytes, 1024 );
                  if ( MIBYTE <= MediaBytes ) {
                    pUnits = "TiBytes";
                    MediaBytes = DivU64x32 ( MediaBytes, 1024 );
                    if ( MIBYTE <= MediaBytes ) {
                      pUnits = "PiBytes";
                      MediaBytes = DivU64x32 ( MediaBytes, 1024 );
                    }
                  }
                }
              }
              BytesToRead = (UINT32)MediaBytes;
              Print ( L"    0x%016Lx: %d.%03d %a\r\n",
                      (UINT64)(UINTN)Handle,
                      BytesToRead / 1024,
                      ( BytesToRead % 1024 ) * 1000 / 1024,
                      pUnits );
            }
            else {
              BytesToRead = (UINT32)MediaBytes;
              Print ( L"    0x%016Lx: %d %a\r\n",
                      (UINT64)(UINTN)Handle,
                      BytesToRead,
                      pUnits );
            }
          }
        }
      }

      //
      //  Free the handle buffer
      //
      gBS->FreePool ( pBlockIo2Array );
    }
    else {
      Print ( L"    No block I/O 2 handles found!\r\n" );
    }

    Print ( L"The test reads %d KiByte buffers as fast as it can for a total\r\n",
            BUFFER_LENGTH_IN_BYTES / 1024 );
    Print ( L"of %d seconds.  At the end of the time, the performance is computed\r\n",
            TEST_TIME_IN_SECONDS );
    Print ( L"by dividing the number of bytes received prior to the timer expiring\r\n" );
    Print ( L"by the test duration.  The test reads at most %d MiBytes from the\r\n",
            MAX_MBYTES );
    Print ( L"beginning of the media before wrapping around to the beginning again.\r\n" );
    Print ( L"As a warm-up, two buffers worth of data are read from the end of the\r\n" );
    Print ( L"%d MiByte region.\r\n",
            MAX_MBYTES );
    return EFI_NOT_STARTED;
  }

  //
  //  Determine if the block I/O 2 protocol should be used
  //
  bBlockIo2 = FALSE;
  pToken = &mTokens[ 0 ];
  pTokenEnd = &pToken[ DIM ( mTokens )];
  if ( 0 == StrCmp ( L"/2", Argv[ 1 ])) {
    bBlockIo2 = TRUE;
  }

  //
  //  Get the handle address
  //
  HandleCount = bBlockIo2 ? 2 : 1;
  Handle = (EFI_HANDLE)StrHexToUintn ( Argv[ HandleCount ]);
  if ( NULL == Handle ) {
    Print ( L"ERROR - Invalid handle value\r\n" );
    return EFI_INVALID_PARAMETER;
  }

  //
  //  Validate the handle
  //
  pBlockIo = NULL;
  pBlockIo2 = NULL;
  Status = gBS->OpenProtocol ( Handle,
                               bBlockIo2
                               ? &gEfiBlockIo2ProtocolGuid
                               : &gEfiBlockIoProtocolGuid,
                               bBlockIo2
                               ? (VOID *)&pBlockIo2
                               : (VOID *)&pBlockIo,
                               NULL,
                               NULL,
                               EFI_OPEN_PROTOCOL_GET_PROTOCOL );
  if ( EFI_ERROR ( Status )) {
    Print ( L"ERROR - %r\r\n", Status );
    return (( Status & MAX_BIT ) ? 0x80000000 : 0 )
            | ( Status & 0x7fffffff );
  }

  //
  //  Create the necessary events
  //
  Status = gBS->CreateEvent ( EVT_TIMER | EVT_NOTIFY_SIGNAL,
                              TPL_NOTIFY,
                              TestComplete,
                              NULL,
                              &TimeoutEvent );
  if ( EFI_ERROR ( Status )) {
    Print ( L"ERROR - Failed to create event, Status: %r\r\n",
             Status );
    return (( Status & MAX_BIT ) ? 0x80000000 : 0 )
            | ( Status & 0x7fffffff );
  }

  while ( pTokenEnd > pToken ) {
    Status = gBS->CreateEvent ( 0,
                                TPL_NOTIFY,
                                NULL,
                                NULL,
                                &pToken->Event );
    if ( EFI_ERROR ( Status )) {
      Print ( L"ERROR - Failed to create token event, Status: %r\r\n",
               Status );
      break;
    }
    pToken += 1;
  }
  if ( !EFI_ERROR ( Status )) {
    //
    //  Display the media parameters
    //
    pMedia = bBlockIo2 ? pBlockIo2->Media : pBlockIo->Media;
    Print ( L"\r\nMedia Parameters:\r\n" );
    Print ( L"  BlockSize: %d bytes\r\n", pMedia->BlockSize );
    Print ( L"  IoAlign: %d bytes\r\n", pMedia->IoAlign );
    Print ( L"  LastBlock: 0x%Lx\r\n", pMedia->LastBlock );
    Print ( L"  LogicalBlocksPerPhysicalBlock: %d\r\n", pMedia->LogicalBlocksPerPhysicalBlock );
    Print ( L"  LogicalPartition: %a\r\n", pMedia->LogicalPartition ? "TRUE" : "FALSE" );
    Print ( L"  LowestAlignedLba: 0x%Lx\r\n", pMedia->LowestAlignedLba );
    Print ( L"  MediaId: 0x%08x\r\n", pMedia->MediaId );
    Print ( L"  MediaPresent: %a\r\n", pMedia->MediaPresent ? "TRUE" : "FALSE" );
#ifdef  EFI_BLOCK_IO_PROTOCOL_REVISION3
    Print ( L"  OptimalTransferLengthGranularity: %d blocks\r\n", pMedia->OptimalTransferLengthGranularity );
#endif  //  EFI_BLOCK_IO_PROTOCOL_REVISION3
    Print ( L"  ReadOnly: %a\r\n", pMedia->ReadOnly ? "TRUE" : "FALSE" );
    Print ( L"  RemovableMedia: %a\r\n", pMedia->RemovableMedia ? "TRUE" : "FALSE" );
    Print ( L"  WriteCaching: %a\r\n", pMedia->WriteCaching ? "TRUE" : "FALSE" );
    Print ( L"\r\n" );

    //
    //  Locate the end of the media
    //
    BlockSize = pMedia->BlockSize;
    MediaBytes = pMedia->LastBlock + 1;
    MediaBytes = MultU64x32 ( MediaBytes, BlockSize );
    BlocksPerRead = sizeof ( mBuffer ) / BlockSize;
    BytesToRead = BlocksPerRead * BlockSize;
    ASSERT ( 0 < BlocksPerRead );
    LbaMax = MAX_MBYTES * 1024L * 1024L;
    LbaMax = DivU64x32 ( LbaMax, BlocksPerRead );
    LbaMax = MultU64x32 ( LbaMax, BlocksPerRead );
    LbaMax = DivU64x32 ( LbaMax, BlockSize );
    if ( LbaMax > pMedia->LastBlock ) {
      LbaMax = MultU64x32 ( DivU64x32 ( pMedia->LastBlock + 1,
                                        BlocksPerRead ),
                            BlocksPerRead );
    }
    MaxReads = DivU64x32 ( LbaMax, BlocksPerRead );
    MaxMBytes = DivU64x32 ( MultU64x32 ( MaxReads,
                                         BlocksPerRead * BlockSize ),
                            1024 * 1024 );
    WarmUpBlocks = WARM_UP_READS * BlocksPerRead;
    if ( LbaMax < WarmUpBlocks ) {
      WarmUpBlocks = ((UINT32)DivU64x32 ( LbaMax, BlocksPerRead )) * BlocksPerRead;
    }

    //
    //  Get the test duration
    //
    Seconds = TEST_TIME_IN_SECONDS;

    //
    //  Display the test parameters
    //
    Print ( L"Test Parameters:\r\n" );
    Print ( L"  Using: BlockIo%sProtocol\r\n", bBlockIo2 ? "2" : "" );
    Print ( L"  BlockSize: %d bytes\r\n", BlockSize );
    Print ( L"  Blocks/Read: %d\r\n", BlocksPerRead );
    Print ( L"  Duration: %d seconds\r\n", Seconds );
    Print ( L"  LBA max: %Ld\r\n", LbaMax );
    Print ( L"  Max data: %Ld MiBytes\r\n", MaxMBytes );
    Print ( L"  Max reads: %Ld\r\n", MaxReads );
    if (( 1000 * 1000 * 1000 ) <= MediaBytes ) {
      UINT32 Remainder;
      UINT64 GByte;
      GByte = DivU64x32Remainder ( MediaBytes,
                                   1000 * 1000 * 1000,
                                   &Remainder );
      Print ( L"  Media Size: %d.%03d GiBytes (%d.%03d GBytes)\r\n",
              (UINT32)( MediaBytes / GIBYTE ),
              ((UINT32)(( MediaBytes % GIBYTE ) / MIBYTE ) * 1000 ) / KIBYTE,
              (UINT32)GByte,
              Remainder / ( 1000 * 1000 ));
    }
    else {
      UINT32 Remainder;
      UINT64 MByte;
      MByte = DivU64x32Remainder ( MediaBytes,
                                   1000 * 1000,
                                   &Remainder );
      Print ( L"  Media Size: %d.%03d MiBytes (%d.%03d MBytes)\r\n",
              (UINT32)DivU64x32 ( MediaBytes, MIBYTE ),
              ((UINT32)(( MediaBytes % MIBYTE ) / KIBYTE ) * 1000 ) / KIBYTE,
              (UINT32)MByte,
              Remainder / 1000 );
    }
    Print ( L"  Test Count: %d runs\r\n", TEST_COUNT );
    Print ( L"  Warm-up reads: %d\r\n", WarmUpBlocks / BlocksPerRead );
    Print ( L"\r\n" );

    //
    //  Compute the timeout
    //
    Timeout = Seconds;
    Timeout *= 1000L * 1000L * 10L;

    //
    //  Get the media ID value
    //
    MediaId = pMedia->MediaId;

    //
    //  Warm-up the data path
    //
    Lba = LbaMax - WarmUpBlocks;
    while ( LbaMax > Lba ) {
      if ( bBlockIo2 ) {
        Status = pBlockIo2->ReadBlocksEx ( pBlockIo2,
                                           MediaId,
                                           Lba,
                                           NULL,
                                           BytesToRead,
                                           &mBuffer[ 0 ]);
      }
      else {
        Status = pBlockIo->ReadBlocks ( pBlockIo,
                                        MediaId,
                                        Lba,
                                        BytesToRead,
                                        &mBuffer[ 0 ]);
      }
      if ( EFI_ERROR ( Status )) {
        Print ( L"ERROR - Read failure during warm-up, Lba: 0x%016Lx, Status: %r\r\n",
                Lba,
                Status );
        break;
      }
      Lba += BlocksPerRead;
    }
    if ( !EFI_ERROR ( Status )) {
      //
      //  Perform each of the tests
      //
      for ( TestCount = 1; TEST_COUNT >= TestCount; TestCount++ ) {
        //
        //  Initialize the tokens
        //
        pToken = &mTokens[ 0 ];
        while ( pTokenEnd > pToken ) {
          gBS->SignalEvent ( pToken->Event );
          pToken->TransactionStatus = EFI_SUCCESS;
          pToken += 1;
        }
        pToken = &mTokens[ 0 ];

        //
        //  Start the timer
        //
        bTestRunning = TRUE;
        ReadCount = 0;
        bReadComplete = TRUE;
        gBS->CheckEvent ( TimeoutEvent );
        Status = gBS->SetTimer ( TimeoutEvent,
                                 TimerRelative,
                                 Timeout );
        ASSERT ( EFI_SUCCESS == Status );
        if ( bBlockIo2 ) {
          //
          //  Run the test using Block I/O 2 protocol
          //
          do {
            Lba = 0;
            while ( bTestRunning && ( LbaMax > Lba )) {
              //
              //  Wrap the token list if necessary
              //
              if ( pTokenEnd <= pToken ) {
                pToken = &mTokens[ 0 ];
              }

              //
              //  Wait for the next token
              //
              gBS->WaitForEvent ( 1, &pToken->Event, &Index );

              //
              //  Verify the read status
              //
              Status = pToken->TransactionStatus;
              if ( EFI_ERROR ( Status )) {
                break;
              }

              //
              //  Start the next read
              //
              Status = pBlockIo2->ReadBlocksEx ( pBlockIo2,
                                                 MediaId,
                                                 Lba,
                                                 pToken,
                                                 BytesToRead,
                                                 &mBuffer[ 0 ]);
              if ( EFI_ERROR ( Status )) {
                bReadComplete = FALSE;
                break;
              }

              //
              //  Account for this read
              //
              ReadCount += 1;
              Lba += BlocksPerRead;
              pToken += 1;
            }
          }while ( bTestRunning );

          //
          //  Wait for the rest of the reads to complete
          //
          pToken = &mTokens[ 0 ];
          while ( pTokenEnd > pToken ) {
            gBS->WaitForEvent ( 1,
                                &pToken->Event,
                                &Index );
            pToken += 1;
          }

          //
          //  Display any errors
          //
          if ( EFI_ERROR ( Status )) {
            Print ( L"ERROR - Read %afailure, Lba: 0x%016Lx, Status: %r\r\n",
                    bReadComplete ? "" : "queue ",
                    bReadComplete ? Lba - ( DIM ( mTokens ) * BlocksPerRead ) : Lba,
                    Status );
            break;
          }
        }
        else {
          //
          //  Run the test using Block I/O protocol
          //
          do {
            Lba = 0;
            while ( bTestRunning && ( LbaMax > Lba )) {
              Status = pBlockIo->ReadBlocks ( pBlockIo,
                                              MediaId,
                                              Lba,
                                              BytesToRead,
                                              &mBuffer[ 0 ]);
              if ( EFI_ERROR ( Status )) {
                Print ( L"ERROR - Read failure, Lba: 0x%016Lx, Status: %r\r\n",
                        Lba,
                        Status );
                return (( Status & MAX_BIT ) ? 0x80000000 : 0 )
                        | ( Status & 0x7fffffff );
              }
              ReadCount += 1;
              Lba += BlocksPerRead;
            }
          } while ( bTestRunning );
        }

        //
        //  Adjust the read count for the tokens
        //
        if ( bBlockIo2 ) {
          FinalReadCount = ( DIM ( mTokens ) >= FinalReadCount )
                         ? 0
                         : FinalReadCount - DIM ( mTokens );
        }

        //
        //  Compute the results
        //
        TotalReadCount += FinalReadCount;
        DataRead = FinalReadCount;
        DataRead = MultU64x32 ( DataRead, BytesToRead );
        BytesPerSecond = (UINT32)DivU64x32 ( DataRead, Seconds );

        //
        //  Display the test results
        //
        Print ( L"\r\nTest %d Results:\r\n", TestCount );
        Print ( L"  Reads: %d\r\n", FinalReadCount );
        Print ( L"  %d.%03d MiBytes/Second (%d.%03d Mbit/Second)\r\n",
                  BytesPerSecond / ( 1024 * 1024 ),
                  ((( BytesPerSecond % ( 1024 * 1024 )) / 1024 ) * 1000 ) / 1024,
                  ( BytesPerSecond * 8 ) / ( 1000 * 1000 ),
                  (( BytesPerSecond * 8 ) % ( 1000 * 1000 )) / 1000 );
      }

      //
      //  Compute the results
      //
      DataRead = TotalReadCount;
      DataRead = MultU64x32 ( DataRead, BytesToRead );
      DataRead = DivU64x32 ( DataRead, TEST_COUNT );
      BytesPerSecond = (UINT32)DivU64x32 ( DataRead, Seconds );

      //
      //  Display the test results
      //
      Print ( L"\r\nAverage Test Results:\r\n" );
      Print ( L"  Total Reads: %d\r\n", TotalReadCount );
      Print ( L"  %d.%03d MiBytes/Second (%d.%03d Mbit/Second)\r\n",
                BytesPerSecond / ( 1024 * 1024 ),
                ((( BytesPerSecond % ( 1024 * 1024 )) / 1024 ) * 1000 ) / 1024,
                ( BytesPerSecond * 8 ) / ( 1000 * 1000 ),
                (( BytesPerSecond * 8 ) % ( 1000 * 1000 )) / 1000 );
    }
  }

  //
  //  Done with the events
  //
  gBS->CloseEvent ( TimeoutEvent );
  pToken = &mTokens[ 0 ];
  while ( pTokenEnd > pToken ) {
    if ( NULL != pToken->Event ) {
      gBS->CloseEvent ( pToken->Event );
    }
    pToken += 1;
  }

  //
  //  Return the test status
  //
  return (( Status & MAX_BIT ) ? 0x80000000 : 0 )
          | ( Status & 0x7fffffff );
}

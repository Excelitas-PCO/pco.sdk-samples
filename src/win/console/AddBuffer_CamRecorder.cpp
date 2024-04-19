#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include "pco_err.h"
#include "sc2_SDKStructures.h"
#include "SC2_SDKAddendum.h"
#include "SC2_CamExport.h"
#include "SC2_Defs.h"

#ifdef _FILEFUNCTION_
char filename[50];
#include "file12.h"
#include "file12.cpp"
#endif

void print_transferpar(HANDLE cam);


#define BUFNUM 4

int main(int argc, char* argv[])
{
  int iRet;
  HANDLE cam;
  HANDLE BufEvent[BUFNUM];
  short BufNum[BUFNUM];
  WORD *BufAdr[BUFNUM];

  PCO_Description strDescription;
  WORD RecordingState;
  DWORD waitstat;

  printf("Get Handle to connected camera\n");
  iRet = PCO_OpenCamera(&cam, 0);
  if (iRet != PCO_NOERROR)
  {
     printf("No camera found\n");
     printf("Press <Enter> to end\n");
     iRet = getchar();
     return -1;
  }

  strDescription.wSize=sizeof(PCO_Description);
  iRet = PCO_GetCameraDescription(cam,&strDescription);

  if(strDescription.dwGeneralCapsDESC1&GENERALCAPS1_NO_RECORDER)
  {
     printf("Camera found, but no recorder available\n");
     printf("Press <Enter> to end\n");
     iRet = getchar();
     iRet = PCO_CloseCamera(cam);
     return -1;
  }

  iRet = PCO_GetRecordingState(cam, &RecordingState);
  if(RecordingState)
  {
     iRet = PCO_SetRecordingState(cam, 0);
  }

//set camera to default state
  iRet = PCO_ResetSettingsToDefault(cam);

#ifdef _FILEFUNCTION_
  iRet = PCO_SetTimestampMode(cam,TIMESTAMP_MODE_BINARYANDASCII);
#endif

  iRet = PCO_ArmCamera(cam);

  DWORD CameraWarning, CameraError, CameraStatus;
  iRet = PCO_GetCameraHealthStatus(cam, &CameraWarning, &CameraError, &CameraStatus);
  if(CameraError!=0)
  {
     printf("Camera has ErrorStatus\n");
     printf("Press <Enter> to end\n");
     iRet = getchar();
     iRet = PCO_CloseCamera(cam);
     return -1;
  }

  print_transferpar(cam);

  printf("Start and after some time stop camera\n");
  iRet = PCO_SetRecordingState(cam, 1);

//wait while camera is recording
  Sleep(500);

  iRet = PCO_SetRecordingState(cam, 0);

  DWORD ValidImageCnt, MaxImageCnt;
  WORD Segment=1; //this is the default segment

  iRet = PCO_GetNumberOfImagesInSegment(cam, Segment, &ValidImageCnt, &MaxImageCnt);

//readout only if atleast one image is in camera
  if(ValidImageCnt >= 1)
  {
    DWORD bufsize,StatusDll,StatusDrv,set;
    WORD XResAct, YResAct, XBin, YBin;
    WORD RoiX0, RoiY0, RoiX1, RoiY1;
    iRet = PCO_GetSegmentImageSettings(cam,Segment, &XResAct, &YResAct, 
               &XBin, &YBin, &RoiX0, &RoiY0, &RoiX1, &RoiY1);


    for(int b=0;b<BUFNUM;b++)
    {
      BufEvent[b] = NULL;
      BufNum[b] = -1;
      BufAdr[b]=NULL;
    }

    bufsize = XResAct*YResAct*sizeof(WORD);
    for(int b=0;b<BUFNUM;b++)
    {
     iRet = PCO_AllocateBuffer(cam, &BufNum[b], bufsize, &BufAdr[b], &BufEvent[b]);
    }
    iRet = PCO_SetImageParameters(cam, XResAct, YResAct,IMAGEPARAMETERS_READ_FROM_SEGMENTS,NULL,0);

    int test,next,multi;
    test=next=multi=0;
    printf("Grab recorded images from camera actual valid %d\n",ValidImageCnt);

    set=1;
    for(int b=0;b<BUFNUM;b++)
    {
     if(ValidImageCnt >= set)
     {
      iRet = PCO_AddBufferEx(cam,set,set, BufNum[b], XResAct, YResAct, 16);
      set++;
     }
    }

//this example does readout only first 10 images, adapt this e.g. use ValidImageCnt
    for(DWORD i=1;i<=10;i++)
    {
      printf("%02d. image ",i);
      if(ValidImageCnt < i)
      {
       printf("not available \n");
       break;
      }
      multi=0;
      printf("wait ");
      waitstat=WaitForMultipleObjects(BUFNUM,BufEvent,FALSE,1000);
      if(waitstat==WAIT_TIMEOUT)
      {
       printf("failed\n");
       break;
      }

// WaitForMultipleObjects might return with 2 or more events set, so all buffers must be checked
// 'test' and 'next' help to start check at last successfull buffer
// 'multi' counts the number of buffers, which have their event set
      test=next;
      for(int b=0;b<BUFNUM;b++)
      {
        waitstat=WaitForSingleObject(BufEvent[test],0);
        if(waitstat==WAIT_OBJECT_0)
        {
          multi++;
          ResetEvent(BufEvent[test]);
          iRet = PCO_GetBufferStatus(cam,BufNum[test],&StatusDll,&StatusDrv);

//!!! IMPORTANT StatusDrv must always be checked for errors 
          if(StatusDrv==PCO_NOERROR)
          {
           printf(" done buf%02d status 0x%08x ",test,StatusDrv); 
           if(multi>1)
            printf("multi %02d ",multi); 
          }
          else
          {
           printf("buf%02d error status 0x%08x m %02d ",test,StatusDrv,multi);
           break;
          }

// calculations on the image data can be done here, but calculation time must not exceed
// frametime of camera else images are lost
#ifdef _FILEFUNCTION_
          sprintf(filename,"addrec_image_%02d.tif",i);
          store_tiff(filename, XResAct, YResAct, 0, BufAdr[test]);
          printf("and stored to %s",filename);
#endif

          if(ValidImageCnt >= set)
          {
           iRet = PCO_AddBufferEx(cam,set,set, BufNum[test], XResAct, YResAct, 16);
           set++;
          }
        }
        else
         break;
        test++;
        if(test>=BUFNUM)
         test=0;
        printf("\n");
      }
      next=test;
      fflush(stdout);
    }//end for imacount

//!!! IMPORTANT PCO_CancelImages must always be called, after PCO_AddBuffer...() loops
    iRet = PCO_CancelImages(cam);
    for(int b=0;b<BUFNUM;b++)
     iRet = PCO_FreeBuffer(cam, BufNum[b]);
  }

  iRet = PCO_CloseCamera(cam);

  printf("Press <Enter> to end\n");
  iRet = getchar();
  return 0;
}

void print_transferpar(HANDLE cam)
{
  PCO_CameraType strCamType;
  DWORD iRet;
  strCamType.wSize=sizeof(PCO_CameraType);
  iRet = PCO_GetCameraType(cam,&strCamType);
  if(iRet!=PCO_NOERROR)
  {
   printf("PCO_GetCameraType failed with errorcode 0x%x\n",iRet);
   return;
  }

  if(strCamType.wInterfaceType==INTERFACE_CAMERALINK)
  {
    PCO_SC2_CL_TRANSFER_PARAM cl_par;

    iRet = PCO_GetTransferParameter(cam,(void*)&cl_par,sizeof(PCO_SC2_CL_TRANSFER_PARAM));
    printf("Camlink Settings:\nBaudrate:    %u\nClockfreq:   %u\n",cl_par.baudrate,cl_par.ClockFrequency); 
    printf("Dataformat:  %u 0x%x\nTransmit:    %u\n",cl_par.DataFormat,cl_par.DataFormat,cl_par.Transmit); 
  }
}


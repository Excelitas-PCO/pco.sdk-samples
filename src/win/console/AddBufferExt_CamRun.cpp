//-----------------------------------------------------------------//
// Name        | AddBuffer_CamRun.cpp        | Type: (*) source    //
//-------------------------------------------|       ( ) header    //
// Project     | pco.camera simple examples  |       ( ) others    //
//-----------------------------------------------------------------//
// Platform    | WINDOWS                                           //
//-----------------------------------------------------------------//
// Environment | Microsoft VisualStudio                            //
//             |                                                   //
//-----------------------------------------------------------------//
// Purpose     | Source for showing simple image grabbing          //
//-----------------------------------------------------------------//
// Author      | MBL, PCO AG                                       //
//-----------------------------------------------------------------//
// Revision    | rev. 1.0 rel. 1.00                                //
//-----------------------------------------------------------------//
// Notes       | error handling must be added                      //
//             |                                                   //
//             |                                                   //
//-----------------------------------------------------------------//
// (c) 2016 PCO AG * Donaupark 11 *                                //
// D-93309  Kelheim / Germany * Phone: +49 (0)9441 / 2005-0 *      //
// Fax: +49 (0)9441 / 2005-20 * Email: pco@excelitas.com           //
//-----------------------------------------------------------------//

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
#define IMANUM 10

int main(int argc, char* argv[])
{
  int iRet;
  HANDLE cam;

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

  iRet = PCO_GetRecordingState(cam, &RecordingState);
  if(RecordingState)
  {
     iRet = PCO_SetRecordingState(cam, 0);
  }

//set camera to default state
  iRet = PCO_ResetSettingsToDefault(cam);

  WORD wsize = 0;
  WORD wvers = 0;
  iRet = PCO_SetMetaDataMode(cam, 0, &wsize, &wvers); // Set meta data mode to 'off'

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

  WORD XResAct, YResAct, XResMax, YResMax;
  DWORD bufsize,set;
  HANDLE BufEvent[BUFNUM];
  DWORD Bufset[BUFNUM];
  DWORD StatusDrv[BUFNUM];

  WORD *ImaAdr[IMANUM];

  iRet = PCO_GetSizes(cam, &XResAct, &YResAct, &XResMax, &YResMax);
  bufsize=XResAct*YResAct*sizeof(WORD);


  for(int b=0;b<BUFNUM;b++)
  {
    BufEvent[b] = CreateEvent(NULL,TRUE,FALSE,NULL);
    if(BufEvent[b]==NULL)
    {
     printf("Creating event failed\n");
     printf("Press <Enter> to end\n");
     iRet = getchar();
     iRet = PCO_CloseCamera(cam);
     return -1;
    }
    StatusDrv[b]=-1;
    Bufset[b]=-1;
  }

  for(int i=0;i<IMANUM;i++)
  {
   ImaAdr[i]=(WORD*)malloc(bufsize);
  }

  iRet = PCO_SetImageParameters(cam, XResAct, YResAct,IMAGEPARAMETERS_READ_WHILE_RECORDING,NULL,0);

//for pco.edge with CameraLink interface order of adding the buffers and starting the camera
//must be changed to get also the first exposed image
  printf("Start camera\n");
  iRet = PCO_SetRecordingState(cam, 1);

  set=0;
  for(int b=0;b<BUFNUM;b++)
  {
   if((set<IMANUM)&&(ImaAdr[set]))
   {
    printf("PCO_AddBufferExtern() ImaAdr[%d]\n",set);
    iRet = PCO_AddBufferExtern(cam,BufEvent[b],1,0,0,0,ImaAdr[set],bufsize,&StatusDrv[b]);
    Bufset[b]=set;
    set++;
   }
  }

//if the transfer capability of the camera interface is smaller than the camera recording fps
//intermediate images will be lost
  int test,next,multi;
  test=next=multi=0;
  printf("Grab images from running camera\n");
  for(int i=1;i<=IMANUM;i++)
  {
   multi=0;
   printf("%02d. image wait ",i);
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

//!!! IMPORTANT StatusDrv must always be checked for errors 
     if(StatusDrv[test]==PCO_NOERROR)
     {
      printf("done buf%02d ima%03d ",test,Bufset[test]); 
      if(multi>1)
       printf("multi %02d ",multi); 
     }
     else
     {
      printf("buf%02d error status 0x%08x ima%03d m %02d ",test,StatusDrv[test],Bufset[test],multi);
      break;
     }

     if((set<IMANUM)&&(ImaAdr[set]))
     {
       printf("add ImaAdr[%d]",set);
       iRet = PCO_AddBufferExtern(cam,BufEvent[test],1,0,0,0,ImaAdr[set],bufsize,&StatusDrv[test]);
       Bufset[test]=set;
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
  printf("Stop camera and close connection\n");
  iRet = PCO_SetRecordingState(cam, 0);

  for(int i=0;i<IMANUM;i++)
  {

// calculations on the image data can be done here
#ifdef _FILEFUNCTION_
     sprintf(filename,"addext_image_%02d.tif",i+1);
     store_tiff(filename, XResAct, YResAct, 0, ImaAdr[i]);
     printf("image ima%03d stored to %s \n",i,filename);
#endif
  }

  for(int i=0;i<IMANUM;i++)
  {
   if(ImaAdr[i])
    free(ImaAdr[i]);
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


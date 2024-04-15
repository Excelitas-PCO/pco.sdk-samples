//-----------------------------------------------------------------//
// Name        | GetImage_CamRun.cpp         | Type: (*) source    //
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

int main(int argc, char* argv[])
{
  int iRet;
  HANDLE cam;
  HANDLE BufEvent;
  short BufNum;
  WORD *BufAdr;

  PCO_Description strDescription;
  WORD RecordingState;

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
//check if camera has internal Recorder (CamRam)
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
   iRet = PCO_SetRecordingState(cam, 0);

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

  if(ValidImageCnt >= 1)
  {
    WORD XResAct, YResAct, XBin, YBin;
    WORD RoiX0, RoiY0, RoiX1, RoiY1;
    iRet = PCO_GetSegmentImageSettings(cam,Segment, &XResAct, &YResAct, 
               &XBin, &YBin, &RoiX0, &RoiY0, &RoiX1, &RoiY1);

    BufEvent = NULL;
    BufNum = -1;
    BufAdr = NULL;
    DWORD bufsize = XResAct*YResAct*sizeof(WORD);

    iRet = PCO_AllocateBuffer(cam, &BufNum, bufsize, &BufAdr, &BufEvent);

    iRet = PCO_SetImageParameters(cam, XResAct, YResAct,IMAGEPARAMETERS_READ_FROM_SEGMENTS,NULL,0);

    printf("Grab recorded images from camera actual valid %d\n",ValidImageCnt);
    for(DWORD i=1;i<=10;i++)
    {
      printf("%02d. image ",i);
      if(ValidImageCnt < i)
      {
       printf("not available \n");
       break;
      }

      iRet = PCO_GetImageEx(cam, Segment, i, i, BufNum, XResAct, YResAct, 16);
      if (iRet != PCO_NOERROR)
      {
       printf("failed \n");
       break;
      }
      else
       printf("done ");

#ifdef _FILEFUNCTION_
      sprintf(filename,"rec_image_%02d.tif",i);
      store_tiff(filename, XResAct, YResAct, 0, BufAdr);
      printf("and stored to %s",filename);
#endif
     printf("\n");
    }

    iRet = PCO_FreeBuffer(cam, BufNum);
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



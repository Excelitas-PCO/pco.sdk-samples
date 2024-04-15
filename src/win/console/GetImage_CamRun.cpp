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
//             | some images will be lost, when working with       //
//             | higher framerates                                 //
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
#include "sc2_sdkstructures.h"
#include "sc2_sdkaddendum.h"
#include "sc2_camexport.h"
#include "sc2_defs.h"

#ifdef _FILEFUNCTION_
char file_name[50];
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
  if(iRet != PCO_NOERROR)
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
  DWORD bufsize;

  iRet = PCO_GetSizes(cam, &XResAct, &YResAct, &XResMax, &YResMax);
  bufsize=XResAct*YResAct*sizeof(WORD);

  BufEvent = NULL;
  BufNum = -1;
  BufAdr=NULL;
  iRet = PCO_AllocateBuffer(cam, &BufNum, bufsize, &BufAdr, &BufEvent);

  iRet = PCO_SetImageParameters(cam, XResAct, YResAct,IMAGEPARAMETERS_READ_WHILE_RECORDING,NULL,0);

  printf("Start camera\n");
  iRet = PCO_SetRecordingState(cam, 1);

  printf("Grab single images from running camera\n");
  for(int i=1;i<=10;i++)
  {
   printf("%02d. image ",i);
   iRet = PCO_GetImageEx(cam, 1, 0, 0, BufNum, XResAct, YResAct, 16);
   if (iRet != PCO_NOERROR)
   {
    printf("failed \n");
    break;
   }
   else
    printf("done ");

#ifdef _FILEFUNCTION_
    sprintf(file_name,"image_%02d.tif",i);
    store_tiff(file_name, XResAct, YResAct, 0, BufAdr);
    printf("and stored to %s",file_name);
#endif
     printf("\n");
  }

  printf("Stop camera and close connection\n");
  iRet = PCO_SetRecordingState(cam, 0);
  iRet = PCO_FreeBuffer(cam, BufNum);
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


#include <iostream>
#include <vector>
#include <cstring>


#include "pco_linux_defs.h"

#include "pco_device.h"
#include "sc2_sdkaddendum.h"
#include "sc2_sdkstructures.h"
#include "sc2_camexport.h"
#include "pco_camexport.h"

#include "pco_err.h"

#define MAXDEV 10

int main(int argc, char *argv[])
{
  int help=0;
  int err=PCO_NOERROR;
  char errstr[100];
  char fill[50];

  err = PCO_InitializeLib();
  if (err)
  {
    return err;
  }

  for(int i=argc;i>1;)
  {
   i--;

   if(strstr(argv[i],"-h"))
    help=1;
   if(strstr(argv[i],"-?"))
    help=1;
   if(strstr(argv[i],"?"))
    help=1;
  }

  if(help)
  {
   printf("usage: %s options\n"
          "options: \n"
          "-h,-?,? this message\n",argv[0]);
   exit(0);
  }

  std::vector<HANDLE> pcocam(MAXDEV);
  std::vector<PCO_Device> pcodevices(MAXDEV);
  WORD num_devices = 0;

  err=PCO_ScanCameras(0,&num_devices, pcodevices.data(), sizeof(PCO_Device) * pcodevices.size());
  printf("PCO_ScanCameras done err 0x%x num_devices %d\n",err,num_devices);
  if(num_devices<1)
  {
    printf("PCO_ScanCameras no camera found exit\n");
    exit(0);
  }

  pcodevices.resize(num_devices);
  pcocam.resize(num_devices);
  std::fill (pcocam.begin(),pcocam.end(),nullptr);
  printf("\n");

  for(WORD dev=0;dev<num_devices;dev++)
  {
    err=PCO_OpenCameraDevice(&pcocam[dev],pcodevices[dev].id);
    if(err!=PCO_NOERROR)
    {
      printf("PCO_OpenCameraDevice id %d failed Error 0x%x pcocam %p\n",pcodevices[dev].id,err,pcocam[dev]);
    }
    else
      printf("PCO_OpenCameraDevice id %d done successful pcocam %p\n",pcodevices[dev].id,pcocam[dev]);
  }
  printf("\n");

  for(size_t dev=0;dev<pcocam.size();dev++)
  {
    PCO_FW_Vers strFirmWareVersion;
    PCO_GetCameraDeviceStruct(&pcodevices[dev],pcodevices[dev].id);
    {
      printf("%ld. Device ",dev+1);
      if(pcodevices[dev].status&PCODEVICE_STATUS_BITS_CONNECTED)
      {
        printf("is available ");
        if(pcodevices[dev].status&PCODEVICE_STATUS_BITS_ATTACHED)
          printf("attached ");
        if(pcodevices[dev].status&PCODEVICE_STATUS_BITS_OPENED)
          printf("and opened");
      }
      else
        printf("is currently not connected");
      printf("\n");
      printf(" id:                       %d\n",    pcodevices[dev].id);
      printf(" processid:                %d\n",    pcodevices[dev].processid);
      printf(" status :                  0x%08x\n",pcodevices[dev].status);
      printf(" SerialNumber:             %d\n",    pcodevices[dev].SerialNumber);
      printf(" InterfaceType:            %d\n",    pcodevices[dev].PCO_InterfaceType);
      printf(" CameraType:               0x%04x\n",pcodevices[dev].CameraType);
      printf(" CameraSubType:            0x%04x\n",pcodevices[dev].CameraSubType);
      printf(" CameraName:               %s\n",    pcodevices[dev].CameraName);
      printf(" InterfaceName:            %s\n",    pcodevices[dev].PCO_InterfaceName);
      printf("\n");
    }
    
    if(pcocam[dev] && pcodevices[dev].status&PCODEVICE_STATUS_BITS_CONNECTED)
    {
      PCO_CameraType strCamType;
      
      strCamType.wSize=sizeof(PCO_CameraType);
      err=PCO_GetCameraType(pcocam[dev], &strCamType);
      if(err==PCO_NOERROR)
      {
        printf("PCO_CameraType:\n");
        printf(" SerialNumber:             %d\n",    strCamType.dwSerialNumber);
        printf(" CameraType:               0x%04x\n",strCamType.wCamType);
        printf(" CameraSubType:            0x%04x\n",strCamType.wCamSubType);
        printf("\n");
      }
      
      printf("Firmware versions:\n");
      err=PCO_GetFirmwareInfo(pcocam[dev], 0,&strFirmWareVersion);
      if(err!=PCO_NOERROR)
      {
        PCO_GetErrorTextSDK(err, errstr,100);
        printf("PCO_GetFirmwareInfo Block 0 failed err 0x%x\n  %s",err,errstr);
        goto ERR_OUT;
      }
      
      for(WORD i=0;i<strFirmWareVersion.DeviceNum && i<10;i++)
      {
        memset(fill,0,sizeof(fill));
        memset(fill,0x20,22-strlen(strFirmWareVersion.Device[i].szName));
        printf(" %s: %s  %03d  %02d.%02d\n",
               strFirmWareVersion.Device[i].szName,fill,
               strFirmWareVersion.Device[i].wVariant,
               strFirmWareVersion.Device[i].bMajorRev,
               strFirmWareVersion.Device[i].bMinorRev
        );
      }
      
      if(strFirmWareVersion.DeviceNum>10)
      {
        for(WORD j=1;j<=strFirmWareVersion.DeviceNum/10;j++)
        {
          err=PCO_GetFirmwareInfo(pcocam[dev], j,&strFirmWareVersion);
          if(err==PCO_NOERROR)
          {
            for(WORD i=0;i<strFirmWareVersion.DeviceNum-j*10;i++)
            {
              memset(fill,0,sizeof(fill));
              memset(fill,0x20,22-strlen(strFirmWareVersion.Device[i].szName));
              printf(" %s: %s  %03d  %02d.%02d\n",
                     strFirmWareVersion.Device[i].szName,fill,
                     strFirmWareVersion.Device[i].wVariant,
                     strFirmWareVersion.Device[i].bMajorRev,
                     strFirmWareVersion.Device[i].bMinorRev
              );
            }
          }
          else
          {
            PCO_GetErrorTextSDK(err, errstr,100);
            printf("PCO_GetFirmwareInfo Block %d failed err 0x%x\n  %s",j,err,errstr);
            break;
          }
        }
      }
    }
    printf("\n");
  }

  ERR_OUT:
  for(size_t dev=0;dev<pcocam.size();dev++)
  {
    if(pcocam[dev])
    {
      err=PCO_CloseCamera(pcocam[dev]);

      if(err!=PCO_NOERROR)
        printf("PCO_CloseCamera failed Error 0x%x pcocam %p\n",err,pcocam[dev]);
      else
        printf("PCO_CloseCamera done successful pcocam %p\n",pcocam[dev]);
      pcocam[dev]=nullptr;
    }
  }

  printf("\n");
  PCO_CleanupLib();
  return err;
}

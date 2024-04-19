typedef void* HANDLE;

#include "pco_linux_defs.h"

#include "pco_device.h"

#include "sc2_sdkaddendum.h"
#include "sc2_sdkstructures.h"
#include "sc2_camexport.h"
#include "pco_camexport.h"

#include "pco_err.h"

#if !defined (PATH_MAX)
#define PATH_MAX 1024
#endif

#define MAXDEV 8

#include <iostream>
#include <cstring>
#include <vector>
#include <map>
#include <filesystem>

void get_number(char *number,int len);
void get_text(char *text,int len);
void get_hexnumber(int *num,int len);
void get_current_settings(HANDLE hCamera);

typedef struct
{
 WORD wRoiX0;
 WORD wRoiY0;
 WORD wRoiX1;
 WORD wRoiY1;
}Cam_roiset;



int main(int argc, char *argv[])
{
  int help=0;
  uint32_t err=PCO_NOERROR;
  int x;
  char c;

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
/*
   char *a;
   if((a=strstr(argv[i],"-l")))
   {
    x=strtol(a+2,NULL,0);
    printf("Logging enabled, ");
    pco_log=new CPco_Log("pco_scan_test.log");
    if(x>0)
    {
     x|=0x03;
     pco_log->set_logbits(x);
    }
    printf("logbits set to 0x%x\n",pco_log->get_logbits());
   }
*/
  }

  if(help)
  {
    printf("usage: %s options\n"
          "options: \n"
//          "-l[0...]  enable logging, set loglevel\n"
          "-h,-?,? this message\n",argv[0]);
    exit(0);
  }

/*
  std::string path = std::filesystem::current_path();
  std::cout << "in working directory " << path << " " << std::endl;

//  std::filesystem::remove("hallo.txt");

  printf("this is program: %s\n",argv[0]);
  char result[PATH_MAX];
  ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
  result[count]=0;
  if(count)
    printf("this is program: %s\n",result);
*/

  printf("this is program: pco_camera_open\n");

  c=' ';
  while(c!='x')
  {
   int ch;
   c=' ';

   printf("\n");
   printf("x to close program\n");
   printf("o to open/close single camera\n");
   printf("a to open/close all cameras using PCO_OpenCamera\n");
   printf("i to open/close all cameras using PCO_OpenCameraDevice\n");
   printf("l to list connected devices\n");

   fflush(stdin);

   for( x = 0; (x < 2) &&  ((ch = getchar()) != EOF)
                        && (ch != '\n'); x++ )
    c=(char)ch;

   if(c=='o')
   {
    HANDLE pcocam=nullptr;

    err=PCO_OpenNextCamera(&pcocam);
    if(err!=PCO_NOERROR)
      printf("PCO_OpenCamera failed Error 0x%x pcocam %p\n",err,pcocam);
    else
      printf("PCO_OpenCamera done successful pcocam %p\n",pcocam);

    printf("\n");
    if(err==PCO_NOERROR)
    {
     printf("get current settings\n");
     get_current_settings(pcocam);
    }

    if(pcocam)
    {
      PCO_Description strDescription;
      strDescription.wSize = sizeof(PCO_Description);
      err = PCO_GetCameraDescription(pcocam,(PCO_Description *)&strDescription);
      if(err!=PCO_NOERROR)
        printf("PCO_GetCameraDescription failed Error 0x%x\n",err);
      else
        printf("PCO_GetCameraDescription done successful\n");
    }

    if(pcocam)
    {
      err=PCO_EnableSoftROI(pcocam,APIMANAGEMENTFLAG_SOFTROI, NULL,0);
      if(err!=PCO_NOERROR)
        printf("PCO_EnableSoftROI Flag 0x%04x failed Error 0x%x\n",APIMANAGEMENTFLAG_SOFTROI,err);
      else
        printf("PCO_EnableSoftROI Flag 0x%04x done successful\n",APIMANAGEMENTFLAG_SOFTROI);

      err=PCO_EnableSoftROI(pcocam,0, NULL,0);
      if(err!=PCO_NOERROR)
        printf("PCO_EnableSoftROI Flag 0x%04x failed Error 0x%x\n",err,0);
      else
        printf("PCO_EnableSoftROI Flag 0x%04x done successful\n",0);
    }

    if(pcocam)
    {
     err=PCO_CloseCamera(pcocam);
     pcocam=nullptr;
     if(err!=PCO_NOERROR)
       printf("PCO_CloseCamera failed Error 0x%x pcocam %p\n",err,pcocam);
     else
       printf("PCO_CloseCamera done successful pcocam %p\n",pcocam);
    }
   }
   else if(c=='a')
   {
    HANDLE pcocam[MAXDEV];
    for(int i=0;i<MAXDEV;i++)
      pcocam[i]=nullptr;

    for(int i=0;i<MAXDEV;i++)
    {
      err=PCO_OpenNextCamera(&pcocam[i]);
      if(err!=PCO_NOERROR)
      {
        printf("%d. PCO_OpenCamera failed Error 0x%x pcocam %p\n",i+1,err,pcocam[i]);
        break;
      }
      else
        printf("%d. PCO_OpenCamera done successful pcocam %p\n",i+1,pcocam[i]);
    }

    for(int i=0;i<MAXDEV;i++)
    {
      if(pcocam[i])
      {
        printf("%d. camera get current settings\n",i+1);
        get_current_settings(pcocam[i]);
      }
    }

    for(int i=0;i<MAXDEV;i++)
    {
      if(pcocam[i])
      {
        err=PCO_CloseCamera(pcocam[i]);
        pcocam[i]=nullptr;
        if(err!=PCO_NOERROR)
          printf("PCO_CloseCamera failed Error 0x%x pcocam %p\n",err,pcocam[i]);
        else
          printf("PCO_CloseCamera done successful pcocam %p\n",pcocam[i]);
      }
    }
   }
   else if(c=='i')
   {
    std::vector<HANDLE> pcocam(MAXDEV);
    std::vector<PCO_Device> pcodevices(MAXDEV);
    WORD num_devices = 0;

    err=PCO_ScanCameras(0,&num_devices, pcodevices.data(), sizeof(PCO_Device) * pcodevices.size());
    printf("PCO_ScanCameras done err 0x%x num_devices %d\n",err,num_devices);
    pcodevices.resize(num_devices);
    pcocam.resize(num_devices);

    std::fill (pcocam.begin(),pcocam.end(),nullptr);

    for(int i=num_devices-1;i>=0;i--)
    {
      err=PCO_OpenCameraDevice(&pcocam[i],pcodevices[i].id);
      if(err!=PCO_NOERROR)
      {
        printf("PCO_OpenCameraDevice id %d failed Error 0x%x pcocam %p\n",pcodevices[i].id,err,pcocam[i]);
        continue;
      }
      else
        printf("PCO_OpenCameraDevice id %d done successful pcocam %p\n",pcodevices[i].id,pcocam[i]);
    }

    for(size_t i=0;i<pcocam.size();i++)
    {
      if(pcocam[i])
      {
        printf("Camera id %d get current settings\n",pcodevices[i].id);
        get_current_settings(pcocam[i]);
      }
    }

    for(size_t i=0;i<pcocam.size();i++)
    {
      if(pcocam[i])
      {
        err=PCO_CloseCamera(pcocam[i]);
        pcocam[i]=nullptr;
        if(err!=PCO_NOERROR)
          printf("PCO_CloseCamera failed Error 0x%x pcocam %p\n",err,pcocam[i]);
        else
          printf("PCO_CloseCamera done successful pcocam %p\n",pcocam[i]);
      }
    }
   }
   else if(c=='l')
   {
    int max_device_count = 5;
    std::vector<PCO_Device> pcodevices(max_device_count);
    WORD num_devices = 0;

    err=PCO_ScanCameras(0,&num_devices, pcodevices.data(), sizeof(PCO_Device) * pcodevices.size());
    printf("PCO_ScanCameras done err 0x%x num_devices %d\n",err,num_devices);
    pcodevices.resize(num_devices);

    for(WORD i=0;i<num_devices;i++)
    {
//     if(pcodev[i].status&PCODEVICE_STATUS_BITS_CONNECTED)
     {
       printf("%d. Device ",i+1);
       if(pcodevices[i].status&PCODEVICE_STATUS_BITS_CONNECTED)
       {
        printf("is available ");
        if(pcodevices[i].status&PCODEVICE_STATUS_BITS_ATTACHED)
          printf("attached ");
        if(pcodevices[i].status&PCODEVICE_STATUS_BITS_OPENED)
          printf("and opened");
       }
       else
        printf("is currently not connected");
       printf("\n");
       printf(" id:                       %d\n",    pcodevices[i].id);
       printf(" processid:                %d\n",    pcodevices[i].processid);
       printf(" status :                  0x%08x\n",pcodevices[i].status);
       printf(" SerialNumber:             %d\n",    pcodevices[i].SerialNumber);
       printf(" InterfaceType:            %d\n",    pcodevices[i].PCO_InterfaceType);
       printf(" CameraType:               0x%04x\n",pcodevices[i].CameraType);
       printf(" CameraSubType:            0x%04x\n",pcodevices[i].CameraSubType);
       printf(" CameraName:               %s\n",    pcodevices[i].CameraName);
       printf(" InterfaceName:            %s\n",    pcodevices[i].PCO_InterfaceName);
       printf("\n");
      }
     }
   }
  }

  printf("\n");
  PCO_CleanupLib();
  return err;
}

void get_number(char *number,int len)
{
   int ret_val;
   int x=0;

   while(((ret_val=getchar())!=10)&&(x<len-1))
   {
    if(isdigit(ret_val))
     number[x++]=ret_val;
   }
   number[x]=0;
}

void get_text(char *text,int len)
{
   int ret_val;
   int x=0;

   while(((ret_val=getchar())!=10)&&(x<len-1))
   {
    if(isprint(ret_val))
     text[x++]=ret_val;
   }
   text[x]=0;
}


void get_hexnumber(int *num,int len)
{
  int ret_val;
  int c=0;
  int cmd=0;
  while(((ret_val=getchar())!=10)&&(len > 0))
  {
   if(isxdigit(ret_val))
   {
    if(ret_val<0x3A)
     cmd=(ret_val-0x30)+cmd*0x10;
    else if(ret_val<0x47)
     cmd=(ret_val-0x41+0x0A)+cmd*0x10;
    else
     cmd=(ret_val-0x61+0x0A)+cmd*0x10;
    len--;
    c++;
   }
  }
  if(c>0)
   *num=cmd;
  else
   *num=-1;
}


void get_current_settings(HANDLE hCamera)
{
  int err=PCO_NOERROR;
  char errstr[100];

  char cTimebas[3][3]={"ns","us","ms"};
  char cTrigger[5][20]={"Auto","Software","Extern","ExternExpos","Special"};

  WORD cur_recstate;
  Cam_roiset cur_roiset;
  WORD bin_x,bin_y;
  DWORD del,exp;
  WORD tb_del,tb_exp;
  WORD storagemode;
  WORD timestamp;
  DWORD pixelrate;
  WORD triggermode;
  WORD num_adc;

  WORD wMetaDataMode,wMetaDataSize,wMetaDataVersion;
//  bool MetaData_available;

  printf("Settings of %p:\n",hCamera);

  err=PCO_GetRecordingState(hCamera,&cur_recstate);
  if(err!=PCO_NOERROR)
  {
     PCO_GetErrorTextSDK(err, errstr,100);
     printf(" PCO_GetRecordingState failed\n  %s\n",errstr);
  }
  else
    printf(" RecordingState            %d %s\n",cur_recstate,cur_recstate ? "RUN" : "STOP" );

  err=PCO_GetDelayExposureTime(hCamera,&del,&exp,&tb_del,&tb_exp);
  if(err!=PCO_NOERROR)
  {
    PCO_GetErrorTextSDK(err, errstr,100);
    printf(" PCO_GetDelayExposureTime failed\n  %s\n",errstr);
  }
  else
  {
    printf(" Delaytime:                %d%s\n",del,cTimebas[tb_del]);
    printf(" Exposuretime:             %d%s\n",exp,cTimebas[tb_exp]);
  }

  err=PCO_GetTimestampMode(hCamera,&timestamp);
  if(err!=PCO_NOERROR)
  {
    PCO_GetErrorTextSDK(err, errstr,100);
    printf(" GetTimestampMode  failed\n  %s\n",errstr);
  }
  else
    printf(" TimestampMode:            %d\n",timestamp);

  err=PCO_GetTriggerMode(hCamera,&triggermode);
  if(err!=PCO_NOERROR)
  {
    PCO_GetErrorTextSDK(err, errstr,100);
    printf(" PCO_GetTriggerMode  failed\n  %s\n",errstr);
  }
  else
    printf(" Triggermode:              %d %s\n",triggermode,triggermode>3 ? cTrigger[4] : cTrigger[triggermode]);


  err=PCO_GetPixelRate(hCamera,&pixelrate);
  if(err!=PCO_NOERROR)
  {
    PCO_GetErrorTextSDK(err, errstr,100);
    printf("GetPixelRate  failed\n  %s\n",errstr);
  }
  else
    printf(" Pixelrate:                %dHz\n",pixelrate);

  err=PCO_GetMetaDataMode(hCamera,&wMetaDataMode,&wMetaDataSize,&wMetaDataVersion);
  if(err!=PCO_NOERROR)
  {
    PCO_GetErrorTextSDK(err, errstr,100);
    printf(" PCO_GetMetaDataMode  failed\n  %s\n",errstr);
//    MetaData_available=false;
  }
  else
  {
//    MetaData_available=true;
    printf(" MetaDataMode:             %d Version %d Size %d\n",wMetaDataMode,wMetaDataVersion,wMetaDataSize);
  }

  err=PCO_GetBinning(hCamera,&bin_x,&bin_y);
  if(err!=PCO_NOERROR)
  {
    PCO_GetErrorTextSDK(err, errstr,100);
    printf(" PCO_GetBinning failed\n  %s\n",errstr);
  }
  else
  {
    printf(" Binning horizontal:       %d\n",bin_x);
    printf("         vertical:         %d\n",bin_y);
  }

  err=PCO_GetROI(hCamera,&cur_roiset.wRoiX0,&cur_roiset.wRoiY0,&cur_roiset.wRoiX1,&cur_roiset.wRoiY1);
  if(err!=PCO_NOERROR)
  {
    PCO_GetErrorTextSDK(err, errstr,100);
    printf(" PCO_GetROI failed\n  %s\n",errstr);
  }
  else
  {
    printf(" ROI_X                     %04d - %04d\n",cur_roiset.wRoiX0,cur_roiset.wRoiX1);
    printf(" ROI_Y                     %04d - %04d\n",cur_roiset.wRoiY0,cur_roiset.wRoiY1);
    printf(" ROI width                 %04d\n",cur_roiset.wRoiX1-cur_roiset.wRoiX0+1);
    printf(" ROI height                %04d\n",cur_roiset.wRoiY1-cur_roiset.wRoiY0+1);
  }

  err=PCO_GetStorageMode(hCamera,&storagemode);
  if(err!=PCO_NOERROR)
  {
    PCO_GetErrorTextSDK(err, errstr,100);
    printf(" PCO_GetStorageMode failed\n  %s\n",errstr);
  }
  else
    printf(" StorageMode:              %d\n",storagemode);

  err=PCO_GetADCOperation(hCamera,&num_adc);
  if(err!=PCO_NOERROR)
  {
    PCO_GetErrorTextSDK(err, errstr,100);
    printf(" PCO_GetADCOperation  failed\n  %s\n",errstr);
  }
  else
    printf(" ADCOperation:             %d\n",num_adc);
  printf("\n");
}



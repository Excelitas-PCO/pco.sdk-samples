#include <iostream>
#include <cstring>
#include <thread>

#ifdef _FILEFUNCTION_
#include "../libtiff_header/tiff.h"
#include "../libtiff_header/tiffio.h"
#endif

#include <pco_linux_defs.h>

#include "pco_device.h"
#include "sc2_defs.h"
#include "sc2_common.h"
#include <sc2_sdkaddendum.h>
#include <sc2_sdkstructures.h>
#include <sc2_camexport.h>
#include <pco_camexport.h>

#include <pco_err.h>

#define ERRABORT(cam, err)                                   \
  if (err != PCO_NOERROR)                                    \
  {                                                          \
    char err_string[256];                                    \
    PCO_GetErrorTextSDK(err, err_string, 256);               \
    printf("%s\nLine: %d\n", err_string, __LINE__);          \
    DWORD err_close = PCO_CloseCamera(cam);                  \
    if (err_close)                                           \
      printf("PCO_CloseCamera failed: 0x%08x\n", err_close); \
    PCO_CleanupLib();                                        \
    return (int)err;                                         \
  }

#define ERRBREAK(cam, err)                          \
  if (err != PCO_NOERROR)                           \
  {                                                 \
    char err_string[256];                           \
    PCO_GetErrorTextSDK(err, err_string, 256);      \
    printf("%s\nLine: %d\n", err_string, __LINE__); \
    break;                                          \
  }

#ifdef FILEFUNCTION
void write_tiff(const char *file_path, WORD *data, DWORD width, DWORD height, WORD bits_per_pixel);
#endif

#define IMAGE_SEQUENCE_COUNT 10
#define BUFFER_ARRAY_SIZE 4


int main()
{

#ifdef _DEBUG
  char result[MAX_PATH];

  ssize_t count = readlink("/proc/self/exe", result, MAX_PATH);
  if(count)
  {
    result[count]=0;
    char* name;
    name = strrchr(result,'/');
    name+=1;
    printf("\nProgram '%s' created with _DEBUG at %s %s\n",name,__DATE__,__TIME__);
    *name=0;
    printf("started from path: %s\n",result);
  }
  else
    printf("\nProgram created with _DEBUG at %s %s\n",__DATE__,__TIME__);

#endif

  DWORD pco_error = PCO_InitializeLib();
  if (pco_error)
  {
    return pco_error;
  }

  HANDLE pco_cam = nullptr;

  printf("\nCall PCO_OpenCamera() \n");

  pco_error = PCO_OpenNextCamera(&pco_cam);
  if (pco_error != PCO_NOERROR)
  {
    char err_string[256];
    PCO_GetErrorTextSDK(pco_error, err_string, 256);
    printf("%s\nLine: %d.\n", err_string, __LINE__ - 5);
    PCO_CleanupLib();
    return (int)pco_error;
  }

  printf("Get Camera description \n");
  PCO_Description descriptor = PCO_Description{sizeof(PCO_Description)};
  pco_error = PCO_GetCameraDescription(pco_cam, &descriptor);
  ERRABORT(pco_cam, pco_error);

  if(descriptor.dwGeneralCapsDESC1&GENERALCAPS1_NO_RECORDER)
  {
    pco_error = PCO_CloseCamera(pco_cam);
    printf("Camera found, but no recorder available\n");
    printf("Press <Enter> to end\n");
    getchar();
    PCO_CleanupLib();
    return -1;
  }

  WORD recording_state;
  pco_error = PCO_GetRecordingState(pco_cam, &recording_state);
  if (recording_state)
  {
    pco_error = PCO_SetRecordingState(pco_cam, 0);
    ERRABORT(pco_cam, pco_error);
  }

  // set camera to default state
  pco_error = PCO_ResetSettingsToDefault(pco_cam);
  ERRABORT(pco_cam, pco_error);

#ifdef FILEFUNCTION
  pco_error = PCO_SetTimestampMode(pco_cam, TIMESTAMP_MODE_BINARYANDASCII);
  ERRABORT(pco_cam, pco_error);
#endif

  printf("Call PCO_ArmCamera \n");
  pco_error = PCO_ArmCamera(pco_cam);
  ERRABORT(pco_cam, pco_error);

  DWORD cam_warning, cam_error, cam_status;
  pco_error = PCO_GetCameraHealthStatus(pco_cam, &cam_warning, &cam_error, &cam_status);
  ERRABORT(pco_cam, cam_error);

  WORD timestamp_mode;
  pco_error = PCO_GetTimestampMode(pco_cam,&timestamp_mode);


  DWORD sec,nsec,msec;
  pco_error = PCO_GetCOCRuntime(pco_cam, &sec,&nsec);
  ERRABORT(pco_cam, cam_error);
  printf("Time for one image %dms\n",sec*1000+nsec/1000000);

  msec=sec*1000+nsec/1000000;
  msec+=100;
  printf("Start and after %d seconds stop camera \n",(msec*IMAGE_SEQUENCE_COUNT)/1000);
  pco_error = PCO_SetRecordingState(pco_cam, 1);

//wait while camera is recording
  for(int i=0;i<IMAGE_SEQUENCE_COUNT; i++)
  {
    std::this_thread::sleep_for (std::chrono::milliseconds(msec));
    printf(".");
    fflush(stdout);
  }
  printf("\n");

  pco_error = PCO_SetRecordingState(pco_cam, 0);

  DWORD ValidImageCnt, MaxImageCnt;
  WORD Segment=1; //this is the default segment
  pco_error = PCO_GetNumberOfImagesInSegment(pco_cam, Segment, &ValidImageCnt, &MaxImageCnt);
  ERRABORT(pco_cam, cam_error);

  if(ValidImageCnt >= 1)
  {

    WORD width, height, XBin, YBin;
    WORD RoiX0, RoiY0, RoiX1, RoiY1;
    pco_error = PCO_GetSegmentImageSettings(pco_cam,Segment, &width, &height,
               &XBin, &YBin, &RoiX0, &RoiY0, &RoiX1, &RoiY1);
    ERRABORT(pco_cam, cam_error);

    DWORD buffer_size = width*height*sizeof(WORD);

    short buffer_num = -1;
    WORD *buffer_addr = nullptr;
    pco_error = PCO_AllocateBuffer(pco_cam, &buffer_num, buffer_size, &buffer_addr, nullptr); // no event parameter
    ERRABORT(pco_cam, pco_error);

    pco_error = PCO_SetImageParameters(pco_cam, width, height, IMAGEPARAMETERS_READ_FROM_SEGMENTS, nullptr, 0);
    ERRABORT(pco_cam, pco_error);

    printf("Read recorded images from camera actual valid %d\n",ValidImageCnt);
    DWORD img_num=1;

    for (int i = 0; i < IMAGE_SEQUENCE_COUNT; i++,img_num++)
    {
      printf("%02d. image ", img_num);
      if(ValidImageCnt < img_num)
      {
       printf("not available \n");
       break;
      }

      pco_error = PCO_GetImageEx(pco_cam, 1, img_num, img_num, buffer_num, width, height, 16);
      if (pco_error != PCO_NOERROR)
      {
        printf("failed: 0x%08x\n", pco_error);
        break;
      }
      printf("done ");

      if((timestamp_mode==TIMESTAMP_MODE_BINARY)||(timestamp_mode==TIMESTAMP_MODE_BINARYANDASCII))
      {
        PCO_TIMESTAMP_STRUCT strTimeStamp;
        strTimeStamp.wSize = sizeof(PCO_TIMESTAMP_STRUCT);
        pco_error = PCO_GetTimeStamp(pco_cam, buffer_num, nullptr,&strTimeStamp);
        printf("img_num % 4d",strTimeStamp.dwImgCounter);
      }

      printf("\n");

#ifdef FILEFUNCTION
      char file_path[256];
      snprintf(file_path, 256, "image_single_mono16_(%02d).tif", img_num);
      write_tiff(file_path, buffer_addr, width, height, 16);
#endif
    }

    pco_error = PCO_CancelImages(pco_cam);
    ERRABORT(pco_cam, pco_error);

    pco_error = PCO_FreeBuffer(pco_cam, buffer_num);
    ERRABORT(pco_cam, pco_error);
  }
  printf("Close connection\n");

  pco_error = PCO_CloseCamera(pco_cam);
  if (pco_error != PCO_NOERROR)
  {
    char err_string[256];
    PCO_GetErrorTextSDK(pco_error, err_string, 256);
    printf("%s\nLine: %d.\n", err_string, __LINE__ - 5);
    PCO_CleanupLib();
    return (int)pco_error;
  }
  PCO_CleanupLib();
  printf("Exit: Success\n");
  return EXIT_SUCCESS;
}


#ifdef FILEFUNCTION
void write_tiff(const char *file_path, WORD *data, DWORD width, DWORD height, WORD bits_per_pixel)
{
  TIFF *tif_out = TIFFOpen(file_path, "w");
  if (!tif_out)
  {
    return;
  }

  TIFFSetField(tif_out, TIFFTAG_IMAGEWIDTH, width);
  TIFFSetField(tif_out, TIFFTAG_IMAGELENGTH, height);
  TIFFSetField(tif_out, TIFFTAG_BITSPERSAMPLE, bits_per_pixel);
  TIFFSetField(tif_out, TIFFTAG_SAMPLESPERPIXEL, 1);

  TIFFSetField(tif_out, TIFFTAG_ROWSPERSTRIP, 1);
  TIFFSetField(tif_out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT); // orientation); // 1?
  TIFFSetField(tif_out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
  TIFFSetField(tif_out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
  TIFFSetField(tif_out, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
  TIFFSetField(tif_out, TIFFTAG_COMPRESSION, COMPRESSION_NONE);

  size_t bytes_per_pixel = (bits_per_pixel + 7) / 8;
  for (uint32 s = 0; s < height; ++s)
  {
    TIFFWriteRawStrip(tif_out, s, data + s * width, width * bytes_per_pixel);
  }
  TIFFClose(tif_out);
  return;
}
#endif

#include <iostream>
#include <vector>
#include <cstring>

#ifdef _FILEFUNCTION_
#include "../libtiff_header/tiff.h"
#include "../libtiff_header/tiffio.h"
#endif

#include <pco_linux_defs.h>

#include <pco_device.h>
#include <sc2_defs.h>
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

void print_transfer_parameter(HANDLE cam);
#ifdef FILEFUNCTION
void write_tiff(const char *file_path, WORD *data, DWORD width, DWORD height, WORD bits_per_pixel);
#endif

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

  print_transfer_parameter(pco_cam);

  WORD width, height, width_max, height_max;
  DWORD buffer_size;
  pco_error = PCO_GetSizes(pco_cam, &width, &height, &width_max, &height_max);
  ERRABORT(pco_cam, pco_error);

  buffer_size = (DWORD)width * (DWORD)height * sizeof(WORD);

  short buffer_num = -1;
  WORD *buffer_addr = nullptr;
  pco_error = PCO_AllocateBuffer(pco_cam, &buffer_num, buffer_size, &buffer_addr, nullptr); // no event parameter
  ERRABORT(pco_cam, pco_error);

  pco_error = PCO_SetImageParameters(pco_cam, width, height, IMAGEPARAMETERS_READ_WHILE_RECORDING, nullptr, 0);
  ERRABORT(pco_cam, pco_error);

  printf("Start recording ...\n");
  pco_error = PCO_SetRecordingState(pco_cam, 1);

  printf("Grab single images from running camera\n");

  DWORD current_status_driver, current_status_sdk;
  WORD *current_buffer_addr = buffer_addr;
  HANDLE not_used;
#define IMAGE_TIMEOUT_MS 2'000
#define IMAGE_SEQUENCE_COUNT 10
  for (int img_num = 0; img_num < IMAGE_SEQUENCE_COUNT; ++img_num)
  {
    printf("%02d. image wait ... ", img_num);

    pco_error = PCO_GetImageEx(pco_cam, 1, 0, 0, buffer_num, width, height, 16);

    if (pco_error != PCO_NOERROR)
    {
      printf("failed: 0x%08x\n", pco_error);
      break;
    }

    printf(" pco buffer %d ", buffer_num);

    current_status_driver = PCO_ERROR_WRONGVALUE;
    pco_error = PCO_GetBufferStatus(pco_cam, buffer_num, &current_status_sdk, &current_status_driver);
    ERRBREAK(pco_cam, pco_error);

    // always check driver status for errors:
    if (current_status_driver == PCO_NOERROR && pco_error == PCO_NOERROR)
    {
      printf("done status 0x%08x\n", current_status_driver);
    }
    else
    {
      printf("failed image transmission pco buffer %02d. Error: 0x%08x\n", buffer_num, current_status_driver);
      break;
    }

    pco_error = PCO_GetBuffer(pco_cam, buffer_num, &current_buffer_addr, &not_used);
    ERRBREAK(pco_cam, pco_error);

#ifdef FILEFUNCTION
    char file_path[256];
    snprintf(file_path, 256, "image_single_mono16_(%02d).tif", img_num);
    write_tiff(file_path, buffer_addr, width, height, 16);
#endif
  }

  pco_error = PCO_CancelImages(pco_cam);
  ERRABORT(pco_cam, pco_error);

  printf("Stop camera and close connection\n");

  pco_error = PCO_SetRecordingState(pco_cam, 0);
  ERRABORT(pco_cam, pco_error);

  pco_error = PCO_FreeBuffer(pco_cam, buffer_num);
  ERRABORT(pco_cam, pco_error);

  pco_error = PCO_CloseCamera(pco_cam);
  if (pco_error != PCO_NOERROR)
  {
    char err_string[256];
    PCO_GetErrorTextSDK(pco_error, err_string, 256);
    printf("%s\nLine: %d.\n", err_string, __LINE__ - 5);
    PCO_CleanupLib();
    return (int)pco_error;
  }
  printf("Exit: Success\n");
  PCO_CleanupLib();
  return EXIT_SUCCESS;
}

void print_transfer_parameter(HANDLE cam)
{
  return;
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

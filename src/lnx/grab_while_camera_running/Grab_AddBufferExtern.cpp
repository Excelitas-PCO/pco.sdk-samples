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
  DWORD word_buffer_size;
  DWORD buffer_size;
  pco_error = PCO_GetSizes(pco_cam, &width, &height, &width_max, &height_max);
  ERRABORT(pco_cam, pco_error);

  word_buffer_size=(DWORD)width * (DWORD)height;
  buffer_size = word_buffer_size * sizeof(WORD);

#define BUFFER_ARRAY_SIZE 4
  DWORD buffer_extern_status[BUFFER_ARRAY_SIZE]{PCO_ERROR_WRONGVALUE, PCO_ERROR_WRONGVALUE, PCO_ERROR_WRONGVALUE, PCO_ERROR_WRONGVALUE};

  std::vector<WORD> buffer_extern[BUFFER_ARRAY_SIZE];
  for (int b = 0; b < BUFFER_ARRAY_SIZE; ++b)
  {
    buffer_extern[b].resize(word_buffer_size, 0);
  }

  pco_error = PCO_SetImageParameters(pco_cam, width, height, IMAGEPARAMETERS_READ_WHILE_RECORDING, nullptr, 0);
  ERRABORT(pco_cam, pco_error);

  printf("Start recording ...\n");
  pco_error = PCO_SetRecordingState(pco_cam, 1);
  for (int b = 0; b < BUFFER_ARRAY_SIZE; ++b)
  {
    pco_error = PCO_AddBufferExtern(pco_cam, nullptr, 1, 0, 0, 0, buffer_extern[b].data(), buffer_size, &buffer_extern_status[b]);
    ERRABORT(pco_cam, pco_error);
  }

  printf("Grab images from running camera\n");

  DWORD current_status_driver=PCO_ERROR_DRIVER_BUFFERS_PENDING;
  short current_buffer_num = -1;
  void *current_buffer_addr = nullptr;
#define IMAGE_TIMEOUT_MS 2'000
#define IMAGE_SEQUENCE_COUNT 10
  for (int img_num = 0; img_num < IMAGE_SEQUENCE_COUNT; ++img_num)
  {
    printf("%02d. image wait ... ", img_num);

    pco_error = PCO_WaitforNextBufferAdr(pco_cam, &current_buffer_addr, IMAGE_TIMEOUT_MS);
    if ((pco_error & PCO_ERROR_TIMEOUT) == PCO_ERROR_TIMEOUT)
    {
      printf("failed: timed out.\n");
    }
    else if (pco_error != PCO_NOERROR)
    {
      printf("failed: 0x%08x\n", pco_error);
      break;
    }

    for (int b = 0; b < BUFFER_ARRAY_SIZE; ++b)
    {
      if (current_buffer_addr == buffer_extern[b].data())
      {
        current_status_driver = buffer_extern_status[b];
        current_buffer_num = b;
      }
    }

    // always check driver status for errors:
    if (current_status_driver == PCO_NOERROR)
    {
      printf("done buffer %02d status 0x%08x\n", current_buffer_num, current_status_driver);
    }
    else
    {
      printf("failed image transmission buffer %02d. Error: 0x%08x\n", current_buffer_num, current_status_driver);
      continue;
    }

    if(current_buffer_num>=0)
    {
#ifdef FILEFUNCTION
      char file_path[256];
      snprintf(file_path, 256, "image_ext_mono16_(%02d).tif", img_num);
      write_tiff(file_path, (WORD*)current_buffer_addr, width, height, 16);
#endif

      buffer_extern_status[current_buffer_num]=PCO_ERROR_WRONGVALUE;
      pco_error = PCO_AddBufferExtern(pco_cam, nullptr, 1, 0, 0, 0, current_buffer_addr, buffer_size, &buffer_extern_status[current_buffer_num]);
      ERRABORT(pco_cam, pco_error);
    }
  }

  pco_error = PCO_CancelImages(pco_cam);
  ERRABORT(pco_cam, pco_error);

  printf("Stop camera and close connection\n");

  pco_error = PCO_SetRecordingState(pco_cam, 0);
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
  PCO_CleanupLib();
  printf("Exit: Success\n");
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

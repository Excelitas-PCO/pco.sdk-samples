#include <iostream>
#include <vector>
#include <cstring>

#include <pco_linux_defs.h>

#include <pco_device.h>
#include <sc2_defs.h>
#include <sc2_sdkaddendum.h>
#include <sc2_sdkstructures.h>
#include <sc2_camexport.h>
#include <pco_camexport.h>

#include <pco_err.h>

#define UNDEF_W 0xFFFF

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

typedef WORD camera_interface;
DWORD open(WORD ci = UNDEF_W);
DWORD open(WORD ci)
{
  int err = PCO_NOERROR;
  HANDLE sdk_ = nullptr;

  // create struct containing information about to-be-opened camera
  PCO_OpenStruct open_information{sizeof(PCO_OpenStruct)};

  std::vector<camera_interface> interfaces =
      {PCO_INTERFACE_USB3, PCO_INTERFACE_CLHS,
       PCO_INTERFACE_CL_ME4, PCO_INTERFACE_GENICAM,
       PCO_INTERFACE_GIGE, PCO_INTERFACE_USB,
       PCO_INTERFACE_FW}; // in order of appearance

  // If an interface is specified, only scan for this one
  if (ci != UNDEF_W)
  {
    interfaces.clear();
    interfaces.push_back(ci);
  }

  for (auto &ci_iter : interfaces)
  {
    open_information.wInterfaceType = ci_iter;
    for (WORD i = 0; i <= 10; i++)
    {
      open_information.wCameraNumAtInterface = i;
      open_information.wCameraNumber = i;
      printf("call PCO_OpenCameraEx if %d num %d\n",open_information.wInterfaceType,open_information.wCameraNumber);
      err = PCO_OpenCameraEx(&sdk_, &open_information);
//if camera is not found open_information.wCameraNumber is set to 0xFFFF     
      printf("PCO_OpenCameraEx if %d num %d str_num %d returned err 0x%x\n",open_information.wInterfaceType,i,open_information.wCameraNumber,err);
      if (err == PCO_NOERROR) // camera was found on port i
      {
        break;
      }
      else if ((err & 0x8000FFFF) == PCO_ERROR_DRIVER_NOTINIT) // no camera found on port i
      {
        printf("err PCO_ERROR_DRIVER_NOTINIT continue\n");
        continue;
      }
      else if ((err & 0x8000FFFF) == PCO_ERROR_DRIVER_DEVICEBUSY) // no camera found on port i
      {
        printf("err PCO_ERROR_DRIVER_DEVICEBUSY continue\n");
        continue;
      }
      else // no camera found on this interface
      {
        printf("other error break\n");
        break;
      }
    }
    if (err == PCO_NOERROR) // camera was found on port i
      break;
    // std::cout << "err: " << std::hex << err << std::dec << "\n";
  }
  ERRABORT(sdk_, err);

  if(err==PCO_NOERROR)
  {
    fflush(stdin);
    printf("An key 'CR' to close camera and application\n");
    getchar();
    err = PCO_CloseCamera(sdk_);
    if (err)
      printf("PCO_CloseCamera failed: 0x%08x\n", err);
  }
  PCO_CleanupLib();
  return err;
}

int main()
{
  DWORD err;

  err = PCO_InitializeLib();
  if (err)
  {
    return err;
  }

  err = open();
  if (err != PCO_NOERROR)
    printf("Exit: Failed (0x%x)", err);
  printf("Exit: Success\n");
  
  err = open(PCO_INTERFACE_USB);
  if (err != PCO_NOERROR)
    printf("Exit: Failed (0x%x)", err);
  printf("Exit: Success\n");
  
  PCO_CleanupLib();
  return 0;
}

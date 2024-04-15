# pco.sdk-samples
This project contains different sample projects showing how to use Excelitas PCO's pco.sdk package,   
which can be downloaded here: [pco.sdk](https://www.excelitas.com/de/product/pco-software-development-kits#custom-tab-general-sdk)

If you are looking for a more high-level, easier, API, please have a look at our
[pco.cpp](https://www.excelitas.com/product/pco-software-development-kits#custom-tab-c__) package and the corresponding
[pco.cpp-samples](https://github.com/Excelitas-PCO/pco.cpp-samples) on GitHub.

## Project Structure
 
```
.gitignore
README.md
LICENSE
CMakePresets.json
CMakeLists.txt
- externals
  - pco
- src
  - win
    - console
    - sc2_demo
    - test_cvdlg
  - lnx
    - grab_while_camera_running
    - read_from_camera_camram
    - simple_open
    - libtiff_header
```

**CMakeLists.txt** is the main cmake file and **CMakePresets.json** contains already predefined presets for building debug and release,
both on windows and linux platforms

All examples are in the **src** subfolder.  
The **externals/pco** folder contains also a **CMakeLists.txt** file which handles the pco.sdk dependencies

## Sample Description

Depending on the operating system there is a different set of examples available, for windows and linux

### Windows Samples

#### console
A set of basic console examples showing how to access PCO cameras and record images

#### sc2_demo
A small mfc-based GUI software to show how to build image acquisition software for PCO cameras

#### test_cvdlg
A graphical, mfc-based, test app, showing how to use the convert dialog in custom software to do image (color) conversion.

### Linux Samples

#### simple_open
A set of basic console examples showing how to access/open PCO cameras and query information

#### grab_while_camera_running
A set of console examples showing how to open, record and get images from a PCO camera

#### read_from_camera_camram
A set of console examples showing how to read images from a PCO camera with internal memory


## Installation

To use this example project you can either clone, fork or download the source code. 
After you have configured everything to your needs you can simply configure, build and install it using cmake.

### Configuration

The **CMakePresets.json** contain already predefined configurations for cmake builds on windows and linux.  

Beside of the preset name and description we have the following variables which you can configure to your needs: 

#### generator 
Here we use *Ninja* as it is available both on linux and on windows systems, but you can of course change this

#### binaryDir
This defines where the build files go to.  
Our default here is *<preset name>/build*, so e.g. *release_lnx/build* for the *release_lnx* preset

#### CMAKE_BUILD_TYPE
Build type. This matches with our preset names

#### CMAKE_INSTALL_PREFIX
This defines where the files should be installed to when calling ```cmake --install```
Our default here is *<preset name>/install*, so e.g. *release_lnx/install* for the *release_lnx* preset

#### PCO_PACKAGE_INSTALL_DIR
This specifies the root path to your installation of the **pco.sdk** package.  
Our default here is the system wide installation path, so normally you do not need to change it.  
If you installed pco.sdk on windows as user, you need to adapt this to the actual installation path of pco.sdk

#### AUTO_UPDATE_PCO_PACKAGE
If this flag is set to true, the *./externals/pco/CMakeLists.txt* will automatically update the **pco.sdk** related files from the pco.sdk install path, e.g. when you install a new version of pco.sdk the examples will automatically be updated on the next reconfiguration.

If you want to disable this mechanism, just set ```"AUTO_UPDATE_PCO_PACKAGE": false``` 

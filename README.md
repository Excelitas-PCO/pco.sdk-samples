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
  - ColorConvertExample
  - MultiCameraExample
  - SimpleExample
  - SimpleExample_CamRam
  - SimpleExample_FIFO
```

**CMakeLists.txt** is the main cmake file and **CMakePresets.json** contains already predefined presets for building debug and release,
both on windows and linux platforms

All examples are in the **src** subfolder.  
The **externals/pco** folder contains also a **CMakeLists.txt** file which handles the pco.cpp dependencies

## Sample Description



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
This specifies the root path to your installation of the **pco.cpp** package.  
Our default here is the system wide installation path, so normally you do not need to change it.  
If you installed pco.cpp on windows as user, you need to adapt this to the actual installation path of pco.cpp

#### AUTO_UPDATE_PCO_PACKAGE
If this flag is set to true, the *./externals/pco/CMakeLists.txt* will automatically update the **pco.cpp** related files from the pco.cpp install path, e.g. when you install a new version of pco.cpp the examples will automatically be updated on the next reconfiguration.

If you want to disable this mechanism, just set ```"AUTO_UPDATE_PCO_PACKAGE": false``` 

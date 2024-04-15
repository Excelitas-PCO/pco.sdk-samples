set(PROJECT_NAME SC2_Demo)

set(PROJECT_SOURCES
  ChildView.cpp
  MainFrm.cpp
  stdafx.cpp
  SC2_Class.cpp
  SC2_Demo.cpp
  SC2_Demo.rc
  resource.h
)

set(CMAKE_MFC_FLAG 1)
    
add_executable(${PROJECT_NAME} WIN32 ${PROJECT_SOURCES})
set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

include_directories(${PCO_FOLDER})
include_directories(${PCO_FOLDER}/include)

target_link_libraries(${PROJECT_NAME} PRIVATE pco_convert)
target_link_libraries(${PROJECT_NAME} PRIVATE sc2_cam)
target_link_libraries(${PROJECT_NAME} PRIVATE sc2_dlg)
target_link_libraries(${PROJECT_NAME} PRIVATE pco_conv_dlg)

install(TARGETS ${PROJECT_NAME})

set(PROJECT_NAME Test_cvDlg)

set(PROJECT_SOURCES
  File12.cpp
  ChildView.cpp
  MainFrm.cpp
  stdafx.cpp
  Test_cvDlg.cpp
  Test_cvDlg.rc
  resource.h
)

set(CMAKE_MFC_FLAG 1)
    
add_executable(${PROJECT_NAME} WIN32 ${PROJECT_SOURCES})
set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

include_directories(${PCO_FOLDER})
include_directories(${PCO_FOLDER}/include)

target_link_libraries(${PROJECT_NAME} PRIVATE pco_convert)
target_link_libraries(${PROJECT_NAME} PRIVATE sc2_cam)
#target_link_libraries(${PROJECT_NAME} PRIVATE sc2_dlg)
target_link_libraries(${PROJECT_NAME} PRIVATE pco_conv_dlg)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/test.tif" DESTINATION "${CMAKE_BINARY_DIR}")
install(FILES ${CMAKE_BINARY_DIR}/test.tif DESTINATION bin)

install(TARGETS ${PROJECT_NAME})

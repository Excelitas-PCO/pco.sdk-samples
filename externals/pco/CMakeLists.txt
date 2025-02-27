# Copy pco.cpp files to local externals/pco
# If you do not want an auto update, set AUTO_UPDATE_PCO_PACKAGE to false
if(${AUTO_UPDATE_PCO_PACKAGE})
  if(WIN32)
    file(COPY
      ${PCO_PACKAGE_INSTALL_DIR}/include
      DESTINATION
      ${CMAKE_CURRENT_SOURCE_DIR}
      )
      file(GLOB LIB_FILES "${PCO_PACKAGE_INSTALL_DIR}/lib64/*.lib")
      file(GLOB DLL_FILES "${PCO_PACKAGE_INSTALL_DIR}/bin64/*.dll")
      file(COPY ${DLL_FILES} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)
      file(COPY ${LIB_FILES} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/lib)
      file(COPY ${PCO_PACKAGE_INSTALL_DIR}/bin64/genicam DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)
  else()
    file(COPY
      ${PCO_PACKAGE_INSTALL_DIR}/include
      ${PCO_PACKAGE_INSTALL_DIR}/lib
      DESTINATION
      ${CMAKE_CURRENT_SOURCE_DIR}
      )
  endif()
endif()

add_library(pco_convert SHARED IMPORTED GLOBAL)
add_library(sc2_cam SHARED IMPORTED GLOBAL)

if(WIN32)
  set_property(TARGET pco_convert PROPERTY IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/lib/pco_conv.lib")
  set_property(TARGET pco_convert PROPERTY IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/bin/pco_conv.dll")
  set_property(TARGET sc2_cam PROPERTY IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/lib/sc2_cam.lib")
  set_property(TARGET sc2_cam PROPERTY IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/bin/sc2_cam.dll")

  add_library(sc2_dlg SHARED IMPORTED GLOBAL)
  set_property(TARGET sc2_dlg PROPERTY IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/lib/sc2_dlg.lib")
  set_property(TARGET sc2_dlg PROPERTY IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/bin/sc2_dlg.dll")

  add_library(pco_conv_dlg SHARED IMPORTED GLOBAL)
  set_property(TARGET pco_conv_dlg PROPERTY IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/lib/pco_cdlg.lib")
  set_property(TARGET pco_conv_dlg PROPERTY IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/bin/pco_cdlg.dll")
  
  file(GLOB SC2_LIB_FILES "${CMAKE_CURRENT_SOURCE_DIR}/bin/sc2_*.dll")
  file(GLOB PCO_LIB_FILES "${CMAKE_CURRENT_SOURCE_DIR}/bin/pco_*.dll")

  file(COPY ${SC2_LIB_FILES} ${PCO_LIB_FILES} DESTINATION "${CMAKE_BINARY_DIR}")
  file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/bin/genicam" DESTINATION "${CMAKE_BINARY_DIR}")
  install(FILES ${SC2_LIB_FILES} ${PCO_LIB_FILES} DESTINATION bin)
  install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/genicam" DESTINATION bin)

else()
  set_property(TARGET pco_convert PROPERTY IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/lib/libpco_convert.so")
  set_property(TARGET pco_convert PROPERTY IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/libpco_convert.so")
  set_property(TARGET sc2_cam PROPERTY IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/lib/libpco_sc2cam.so")
  set_property(TARGET sc2_cam PROPERTY IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/libpco_sc2cam.so")
  file(GLOB LIB_FILES "${CMAKE_CURRENT_SOURCE_DIR}/lib/libpco*.so*")
  file(COPY ${LIB_FILES} DESTINATION "${CMAKE_BINARY_DIR}")
  file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/lib/genicam" DESTINATION "${CMAKE_BINARY_DIR}")

  install(FILES ${LIB_FILES} DESTINATION lib)
  install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib/genicam" DESTINATION lib)
endif()


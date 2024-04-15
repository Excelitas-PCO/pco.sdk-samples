set(PROJECT_NAME AddBuffer_CamRecorder)

set(PROJECT_SOURCES
  AddBuffer_CamRecorder.cpp
)
    
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

include_directories(${PCO_FOLDER})
include_directories(${PCO_FOLDER}/include)

target_link_libraries(${PROJECT_NAME} PRIVATE pco_convert)
target_link_libraries(${PROJECT_NAME} PRIVATE sc2_cam)

install(TARGETS ${PROJECT_NAME})

####################################################################

set(PROJECT_NAME AddBuffer_CamRun)

set(PROJECT_SOURCES
  AddBuffer_CamRun.cpp
)
    
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

include_directories(${PCO_FOLDER})
include_directories(${PCO_FOLDER}/include)

target_link_libraries(${PROJECT_NAME} PRIVATE pco_convert)
target_link_libraries(${PROJECT_NAME} PRIVATE sc2_cam)

install(TARGETS ${PROJECT_NAME})

####################################################################

set(PROJECT_NAME GetImage_CamRecorder)

set(PROJECT_SOURCES
  GetImage_CamRecorder.cpp
)
    
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

include_directories(${PCO_FOLDER})
include_directories(${PCO_FOLDER}/include)

target_link_libraries(${PROJECT_NAME} PRIVATE pco_convert)
target_link_libraries(${PROJECT_NAME} PRIVATE sc2_cam)

install(TARGETS ${PROJECT_NAME})

####################################################################

set(PROJECT_NAME GetImage_CamRun)

set(PROJECT_SOURCES
  GetImage_CamRun.cpp
)
    
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

include_directories(${PCO_FOLDER})
include_directories(${PCO_FOLDER}/include)

target_link_libraries(${PROJECT_NAME} PRIVATE pco_convert)
target_link_libraries(${PROJECT_NAME} PRIVATE sc2_cam)

install(TARGETS ${PROJECT_NAME})

####################################################################

set(PROJECT_NAME AddBufferExt_CamRun)

set(PROJECT_SOURCES
  AddBufferExt_CamRun.cpp
)
    
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

include_directories(${PCO_FOLDER})
include_directories(${PCO_FOLDER}/include)

target_link_libraries(${PROJECT_NAME} PRIVATE pco_convert)
target_link_libraries(${PROJECT_NAME} PRIVATE sc2_cam)

install(TARGETS ${PROJECT_NAME})

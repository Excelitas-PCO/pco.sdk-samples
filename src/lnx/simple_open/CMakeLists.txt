
set(PROJECT_NAME pco_cam_open)

set(PROJECT_SOURCES
    pco_cam_open.cpp
)
    
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

include_directories(${PCO_FOLDER})
include_directories(${PCO_FOLDER}/include)

target_link_libraries(${PROJECT_NAME} PRIVATE pco_convert)
target_link_libraries(${PROJECT_NAME} PRIVATE sc2_cam)

install(TARGETS ${PROJECT_NAME})

####################################################################

set(PROJECT_NAME pco_cam_openex)


set(PROJECT_SOURCES
    pco_cam_openex.cpp
)
    
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

include_directories(${PCO_FOLDER})
include_directories(${PCO_FOLDER}/include)

target_link_libraries(${PROJECT_NAME} PRIVATE pco_convert)
target_link_libraries(${PROJECT_NAME} PRIVATE sc2_cam)

install(TARGETS ${PROJECT_NAME})

####################################################################

set(PROJECT_NAME pco_cam_getversions)

set(PROJECT_SOURCES
    pco_cam_getversions.cpp
)
    
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

include_directories(${PCO_FOLDER})
include_directories(${PCO_FOLDER}/include)

target_link_libraries(${PROJECT_NAME} PRIVATE pco_convert)
target_link_libraries(${PROJECT_NAME} PRIVATE sc2_cam)

install(TARGETS ${PROJECT_NAME})

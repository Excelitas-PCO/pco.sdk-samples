set(PROJECT_NAME Grab_AddBufferEx)

set(PROJECT_SOURCES
    Grab_AddBufferEx.cpp
)
    
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

include_directories(${PCO_FOLDER})
include_directories(${PCO_FOLDER}/include)

target_link_libraries(${PROJECT_NAME} PRIVATE pco_convert)
target_link_libraries(${PROJECT_NAME} PRIVATE sc2_cam)

install(TARGETS ${PROJECT_NAME})

####################################################################

set(PROJECT_NAME Grab_AddBufferExtern)

set(PROJECT_SOURCES
    Grab_AddBufferExtern.cpp
)
    
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

include_directories(${PCO_FOLDER})
include_directories(${PCO_FOLDER}/include)

target_link_libraries(${PROJECT_NAME} PRIVATE pco_convert)
target_link_libraries(${PROJECT_NAME} PRIVATE sc2_cam)

install(TARGETS ${PROJECT_NAME})

####################################################################

set(PROJECT_NAME Grab_GetImageEx)

set(PROJECT_SOURCES
    Grab_GetImageEx.cpp
)
    
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

include_directories(${PCO_FOLDER})
include_directories(${PCO_FOLDER}/include)

target_link_libraries(${PROJECT_NAME} PRIVATE pco_convert)
target_link_libraries(${PROJECT_NAME} PRIVATE sc2_cam)

install(TARGETS ${PROJECT_NAME})

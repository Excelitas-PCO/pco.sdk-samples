set(PROJECT_NAME Read_AddBufferEx)

set(PROJECT_SOURCES
    Read_AddBufferEx.cpp
)
    
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

include_directories(${PCO_FOLDER})
include_directories(${PCO_FOLDER}/include)

target_link_libraries(${PROJECT_NAME} PRIVATE pco_convert)
target_link_libraries(${PROJECT_NAME} PRIVATE sc2_cam)

install(TARGETS ${PROJECT_NAME})

####################################################################

set(PROJECT_NAME Read_AddBufferExtern)

set(PROJECT_SOURCES
    Read_AddBufferExtern.cpp
)
    
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

include_directories(${PCO_FOLDER})
include_directories(${PCO_FOLDER}/include)

target_link_libraries(${PROJECT_NAME} PRIVATE pco_convert)
target_link_libraries(${PROJECT_NAME} PRIVATE sc2_cam)

install(TARGETS ${PROJECT_NAME})

####################################################################

set(PROJECT_NAME Read_GetImageEx)

set(PROJECT_SOURCES
    Read_GetImageEx.cpp
)
    
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

include_directories(${PCO_FOLDER})
include_directories(${PCO_FOLDER}/include)

target_link_libraries(${PROJECT_NAME} PRIVATE pco_convert)
target_link_libraries(${PROJECT_NAME} PRIVATE sc2_cam)

install(TARGETS ${PROJECT_NAME})

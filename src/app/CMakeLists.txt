project(PPLaserRemoteServer)

############### Files #################

set (SOURCES src/bluetoothserver.cpp
             src/tcpserver.cpp
             src/logger.cpp
             src/gui.cpp
             src/main.cpp)
set (HEADERS src/bluetoothserver.hpp
             src/tcpserver.hpp
             src/logger.hpp
             src/gui.hpp
             src/resource.h)
set (RESOURCES resources/ppremote.rc
               resources/bluetooth.ico
               resources/laser_icon.ico
               resources/laser_icon_off.ico
               resources/laser_icon_on.ico
               resources/tcp.ico
               resources/update.ico)

#######################################


########### Group Sources #############

source_group("Resources" FILES ${RESOURCES})

#######################################



########### Include dirs ##############

include_directories (src resources)

#######################################


######### Config Executable ###########

add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS} ${RESOURCES})
if(WIN32)
    target_link_libraries(${PROJECT_NAME} ws2_32)
endif()
target_link_libraries(${PROJECT_NAME} UpdateDownloader XmlConfig)
add_definitions(-DUNICODE -D_USE_MATH_DEFINES)

#######################################

############ Installation #############

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}_config
        ARCHIVE DESTINATION lib COMPONENT lib
        LIBRARY DESTINATION lib COMPONENT lib
        RUNTIME DESTINATION bin COMPONENT bin
)
    
    
#######################################
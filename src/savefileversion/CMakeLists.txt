project(SaveFileInfo)

############### Files #################

set (SOURCES savefileinfo.cpp)

#######################################


######### Config Executable ###########

add_executable(${PROJECT_NAME} ${SOURCES})
if(WIN32)
    target_link_libraries(${PROJECT_NAME} Version)
endif()
target_link_libraries(${PROJECT_NAME} UpdateDownloader)

#######################################

############ Installation #############

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}_config
        ARCHIVE DESTINATION lib COMPONENT lib
        LIBRARY DESTINATION lib COMPONENT lib
        RUNTIME DESTINATION bin COMPONENT bin
)
    
#######################################
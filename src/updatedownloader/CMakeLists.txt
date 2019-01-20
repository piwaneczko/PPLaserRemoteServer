project(UpdateDownloader)

############### Files #################

set (SOURCES src/updatedownloader.cpp)
set (HEADERS include/fileinfo.h
             include/updatedownloader.hpp)

#######################################


########### Include dirs ##############

include_directories (include)

#######################################


######### Config Executable ###########

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})
if(WIN32)
    target_link_libraries(${PROJECT_NAME} ws2_32)
endif()
add_definitions(-DUNICODE)

#######################################

############ Installation #############

target_include_directories(${PROJECT_NAME} 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE src
)

install(TARGETS ${PROJECT_NAME}
        ARCHIVE DESTINATION lib COMPONENT lib
        LIBRARY DESTINATION lib COMPONENT lib
        RUNTIME DESTINATION bin COMPONENT bin
)
    
#######################################
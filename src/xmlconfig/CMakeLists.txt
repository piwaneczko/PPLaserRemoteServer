project(XmlConfig)

############### Files #################

set (SOURCES src/xmlconfig.cpp
             src/xmlconfigfile.cpp
             src/pugixml.cpp)
set (HEADERS include/xmlconfig.hpp
             include/pugiconfig.hpp
             include/pugixml.hpp)

#######################################



########### Include dirs ##############

include_directories (include)

#######################################


######### Config Executable ###########

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})

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
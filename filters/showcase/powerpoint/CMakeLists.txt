include_directories(${ZLIB_INCLUDE_DIR} ../../libmso)

if(NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions -fPIC")
endif(NOT WIN32)

kde4_add_library(ppttoodplib STATIC PptToOdp.cpp ParsedPresentation.cpp DateTimeFormat.cpp pptstyle.cpp)
target_link_libraries(ppttoodplib kflake mso ${ZLIB_LIBRARIES})

kde4_add_executable(ppttoodp ppttoodpmain.cpp)
target_link_libraries(ppttoodp ppttoodplib)

kde4_add_plugin(powerpointimport PowerPointImport.cpp)
target_link_libraries(powerpointimport ppttoodplib komain)

install(TARGETS powerpointimport  DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES  showcase_powerpoint_import.desktop  DESTINATION ${SERVICES_INSTALL_DIR})


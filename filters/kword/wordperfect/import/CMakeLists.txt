include_directories(${WPD_INCLUDE_DIR} ${LIBREVENGE_INCLUDE_DIR})

########### next target ###############

set(wpimport_PART_SRCS wpimport.cc )

kde4_add_plugin(wpimport ${wpimport_PART_SRCS})

target_link_libraries(wpimport komain ${WPD_LIBRARIES} ${LIBREVENGE_LIBRARIES})

########### install files ###############

install(TARGETS wpimport DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kword_wp_import.desktop   DESTINATION ${SERVICES_INSTALL_DIR})

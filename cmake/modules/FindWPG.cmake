# - Try to find LibWpg
# Once done this will define
#
#  LIBWPG_FOUND       - libwpg is available
#  LIBWPG_STREAM_FOUND - libwps is available
#  LIBWPG_INCLUDE_DIR - include directory, e.g. /usr/include
#  LIBWPG_LIBRARIES   - the libraries needed to use LibWpg
#  LIBWPG_STREAM_INCLUDE_DIR - include directory, e.g. /usr/include
#  LIBWPG_STREAM_LIBRARIES   - the libraries needed to use LibWps
#  LIBREVENGE_INCLUDE_DIR - include directory, e.g. /usr/include
#  LIBREVENGE_LIBRARIES   - the libraries needed to use LibWps
#  LIBWPG_DEFINITIONS - Compiler switches required for using LibWpg
#
# Copyright (C) 2007 Ariya Hidayat <ariya@kde.org>
# Redistribution and use is allowed according to the terms of the BSD license.

IF (LIBWPG_INCLUDE_DIR AND LIBWPG_LIBRARIES AND LIBWPG_STREAM_INCLUDE_DIR AND LIBWPG_STREAM_LIBRARIES AND LIBREVENGE_INCLUDE_DIR AND LIBREVENGE_LIBRARIES)

  # Already in cache
  set(LIBWPG_FOUND TRUE)
  
ELSE (LIBWPG_INCLUDE_DIR AND LIBWPG_LIBRARIES AND LIBWPG_STREAM_INCLUDE_DIR AND LIBWPG_STREAM_LIBRARIES LIBREVENGE_INCLUDE_DIR AND LIBREVENGE_LIBRARIES)

  IF (NOT WIN32)
    INCLUDE(FindPkgConfig)
    pkg_check_modules(LIBWPG libwpg-0.3)
    pkg_check_modules(LIBWPS libwps-0.3)
    pkg_check_modules(LIBREVENGE librevenge-0.0)
  ENDIF (NOT WIN32)

  FIND_LIBRARY(LIBWPG_STREAM_LIBRARIES NAMES libwps-0.3
	  ${LIBWPG_LIBRARIES}
  )

  FIND_LIBRARY(LIBREVENGE_LIBRARIES NAMES librevenge-0.0
	  ${LIBREVENGE_LIBRARIES}
  )

  FIND_PATH(LIBWPG_INCLUDE_DIR libwpg/libwpg.h
    PATHS
    ${LIBWPG_INCLUDE_DIRS}
    PATH_SUFFIXES libwpg
    )

  FIND_PATH(LIBWPG_STREAM_INCLUDE_DIR libwps/libwps.h
    /usr/include/libwps-0.3
    )

  FIND_PATH(LIBREVENGE_INCLUDE_DIR librevenge/librevenge.h
    /usr/include/librevenge-0.0
    )

  IF (LIBWPG_INCLUDE_DIR AND LIBWPG_LIBRARIES)
    SET(LIBWPG_FOUND TRUE)
  ELSE (LIBWPG_INCLUDE_DIR AND LIBWPG_LIBRARIES) 
    SET(LIBWPG_FOUND FALSE)
  ENDIF (LIBWPG_INCLUDE_DIR AND LIBWPG_LIBRARIES)
  
  IF (LIBWPG_STREAM_INCLUDE_DIR AND LIBWPG_STREAM_LIBRARIES)
    SET(LIBWPG_STREAM_FOUND TRUE)
  ELSE (LIBWPG_STREAM_INCLUDE_DIR AND LIBWPG_STREAM_LIBRARIES) 
    SET(LIBWPG_STREAM_FOUND FALSE)
  ENDIF (LIBWPG_STREAM_INCLUDE_DIR AND LIBWPG_STREAM_LIBRARIES)

  IF (LIBREVENGE_INCLUDE_DIR AND LIBREVENGE_LIBRARIES)
    SET(LIBREVENGE_FOUND TRUE)
  ELSE (LIBREVENGE_INCLUDE_DIR AND LIBREVENGE_LIBRARIES) 
    SET(LIBREVENGE_FOUND FALSE)
  ENDIF (LIBREVENGE_INCLUDE_DIR AND LIBREVENGE_LIBRARIES)
  
  IF (LIBWPG_FOUND)
    MESSAGE(STATUS "Found libwpg: ${LIBWPG_LIBRARIES}")
    MESSAGE("libwpg found " ${LIBWPG_FOUND})
    MESSAGE("libwpg include dir " ${LIBWPG_INCLUDE_DIR})
    MESSAGE("libwpg lib dir " ${LIBWPG_LIBRARY_DIRS})
    MESSAGE("libwpg library " ${LIBWPG_LIBRARIES})
    MESSAGE("libwpg cflags " ${LIBWPG_DEFINITIONS})
  ELSE (LIBWPG_FOUND)
    IF (WPG_FIND_REQUIRED)
      MESSAGE(SEND_ERROR "Could NOT find libwpg")
    ENDIF (WPG_FIND_REQUIRED)
  ENDIF (LIBWPG_FOUND)

  IF (LIBREVENGE_FOUND)
    MESSAGE(STATUS "Found librevenge: ${LIBREVENGE_LIBRARIES}")
    MESSAGE("librevenge found " ${LIBREVENGE_FOUND})
    MESSAGE("librevenge include dir " ${LIBREVENGE_INCLUDE_DIR})
    MESSAGE("librevenge library " ${LIBREVENGE_LIBRARIES})
    MESSAGE("librevenge cflags " ${LIBREVENGE_DEFINITIONS})
  ELSE (LIBREVENGE_FOUND)
    IF (WPG_FIND_REQUIRED)
      MESSAGE(SEND_ERROR "Could NOT find librevenge")
    ENDIF (WPG_FIND_REQUIRED)
  ENDIF (LIBREVENGE_FOUND)

  IF (LIBWPG_STREAM_FOUND)
    MESSAGE(STATUS "Found libwps: ${LIBWPG_STREAM_LIBRARIES}")
    MESSAGE("libwps found " ${LIBWPG_STREAM_FOUND})
    MESSAGE("libwps include dir " ${LIBWPG_STREAM_INCLUDE_DIR})
    MESSAGE("libwps library " ${LIBWPG_STREAM_LIBRARIES})
    MESSAGE("libwps cflags " ${LIBWPG_DEFINITIONS})
  ELSE (LIBWPG_STREAM_FOUND)
    IF (WPG_FIND_REQUIRED)
      MESSAGE(SEND_ERROR "Could NOT find libwps")
    ENDIF (WPG_FIND_REQUIRED)
  ENDIF (LIBWPG_STREAM_FOUND)

ENDIF (LIBWPG_INCLUDE_DIR AND LIBWPG_LIBRARIES AND LIBWPG_STREAM_INCLUDE_DIR AND LIBWPG_STREAM_LIBRARIES AND LIBREVENGE_INCLUDE_DIR AND LIBREVENGE_LIBRARIES)

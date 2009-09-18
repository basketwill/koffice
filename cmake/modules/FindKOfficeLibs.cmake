# - Try to find KOffice Libraries
# Once done this will define
#
#  KOFFICELIBS_FOUND - system has KOffice
#  KOFFICELIBS_INCLUDE_DIR - the Koffice include directory
# KOMAIN_LIBRARY
# KSTORE_LIBRARY
# KOTEXT_LIBRARY
# KOACTION_LIBRARY
# KOPLUGIN_LIBRARY
# KOBASE_LIBRARY
# KOODF_LIBRARY
# KORESOURCES_LIBRARY
# KOPAGEAPP_LIBRARY
# KOKROSS_LIBRARY
# FLAKE_LIBRARY
# KOCOLORWIDGETS_LIBRARY
# KOWIDGETS_LIBRARY
# PIGMENTCMS_LIBRARY
#  KOFFICELIBS_DEFINITIONS - Compiler switches required for using Koffice
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

if ( KOFFICELIBS_INCLUDE_DIR AND KOFFICELIBS_LIBRARIES )
   # in cache already
   SET( KOFFICELIBS_FIND_QUIETLY TRUE )
endif ( KOFFICELIBS_INCLUDE_DIR AND KOFFICELIBS_LIBRARIES )

FIND_PATH( KOFFICELIBS_INCLUDE_DIR NAMES KoDocument.h)# HINTS ${CMAKE_INSTALL_PREFIX}/include)

FIND_LIBRARY(KOMAIN_LIBRARY NAMES komain)
FIND_LIBRARY(KOSTORE_LIBRARY NAMES kostore)
FIND_LIBRARY(KOTEXT_LIBRARY NAMES kotext)
FIND_LIBRARY(KOACTION_LIBRARY NAMES koaction)
FIND_LIBRARY(KOPLUGIN_LIBRARY NAMES koplugin)
FIND_LIBRARY(KOBASE_LIBRARY NAMES kobase)
FIND_LIBRARY(KOODF_LIBRARY NAMES koodf)
FIND_LIBRARY(KORESOURCES_LIBRARY NAMES koresources)
FIND_LIBRARY(KOPAGEAPP_LIBRARY NAMES kopageapp)
FIND_LIBRARY(KOKROSS_LIBRARY NAMES kokross)
FIND_LIBRARY(FLAKE_LIBRARY NAMES flake)
FIND_LIBRARY(PIGMENTCMS_LIBRARY NAMES pigmentcms)
FIND_LIBRARY(KOCOLORWIDGETS_LIBRARY NAMES kocolorwidgets)
FIND_LIBRARY(KOWIDGETS_LIBRARY NAMES kowidgets)


SET( KOFFICECORE_LIBRARIES
    ${KOSTORE_LIBRARY}
    ${KOBASE_LIBRARY}
    ${KOODF_LIBRARY}
    ${FLAKE_LIBRARY}
    ${KOTEXT_LIBRARY}
    ${KOPAGEAPP_LIBRARY}
    ${KOPLUGIN_LIBRARY}
)

SET( KOFFICELIBS_LIBRARIES
    ${KORESOURCES_LIBRARY}
    ${KOKROSS_LIBRARY}
    ${PIGMENTCMS_LIBRARY}
    ${KOCOLORWIDGETS_LIBRARY}
    ${KOACTION_LIBRARY}
    ${KOWIDGETS_LIBRARY}
    ${KOMAIN_LIBRARY}
)

include( FindPackageHandleStandardArgs )

FIND_PACKAGE_HANDLE_STANDARD_ARGS( KOfficeLibs DEFAULT_MSG KOFFICELIBS_INCLUDE_DIR KOFFICECORE_LIBRARIES )

# show the KOFFICELIBS_INCLUDE_DIR and KOFFICELIBS_LIBRARIES variables only in the advanced view
MARK_AS_ADVANCED(KOFFICELIBS_INCLUDE_DIR
    KOMAIN_LIBRARY
    KSTORE_LIBRARY
    KOTEXT_LIBRARY
    KOACTION_LIBRARY
    KOPLUGIN_LIBRARY
    KOBASE_LIBRARY
    KOODF_LIBRARY
    KORESOURCES_LIBRARY
    KOPAGEAPP_LIBRARY
    KOKROSS_LIBRARY
    FLAKE_LIBRARY
    KOCOLORWIDGETS_LIBRARY
    KOWIDGETS_LIBRARY
    PIGMENTCMS_LIBRARY
    KOFFICELIBS_LIBRARIES )


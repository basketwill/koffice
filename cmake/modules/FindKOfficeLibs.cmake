# - Try to find KOffice Libraries
# Once done this will define
#
#  KOFFICELIBS_FOUND - system has Koffice
#  KOFFICELIBS_INCLUDE_DIR - the Koffice include directory
# KOMAIN_LIBRARY 
# KSTORE_LIBRARY
# KOTEXT_LIBRARY
# KOGUIUTILS_LIBRARY
# KOODF_LIBRARY
# KORESOURCES_LIBRARY
# KOPROPERTY_LIBRARY
# KOPAGEAPP_LIBRARY
# KOKROSS_LIBRARY
# FLAKE_LIBRARY
# PIGMENTCMS_LIBRARY
#  KOFFICELIBS_DEFINITIONS - Compiler switches required for using Koffice
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if ( KOFFICELIBS_INCLUDE_DIR AND KOFFICELIBS_LIBRARIES )
   # in cache already
   SET( KOFFICELIBS_FIND_QUIETLY TRUE )
endif ( KOFFICELIBS_INCLUDE_DIR AND KOFFICELIBS_LIBRARIES )

FIND_PATH( KOFFICELIBS_INCLUDE_DIR NAMES KoChild.h
)

FIND_LIBRARY( KOTEXT_LIBRARY NAMES kotext )
FIND_LIBRARY( KSTORE_LIBRARY NAMES kstore )
FIND_LIBRARY( KOMAIN_LIBRARY NAMES komain )
FIND_LIBRARY( KOGUIUTILS_LIBRARY NAMES koguiutils )
FIND_LIBRARY( KORESOURCES_LIBRARY NAMES koresources )
FIND_LIBRARY( KOODF_LIBRARY NAMES koodf )
FIND_LIBRARY( KOPROPERTY_LIBRARY NAMES koproperty )
FIND_LIBRARY( KOPAGEAPP_LIBRARY NAMES kopageapp )
FIND_LIBRARY( KOKROSS_LIBRARY NAMES kokross )
FIND_LIBRARY( FLAKE_LIBRARY NAMES flake )
FIND_LIBRARY( PIGMENTCMS_LIBRARY NAMES pigmentcms )

SET( KOFFICELIBS_LIBRARIES ${KOMAIN_LIBRARY} 
                           ${KSTORE_LIBRARY}
                           ${KOTEXT_LIBRARY}
                           ${KOGUIUTILS_LIBRARY}
                           ${KOODF_LIBRARY}
                           ${KORESOURCES_LIBRARY}
                           ${KOPROPERTY_LIBRARY}
                           ${KOPAGEAPP_LIBRARY}
                           ${KOKROSS_LIBRARY}
                           ${FLAKE_LIBRARY}
                           ${PIGMENTCMS_LIBRARY}
)

include( FindPackageHandleStandardArgs )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( komain DEFAULT_MSG KOFFICELIBS_INCLUDE_DIR KOMAIN_LIBRARY )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( kstore DEFAULT_MSG KOFFICELIBS_INCLUDE_DIR KSTORE_LIBRARY )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( kotext DEFAULT_MSG KOFFICELIBS_INCLUDE_DIR KOTEXT_LIBRARY )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( koguiutils DEFAULT_MSG KOFFICELIBS_INCLUDE_DIR KOGUIUTILS_LIBRARY )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( koodf DEFAULT_MSG KOFFICELIBS_INCLUDE_DIR KOODF_LIBRARY )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( koresources DEFAULT_MSG KOFFICELIBS_INCLUDE_DIR KORESOURCES_LIBRARY )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( koproperty DEFAULT_MSG KOFFICELIBS_INCLUDE_DIR KOPROPERTY_LIBRARY )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( kopageapp DEFAULT_MSG KOFFICELIBS_INCLUDE_DIR KOPAGEAPP_LIBRARY )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( kokross DEFAULT_MSG KOFFICELIBS_INCLUDE_DIR KOKROSS_LIBRARY )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( flake DEFAULT_MSG KOFFICELIBS_INCLUDE_DIR FLAKE_LIBRARY )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( pigmentcms DEFAULT_MSG KOFFICELIBS_INCLUDE_DIR PIGMENTCMS_LIBRARY )
#FIND_PACKAGE_HANDLE_STANDARD_ARGS( KOfficeLibs DEFAULT_MSG KOFFICELIBS_INCLUDE_DIR KOFFICELIBS_LIBRARIES )

# show the KOFFICELIBS_INCLUDE_DIR and KOFFICELIBS_LIBRARIES variables only in the advanced view
MARK_AS_ADVANCED( KOFFICELIBS_INCLUDE_DIR 
                  KOMAIN_LIBRARY 
                  KSTORE_LIBRARY
                  KOTEXT_LIBRARY
                  KOGUIUTILS_LIBRARY
                  KOODF_LIBRARY
                  KORESOURCES_LIBRARY
                  KOPROPERTY_LIBRARY
                  KOPAGEAPP_LIBRARY
                  KOKROSS_LIBRARY
                  FLAKE_LIBRARY
                  PIGMENTCMS_LIBRARY
                  KOFFICELIBS_LIBRARIES )


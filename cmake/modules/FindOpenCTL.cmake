     
INCLUDE(FindPkgConfig)

pkg_check_modules(OPENCTL OpenCTL>=0.9.13)
pkg_check_modules(OPENCTL_913 OpenCTL>=0.9.13)

if (OPENCTL_FOUND)
    set(HAVE_OPENCTL TRUE)
    message(STATUS "OpenCTL Found Version: " ${OPENCTL_VERSION})
    if (OPENCTL_913_FOUND)
	set(HAVE_OPENCTL_913 TRUE)
    else()
	set(HAVE_OPENCTL_913 FALSE)
    endif()

    if (NOT OpenCTL_FIND_QUIETLY )
        message(STATUS "Found OPENCTL: ${OPENCTL_LIBRARIES}")
    endif (NOT OpenCTL_FIND_QUIETLY)
else ()
    if (NOT OpenCTL_FIND_QUIETLY)
        message(STATUS "OpenCTL was NOT found.")
    endif ()
    if (OpenCTL_FIND_REQUIRED)
        message(FATAL_ERROR "Could NOT find OPENCTL")
    endif ()
endif ()

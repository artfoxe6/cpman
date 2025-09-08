# Try config-mode first
find_package(QHotkey CONFIG QUIET)
if (TARGET QHotkey::QHotkey)
  set(QHOTKEY_FOUND TRUE)
  return()
endif()

# Fallback: find header and library
find_path(QHOTKEY_INCLUDE_DIR NAMES QHotkey QHotkey/QHotkey.h PATH_SUFFIXES include)
find_library(QHOTKEY_LIBRARY NAMES QHotkey qhotkey)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(QHotkey DEFAULT_MSG QHOTKEY_INCLUDE_DIR QHOTKEY_LIBRARY)

if (QHOTKEY_FOUND)
  add_library(QHotkey::QHotkey UNKNOWN IMPORTED)
  set_target_properties(QHotkey::QHotkey PROPERTIES
    IMPORTED_LOCATION "${QHOTKEY_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${QHOTKEY_INCLUDE_DIR}"
  )
endif()


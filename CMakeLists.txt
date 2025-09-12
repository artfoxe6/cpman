cmake_minimum_required(VERSION 3.20)

project(cpman LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTORCC ON)

# CMake find modules
list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(Qt6 REQUIRED COMPONENTS Widgets Gui Sql Concurrent Network)

add_subdirectory(app)

# CPack configuration for packaging
set(CPACK_PACKAGE_NAME "cpman")
set(CPACK_PACKAGE_VERSION "0.1.0")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Cross-platform clipboard history manager")
set(CPACK_PACKAGE_VENDOR "cpman")
set(CPACK_PACKAGE_CONTACT "cpman@cpman.com")

if(APPLE)
  enable_language(OBJCXX)
endif()

if(APPLE)
  # macOS DMG configuration
  set(CPACK_GENERATOR "DragNDrop")
  set(CPACK_DMG_VOLUME_NAME "cpman ${CPACK_PACKAGE_VERSION}")
  set(CPACK_DMG_FORMAT "UDBZ")
  set(CPACK_DMG_DISABLE_APPLICATIONS_SYMLINK ON)
  set(CPACK_DMG_SLA_USE_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
  set(CPACK_PACKAGE_FILE_NAME "cpman-${CPACK_PACKAGE_VERSION}-macOS")
elseif(WIN32)
  # Windows NSIS configuration
  set(CPACK_GENERATOR "NSIS")
  set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/app/icons/windows/cpman.ico")
  set(CPACK_NSIS_MUI_UNIICON "${CMAKE_CURRENT_SOURCE_DIR}/app/icons/windows/cpman.ico")
  set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\cpman.exe")
  set(CPACK_NSIS_DISPLAY_NAME "cpman")
  set(CPACK_NSIS_PACKAGE_NAME "cpman")
  set(CPACK_NSIS_HELP_LINK "https://github.com/xxxx/xxxx")
  set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/xxxx/xxxx")
  set(CPACK_NSIS_CONTACT "cpman@cpman.com")
  set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
  set(CPACK_PACKAGE_FILE_NAME "cpman-${CPACK_PACKAGE_VERSION}-Windows")
  set(CPACK_NSIS_MENU_LINKS
    "bin/cpman.exe" "cpman"
    "https://github.com/xxxx/xxxx" "cpman on GitHub"
  )
elseif(UNIX)
  # Linux AppImage configuration
  set(CPACK_GENERATOR "External")
  set(CPACK_EXTERNAL_PACKAGE_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/scripts/linux/build_appimage.sh")
  set(CPACK_PACKAGE_FILE_NAME "cpman-${CPACK_PACKAGE_VERSION}-Linux")
endif()

include(CPack)

set(SRC
  core/Settings.cpp
  core/Database.cpp
  core/InMemoryStore.cpp
  core/ImageStore.cpp
  core/ClipboardWatcher.cpp
  core/HotkeyManager.cpp
  core/AutoPaster.cpp
  core/SingleInstance.cpp
  core/FocusManager.cpp

  ui/MainPopup.cpp
  ui/HistoryListModel.cpp
  ui/HistoryItemDelegate.cpp
  ui/PreviewPane.cpp
  ui/TrayIcon.cpp
  ui/SettingsDialog.cpp
  ui/Theme.cpp

  main.cpp
  resources.qrc
)

set(HDR
  core/Settings.h
  core/Database.h
  core/InMemoryStore.h
  core/ImageStore.h
  core/ClipboardWatcher.h
  core/HotkeyManager.h
  core/AutoPaster.h
  core/SingleInstance.h
  core/FocusManager.h
  core/Types.h

  ui/MainPopup.h
  ui/HistoryListModel.h
  ui/HistoryItemDelegate.h
  ui/PreviewPane.h
  ui/TrayIcon.h
  ui/SettingsDialog.h
  ui/Theme.h
)

add_executable(cpman MACOSX_BUNDLE
  ${SRC}
  ${HDR}
)

target_link_libraries(cpman PRIVATE Qt6::Widgets Qt6::Gui Qt6::Sql Qt6::Concurrent Qt6::Network)

option(HAVE_QHOTKEY "Enable QHotkey integration" ON)
if(HAVE_QHOTKEY)
  # Prefer vendored QHotkey if present
  if (EXISTS "${CMAKE_SOURCE_DIR}/third_party/QHotkey/CMakeLists.txt")
    message(STATUS "Using vendored QHotkey at third_party/QHotkey")
    # Ensure QHotkey builds against Qt 6 to match this project
    set(QT_DEFAULT_MAJOR_VERSION 6 CACHE STRING "Qt version for QHotkey" FORCE)
    # Avoid extra install/exports from the subproject
    set(QHOTKEY_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(QHOTKEY_INSTALL OFF CACHE BOOL "" FORCE)
    add_subdirectory("${CMAKE_SOURCE_DIR}/third_party/QHotkey" third_party/QHotkey-build EXCLUDE_FROM_ALL)
  endif()

  # Try to find a system-provided QHotkey first (or the vendored target)
  find_package(QHotkey QUIET)

  # If not found and no vendored target, fetch from GitHub
  if (NOT QHOTKEY_FOUND AND NOT TARGET QHotkey::QHotkey)
    include(FetchContent)
    message(STATUS "QHotkey not found locally; fetching from GitHub...")
    set(QHOTKEY_GIT_TAG "v1.5.0" CACHE STRING "Git tag/branch/commit for QHotkey")
    FetchContent_Declare(
      QHotkey
      GIT_REPOSITORY https://github.com/Skycoder42/QHotkey.git
      # Pin to a known good tag; override with -DQHOTKEY_GIT_TAG=... if needed
      GIT_TAG ${QHOTKEY_GIT_TAG}
      GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(QHotkey)
  endif()

  if(QHOTKEY_FOUND OR TARGET QHotkey::QHotkey)
    target_compile_definitions(cpman PRIVATE HAVE_QHOTKEY)
    target_link_libraries(cpman PRIVATE QHotkey::QHotkey)
    if(APPLE)
      # QHotkey on macOS relies on Carbon/HIToolbox symbols
      find_library(FW_Carbon Carbon)
      find_library(FW_HIToolbox HIToolbox)
      if(FW_Carbon)
        target_link_libraries(cpman PRIVATE ${FW_Carbon})
      endif()
      if(FW_HIToolbox)
        target_link_libraries(cpman PRIVATE ${FW_HIToolbox})
      endif()
    endif()
  else()
    message(WARNING "HAVE_QHOTKEY=ON but QHotkey not found/fetched; building without global hotkey support.")
  endif()
endif()

if(WIN32)
  target_compile_definitions(cpman PRIVATE CP_PLATFORM_WINDOWS)
  target_sources(cpman PRIVATE core/AppInfo.cpp)
  target_sources(cpman PRIVATE platform/AutoPaste_win.cpp)
elseif(APPLE)
  target_compile_definitions(cpman PRIVATE CP_PLATFORM_MAC)
  # Ensure Objective-C++ for mac file
  set_source_files_properties(platform/AutoPaste_mac.mm PROPERTIES LANGUAGE OBJCXX)
  set_source_files_properties(platform/AppInfo_mac.mm PROPERTIES LANGUAGE OBJCXX)
  set_source_files_properties(core/FocusManager.cpp PROPERTIES LANGUAGE OBJCXX)
  target_sources(cpman PRIVATE platform/AutoPaste_mac.mm platform/AppInfo_mac.mm)
  # Link ApplicationServices (CGEvent) and Cocoa if not already linked via Qt
  find_library(FW_ApplicationServices ApplicationServices)
  if(FW_ApplicationServices)
    target_link_libraries(cpman PRIVATE ${FW_ApplicationServices})
  endif()
elseif(UNIX)
  target_compile_definitions(cpman PRIVATE CP_PLATFORM_LINUX)
  target_sources(cpman PRIVATE platform/AutoPaste_x11.cpp platform/AutoPaste_wayland.cpp platform/AppInfo_x11.cpp)
  find_package(X11)
  if(X11_FOUND)
    if(X11_Xtst_LIB)
      target_link_libraries(cpman PRIVATE ${X11_Xtst_LIB} ${X11_LIBRARIES})
      target_compile_definitions(cpman PRIVATE CP_HAVE_XTEST)
    endif()
  endif()
endif()

target_include_directories(cpman PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(APPLE)
  set_target_properties(cpman PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.cpman.app"
    MACOSX_BUNDLE_BUNDLE_NAME "cpman"
    MACOSX_BUNDLE_BUNDLE_VERSION "0.1.0"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1.0"
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/macos/Info.plist.in"
  )
endif()

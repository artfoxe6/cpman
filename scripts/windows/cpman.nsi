; NSIS script for cpman installer
; This will be used by CPack/NSIS

; Modern UI
!include "MUI2.nsh"

; General
Name "cpman"
OutFile "cpman-0.1.0-Installer.exe"
InstallDir "$PROGRAMFILES64\cpman"
RequestExecutionLevel admin

; Interface settings
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; Pages
!insertmacro MUI_PAGE_LICENSE "${PROJECT_DIR}\LICENSE"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; Languages
!insertmacro MUI_LANGUAGE "English"

; Installer sections
Section "cpman" SEC01
  SetOutPath "$INSTDIR"
  
  ; Copy files
  File "${BUILD_DIR}\bin\cpman.exe"
  File "${BUILD_DIR}\bin\Qt6Widgets.dll"
  File "${BUILD_DIR}\bin\Qt6Gui.dll"
  File "${BUILD_DIR}\bin\Qt6Core.dll"
  File "${BUILD_DIR}\bin\Qt6Sql.dll"
  File "${BUILD_DIR}\bin\Qt6Concurrent.dll"
  File "${BUILD_DIR}\bin\Qt6Network.dll"
  
  ; Create directories
  CreateDirectory "$INSTDIR\platforms"
  CreateDirectory "$INSTDIR\styles"
  
  ; Copy Qt plugins
  File "${BUILD_DIR}\bin\platforms\qwindows.dll"
  File "${BUILD_DIR}\bin\styles\qwindowsvistastyle.dll"
  
  ; Create uninstaller
  WriteUninstaller "$INSTDIR\uninstall.exe"
  
  ; Create registry keys
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\cpman" "DisplayName" "cpman"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\cpman" "UninstallString" "$INSTDIR\uninstall.exe"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\cpman" "InstallLocation" "$INSTDIR"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\cpman" "Publisher" "cpman"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\cpman" "DisplayVersion" "0.1.0"
  
  ; Add to startup registry for autostart
  WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "cpman" "$INSTDIR\cpman.exe"
SectionEnd

; Uninstaller section
Section "Uninstall"
  ; Remove files
  Delete "$INSTDIR\cpman.exe"
  Delete "$INSTDIR\Qt6Widgets.dll"
  Delete "$INSTDIR\Qt6Gui.dll"
  Delete "$INSTDIR\Qt6Core.dll"
  Delete "$INSTDIR\Qt6Sql.dll"
  Delete "$INSTDIR\Qt6Concurrent.dll"
  Delete "$INSTDIR\Qt6Network.dll"
  Delete "$INSTDIR\platforms\qwindows.dll"
  Delete "$INSTDIR\styles\qwindowsvistastyle.dll"
  Delete "$INSTDIR\uninstall.exe"
  
  ; Remove directories
  RMDir "$INSTDIR\platforms"
  RMDir "$INSTDIR\styles"
  RMDir "$INSTDIR"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\cpman"
  DeleteRegValue HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "cpman"
SectionEnd
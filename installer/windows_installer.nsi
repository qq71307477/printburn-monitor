; NSIS Installer Script for PrintBurn Monitor
;--------------------------------

!define APPNAME "打印刻录安全监控系统"
!define COMPANYNAME "Security Solutions Inc."
!define DESCRIPTION "打印刻录安全监控系统 - 单机版"
!define VERSIONMAJOR 1
!define VERSIONMINOR 0
!define VERSIONBUILD 0

; 注意: 应用图标文件需要放置在当前目录下
!define MUI_ICON "app_icon.ico"
!define MUI_UNICON "app_unicon.ico"

; 包含MUI 2界面
!include "MUI2.nsh"
!include "Sections.nsh"

;--------------------------------
; General

; 应用程序名称
Name "${APPNAME}"
BrandingText "${COMPANYNAME} - ${DESCRIPTION}"

; 安装程序输出文件名
OutFile "PrintBurnMonitor_Setup.exe"

; 目标安装目录
InstallDir $PROGRAMFILES\${COMPANYNAME}\${APPNAME}

; 使安装程序保持向后兼容
SetCompressor /SOLID lzma

;--------------------------------
; Variables

Var StartMenuFolder

;--------------------------------
; Interface Settings

!define MUI_ABORTWARNING

;--------------------------------
; Pages

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "LICENSE.txt"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
; Languages

!insertmacro MUI_LANGUAGE "SimpChinese"

;--------------------------------
; Installer Sections

Section "主要程序" SecMain
    SectionIn RO

    ; 创建安装目录
    SetOutPath "$INSTDIR"

    ; 复制主程序文件
    File "..\build\PrintBurnMonitor.exe"

    ; 复制Qt运行时库
    File "..\build\Qt6Core.dll"
    File "..\build\Qt6Gui.dll"
    File "..\build\Qt6Widgets.dll"
    File "..\build\Qt6Sql.dll"
    File "..\build\Qt6Network.dll"

    ; 复制平台插件
    SetOutPath "$INSTDIR\platforms"
    File "..\build\platforms\qwindows.dll"

    ; 复制样式插件
    SetOutPath "$INSTDIR\styles"
    File "..\build\styles\qwindowsvistastyle.dll"

    ; 创建开始菜单快捷方式
    !ifdef STARTMENU_FOLDER
        SetShellFolder $SMPrograms $StartMenuFolder
    !else
        SetShellFolder $SMPrograms "${APPNAME}"
    !endif
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\${APPNAME}.lnk" "$INSTDIR\PrintBurnMonitor.exe" "" "$INSTDIR\PrintBurnMonitor.exe" 0 SW_SHOWNORMAL \
                 ALT|CONTROL|SHIFT|F5 "Launch ${APPNAME}"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe" "" "$INSTDIR\Uninstall.exe" 0 SW_SHOWNORMAL \
                 "" "Uninstall ${APPNAME}"

    ; 创建桌面快捷方式
    CreateShortCut "$DESKTOP\${APPNAME}.lnk" "$INSTDIR\PrintBurnMonitor.exe" "" "$INSTDIR\PrintBurnMonitor.exe" 0 SW_SHOWNORMAL

    ; 写入卸载信息到注册表
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayName" "${APPNAME}"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "UninstallString" "$INSTDIR\Uninstall.exe"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayIcon" "$INSTDIR\PrintBurnMonitor.exe"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "Publisher" "${COMPANYNAME}"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayVersion" "${VERSIONMAJOR}.${VERSIONMINOR}.${VERSIONBUILD}"
    WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "VersionMajor" ${VERSIONMAJOR}
    WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "VersionMinor" ${VERSIONMINOR}

    WriteUninstaller "$INSTDIR\Uninstall.exe"
SectionEnd

; 附加组件示例
Section "文档" SecDocs
    SetOutPath "$INSTDIR\docs"
    File "..\docs\readme.txt"
    File "..\docs\license.txt"
    File "..\docs\changelog.txt"
SectionEnd

;--------------------------------
; Descriptions

LangString DESC_SecMain ${LANG_SIMPCHINESE} "主要应用程序文件"
LangString DESC_SecDocs ${LANG_SIMPCHINESE} "用户文档和支持文件"

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
!insertmacro MUI_DESCRIPTION_TEXT ${SecMain} $(DESC_SecMain)
!insertmacro MUI_DESCRIPTION_TEXT ${SecDocs} $(DESC_SecDocs)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
; Uninstaller Section

Section "Uninstall"
    ; 删除应用程序文件
    Delete "$INSTDIR\PrintBurnMonitor.exe"
    Delete "$INSTDIR\Qt6Core.dll"
    Delete "$INSTDIR\Qt6Gui.dll"
    Delete "$INSTDIR\Qt6Widgets.dll"
    Delete "$INSTDIR\Qt6Sql.dll"
    Delete "$INSTDIR\Qt6Network.dll"

    ; 删除插件目录及文件
    Delete "$INSTDIR\platforms\qwindows.dll"
    RMDir "$INSTDIR\platforms"

    Delete "$INSTDIR\styles\qwindowsvistastyle.dll"
    RMDir "$INSTDIR\styles"

    ; 删除文档
    Delete "$INSTDIR\docs\*.*"
    RMDir "$INSTDIR\docs"

    ; 删除应用程序目录
    Delete "$INSTDIR\Uninstall.exe"
    RMDir "$INSTDIR"

    ; 删除开始菜单快捷方式
    !ifdef STARTMENU_FOLDER
        Delete "$SMPROGRAMS\$StartMenuFolder\${APPNAME}.lnk"
        Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"
        RMDir "$SMPROGRAMS\$StartMenuFolder"
    !else
        Delete "$SMPROGRAMS\${APPNAME}\*.*"
        RMDir "$SMPROGRAMS\${APPNAME}"
    !endif

    ; 删除桌面快捷方式
    Delete "$DESKTOP\${APPNAME}.lnk"

    ; 从注册表删除安装信息
    DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"
SectionEnd
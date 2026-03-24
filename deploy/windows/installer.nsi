; NSIS Installer Script for PrintBurn Monitor
; 打印刻录安全监控系统 - Windows安装程序
;================================

!define APPNAME "PrintBurnMonitor"
!define APPNAME_CN "打印刻录安全监控系统"
!define COMPANYNAME "PrintBurn Security"
!define DESCRIPTION "打印刻录安全监控系统 - 单机版"
!define VERSIONMAJOR 1
!define VERSIONMINOR 0
!define VERSIONBUILD 0
!define VERSION "${VERSIONMAJOR}.${VERSIONMINOR}.${VERSIONBUILD}"

; 应用图标 (需要放在 deploy/windows 目录下)
!define MUI_ICON "app_icon.ico"
!define MUI_UNICON "app_unicon.ico"

; 包含MUI 2界面
!include "MUI2.nsh"
!include "Sections.nsh"
!include "FileFunc.nsh"
!include "x64.nsh"

;================================
; General

; 应用程序名称
Name "${APPNAME_CN}"
BrandingText "${COMPANYNAME}"

; 安装程序输出文件名
OutFile "PrintBurnMonitor-Setup-${VERSION}.exe"

; 请求管理员权限
RequestExecutionLevel admin

; 目标安装目录
InstallDir "$PROGRAMFILES64\${COMPANYNAME}\${APPNAME}"

; 从注册表获取安装目录
InstallDirRegKey HKLM "Software\${COMPANYNAME}\${APPNAME}" "InstallDir"

; 压缩设置
SetCompressor /SOLID lzma
SetCompressorDictSize 32

;================================
; Variables

Var StartMenuFolder

;================================
; Interface Settings

!define MUI_ABORTWARNING
!define MUI_WELCOMEPAGE_TITLE "欢迎使用 ${APPNAME_CN} 安装向导"
!define MUI_WELCOMEPAGE_TEXT "本向导将引导您完成 ${APPNAME_CN} 的安装过程。\r\n\r\n建议您在安装前关闭所有其他应用程序。\r\n\r\n点击'下一步'继续。"

;================================
; Pages

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "LICENSE.txt"
!insertmacro MUI_PAGE_DIRECTORY

; 开始菜单页面
!define MUI_STARTMENUPAGE_REGISTRY_ROOT HKLM
!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\${COMPANYNAME}\${APPNAME}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "StartMenuFolder"
!insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder

!insertmacro MUI_PAGE_INSTFILES

; 完成页面
!define MUI_FINISHPAGE_RUN "$INSTDIR\${APPNAME}.exe"
!define MUI_FINISHPAGE_RUN_TEXT "运行 ${APPNAME_CN}"
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\README.txt"
!define MUI_FINISHPAGE_SHOWREADME_TEXT "查看自述文件"
!insertmacro MUI_PAGE_FINISH

; 卸载页面
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

;================================
; Languages

!insertmacro MUI_LANGUAGE "SimpChinese"

;================================
; Installer Sections

Section "!主程序" SecMain
    SectionIn RO

    ; 创建安装目录
    SetOutPath "$INSTDIR"

    ; 复制主程序文件
    File /oname=${APPNAME}.exe "${BUILD_DIR}\${APPNAME}.exe"

    ; 复制Qt运行时库 (Qt5)
    File "${QT_DIR}\bin\Qt5Core.dll"
    File "${QT_DIR}\bin\Qt5Gui.dll"
    File "${QT_DIR}\bin\Qt5Widgets.dll"
    File "${QT_DIR}\bin\Qt5Sql.dll"
    File "${QT_DIR}\bin\Qt5Network.dll"
    File "${QT_DIR}\bin\Qt5PrintSupport.dll"

    ; 复制SQLite驱动
    SetOutPath "$INSTDIR\sqldrivers"
    File "${QT_DIR}\plugins\sqldrivers\qsqlite.dll"

    ; 复制平台插件
    SetOutPath "$INSTDIR\platforms"
    File "${QT_DIR}\plugins\platforms\qwindows.dll"

    ; 复制样式插件
    SetOutPath "$INSTDIR\styles"
    File "${QT_DIR}\plugins\styles\qwindowsvistastyle.dll"

    ; 复制应用插件
    SetOutPath "$INSTDIR\plugins"
    File "${BUILD_DIR}\plugins\ClientPlugin.dll"
    File "${BUILD_DIR}\plugins\TaskPlugin.dll"
    File "${BUILD_DIR}\plugins\ApprovalPlugin.dll"
    File "${BUILD_DIR}\plugins\SecurityPlugin.dll"

    ; 复制资源文件
    SetOutPath "$INSTDIR\resources"
    File /r "${RESOURCES_DIR}\*.*"

    ; 复制文档文件
    SetOutPath "$INSTDIR"
    File "LICENSE.txt"
    File "README.txt"

    ; 创建数据目录
    CreateDirectory "$INSTDIR\data"
    CreateDirectory "$INSTDIR\logs"

    ; 创建开始菜单快捷方式
    !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
        CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
        CreateShortCut "$SMPROGRAMS\$StartMenuFolder\${APPNAME_CN}.lnk" "$INSTDIR\${APPNAME}.exe" "" "$INSTDIR\${APPNAME}.exe" 0
        CreateShortCut "$SMPROGRAMS\$StartMenuFolder\卸载.lnk" "$INSTDIR\Uninstall.exe" "" "$INSTDIR\Uninstall.exe" 0
    !insertmacro MUI_STARTMENU_WRITE_END

    ; 创建桌面快捷方式
    CreateShortCut "$DESKTOP\${APPNAME_CN}.lnk" "$INSTDIR\${APPNAME}.exe" "" "$INSTDIR\${APPNAME}.exe" 0

    ; 写入卸载信息到注册表
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayName" "${APPNAME_CN}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "UninstallString" "$\"$INSTDIR\Uninstall.exe$\""
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "QuietUninstallString" "$\"$INSTDIR\Uninstall.exe$\" /S"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayIcon" "$INSTDIR\${APPNAME}.exe"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "Publisher" "${COMPANYNAME}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayVersion" "${VERSION}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "URLInfoAbout" "https://www.printburn-security.com"
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "VersionMajor" ${VERSIONMAJOR}
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "VersionMinor" ${VERSIONMINOR}
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "NoModify" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "NoRepair" 1

    ; 计算安装大小
    ${GetSize} "$INSTDIR" "/S=0K" $0
    IntFmt $0 "0x%08X" $0
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "EstimatedSize" "$0"

    ; 写入安装目录到注册表
    WriteRegStr HKLM "Software\${COMPANYNAME}\${APPNAME}" "InstallDir" "$INSTDIR"
    WriteRegStr HKLM "Software\${COMPANYNAME}\${APPNAME}" "Version" "${VERSION}"

    ; 创建卸载程序
    WriteUninstaller "$INSTDIR\Uninstall.exe"

SectionEnd

;================================
; Descriptions

LangString DESC_SecMain ${LANG_SIMPCHINESE} "安装主程序及必要的运行时文件"

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
!insertmacro MUI_DESCRIPTION_TEXT ${SecMain} $(DESC_SecMain)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;================================
; Callback Functions

Function .onInit
    ; 检测是否已安装
    ReadRegStr $0 HKLM "Software\${COMPANYNAME}\${APPNAME}" "InstallDir"
    ${If} $0 != ""
        MessageBox MB_YESNO|MB_ICONQUESTION "${APPNAME_CN} 已安装在 $0$\r$\n是否覆盖安装？" IDYES +2
        Abort
    ${EndIf}
FunctionEnd

;================================
; Uninstaller Section

Section "Uninstall"
    ; 关闭正在运行的程序
    nsExec::Exec 'taskkill /F /IM ${APPNAME}.exe'

    ; 删除主程序
    Delete "$INSTDIR\${APPNAME}.exe"

    ; 删除Qt运行时库
    Delete "$INSTDIR\Qt5Core.dll"
    Delete "$INSTDIR\Qt5Gui.dll"
    Delete "$INSTDIR\Qt5Widgets.dll"
    Delete "$INSTDIR\Qt5Sql.dll"
    Delete "$INSTDIR\Qt5Network.dll"
    Delete "$INSTDIR\Qt5PrintSupport.dll"

    ; 删除SQLite驱动
    Delete "$INSTDIR\sqldrivers\qsqlite.dll"
    RMDir "$INSTDIR\sqldrivers"

    ; 删除平台插件
    Delete "$INSTDIR\platforms\qwindows.dll"
    RMDir "$INSTDIR\platforms"

    ; 删除样式插件
    Delete "$INSTDIR\styles\qwindowsvistastyle.dll"
    RMDir "$INSTDIR\styles"

    ; 删除应用插件
    Delete "$INSTDIR\plugins\ClientPlugin.dll"
    Delete "$INSTDIR\plugins\TaskPlugin.dll"
    Delete "$INSTDIR\plugins\ApprovalPlugin.dll"
    Delete "$INSTDIR\plugins\SecurityPlugin.dll"
    RMDir "$INSTDIR\plugins"

    ; 删除资源文件
    RMDir /r "$INSTDIR\resources"

    ; 删除文档
    Delete "$INSTDIR\LICENSE.txt"
    Delete "$INSTDIR\README.txt"

    ; 删除数据目录 (询问用户)
    IfFileExists "$INSTDIR\data\*.*" 0 +3
        MessageBox MB_YESNO|MB_ICONQUESTION "是否删除数据文件？" IDNO +2
        RMDir /r "$INSTDIR\data"

    ; 删除日志目录
    RMDir /r "$INSTDIR\logs"

    ; 删除卸载程序
    Delete "$INSTDIR\Uninstall.exe"

    ; 删除安装目录
    RMDir "$INSTDIR"

    ; 删除开始菜单快捷方式
    !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
    Delete "$SMPROGRAMS\$StartMenuFolder\${APPNAME_CN}.lnk"
    Delete "$SMPROGRAMS\$StartMenuFolder\卸载.lnk"
    RMDir "$SMPROGRAMS\$StartMenuFolder"

    ; 删除桌面快捷方式
    Delete "$DESKTOP\${APPNAME_CN}.lnk"

    ; 从注册表删除安装信息
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"
    DeleteRegKey HKLM "Software\${COMPANYNAME}\${APPNAME}"
    DeleteRegKey /ifempty HKLM "Software\${COMPANYNAME}"

SectionEnd

;================================
; Uninstall Callback

Function un.onInit
    MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "确定要卸载 ${APPNAME_CN} 吗？" IDYES +2
    Abort
FunctionEnd
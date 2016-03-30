!include "MUI2.nsh"
!include "fileassoc.nsh"

!ifndef PRODUCT_NAME
!define PRODUCT_NAME "unspecified-product"
!endif

!ifndef VERSION
!define VERSION "unspecified-version"
!endif

!ifndef PUBLISHER
!define PUBLISHER "unspecified-publisher"
!endif

!ifndef COPYRIGHT
!define COPYRIGHT "unspecified-copyright"
!endif

!searchreplace COPYRIGHT "${COPYRIGHT}" "(c)" "�"

!define UNINSTALL_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"

Var INSTDIR_BASE

Name "${PRODUCT_NAME}"

VIProductVersion "1.0.0.0"
VIAddVersionKey ProductName "${PRODUCT_NAME}"
VIAddVersionKey Comments "${PRODUCT_NAME}"
VIAddVersionKey LegalCopyright "${COPYRIGHT}"
VIAddVersionKey FileDescription "${PRODUCT_NAME}"
VIAddVersionKey FileVersion "1.0.0.0"
VIAddVersionKey ProductVersion "1.0.0.0"

InstallDir ""

; Take the highest execution level available
; This means that if it's possible to, we become an administrator
RequestExecutionLevel highest

!macro ONINIT un
	Function ${un}.onInit
		; The value of SetShellVarContext detetmines whether SHCTX is HKLM or HKCU
		; and whether SMPROGRAMS refers to all users or just the current user
		UserInfo::GetAccountType
		Pop $0
		${If} $0 == "Admin"
			; If we're an admin, default to installing to C:\Program Files
			SetShellVarContext all
			StrCpy $INSTDIR_BASE "$PROGRAMFILES64"
		${Else}
			; If we're just a user, default to installing to ~\AppData\Local
			SetShellVarContext current
			StrCpy $INSTDIR_BASE "$LOCALAPPDATA"
		${EndIf}

		${If} $INSTDIR == ""
			; This only happens in the installer, because the uninstaller already knows INSTDIR
			ReadRegStr $0 SHCTX "Software\${PRODUCT_NAME}" ""

			${If} $0 != ""
				; If we're already installed, use the existing directory
				StrCpy $INSTDIR "$0"
			${Else}
				StrCpy $INSTDIR "$INSTDIR_BASE\${PRODUCT_NAME}"
			${Endif}
		${Endif}
	FunctionEnd
!macroend

; Define the function twice, once for the installer and again for the uninstaller
!insertmacro ONINIT ""
!insertmacro ONINIT "un"

; Installer Icons
!insertmacro MUI_DEFAULT MUI_ICON "source\icon\Icon.ico"
!insertmacro MUI_DEFAULT MUI_UNICON "source\icon\Icon.ico"

Icon "${MUI_ICON}"
UninstallIcon "${MUI_UNICON}"

WindowIcon on

!define MUI_ABORTWARNING

!define MUI_COMPONENTSPAGE_NODESC
!insertmacro MUI_PAGE_COMPONENTS

!insertmacro MUI_PAGE_DIRECTORY

Var STARTMENU_FOLDER
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "SHCTX"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\${PRODUCT_NAME}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
!insertmacro MUI_PAGE_STARTMENU ${PRODUCT_NAME} $STARTMENU_FOLDER

!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

Section "-Main Component"
	SetOutPath "$INSTDIR"

	File /r "build\*.*"

	WriteRegStr SHCTX "Software\${PRODUCT_NAME}" "" $INSTDIR

	; These registry entries are necessary for the program to show up in the Add/Remove programs dialog
	WriteRegStr SHCTX "${UNINSTALL_KEY}" "DisplayName" "${PRODUCT_NAME}"
	WriteRegStr SHCTX "${UNINSTALL_KEY}" "DisplayVersion" "${VERSION}"
	WriteRegStr SHCTX "${UNINSTALL_KEY}" "DisplayIcon" "$INSTDIR\${PRODUCT_NAME}.exe"
	WriteRegStr SHCTX "${UNINSTALL_KEY}" "Publisher" "${PUBLISHER}"
	WriteRegDWORD SHCTX "${UNINSTALL_KEY}" "EstimatedSize" "${BUILD_SIZE}"
	WriteRegStr SHCTX "${UNINSTALL_KEY}" "InstallLocation" "$INSTDIR"
	WriteRegStr SHCTX "${UNINSTALL_KEY}" "UninstallString" '"$INSTDIR\Uninstall.exe"'
	WriteRegDWORD SHCTX "${UNINSTALL_KEY}" "NoModify" 1
	WriteRegDWORD SHCTX "${UNINSTALL_KEY}" "NoRepair" 1

	WriteUninstaller "$INSTDIR\Uninstall.exe"

	!insertmacro MUI_STARTMENU_WRITE_BEGIN ${PRODUCT_NAME}
		CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER\"
		CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\${PRODUCT_NAME}.lnk" "$INSTDIR\${PRODUCT_NAME}.exe"
	!insertmacro MUI_STARTMENU_WRITE_END

	!insertmacro UPDATEFILEASSOC
SectionEnd

; File Associations
SectionGroup "File associations"
    Section "${PRODUCT_NAME} Graph Modelling Language file (.gml)"
        !insertmacro APP_ASSOCIATE "gml" "${PRODUCT_NAME}.gml" "${PRODUCT_NAME} GML File" \
            "$INSTDIR\${PRODUCT_NAME}.exe,0" "Open with ${PRODUCT_NAME}" \
						"$INSTDIR\${PRODUCT_NAME}.exe $\"%1$\""
    SectionEnd
SectionGroupEnd

Section "Desktop shortcut"
	CreateShortCut "$DESKTOP\${PRODUCT_NAME}.lnk" "$INSTDIR\${PRODUCT_NAME}.exe"
SectionEnd

Section "Uninstall"
	RMDir /r "$INSTDIR"

	!insertmacro MUI_STARTMENU_GETFOLDER ${PRODUCT_NAME} $STARTMENU_FOLDER
	Delete "$SMPROGRAMS\$STARTMENU_FOLDER\${PRODUCT_NAME}.lnk"
	RMDir /r "$SMPROGRAMS\$STARTMENU_FOLDER"

	Delete "$DESKTOP\${PRODUCT_NAME}.lnk"

	DeleteRegKey /ifempty SHCTX "Software\${PRODUCT_NAME}"

	DeleteRegKey SHCTX "${UNINSTALL_KEY}"

	!insertmacro APP_UNASSOCIATE "gml" "${PRODUCT_NAME}.gml"
	!insertmacro UPDATEFILEASSOC
SectionEnd

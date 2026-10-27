#ifndef AppVersion
  #define AppVersion "2026-10-27.2"
#endif
#ifndef WindowsVersion
  #define WindowsVersion "2026.10.27.2"
#endif
#ifndef PayloadDir
  #error PayloadDir must name the verified portable payload directory.
#endif
#ifndef ReleaseDir
  #define ReleaseDir "..\dist"
#endif

[Setup]
AppId={{A9428881-DBA2-4CD8-9392-07BE1FD88F86}
AppName=ExcellentZip
AppVersion={#AppVersion}
AppVerName=ExcellentZip {#AppVersion}
AppPublisher=ExcellentZip contributors
AppPublisherURL=https://github.com/abelokoj/ExcellentZip
AppSupportURL=https://github.com/abelokoj/ExcellentZip/issues
AppUpdatesURL=https://github.com/abelokoj/ExcellentZip/releases
DefaultDirName={localappdata}\Programs\ExcellentZip
DefaultGroupName=ExcellentZip
DisableProgramGroupPage=yes
PrivilegesRequired=lowest
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
MinVersion=10.0.19041
OutputDir={#ReleaseDir}
OutputBaseFilename=ExcellentZip-{#AppVersion}-Setup-x64
SetupIconFile=..\icons\Packs\4-monogram-ez\app.ico
UninstallDisplayIcon={app}\ExcellentZip.exe
VersionInfoVersion={#WindowsVersion}
VersionInfoProductTextVersion={#AppVersion}
VersionInfoDescription=ExcellentZip Setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
CloseApplications=yes
RestartApplications=no
ChangesAssociations=yes
LicenseFile=..\License.md

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "fileassociations"; Description: "Register ExcellentZip in Windows Default apps"; GroupDescription: "File types:"; Flags: checkedonce
Name: "fileassociations\defaultapps"; Description: "Choose ExcellentZip as the default for supported file types (Windows confirmation)"; Flags: unchecked
Name: "desktopicon"; Description: "Create a desktop shortcut"; GroupDescription: "Shortcuts:"; Flags: unchecked

[InstallDelete]
Type: filesandordirs; Name: "{app}\IconPacks"

[Files]
Source: "{#PayloadDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\ExcellentZip"; Filename: "{app}\ExcellentZip.exe"
Name: "{autodesktop}\ExcellentZip"; Filename: "{app}\ExcellentZip.exe"; Tasks: desktopicon

[Registry]
Root: HKCU; Subkey: "Software\Classes\Applications\ExcellentZip.exe"; ValueType: string; ValueName: "FriendlyAppName"; ValueData: "ExcellentZip"; Flags: uninsdeletevalue; Tasks: fileassociations
Root: HKCU; Subkey: "Software\RegisteredApplications"; ValueType: string; ValueName: "ExcellentZip"; ValueData: "Software\ExcellentZip\Capabilities"; Flags: uninsdeletevalue; Tasks: fileassociations
Root: HKCU; Subkey: "Software\ExcellentZip\Capabilities"; ValueType: string; ValueName: "ApplicationName"; ValueData: "ExcellentZip"; Flags: uninsdeletekey; Tasks: fileassociations
Root: HKCU; Subkey: "Software\ExcellentZip\Capabilities"; ValueType: string; ValueName: "ApplicationDescription"; ValueData: "Open, create and extract archives with ExcellentZip."; Tasks: fileassociations
Root: HKCU; Subkey: "Software\ExcellentZip\Capabilities"; ValueType: string; ValueName: "ApplicationIcon"; ValueData: "{app}\ExcellentZip.exe,0"; Tasks: fileassociations
#include "ExcellentZip.Associations.iss"

[Run]
Filename: "{app}\ExcellentZip.exe"; Description: "Launch ExcellentZip"; Flags: nowait postinstall skipifsilent
Filename: "ms-settings:defaultapps?registeredAppUser=ExcellentZip"; Flags: shellexec skipifsilent; Tasks: fileassociations\defaultapps

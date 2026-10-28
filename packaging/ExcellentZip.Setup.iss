#ifndef AppVersion
  #define AppVersion "2026-10-28.1"
#endif
#ifndef WindowsVersion
  #define WindowsVersion "2026.10.28.1"
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
Filename: "{app}\ExcellentZip.exe"; Description: "Launch ExcellentZip"; Flags: nowait postinstall skipifsilent; Check: SkipDefaultsGuide
Filename: "{app}\ExcellentZip.exe"; Parameters: "--default-types {code:DefaultTypesArgument}"; Flags: nowait skipifsilent; Check: ConfigureDefaults


[Code]
var
  DefaultTypesPage: TWizardPage;
  KeepDefaults, AllDefaults, SelectedDefaults: TNewRadioButton;
  ExtensionsEdit: TNewEdit;
  Selection: String;
  NormalizedSelection: String;

procedure DefaultChoiceChanged(Sender: TObject);
begin
  ExtensionsEdit.Enabled := SelectedDefaults.Checked;
end;

procedure InitializeWizard;
var
  Details: TNewStaticText;
begin
  DefaultTypesPage := CreateCustomPage(wpSelectTasks, 'Default file types',
    'Choose which files ExcellentZip should open');
  Details := TNewStaticText.Create(DefaultTypesPage);
  Details.Parent := DefaultTypesPage.Surface;
  Details.SetBounds(0, 0, DefaultTypesPage.SurfaceWidth, ScaleY(38));
  Details.AutoSize := False;
  Details.WordWrap := True;
  Details.Caption := 'Windows will ask you to confirm default-app changes after installation. Other apps remain your defaults until you confirm.';
  KeepDefaults := TNewRadioButton.Create(DefaultTypesPage);
  KeepDefaults.Parent := DefaultTypesPage.Surface;
  KeepDefaults.SetBounds(0, ScaleY(48), DefaultTypesPage.SurfaceWidth, ScaleY(22));
  KeepDefaults.Caption := 'Keep my current default apps';
  KeepDefaults.Checked := True;
  KeepDefaults.OnClick := @DefaultChoiceChanged;
  AllDefaults := TNewRadioButton.Create(DefaultTypesPage);
  AllDefaults.Parent := DefaultTypesPage.Surface;
  AllDefaults.SetBounds(0, ScaleY(82), DefaultTypesPage.SurfaceWidth, ScaleY(22));
  AllDefaults.Caption := 'Make ExcellentZip the default for all supported file types';
  AllDefaults.OnClick := @DefaultChoiceChanged;
  SelectedDefaults := TNewRadioButton.Create(DefaultTypesPage);
  SelectedDefaults.Parent := DefaultTypesPage.Surface;
  SelectedDefaults.SetBounds(0, ScaleY(116), DefaultTypesPage.SurfaceWidth, ScaleY(22));
  SelectedDefaults.Caption := 'Make ExcellentZip the default for only these extensions:';
  SelectedDefaults.OnClick := @DefaultChoiceChanged;
  ExtensionsEdit := TNewEdit.Create(DefaultTypesPage);
  ExtensionsEdit.Parent := DefaultTypesPage.Surface;
  ExtensionsEdit.SetBounds(ScaleX(18), ScaleY(146), DefaultTypesPage.SurfaceWidth - ScaleX(18), ScaleY(24));
  ExtensionsEdit.MaxLength := 4096;
  ExtensionsEdit.Enabled := False;
  Details := TNewStaticText.Create(DefaultTypesPage);
  Details.Parent := DefaultTypesPage.Surface;
  Details.SetBounds(ScaleX(18), ScaleY(182), DefaultTypesPage.SurfaceWidth - ScaleX(18), ScaleY(52));
  Details.AutoSize := False;
  Details.WordWrap := True;
  Details.Caption := 'For example: zip, 7z, rar. Separate extensions with commas, spaces or semicolons. You can change this later in Tools > Options > File types.';
end;

function ShouldSkipPage(PageID: Integer): Boolean;
begin
  Result := (PageID = DefaultTypesPage.ID) and not WizardIsTaskSelected('fileassociations');
end;

function ValidateExtensions: Boolean;
var
  Text, Token: String;
  Start, I: Integer;
begin
  Result := False;
  Selection := ';';
  NormalizedSelection := '';
  Text := Lowercase(ExtensionsEdit.Text) + ',';
  Start := 1;
  for I := 1 to Length(Text) do
    if (Text[I] = ',') or (Text[I] = ';') or (Text[I] = ' ') or (Text[I] = #9) then
    begin
      Token := Copy(Text, Start, I - Start);
      Start := I + 1;
      if (Length(Token) > 0) and (Token[1] = '.') then Delete(Token, 1, 1);
      if Token <> '' then
      begin
        if Pos(';' + Token + ';', ';{#SupportedExtensions};') = 0 then
        begin
          MsgBox('Unsupported extension: ' + Token + '. Enter supported archive types such as zip, 7z, rar.', mbError, MB_OK);
          WizardForm.ActiveControl := ExtensionsEdit;
          Exit;
        end;
        if Pos(';' + Token + ';', Selection) = 0 then
        begin
          Selection := Selection + Token + ';';
          if NormalizedSelection <> '' then NormalizedSelection := NormalizedSelection + ',';
          NormalizedSelection := NormalizedSelection + Token;
        end;
      end;
    end;
  Result := NormalizedSelection <> '';
  if not Result then
  begin
    MsgBox('Enter at least one supported extension, for example zip, 7z, rar.', mbError, MB_OK);
    WizardForm.ActiveControl := ExtensionsEdit;
  end;
end;

function NextButtonClick(CurPageID: Integer): Boolean;
begin
  Result := True;
  if (CurPageID = DefaultTypesPage.ID) and SelectedDefaults.Checked then
    Result := ValidateExtensions;
end;

function ShouldRegisterExtension(Extension: String): Boolean;
begin
  Result := not SelectedDefaults.Checked or
    (Pos(';' + Extension + ';', Selection) > 0);
end;

function ConfigureDefaults: Boolean;
begin
  Result := WizardIsTaskSelected('fileassociations') and
    (AllDefaults.Checked or SelectedDefaults.Checked);
end;

function SkipDefaultsGuide: Boolean;
begin
  Result := not ConfigureDefaults;
end;

function DefaultTypesArgument(Param: String): String;
begin
  if AllDefaults.Checked then Result := 'all'
  else Result := NormalizedSelection;
end;

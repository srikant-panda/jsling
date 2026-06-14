; Inno Setup script for JSling
; Generates JSling-Setup.exe to install jsling on Windows
; Build with: ISCC.exe jsling.iss  (or use build-windows-installer.sh on Linux)

#define MyAppName "JSling"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "Srikant Panda"
#define MyAppURL "https://github.com/srikant-panda/jsling"
#define MyAppExeName "jsling.exe"

[Setup]
AppId={{5E615B9F-8BE5-4089-BB8B-BF06A42921D3}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={localappdata}\jsling
DisableDirPage=no
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
PrivilegesRequired=lowest
OutputBaseFilename=JSling-Setup
OutputDir=dist
Compression=lzma
SolidCompression=yes
WizardStyle=modern
UninstallDisplayName={#MyAppName}
SetupIconFile=assets\jsling.ico
UninstallDisplayIcon={app}\jsling.exe

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "envPath"; Description: "Add JSling to the system PATH (recommended)"; GroupDescription: "Additional configuration:"; Flags: checkedonce
Name: "desktopicon"; Description: "Create a desktop shortcut"; GroupDescription: "Additional icons:"

[Files]
Source: "jsling.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "assets\jsling.ico"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\JSling REPL"; Filename: "{cmd}"; Parameters: "/k ""{app}\jsling.exe"""; WorkingDir: "{%USERPROFILE}"; IconFilename: "{app}\jsling.ico"; Comment: "Start JSling JavaScript REPL"
Name: "{group}\Uninstall JSling"; Filename: "{uninstallexe}"
Name: "{autodesktop}\JSling"; Filename: "{cmd}"; Parameters: "/k ""{app}\jsling.exe"""; WorkingDir: "{%USERPROFILE}"; IconFilename: "{app}\jsling.ico"; Tasks: desktopicon

[Run]
Filename: "{cmd}"; Parameters: "/k ""{app}\jsling.exe"" --version"; Description: "Verify installation"; Flags: postinstall nowait skipifsilent
Filename: "{cmd}"; Parameters: "/k ""{app}\jsling.exe"""; Description: "Launch JSling REPL"; Flags: postinstall nowait skipifsilent unchecked

[UninstallDelete]
Type: filesandordirs; Name: "{app}"

[Registry]
Root: HKCU; Subkey: "Environment"; ValueType: expandsz; ValueName: "Path"; ValueData: "{olddata};{app}"; Check: NeedsAddPath_User; Tasks: envPath

[Code]
// Win32 API import - required for BroadcastPathChange below
// Note: lParam is declared as 'string' so Inno's marshaling passes a
// pointer to the string data automatically (the standard pattern for
// broadcasting WM_SETTINGCHANGE with "Environment" as lParam).
function SendMessageTimeout(hWnd: Longint; Msg: Longint; wParam: Longint; lParam: string;
  fuFlags: Longint; uTimeout: Longint; var lpdwResult: DWORD): Longint;
  external 'SendMessageTimeoutW@user32.dll stdcall';

// Helper function to check if the app directory is already in the User Path
function NeedsAddPath_User(): Boolean;
var
  OldPath: string;
begin
  if RegQueryStringValue(HKEY_CURRENT_USER, 'Environment', 'Path', OldPath) then
  begin
    Result := Pos(UpperCase(ExpandConstant('{app}')), UpperCase(OldPath)) = 0;
  end
  else
  begin
    Result := True;
  end;
end;

// Broadcast WM_SETTINGCHANGE so Explorer/new terminals pick up PATH changes immediately
procedure BroadcastPathChange();
var
  MsgResult: DWORD;
begin
  SendMessageTimeout(
    $FFFF,  // HWND_BROADCAST
    $001A,  // WM_SETTINGCHANGE
    0,
    'Environment',
    2,      // SMTO_ABORTIFHUNG
    5000,
    MsgResult
  );
end;

// Procedure to remove the install directory from the User PATH on uninstall
procedure RemovePathFromUserEnv();
var
  OldPath: string;
  AppPath: string;
  PosApp: Integer;
begin
  AppPath := ExpandConstant('{app}');
  if RegQueryStringValue(HKEY_CURRENT_USER, 'Environment', 'Path', OldPath) then
  begin
    PosApp := Pos(UpperCase(AppPath), UpperCase(OldPath));
    if PosApp > 0 then
    begin
      Delete(OldPath, PosApp, Length(AppPath));
      StringChangeEx(OldPath, ';;', ';', True);
      if (Length(OldPath) > 0) and (OldPath[1] = ';') then
        Delete(OldPath, 1, 1);
      if (Length(OldPath) > 0) and (OldPath[Length(OldPath)] = ';') then
        Delete(OldPath, Length(OldPath), 1);
      RegWriteExpandStringValue(HKEY_CURRENT_USER, 'Environment', 'Path', OldPath);
      BroadcastPathChange();
    end;
  end;
end;

// After install, broadcast PATH change so new terminals see jsling immediately
procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssPostInstall then
  begin
    if WizardIsTaskSelected('envPath') then
      BroadcastPathChange();
  end;
end;

// On uninstall, clean up the PATH entry we added
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  if CurUninstallStep = usPostUninstall then
  begin
    RemovePathFromUserEnv();
  end;
end;

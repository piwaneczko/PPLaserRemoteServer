; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "Laser Remote Server"        
#define MyAppPublisher "Pawel Iwaneczko"
#define MyAppURL "http://www.aircom.ag/" 
#define MyAppExeName "PPLaserRemoteServer.exe"    
#define MyAppVersion GetFileVersion('install\bin\PPLaserRemoteServer.exe') 
#define VC_redist "https://aka.ms/vs/16/release/VC_redist.x86.exe" 

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{9ED29082-9E7E-4678-9605-427ACC84922B}
AppName={#MyAppName}
AppVersion={#MyAppVersion}            
VersionInfoVersion={#MyAppVersion}  
VersionInfoProductVersion={#MyAppVersion}
VersionInfoCompany={#MyAppPublisher}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
OutputBaseFilename=ppremotesetup
OutputDir=install\bin
WizardImageFile=src\app\resources\image.bmp
WizardSmallImageFile=src\app\resources\imagesmall.bmp
WizardImageStretch=True
Compression=lzma
SolidCompression=yes
DisableDirPage=auto      
DisableProgramGroupPage=auto
DisableReadyPage=true         
DisableWelcomePage=no        
;Registry key add PrivilegesRequired
PrivilegesRequired=poweruser
SetupIconFile=src\app\resources\laser_icon.ico
UninstallDisplayIcon={app}\{#MyAppExeName}
LicenseFile=license.txt
CloseApplications=force

[Languages]
Name: "polish"; MessagesFile: "compiler:Languages\Polish.isl"
         
[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked   
Name: Firewall; Description: "Add an exception to the Windows Firewall"; GroupDescription: "Firewall:";              
Name: Autostart; Description: "Run application at Windows Startup"; GroupDescription: "Autostart:"; 

[Dirs]
Name: {app}; Permissions: users-full
         
#include <idp.iss>
        
[UninstallDelete]
Type: files; Name: "{app}\*.*"

[Files]           
Source: "install\bin\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion 
Source: ppremote.cer; DestDir: {tmp}; Flags: deleteafterinstall
; NOTE: Don't use "Flags: ignoreversion" on any shared system files
         
[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; IconFilename: "{app}\{#MyAppExeName}" 
Name: "{group}\Uninstall {#MyAppName}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon; IconFilename: "{app}\{#MyAppExeName}"

[Registry]
Root: HKLM; Subkey: "SOFTWARE\Microsoft\Windows\CurrentVersion\Run"; ValueType: string; ValueName: "{#MyAppName}"; ValueData: """{app}\{#MyAppExeName}"""; Flags: uninsdeletevalue; Tasks: Autostart;
    
[Run]                                                                           
Filename: "{sys}\netsh.exe"; Parameters: "advfirewall firewall add rule name=""Laser Remote Server"" dir=in action=allow program=""{app}\{#MyAppExeName}"" protocol=TCP enable=yes"; StatusMsg: "Adding exception to firewall for pragram: Laser Remote Server.exe..."; Flags: runhidden; Tasks: Firewall;
Filename: "{tmp}\vc_redist.x86.exe"; Parameters: "/q /norestart"; StatusMsg: "Installing Visual C++ Redistributable Packages (x86)..."; Flags: runhidden
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
Filename: "certutil.exe"; Parameters: "-addstore ""TrustedPublisher"" {tmp}\ppremote.cer"; StatusMsg: "Adding trusted publisher..." 

[UninstallRun] 
Filename: "{sys}\netsh.exe"; Parameters: "advfirewall firewall delete rule name=""Laser Remote Server"""; Flags: runhidden; Tasks: Firewall; 
    
[Code]
procedure InitializeWizard();
begin                                                                                          
    idpAddFileSize('{#VC_redist}', ExpandConstant('{tmp}\vc_redist.x86.exe'), 14426128);
    idpDownloadAfter(wpReady);
end;
/////////////////////////////////////////////////////////////////////
function GetUninstallString(): String;
var
  sUnInstPath: String;
  sUnInstallString: String;
begin
  sUnInstPath := ExpandConstant('Software\Microsoft\Windows\CurrentVersion\Uninstall\{#emit SetupSetting("AppId")}_is1');
  sUnInstallString := '';
  if not RegQueryStringValue(HKLM, sUnInstPath, 'UninstallString', sUnInstallString) then
    RegQueryStringValue(HKCU, sUnInstPath, 'UninstallString', sUnInstallString);
  Result := sUnInstallString;
end;

function IsUpgrade(): Boolean;
begin
  Result := (GetUninstallString() <> '');
end;
    
function InitializeSetup: Boolean;
var
  V: Integer;
  iResultCode: Integer;
  sUnInstallString: string;
begin
  Result := True; { in case when no previous version is found }
  if IsUpgrade() then  { Your App GUID/ID }
  begin
    sUnInstallString := GetUninstallString();
    sUnInstallString :=  RemoveQuotes(sUnInstallString);
    Exec(ExpandConstant(sUnInstallString), '/SILENT /NORESTART /SUPPRESSMSGBOXES', '', SW_SHOW, ewWaitUntilTerminated, iResultCode);
    Result := True; { if you want to proceed after uninstall }
    { Exit; //if you want to quit after uninstall }
  end;
end;                      
/////////////////////////////////////////////////////////////////////
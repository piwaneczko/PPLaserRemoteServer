; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "Laser Remote Server"        
#define MyAppPublisher "SilSense Technologies"
#define MyAppURL "http://www.SilSense.pl/" 
#define MyAppExeName "Laser Remote Server.exe"    
#define MyAppVersion GetFileVersion('Release\Laser Remote Server.exe')    

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
DefaultDirName={pf}\{#MyAppPublisher}\{#MyAppName}
DefaultGroupName={#MyAppPublisher}\{#MyAppName}
OutputBaseFilename=Laser Remote Server Setup
;OutputDir=Release
OutputDir=C:\Users\Pawel Iwaneczko\Dysk Google\Programy\PowerPoint\
WizardImageFile=silsense.bmp
WizardSmallImageFile=silsensesmall.bmp
WizardImageStretch=False
WizardImageBackColor=clWhite
Compression=lzma
SolidCompression=yes
DisableDirPage=auto      
DisableProgramGroupPage=auto
DisableReadyPage=true                  
;SignTool=silsense
;Registry key add PrivilegesRequired
PrivilegesRequired=poweruser

[Languages]
Name: "polish"; MessagesFile: "compiler:Languages\Polish.isl"
         
[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked   
Name: Firewall; Description: "Add an exception to the Windows Firewall"; GroupDescription: "Firewall:"; 

[Dirs]
Name: {app}; Permissions: users-full

[Files]           
Source: "Release\{#MyAppExeName}"; DestDir: "{app}"    
Source: "Release\{#MyAppExeName}.xml"; DestDir: "{app}" 
Source: "..\glemm vs\Drivers\vcredist_x86.exe"; DestDir: {tmp}; Flags: deleteafterinstall   
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; IconFilename: "{app}\{#MyAppExeName}" 
Name: "{group}\{#MyAppName}.xml"; Filename: "{app}\{#MyAppExeName}.xml";
Name: "{group}\Uninstall {#MyAppName}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon; IconFilename: "{app}\{#MyAppExeName}"

[Registry]
Root: HKLM; Subkey: "SOFTWARE\Microsoft\Windows\CurrentVersion\Run"; ValueType: string; ValueName: "{#MyAppName}"; ValueData: """{app}\{#MyAppExeName}"""; Flags: uninsdeletevalue

[Run]                                                                           
Filename: "{sys}\netsh.exe"; Parameters: "firewall add allowedprogram ""{app}\Laser Remote Server.exe"" ""Laser Remote Server"""; StatusMsg: "Adding exception to firewall for pragram: Laser Remote Server.exe..."; Flags: runhidden; Tasks: Firewall;
Filename: "{tmp}\vcredist_x86.exe"; Parameters: "/q /norestart"; StatusMsg: "Installing Visual C++ Redistributable Packages (x86)..."; Flags: runhidden
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[UninstallRun] 
Filename: "{sys}\netsh.exe"; Parameters: "firewall delete allowedprogram program=""{app}\Laser Remote Server.exe"""; Flags: runhidden; Tasks: Firewall; 
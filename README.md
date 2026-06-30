# MBRFilter++

This driver is made to Intercept Critical sector writes and stop them 
With a ExRaiseHardError to alert the user Immediatly
Intercepts: 
IRP (Execulsion is MFTMirr Sectors)
For the SCSI protections the Tests confirm no Loss of functionality, stability 
because NTFS.sys manages its components via IRP not SCSI 
MFTMirr is managed freely by NTFS.sys 
That is all what i have to say


## NOTICE!!!

DO NOT INSTALL THE DRIVER IF IT IS NOT SIGNED.

DO NOT ATTEMPT TO UNINSTALL THE DRIVER MANUALLY A SPECIFC TOOL WILL BE PUBLISHED TO UNINSTALL IT AND WILL BE BUILT

FOR EMERGENCY USE 

## DO NOT LOAD THE DRIVER IF

INTEL RAPID STORAGE TECHNOLOGY IS LOADED ON YOUR MACHINE 
THE MACHINE WILL BE BRICKED AND WILL NEED MANUAL RESTORATION

## YOU HAD BEEN WARNED

<img width="1567" height="747" alt="image" src="https://github.com/user-attachments/assets/4884ebcf-a730-4e83-be34-93a8e12d4684" />

# Manual Recovery in case anything goes Sideways

1. Boot to WinRE Open CMD from Trouble Shooting settings 

2. Type Regedit

3. Afterwards select HKEY_LOCAL_MACHINE

4. Click on File and mount the Registry Hive (SYSTEM) path is usually Z:\Windows\System32\Config (Change the drive letter Z: to where your windows install is) then load the hive and name it

5. After loading the Hive from Z:\Windows\System32\Config\SYSTEM
Go to MBRFilterPP usually in HKEY_LOCAL_MACHINE\(HIVE NAME that you named it with)\ControlSet001\Services\MBRFilterPP
set the START value to 4

6. After setting the MBRFilterPP service start key as 4,
Go to HKEY_LOCAL_MACHINE\(HIVE NAME that you named it with)\ControlSet001\Control\Class\{4d36e967-e325-11ce-bfc1-08002be10318}
there should be a UpperFilters key (REG_MULTI_SZ) that has a value of

partmgr 
MBRFilterPP

change to to:
partmgr (NO CAPS & NO SPACE AFTER IT)

[!Warning]
<mark>DO NOT DELETE IT IF YOU DELETED IT THE MACHINE WILL LOOP IN 0x7B OR OTHER ERRORS MAKE SURE TO SET THE VALUE CORRECTLY</mark>

7. Reboot the machine it should boot successfuly again,
after booting successfuly Delete the Service with 
Sc delete MBRFilterPP.

# Note: 
if you face any issues,
Repeat the steps and make sure of: 
1. Typing the Upperfilters Value CORRECTLY there is no spaces after partmgr and do not capitalize it

2. Make sure that you had set the Service to DISABLED

3. If you face any other issues Please contact me and if possible provide mini dumps, and as much info possible. 
thank you for using Salah-Code-Lab Solutions

You can Contact me in Session for Direct messages if you needed support or more info about the Driver(s):
056bf8ea1a057b4f351d8b651944252cd4d88416ce6c11761f0c406f228a302301

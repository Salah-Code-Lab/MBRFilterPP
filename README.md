# MBRFilter++

This driver is made to Intercept Critical sector writes and stop them<br>
With a ExRaiseHardError to alert the user Immediatly<br>
Intercepts:<br>
IRPs <mark>(Execulsion is MFTMirr Sectors)</mark>
For the SCSI protections, the Tests confirm no Loss of functionality, stability<br> 
because NTFS.sys manages its components via <code>IRPs</code> not SCSI<br> 
MFTMirr is managed freely by <mark>NTFS.sys</mark><br> 
That is all what i have to say


## NOTICE!!!
<mark>
DO NOT INSTALL THE DRIVER IF IT IS NOT SIGNED.<br>
DO NOT ATTEMPT TO UNINSTALL THE DRIVER MANUALLY A SPECIFC TOOL IS PUBLISHED TO UNINSTALL IT AND WILL BE PRE-BUILT<br>
FOR EMERGENCY USE</mark> 

## DO NOT LOAD THE DRIVER IF

INTEL RAPID STORAGE TECHNOLOGY (RST) IS LOADED ON YOUR MACHINE
THE MACHINE WILL BE BRICKED AND WILL NEED MANUAL RESTORATION

## YOU HAD BEEN WARNED

<img width="1567" height="747" alt="image" src="https://github.com/user-attachments/assets/4884ebcf-a730-4e83-be34-93a8e12d4684" />

<img width="1149" height="612" alt="image" src="https://github.com/user-attachments/assets/c364dc37-6cb4-400f-9550-43eb3ef24dcf" />

<img width="1152" height="557" alt="image" src="https://github.com/user-attachments/assets/d5871a40-5432-4862-9ef3-5e055f984abb" />



# Manual Recovery in case anything goes Sideways

1. Boot to WinRE Open CMD from Trouble Shooting settings 

2. Type Regedit

3. Afterwards select HKEY_LOCAL_MACHINE

4. Click on File and mount the Registry Hive (SYSTEM) path is usually <code>Z:\Windows\System32\Config</code> (Change the drive letter Z: to where your windows install is) then load the hive and name it

5. After loading the Hive from Z:\Windows\System32\Config\SYSTEM<br>
Go to MBRFilterPP usually in HKEY_LOCAL_MACHINE\(HIVE NAME that you named it with)\ControlSet001\Services\MBRFilterPP
set the START value to 4

6. After setting the MBRFilterPP service start key as 4 <code>(DISABLED)</code>,<br>
Go to HKEY_LOCAL_MACHINE\(HIVE NAME that you named it with)\ControlSet001\Control\Class\{4d36e967-e325-11ce-bfc1-08002be10318}<br>
there should be a UpperFilters key <code>(REG_MULTI_SZ)</code> that has a value of

partmgr<br>
MBRFilterPP

change to to:<br>
partmgr<br>
> (NO CAPS & NO SPACE AFTER IT)

Warning!
<mark>DO NOT DELETE IT IF YOU DELETED IT THE MACHINE WILL LOOP IN 0x7B OR OTHER ERRORS,<br>
MAKE SURE TO SET THE VALUE CORRECTLY</mark>

7. Reboot the machine it should boot successfuly again,<br>
after booting successfuly Delete the Service with<br>
<code>Sc delete MBRFilterPP</code>

# Note: 
if you face any issues,<br>
Repeat the steps and make sure of:<br> 
1. Typing the Upperfilters Value CORRECTLY there is no spaces after partmgr and do not capitalize it

2. Make sure that you had set the Service to <code>DISABLED</code>

3. If you face any other issues Please contact me and if possible provide mini dumps, and as much info possible.<br>
thank you for using Salah-Code-Lab Solutions

You can Contact me in Session for Direct messages if you needed support or more info about the Driver(s):<br>
> 056bf8ea1a057b4f351d8b651944252cd4d88416ce6c11761f0c406f228a302301

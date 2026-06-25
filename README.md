# MBRFilter++

This driver is made to Intercept Critical sector writes and stop them 
With a ExRaiseHardError to alert the user Immediatly
Intercepts: 
IRP (Execulsion is MFTMirr Sectors)
SCSI will bug check on any write in the Sector range 0- 5119 
For the SCSI protections the Tests confirm no Loss of functionality, stability 
because NTFS.sys manages its components via IRP not SCSI 
MFTMirr is managed freely by NTFS.sys 
That is all what i have to say


## NOTICE!!!

DO NOT INSTALL THE DRIVER IF IT IS NOT SIGNED.

DO NOT ATTEMPT TO UNINSTALL THE DRIVER MANUALLY A SPECIFC TOOL WILL BE PUBLISHED TO UNINSTALL IT AND WILL BE BUILT

FOR EMERGENCY USE 

## YOU HAD BEEN WARNED

<img width="1567" height="747" alt="image" src="https://github.com/user-attachments/assets/4884ebcf-a730-4e83-be34-93a8e12d4684" />

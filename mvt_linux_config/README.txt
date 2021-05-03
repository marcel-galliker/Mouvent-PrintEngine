The 'build_server_iso.sh' script creates an ISO based on Ubuntu Server 18.04.4
to automatically install Linux on our printer servers.

The script has been successfully tested:
* On Ubuntu 18.04.5 LTS (Desktop) in a VirtualBox virtual machine,
* On Ubuntu 18.04 (Microsoft) in WSL 2.

================================================================================
Prerequisites
================================================================================

1. If you use a virtual machine in WSL 1 please upgrade to WSL 2.
   Your virtual machine will be seamlessly upgraded to a newer format without any
   apparent change for you. Please read the following documentation:
   https://docs.microsoft.com/en-us/windows/wsl/install-win10
2. Ensure that the program 'xorriso' is installed.
   On Ubuntu for example:
   > sudo apt-get install xorriso
   Note: depending on your Linux flavor the install command line may be different.

================================================================================
How to create a new installer project
================================================================================

1.  Create a 'MyInstaller' directory for the project,
2.  Copy 'build_server_iso.sh', 'post_install.sh' and 'set_partition_disk.sh' to 'MyInstaller',
3.  Customize 'post_install.sh' to your liking.

================================================================================
How to generate the ISO file
================================================================================

For this project (ProjectPath=mvt_linux_config) or a new project (ProjectPath=MyInstaller),
in a command shell type:
> cd ProjectPath
> ./build_server_iso.sh

================================================================================
How to write and use the resulting ISO
================================================================================

The resulting ISO can be written:
* On a CD/DVD, or
* On a USB dongle.

Writing on a USB dongle may result in a non-bootable/non-working media depending on:
* the writing tool used, and/or
* the PC model we install on.

We tried different writing tools, with varied levels of success:
* 'Rufus' (our advised tool), available at https://rufus.ie:
  - After pressing 'START' select 'Write in ISO Image mode (Recommended)',
  - If the resulting USB dongle stops at the beginning, issuing an error
    about '/cdrom' not being mounted, then rewrite it using a different option:
    after pressing 'START' select 'Write in DD Image mode'.
* 'balenaEtcher', available at https://www.balena.io/etcher:
  The resulting USB dongle may work or not.
* 'UNetbootin': do not use it (the Debian installer detects it and issues a warning).

Steps:
1. Write the resulting 'mouvent-server-amd64.iso' to a CD/DVD or USB dongle,
2. Configure the target system to boot in BIOS/legacy mode (so not in UEFI mode),
3. Boot the target system with this media selected as the boot partition,
4. Wait for the target system to reboot (it indicates the end of the installation),
5. Connect to the system using the following logging information:
   user='mouvent'
   password='see CONFIG_USER_PASSWORD in build_server_iso.sh'

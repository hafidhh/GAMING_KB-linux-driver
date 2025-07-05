# Linux GAMING KB Keyboard Driver #

For Chipset `0x258a`:`0x1006`
The kernel reports the chipset as `GAMING KB`

Written for the Digital Alliance Meca Warrior USB Keyboard: [Digital Alliance Meca Warrior](https://digitalalliance.co.id/produk/da-meca-warrior/)

> NOTE: 
* Makefile and instructions are only tested on Arch, however they are known to work on Debian Ubuntu, Fedora, and Manjaro.
* this driver not set yet for 6 key mode, if you switch from 86 key to 6 key mode, keyboard will not working 

Reports suggest it supports the following keyboards as well:

 * Digital Alliance Meca Warrior

Original base: http://swoogan.blogspot.de/2014/09/azio-l70-keyboard-linux-driver.html, https://github.com/Swoogan/aziokbd

## Installation ##
### DKMS ###

    # debian-based:
    sudo apt-get install git build-essential linux-headers-generic dkms
    
    # fedora:
    sudo dnf install kernel-devel kernel-headers
    sudo dnf groupinstall "Development Tools" "Development Libraries"

    # arch:
    sudo pacman -S git base-devel linux-headers dkms
    
    git clone https://github.com/hafidhh/GAMING_KB-linux-driver.git
    cd aziokbd
    sudo ./install.sh dkms
    
### Manual Install ###

    sudo apt-get install git build-essential linux-headers-generic
    git clone https://github.com/hafidhh/GAMING_KB-linux-driver.git
    cd aziokbd
    sudo ./install.sh

## After reboot, the keyboard is not working ##

**NOTE: install.sh attempts to blacklist the driver for you. You shouldn't need to do anything manually. These instructions are to explain the process, in the event something goes wrong.**

You need to blacklist the device from the generic USB hid driver in order for the aziokbd driver to control it.

### Kernel Module ###
If the USB hid driver is compiled as a kernel module you will need to create a quirks file and blacklist it there.

You can determine if the driver is a module by running the following:

    lsmod | grep usbhid

If `grep` finds something, it means that the driver is a module.

Create a file called `/etc/modprobe.d/usbhid.conf` and add the following to it:

    options usbhid quirks=0x258a:0x1006:0x0004

If you find that the generic USB driver is still taking the device, try changing the `0x0004` to a `0x0007`.

### Compiled into Kernel ###
If the generic USB hid driver is compiled into the kernel, then the driver is not loaded as a module and setting the option via `modprobe` will not work. In this case you must pass the option to the driver via the grub boot loader.

Create a new file in `/etc/default/grub.d/`. For example, you might call it `aziokbd.conf`. (If your grub package doesn't have this directory, just modify the generic `/etc/default/grub` configuration file):

    GRUB_CMDLINE_LINUX_DEFAULT='usbhid.quirks=0x258a:0x1006:0x4'

Then run `sudo update-grub` and reboot.

Again, if you find that `0x4` doesn't work, try `0x7`.

### Add that configuration to the initramfs ###

Check if the usbhid driver is in the initramfs:

    lsinitcpio /boot/initramfs-linux.img | grep usbhid

if you use linux lts :

    lsinitcpio /boot/initramfs-linux-lts.img | grep usbhid

If the command returns something like:

    usr/lib/modules/<kernel version>/kernel/usbhid.ko

This means the usbhid driver is loaded early, before the file system is mounted.
Therefore, the usbhid configuration in `/etc/modprobe.d/usbhid.conf` cannot be read.

The solution is to add that configuration to the initramfs:

**WARNING: Be sure to understand what you are doing here. Corrupting you initramfs can prevent you system from booting**

    mkinitcpio -p linux

If properly configured, mkinitcpio should trigger the modconf hook which would automatically add the usbhid configuration into the initramfs.

## ✨ Link

* http://swoogan.blogspot.de/2014/09/azio-l70-keyboard-linux-driver.html
* https://github.com/Swoogan/aziokbd
#!/bin/bash

if [[ $1 != 'dkms' ]]; then
    echo '## Making package ##'
    make

    echo '## Installing package ##'
    make install

    if [[ $1 -ne 4 ]]; then
        quirk='0x258a:0x1006:0x0007'
        echo '## Quirk set to 0007 ##'
    else
        quirk='0x258a:0x1006:0x0004'
        echo '## Quirk set to 0004 ##'
    fi
else
    echo '## Installing package with DKMS ##'
    make dkms

    if [[ $2 -ne 4 ]]; then
        quirk='0x258a:0x1006:0x0007'
        echo '## Quirk set to 0007 ##'
    else
        quirk='0x258a:0x1006:0x0004'
        echo '## Quirk set to 0004 ##'
    fi
fi

modquirk="options usbhid quirks=$quirk"
grubquirk="usbhid.quirks=$quirk"

if (lsmod | grep 'usbhid'); then
    echo '## usbhid is module ##'

    # Making sure the quirk does not get added multiple times
    if ! (cat /etc/modprobe.d/usbhid.conf | grep "$modquirk"); then
        echo '## Writing to /etc/modprobe.d/usbhid.conf ##'
        echo $modquirk >> /etc/modprobe.d/usbhid.conf
        if [[ $1 != 'dkms' ]]; then sudo echo 'GAMING_KBkbd' >> /etc/modules; fi
    else
       echo 'NOTICE - modprobe config files have already been updated'
    fi

    echo '## Starting module ##'
    modprobe GAMING_KBkbd

    # Note: this line may fail if you have other drivers loaded that depend
    # on usbhid. For example, your mouse driver. In that case you would have
    # to remove those drivers first, then load them again.

    echo '## Attempting to reload usbhid module ##'
    rmmod usbhid && modprobe usbhid quirks=$quirk
else
    echo '## usbhid is compiled into kernel ##'

    # Making sure the quirk does not get added multiple times
    if ! (cat /etc/default/grub.d/GAMING_KBkbd.conf | grep "$grubquirk"); then
        echo '## Writing to /etc/default/grub.d/GAMING_KBkbd.conf ##'
        echo $grubquirk >> /etc/default/grub.d/GAMING_KBkbd.conf
        $distro = $(lsb_release -si)
        if ($distro | grep 'Ubuntu'); then
            update-grub
        fi
        if [ -f "/etc/arch-release" ]; then
            update-grub
        fi
        if [ -f "/etc/fedora-release" ]; then
            grub2-mkconfig -o /boot/grub2/grub.cfg
        fi
    else
       echo 'NOTICE - grub config file has already been updated'
    fi

    echo '## You must reboot to load the module ##'
fi 


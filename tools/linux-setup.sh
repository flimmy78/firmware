#! /bin/bash

refreshudev () {
    sudo service udev restart
    sudo udevadm control --reload-rules
    sudo udevadm trigger
}

groupsfunc () {
    sudo usermod -a -G tty $1
    sudo usermod -a -G dialout $1
    sudo usermod -a -G uucp $1
    sudo groupadd plugdev
    sudo usermod -a -G plugdev $1
}

acmrules () {
cat <<EOF
"KERNEL="ttyUSB[0-9]*", TAG+="udev-acl", TAG+="uaccess", OWNER="$1"
"KERNEL="ttyACM[0-9]*", TAG+="udev-acl", TAG+="uaccess", OWNER="$1"
EOF
}

openocdrules () {
cat <<EOF
ACTION!="add|change", GOTO="openocd_rules_end"
SUBSYSTEM!="usb|tty|hidraw", GOTO="openocd_rules_end"

#Please keep this list sorted by VID:PID

#CMSIS-DAP compatible adapters
ATTRS{product}=="*CMSIS-DAP*", MODE="664", GROUP="plugdev"

LABEL="openocd_rules_end"
EOF
}

avrisprules () {
cat <<EOF
SUBSYSTEM!="usb_device", ACTION!="add", GOTO="avrisp_end"
# Atmel Corp. JTAG ICE mkII
ATTR{idVendor}=="03eb", ATTRS{idProduct}=="2103", MODE="660", GROUP="dialout"
# Atmel Corp. AVRISP mkII
ATTR{idVendor}=="03eb", ATTRS{idProduct}=="2104", MODE="660", GROUP="dialout"
# Atmel Corp. Dragon
ATTR{idVendor}=="03eb", ATTRS{idProduct}=="2107", MODE="660", GROUP="dialout"

LABEL="avrisp_end"
EOF
}

dfustm32rules () {
cat <<EOF
# Example udev rules (usually placed in /etc/udev/rules.d)
# Makes STM32 DfuSe device writeable for the "plugdev" group

ACTION=="add", SUBSYSTEM=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="df11", MODE="664", GROUP="plugdev", TAG+="uaccess"
EOF
}

removemm () {
    if [ -f /etc/lsb-release -a ! -f /etc/SuSE-release ] || [ -f /etc/debian_version ] || [ -f /etc/linuxmint/info ]; then
        #Only for Ubuntu/Mint/Debian
        sudo apt-get remove modemmanager
    elif [ -f /etc/SuSE-release ]; then
        #Only for Suse
        sudo zypper remove modemmanager
    elif [ -f /etc/fedora-release ] || [ -f /etc/redhat-release ]; then
        #Only for Red Hat/Fedora/CentOS
        sudo yum remove modemmanager
    fi
}

if [ `uname -s` = "Linux" ]; then
USER=`whoami`
groupsfunc $USER
removemm
mkdir -p $PWD/rulesgen
acmrules $USER > $PWD/rulesgen/90-extraacl.rules
openocdrules > $PWD/rulesgen/98-openocd.rules
avrisprules > $PWD/rulesgen/avrisp.rules
dfustm32rules > $PWD/rulesgen/40-dfuse.rules
sudo mv $PWD/rulesgen/*.rules /etc/udev/rules.d/
rm -rf $PWD/rulesgen
refreshudev
fi
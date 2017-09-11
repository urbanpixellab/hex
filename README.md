# hex
Hexagon mapping


# Installing Raspbian

Below is a very quick and make-shift guide to install RPi with the hex project.

1. Install Raspbian Lite on micro SDHC card, use Raspbian stretch.

    wget https://downloads.raspberrypi.org/raspbian_lite/images/raspbian_lite-2017-07-05/2017-07-05-raspbian-jessie-lite.zip
    dd bs=4M if=unzipped_file_from_download-lite.img of=/dev/mmcblk0

1. Boot RPi from the card with network cable connected.

1. Log in with username `pi` and password `raspberry`.

1. Become root with `sudo su`.

1. Prevent installation of recommenede and suggested packages. Edit `/etc/apt/apt.conf` and add the lines:
    APT::Install-Recommends "false";
    APT::Install-Suggests "false";

1. Start configuration tool `raspi-config`.

1. Change password to default password of our organisation. (The username should remain `pi`.)

1. Change hostname to what is on the label this is on RPi casing. (Do not use this memory card in another RPi as that might result in hostname conflicts and/or unappropriate optimisation.)

1. Change boot options to `B1 Console` and to `B2, don't wait for network`.

1. Change localisation/internationlization to `I1` unselect with space bar `en_GB.UTF-8 UTF-8` and select with space `en_US.UTF-8 UTF-8` and save with `TAB` and `Ok`. Se default system locale to `en_US.UTF-8` and save with `TAB` and `Ok`.

1. Change localisation/internationlization to `I2` and choose `Europe` and `Tilburg`.

1. Change localisation/internationlization to `I3` and choose `Generic 105-key (Intl) PC` then `Other` and then `English (US)` and finally `English (US) - Enlish (US, with euro on 5)`. Then choose `No AltGr key` and `Right Logo key` for compose key.

1. Set localisation/internationlization to `I4` and choose `NL Netherlands` If you get errors on localisation follow this tip: https://raspberrypi.stackexchange.com/questions/22672/locale-errors-when-installing-packages/40146#40146


1. For Interfacing, choose SSH and choose `Yes`.

1. Set overclocking to maximum.

1. Advanced options, choose Memory Split and set to `16` in order to compile openFrameworks quickly. Later we will set this to `128` e.g.

1. Run `Update`.

1. Choose `Finish`.

1. Reboot by typing `reboot`. This will also activate new locale and keyboard settings.

1. Log in with username `pi` and the new password.

1. Become root with `sudo su`.

1. `apt-get update`

1. `apt-get dist-upgrade`

1. `apt-get install git tree`


# Installing USB wifi kernel driver

1. `cd ~`

1. `apt-get install lshw raspberrypi-kernel-headers dkms build-essential`

1. `git clone https://github.com/Mange/rtl8192eu-linux-driver.git`

1. `cd rtl8192eu-linux-driver`

1. edit `Makefile` and set `CONFIG_PLATFORM_I386_PC = y` to `n` and `CONFIG_PLATFORM_ARM_RPI = n` to `y`

1. `sudo dkms add .`

1. `sudo dkms install rtl8192eu/1.0`

1. edit as root `/etc/wpa_supplicant/wpa_supplicant.conf` and add
    network={
        ssid="WIFINAME1"
        psk="PASSWORD1"
    }

1. reboot with `sudo reboot`


# Setting up Wwifi accespoint on raspberry pi

1. Follow this guide: http://elinux.org/RPI-Wireless-Hotspot

1. You might run into troubles that udhcpd is not running. That is because the wireless driver needs time to come up add `sleep 20` to the top of `/etc/init.d/udhcpd` as found here: https://github.com/harryallerston/RPI-Wireless-Hotspot/issues/10 and here: https://unix.stackexchange.com/questions/351875/udhcpd-service-started-but-udhcpd-process-not-running alternatively you can try to add: `ifconfig wlan0 10.10.10.10`
`service udhcpd restart`
to /etc/rc.local (https://github.com/rachelproject/rachelpiOS/issues/15)

# Installing openFrameworks

`cd ~`
`wget http://ci.openframeworks.cc/versions/nightly/of_v20170714_linuxarmv6l_nightly.tar.gz`
`tar xf of_v20170714_linuxarmv6l_nightly.tar.gz`
`cd of_v20170714_linuxarmv6l_release`
`cd scripts/linux/debian`
`sudo ./install_dependencies.sh`
`cd ..`
`./compileOF.sh`
`sudo raspi-config` and set GPU memory to `128`
`apt-get clean`


# Installing hex application

1. `cd ~`

1. `cd of[TAB]`

1. `cd apps/myApps`

1. `git clone https://github.com/z25/hex.git`

1. `make`

1. test app with command `TODO`

1. TODO add startup of app to cron


# Installation status

hostname|model|tuned|wifi |openFrameworks|hex       |screen  |rpi-update
--------|-----|-----|-----|--------------|----------|--------|----------
janna¹  |2    |yes  |yes  |compiling     |uncompiled|blanking|
els     |2    |yes  |build|build         |uncompiled|blanking|
debora  |2    |yes  |build|build         |uncompiled|blanking|
petra   |2    |yes  |build|compiling     |uncompiled|blanking|
inge    |2    |yes  |build|build         |uncompiled|blanking|
hku     |3    |yes  |yes  |compiling     |uncompiled|blanking|
melanie |?    |yes  |     |compiling     |uncompiled|blanking|
qualle  |?    |yes  |     |compiling     |uncompiled|blanking|

1) red LED broken?

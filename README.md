# hex
Hexagon mapping


# Installing RPi

Below is a very quick and make-shift guide to install RPi.

1. Install Raspbian Lite on micro SDHC card, use Raspbian stretch.

1. Boot RPi from the card with network cable connected.

1. Log in with username `pi` and password `raspberry`.

1. Become root with `sudo su`.

1. Start configuration tool `raspi-config`.

1. Change password to default password of our organisation. (The username should remain `pi`.)

1. Change hostname to what is on the label this is on RPi casing. (Do not use this memory card in another RPi as that might result in hostname conflicts and/or unappropriate optimisation.)

1. Change boot options to `B1 Console` and to `B2, don't wait for network`.

1. Change localisation to `I1` unselect with space bar `en_GB.UTF-8 UTF-8` and select with space `en_US.UTF-8 UTF-8` and save with `TAB` and `Ok`. Se default system locale to `en_US.UTF-8` and save with `TAB` and `Ok`.

1. Change localisation to `I2` and choose `Europe` and `Tilburg`.

1. Change localisation to `I3` and choose `Generic 105-key (Intl) PC` then `Other` and then `English (US)` and finally `English (US) - Enlish (US, with euro on 5)`. Then choose `No AltGr key` and `Right Logo key` for compose key.

1. For Interfacing, choose SSH and choose `Yes`.

1. Set overclocking to maximum.

1. Advanced options, choose Memory Split and set to `16` in order to compile openFrameworks quickly. Later we will set this to `128` e.g.

1. Run `Update`.

1. Choose `Finish`.

1. Reboot by typing `reboot`. This will also activate new locale and keyboard settings.

1. If the RPi starts properly and all is well and you are done, stop the device with `halt` and only power off when the shutdown is complete.


# Futher configuration

1. TODO

cd /etc/apt
vi apt.conf

TODO




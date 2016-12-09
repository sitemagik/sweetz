Debian
======
This directory contains files used to package sweetcoind/sweetcoin-qt for Debian-based Linux systems. If you compile sweetcoind/sweetcoin-qt yourself, there are some useful files here.

## paycoin: URI support ##


sweetcoin-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install sweetcoin-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in the .desktop file or copy or symlink your paycoin-qt binary to `/usr/bin` and the `../../share/pixmaps/paycoin128.png` to `/usr/share/pixmaps`

sweetcoin-qt.protocol (KDE)

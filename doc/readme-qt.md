sweetcoin-qt: Qt4 GUI for sweetcoin
===============================

Build instructions
===================

Debian
-------

First, make sure that the required packages for Qt4 development of your
distribution are installed, for Debian these are:

    apt-get install qt4-qmake libqt4-dev build-essential libboost-dev libboost-system-dev \
        libboost-filesystem-dev libboost-program-options-dev libboost-thread-dev \
        libssl-dev libdb4.8++-dev

Ubuntu
------

    apt-get install qt4-qmake libqt4-dev build-essential libboost-dev libboost-system-dev \
        libboost-filesystem-dev libboost-program-options-dev libboost-thread-dev \
        libssl-dev libdb++-dev libminiupnpc-dev

then execute the following:

    qmake
    make

Alternatively, install Qt Creator and open the `sweetcoin-qt.pro` file.

An executable named `sweetcoin-qt` will be built.


Windows
--------

Windows build instructions:

- Download the [QT Windows SDK](http://qt.nokia.com/downloads/sdk-windows-cpp) and install it. You don't need the Symbian stuff, just the desktop Qt.

- Download and extract the [dependencies archive](https://download.visucore.com/sweetcoin/qtgui_deps_1.zip)[1], or compile openssl, boost and dbcxx yourself.

- Copy the contents of the folder "deps" to "X:\\QtSDK\\mingw", replace X:\\ with the location where you installed the Qt SDK. Make sure that the contents of "deps\\include" end up in the current "include" directory.

- Open the .pro file in QT creator and build as normal (ctrl-B)

<sub>[1] PGP signature: [https://download.visucore.com/sweetcoin/qtgui_deps_1.zip.sig](https://download.visucore.com/sweetcoin/qtgui_deps_1.zip) (signed with RSA key ID [610945D0](http://pgp.mit.edu:11371/pks/lookup?op=get&search=0x610945D0))


Mac OS X
--------

- Download and install the [Qt Mac OS X SDK](http://qt.nokia.com/downloads/sdk-mac-os-cpp). It is recommended to also install Apple's Xcode with UNIX tools.

- Download and install [MacPorts](http://www.macports.org/install.php).

- Execute the following commands in a terminal to get the dependencies:

		sudo port selfupdate
		sudo port install boost db48 miniupnpc

- Open the .pro file in Qt Creator and build as normal (cmd-B)


Build configuration options
============================

UPNnP port forwarding
---------------------

To use UPnP for port forwarding behind a NAT router (recommended, as more connections overall allow for a faster and more stable sweetcoin experience), pass the following argument to qmake:

    qmake "USE_UPNP=1"

(in **Qt Creator**, you can find the setting for additional qmake arguments under "Projects" -> "Build Settings" -> "Build Steps", then click "Details" next to **qmake**)

This requires miniupnpc for UPnP port mapping.  It can be downloaded from
[here](http://miniupnp.tuxfamily.org/files/).  UPnP support is not compiled in by default.

Set USE_UPNP to a different value to control this:

	USE_UPNP=- no UPnP support, miniupnpc not required;
	USE_UPNP=0 (the default) built with UPnP, support turned off by default at runtime;
	USE_UPNP=1 build with UPnP support turned on by default at runtime.


Notification support for recent (k)ubuntu versions
---------------------------------------------------

To see desktop notifications on (k)ubuntu versions starting from 10.04, enable usage of the
FreeDesktop notification interface through DBUS using the following qmake option:

    qmake "USE_DBUS=1"

Generation of QR codes
-----------------------

libqrencode may be used to generate QRCode images for payment requests.
It can be downloaded from [here](http://fukuchi.org/works/qrencode/index.html.en), or installed via your package manager. Pass the USE_QRCODE
flag to qmake to control this:

	USE_QRCODE=0 (the default) No QRCode support - libarcode not required
	USE_QRCODE=1 | QRCode support enabled


Berkely DB version warning
==========================

A warning for people using the *static binary* version of sweetcoin on a Linux/UNIX-ish system (tl;dr: **Berkely DB databases are not forward compatible**).

The static binary version of sweetcoin is linked against libdb4.8 (see also [this Debian issue](http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=621425)).

Now the nasty thing is that databases from 5.X are not compatible with 4.X.

If the globally installed development package of Berkely DB installed on your system is 5.X, any source you
build yourself will be linked against that. The first time you run with a 5.X version the database will be upgraded,
and 4.X cannot open the new format. This means that you cannot go back to the old statically linked version without
significant hassle!


Ubuntu 11.10 warning
====================

Ubuntu 11.10 has a package called 'qt-at-spi' installed by default.  At the time of writing, having that package
installed causes sweetcoin-qt to crash intermittently.  The issue has been reported as [launchpad bug 857790](https://bugs.launchpad.net/ubuntu/+source/qt-at-spi/+bug/857790), but
isn't yet fixed.

Until the bug is fixed, you can remove the qt-at-spi package to work around the problem, though this will presumably
disable screen reader functionality for Qt apps:

    sudo apt-get remove qt-at-spi
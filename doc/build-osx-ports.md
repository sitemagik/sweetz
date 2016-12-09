Copyright (c) 2012-2013 Paycoin Developers
Copyright (c) 2009-2012 sweetcoin Developers
Distributed under the MIT software license, see the accompanying file
COPYING or http://www.opensource.org/licenses/mit-license.php. This
product includes software developed by the OpenSSL Project for use in the
OpenSSL Toolkit (https://www.openssl.org/). This product includes cryptographic
software written by Eric Young (eay@cryptsoft.com) and UPnP software written by
Thomas Bernard.


Mac OS X Mountain Lion build instructions
=========================================
How to build paycoind and Paycoin-Qt on 10.8


Prerequisites
=============
Install Xcode from the App Store and launch it

All of the commands should be executed in Terminal.app

1.  Install the Command Line Tools from the Xcode Preferences under Downloads

2.  Download and install MacPorts from http://www.macports.org/install.php

3.  Install dependencies from MacPorts

		sudo port selfupdate
		sudo port install boost db48@+no_java openssl miniupnpc

4.  Clone the github tree to get the source code

	git clone https://github.com/PaycoinFoundation/paycoin.git


Building paycoind
=================

1.  Run the makefile

		cd Paycoin/src/
		make -f makefile.osx RELEASE=true 64BIT=true CXX=g++


Building Paycoin-Qt.app
=======================
You cannot use Qt 4.8.6 if you want to deploy to other systems
http://qt-project.org/forums/viewthread/41925

1.  Download and install Qt 4.8.5 from

		http://download.qt-project.org/archive/qt/4.8/4.8.5/qt-mac-opensource-4.8.5.dmg

2.  Download and install Qt Creator from

		http://qt-project.org/downloads

3.  Use Qt Creator to build the project

		Double-click Paycoin/sweetcoin-qt.pro to open Qt-Creator
		Click the Configure Project button
		Click on the monitor icon on the left bar above Debug and change it to Release
		On the top menu, click Build and Build Project "sweetcoin-qt"


Deploying Paycoin-Qt.app
========================
Deploying your app is needed to run it on non-development systems

1.  Sym-link some shit

		sudo ln -s /opt/local/lib /opt/local/lib/lib
		sudo ln -s /opt/local/lib/db48/libdb_cxx-4.8.dylib /opt/local/lib/libdb_cxx-4.8.dylib

2.  Copy Paycoin-Qt.app and run macdeployqt to bundle required libraries

		mkdir deploy
		cp -r build-sweetcoin-qt-Desktop-Release/Paycoin-Qt.app/ deploy/Paycoin-Qt.app/
		sudo macdeployqt deploy/Paycoin-Qt.app/

3.  Fix a dependency path

		install_name_tool -change "/opt/local/lib/db48/libdb_cxx-4.8.dylib" "@executable_path/../Frameworks/libdb_cxx-4.8.dylib" deploy/Paycoin-Qt.app/Contents/MacOS/Paycoin-Qt

4.  Compress Paycoin-Qt.app into a zip

		Right-click on Paycoin-Qt.app and click Compress
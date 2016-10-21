FTP-Map
==========

Developer notes
================


##### [Skod](https://github.com/Hypsurus/skod) will replace FTP-map

#### For release 1.0 we need TODO:

* Testing if FTPMap is stable .
* Adding more fingerprints.
* security check. (overflows, etc..)

#### FTPMap should work on ( As far I know): 

* linux (Arch, Debian, Fedora etc..) 100%
* ARM (RaspberryPi, Android etc..). 100%
* BSD (FreeBSD, etc..) 100%
* Mac? (I didn't test)
* Windows (Cygwin?)

###### For any question/help, you can reach me in: [@Hypsurus](https://twitter.com/Hypsurus)

#### FTP-Map is open source and free, Please help me with:

* Send bug reports.
* Code! we need more featuers.
* More fingerprints!

About
=====

Ftpmap scans remote FTP servers to indentify what software and what versions
they are running. It uses program-specific fingerprints to discover the name
of the software even when banners have been changed or removed, or when some
features have been disabled. FTP-Map will try to detect exploits by the  
FTP software/version.

Please send the fingerprint and the name of the software to hypsurus@mail.ru.
Another indication that can be displayed if login was successful is the FTP
PORT sequence prediction. If the difficulty is too low, it means that anyone
can steal your files and change their content, even without knowing your
password or sniffing your network.
There are very few known fingerprints yet, but submissions are welcome.


![FtpMap014](https://raw.githubusercontent.com/Hypsurus/ftpmap/master/ftpmap014.png)

Build
=====

    ./configure
    make
    sudo make install 

Usage
======


* Scan server:
    > ftpmap -Sgs localhost

* Scan multiple FTP servers:
    > ftpmap -gL list.txt

* Upload a file.
    > ftpmap -s localhost --user root --password root -U 'topsecretfile.txt'

* Download a file:
    > ftpmap -s localhost --user root --password root -d '/topsecretfile.txt'

* list files:
    > ftpmap -s localhost --user anonymous -p null -l '/'

* use --help for the full options.


Obfuscating FTP servers
=======================


This software was written as a proof of concept that security through
obscurity doesn't work. Many system administrators think that hidding or
changing banners and messages in their server software can improve security.

Don't trust this. Script kiddies are just ignoring banners. If they read
that "XYZ FTP software has a vulnerability", they will try the exploit on
all FTP servers they will find, whatever software they are running. The same
thing goes for free and commercial vulnerability scanners. They are probing
exploits to find potential holes, and they just discard banners and messages.
On the other hand, removing software name and version is confusing for the
system administrator, who has no way to quickly check what's installed on his
servers.

If you want to sleep quietly, the best thing to do is to keep your systems
up to date : subscribe to mailing lists and apply vendor patches.


DISCLAIMER
==========

Usage of FTPMap for attacking targets without prior mutual consent is illegal.
FTPMap developer (@Hypsurus) not responsible to any damage caused by FTPMap.

Get FTP-Map
=============
                git clone git://github.com/Hypsurus/ftpmap 

The END
=========
    
    Copyright 2015 (C) FTP-Map project developers.
    License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.

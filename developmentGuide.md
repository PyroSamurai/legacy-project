LeGACY Project development guide
================================
Development Tools
-----------------
* GNU+Linux 
* g++/gcc
* GNU+Linux DevTools package (make, automake, autoconf, libtool)
* High-end processor for fast compiling convenient. P4 up is recommended.
* Network card for TCP/IP communication between game client and server.
* WinPcap for internal TCP/IP tunneling (TAP Connection)
* MySQL database server/client
* SqlYog
* Subversion
* Development libraries (openssh, zlib, POSIX Thread, dsb)
* Your brain

What to start
-------------
For those who had installed Linux & other tools, you can skip these steps 
and get ready with the source

* Install Linux in Windows. Use coLinux.
  because coLinux can run Windows and doesn't use dual boot system.
  
  So, the server and client can work in 1 machine.
  except you have a pc to be used as server to check bugs 
  and debugging.
   
  More about coLinux:
  http://www.colinux.org/
  
  To Download coLinux & Debian click this link:
  coLinux-stable: http://sourceforge.net/project/showf...ease_id=385643
  Images 2.6.x Debian: http://sourceforge.net/project/showf...ease_id=622510
  
* After coLinux has established, you must install WinPcap in
  Windows dan set coLinux to have an ip address same as Windows.
  Or, if you use dhcp server, coLinux network configuration
  will be configured automatically.
  
  Download WinPcap here: http://www.winpcap.org/


* Install Linux DevTools package in coLinux.
  DevTools consists of files which will be used in developing such as:
  make, automake, autoconf, libtool, etc.
  For coLinux Debian/Ubuntu use this command:```  $ apt-get install devtools```

* Install Subversion in coLinux. http://subversion.tigris.org/
  
  For coLinux Debian/Ubuntu use this command:``` $ apt-get install subversion```

* Install development libraries in coLinux, with this command:
```
  $ apt-get install libssl-dev
  $ apt-get install libmysql++-dev
  $ apt-get install gnulib
  $ apt-get install libpth-dev
  $ apt-get install zlib
  $ apt-get install libZThread
```
* Install MySQL server in Windows. http://www.mysql.com/

* Install SqlYog in Windows. http://www.webyog.com/

Getting the Source
==================

* To download LeGACY Project src codes you must use subversion.
  For coLinux Debian/Ubuntu use this command:
```
  $ svn checkout http://legacy-project.googlecode.com/svn/trunk/ legacy-project
```
Compilation
===========
* Open up legacy-project, type:```cd legacy-project```

* Prepare configuration script (important), type:
```
  $ libtoolize --force
  $ aclocal
  $ autoconf
  $ automake -a
  $ autoheader
```
* Configure project, type:```  $./configure```
  When configure, maybe you'll got some error messages because there are some
  missing libraries, please install all required libraries before proceed to
  next step. Use your brain and google, I will not help you on this!!!

* After finished, type:```$ make```
* If success, edit file ```legacyd.conf``` which located at ```./legacy-project/src/legacyd```

  Edit last 3 lines below according to:
  - MySQL Server IP
  - MySQL Server Port
  - MySQL Username
  - MySQL Password
  - MySQL Database
```sql
LoginDatabaseInfo     = "192.168.0.15;3306;legacy;legacy;realmd"
WorldDatabaseInfo     = "192.168.0.15;3306;legacy;legacy;legacydb"
CharacterDatabaseInfo = "192.168.0.15;3306;legacy;legacy;characters"
```

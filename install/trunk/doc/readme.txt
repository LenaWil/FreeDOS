This is the README for the FreeDOS Install program.

Maintained by Jeremy Davis, <jeremyd@computer.org>
Copyright (C) 1998-2002 Jim Hall, <jhall@freedos.org>

The Install program is distributed under the terms of the GNU GPL (see
the file COPYING for details.)  You may use the Install program for
pretty much anything: commercial distributors may even use it as an
installer for their own software.

For installation instructions, see the INSTALL file.

Before running INSTALL.EXE please correctly set the following 
enviroment variables:
TZ=EST or proper timezone, if missing defaults to GMT0 which means no offsets (unzip)
LANG=ES or proper language, if missing hard-coded english values used
NLSPATH=.\NLS or proper path, if missing hard-coded english values used

E.g.
  CD SAMPLE
  SET TZ=EST5DST
  SET LANG=ES
  SET NLSPATH=.\NLS
  INSTALL


* To run simply type:

  install

* Optionally add the /mono switch for black & white text

* Optionally add the /nolog switch and <dest path>\INSTALL.LOG 
  is NOT created


install.exe [/mono] [/nolog] [/src <path>] [/dst <path>] [/noprompt]

Requires INSTALL.DAT, OEM.TXT, COPYR.TXT, and <diskset>.TXT 
(or localized version of text files) to be in same directory as
INSTALL.EXE (or in language specific directory) when starting.
Additionally, you should set the TZ (timezone), LANG (your language),
and NLSPATH (location of INSTALL.%LANG% catalog file) environment
variables before running install.

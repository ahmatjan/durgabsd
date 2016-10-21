#!/usr/bin/env python2.7
#
# ProFTPD versions - create proftpd versions and dump them into versions.h
#
# Copyright (c) 2015 by Hypsurus
#
#

import sys

# The proftpd versions cycle:
# proftpd-1.3.2rc1 
# proftpd-1.3.2rc2 
# proftpd-1.3.2rc3 
# proftpd-1.3.2rc4 
# proftpd-1.3.2 
# proftpd-1.3.2a 
# proftpd-1.3.2b 
# proftpd-1.3.2c 
# proftpd-1.3.2d 
# proftpd-1.3.2

# Versions
versions = []
VERSION=1

for version_mi in xrange(1, 4):
    # Just in case thay release 1.x.20
    for version_mic in xrange(0, 21):
        fixed = "ProFTPD%d.%d.%d" %(VERSION,version_mi,version_mic)
        versions.append(fixed)
        versions.append(fixed+"rc1")
        versions.append(fixed+"rc2")
        versions.append(fixed+"rc3")
        versions.append(fixed+"rc4")
        versions.append(fixed+"a")
        versions.append(fixed+"b")
        versions.append(fixed+"c")
        versions.append(fixed+"d")
        versions.append(fixed+"e")
        versions.append(fixed+"f")
        versions.append(fixed+"g")

# Fix the versions to file
print("/* version.h - created by the proftpd_versions.py script by Hypsurus */\n\n")
print("const char * versions[] = {")
for version in versions:
    print("\t\t\"%s\"," %version)
print("};")

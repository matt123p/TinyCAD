# TinyCAD version 3.00.02

## Overview

This version is a bug fix release that addreses the following issues:

1. Fix pagesize dialogue
2. Fix update check

# TinyCAD version 3.00.01

## Overview

This version is a bug fix release that addreses the following issues:

1. Embedded JPEGs were not being displayed correctly
2. Netlist output for Protel and TinyCAD format was incorrect
3. The installer had problems installing the Visual Studio redistributable


# TinyCAD version 3.00.00

## Overview

Although this version does not introduce any major new features, there are a few bug fixes and several small enhancements.
This version is also compiled with Visual Studio 2019 which means that it should have much improved compatibility with the
latest versions of Windows.

## New Icon

After all this time I have changed TinyCAD's icon.  If you had a previous version of TinyCAD installed, you will probably
not see the new icon until you reboot.  Please let me know if you like it!


## Installation

_MAKE A BACKUP OF YOUR LIBRARIES_ *before* you install this version of TinyCAD.  Also not do uninstall TinyCAD first, the
previous installer will delete the pre-installed libraries before installing the new ones, which means that if you edited
the pre-installed libraries then your work will also be removed.


## Old-style mdb file format

The old library format used a now abandoned library called the Jet Database.  TinyCAD moved from this format to an open-source
library many years ago, and now it is time to drop support for the old format.  

Technically TinyCAD will still support the old library format for now, however, the installer does not install the required 
library.  If you have libraries in the old format then I recommend:

1. Before installing this version of TinyCAD, upgrade your libraries
2. Do not uninstall the previous version of TinyCAD, just install this one over the top, this will leave the old Jet Database library installed and TinyCAD will be able to open the files.


Here are the changes since 2.90.00:


## New Features:

- Keeps track of Netlist names & numbers so that when a netlist is re-generated the numbers do not change unless they have to
- Page dialogue now includes US Page Sizes
- Unfilled polygons can now be closed
- Allow polygons, circle and other shapes to have a fill but no line
- Allow free curves (not just quarter arcs) on polygons
- Dropped compiled help file in favour of a single HTML file for the manual, which can also be printed


## Bug Fixes:

- Fix crashes in the Find dialogue
- Fix copy/paste in to applications such as Word which were coming out the wrong size


## Documentation

The documentation is now built from the Github wiki, this makes it easier than ever to update.  It also means the documention is now 
in a single HTML file which is easy to print or convert to a PDF.


## Developers

Building TinyCAD should be easier than ever, as all the additional libraries are either included or installed directly from
Nuget.

- Switch to Visual Studio 2019 which should improve compatibility with Windows 10
- Use Nuget packages for pre-compiled libraries


# CSpice

## Changes
  - Add os_windows port;
  - Add cmake build process manager;
  - Fix some code for ANSI-C.

## Requirement

  - Visual Studio
  - Cmake


## For build, run:
```
$ CD PATH_TO_SPICE_3F5_SRC\build
$ cmake ..
```
Open _PATH_TO_SPICE_3F5_SRC\build\Spice3f5.sln_ in Visual Studio and build.

## Notes

For additional information see notes/\*, lib/helpdir\* and man/\*.

## Old readme
```
								July, 1993

	Spice3f4

---------- New features in Spice3f.4

With this release, the University of California will no longer
be providing technical support for Spice3, and no plans have been made
for providing support any time in the future.  Some work on Spice3 will
continue (notably the BSIM-3 model and perhaps other device models).

The following is a list of new features and fixes from the previous
release of Spice3, 3f.3 (note that 3f.4 is a "last minute" release with
only a few new changes):
	Initial conditions specified across voltage sources is now handled
	"alter" command accepts expressions, new syntax is either:
		alter dev param = expr
		alter @dev[param] = expr
	"altermod" command for altering model parameters (was "alter")
	minor bug fixes

The following is a list of new features and fixes from the previous
major release of Spice3 (3f.2) and the previous release (3f.3) (note
that 3f.3 is primarily a bug-fix release over 3f.2):
	Added non-linear controlled source functions "u(x)" and "uramp(x)"
	Fixes to AC sensitivity calculation
	Fixes to initial conditions code
	Fixed spurious error messages when running multiple anaylses in
		batch mode
	Fixed plotting (again)
	Fixed "available memory" calculation
	Fixed default scale to be log for analysis done by decade/octave
		(ac, disto, noise); use plot option "linear" to override
	Fixed asciiplot
	Fixed problem causing BSIM models to not be found
	Removed "spiced" (a remote-spice daemon for BSD unix); added an
		rspice command which uses "rsh" instead.
	Online help info is now identicle to the "Spice3f User's Manual"
	Fixed numerous and various parsing errors
	"TF" analysis output is now identified as "TF" data, not "DC" data
	Complex numbers and Real vectors are now printed by the
		"show"/"showmod" commands
	Fixes to help MacIntosh port
	Port for DEC Alpha running OSF/1
	Misc. changes to improve portability

Additional features since release 3e.2 are:
	AC and DC Sensitivity.
	MOS3 discontinuity fix ("kappa").
	Added a new JFET fitting parameter.
	Minor initial conditions fix.
	Rewritten or fixed "show" and "trace" commands.
	New interactive commands "showmod" and "alter".
	Minor bug-fixes to the Pole-Zero analysis.
	Miscellaneous bug fixes in the front end.


----------  Systems supported

Spice3f.4 has been compiled and and run under the following operating systems:
	OSF 1, DEC Alpha
	Ultrix 4, RISC or VAX
	SunOS 4, Sun3 or Sun4

The following systems have been successfully tested either in the past or
by someone outside of UC Berkeley.
	MS-DOS on the IBM PC, using MicroSoft C 5.1 or later
	AIX V3, RS/6000
	HP-UX 8.0, 9000/700
	Dynix 3.0, Sequent Symmetry or Balance (does _not_ take advantage of
		parallelism)
	HP-UX 7.0, 9000/300
	Irix 3.2, SGI Personal Iris
	NeXT 2.0
	Apple MacIntosh, Using Think C

Other systems may require a small amount of porting effort.  Note that
the 'gcc' C compiler was used successfully to compile Spice3f.4.

Due to the heavy use of floating point math operations, Spice3 on the
PC requires a math co-processor.  Also, on the PC, SVGA displays are
_not_ supported.  Only CGA, EGA, and VGA displays are supported (via
the MicroSoft graphics library) at this time.

A 68020 or better processor and a math co-processor is required for
the MacIntosh.

Systems using the X11 Window System (generally any "workstation" class
of system) must have the MIT Athena Widget Set  available ("libXaw.a"
and possiblely "libXmu.a").  These are frequently not distributed or
distributed as "usupported software" by commercial workstation
vendors.

---------- Unloading Spice3 from disk or tape

The Unix distribution comes on 1/2" 9-track tape, 8mm tape, or DEC TK50 tape
in "tar" format.  The MS-DOS distribution comes on several 3.5" floppy
diskettes (both high and low density) in the standard MS-DOS format.
The contents of both distributions are identical, including filenames,
except for the additional files on the MS-DOS disks used for automatic
unloading.

The source code and associated data files for spice3f.4 require over 6MB,
and up to an additional 22MB may be required to compile under Unix (for a
DEC RISC workstation with the compiler option '-g').  For MS-DOS using
MicroSoft C 5.1 or later, nearly 8MB (beyond the 6MB for the source) is
required.

UNIX:  The UNIX distribution of Spice3f.4 comes in "tar" format.  To
	extract Spice3f.4 first create the directory that you wish to
	hold the distribution and "cd" into that directory.  Then
	execute the command "tar x" (after mounting the tape).  Note
	that some sites may require that you explicitly indicate the
	tape drive name when using the "tar" command; this is done with
	the 'f' flag, for example "tar xf /dev/rmt0h".

MS-DOS:  Spice3f.4 comes on MS-DOS format 3.5" disks.  To extract the
	distribution onto a hard disk, create the directory on the hard
	disk that you wish to hold the source code.  "cd" into that
	directory on the hard disk.  For each of the distributed disks,
	IN ORDER, insert the disk into the drive (we'll assume drive
	"B:" here), and enter "B:UNLOAD B:".  This will use the script
	"unload.bat" to extract the source files off of the disk and
	into the current directory or a subdirectory of the current
	directory.  You might see the error "File not found ????????.???",
	that is normal.

Converting the MS-DOS format disks to UNIX:  The MS-DOS format is not
	directly readable by UNIX systems.  One publically available
	tool for doing this is known as "mtools" (search popular
	ftp sites), but this is not the only method.  Note: In MS-DOS,
	text file lines end with "^M^J", where under UNIX lines end only
	with "^J".  Also, execute permission needs to be set on all
	files in the "util/" subdirectory when moving to UNIX.

MAC:  Spice3 is not distributed in a format for the Apple MacIntosh.  You
	must determine how to transfer the files to a Mac from the media that
	you have.

---------- Compiling Spice3f.4 under UNIX

	To build Spice3f.4 on a Unix system follow the steps below.
	For additional notes on internal changes and porting issues,
	please inspect the subdirectory "notes".

	First you must edit the file "conf/defaults" and change the
	listed parameters to reflect the standard organization of
	software at your site.  A description of each parameter is
	included in this file.

	Second, for each type of system at your site, you must provide
	a file in the same subdirectory ("conf/") which contains
	exceptions to the previously edited "defaults" file; note that
	even if you are supporting one type of system with no
	exceptions this is still necessary.  Several files are supplied for
	the system types that have been tested with this distribution,
	including "mips" (for DECstations), "sun4", "sequent", "irix",
	"hpux", and "rs6000".

	Note that some of these per-system definition files have
	special definitions which are required for the given system and
	which do not appear in the "defaults" file.

	Note also that successful compiling does not depend on the
	particular names given to these 'exception' or 'system
	definition' files.  For example, the "mips" configuration file
	could have been named "decstation"; this name is used for
	generating unique directory names such that different systems
	to not use the same area for the compile process (more detail
	are listed in the "defaults" file).

	Finally, it is possible to combine additional files

	After the defaults file has been edited and a system-dependent
	file created or modified, run the command "util/build system"
	from the directory above the "util" subdirectory; for "system"
	you must substitute the name of the system-dependent file that
	you created or modified in the previous step.  Spice3 will then
	be built via recursive "make" commands (it may take a several
	seconds to get going on some systems).  The total time can be
	as little as 20 minutes or as long as four hours depending on
	the speed and load of your system.  Compiling across NFS will
	slow down compiling significantly.

	If you have trouble using the "build" script, try "build -help"
	for infomation on debugging options.  Some system combinations
	may require listing more than one system name on the "build" line,
	for example "build hpux hp300" to build on an HP 9000/300 as
	opposed to a HP 9000/700.  When the build script has successfully
	completed, see the section below on Installing Spice3.
---------- Additional Notes on Compiling Spice3f.4 under UNIX

	NOTE: Some systems have a problem with directory names containing
	a "-" in either the full source directory name or the full directory
	name of any program used in compiling.  This may show up as an
	unexpected or unreasonable error message.  Avoid directory names
	with a "-" (this is an old bug in the "make" command, which has
	propagated to many vendors before being fixed).

	NOTE: On some systems, the "build" scripts seems sluggish.  This
	may be caused by a long "$PATH", with many entries before "/bin"
	and "/usr/bin"; you may be able to speed up the script by trimming
	your "$PATH" before running the build script.

	NOTE (SunOS):  There is small bug somewhere in one version of the
	Sun-supplied X11 libraries.  This bug results in the following
	routines being undefined in the "link" stage:

		_get_wmShellWidgetClass
		_get_applicationShellWidgetClass

	This is apparently a problem with the dynamic-link version of the
	"Xmu" library.  If you have this problem, the best reported fix is
	to use the following options on the "link" line (embed the following
	in the LIBX variable in "conf/sun4", or wherever):

		-Bstatic -lXmu -Bdynamic

	This is anticipated in a comment in the current copy of "conf/sun4".

	NOTE: GNU make is not compatible with the traditional "make", and
	is not compatible with spice3.  Using GNU make may result in
	infinite recursion.


---------- Compiling under MS-DOS with MicroSoft C 5.1:

	The PC lacks the program "/bin/sh" (and others) which the above
	Unix installation depends on.  Instead we have supplied simple
	compiling scripts for use with MicroSoft C 5.1.  This less
	flexible system requires that you edit several files before building:

		src/bin/tunepc.c (built-in file locations); Change the values
			of the following C variables as appropriate -- leave
			double quote and single quote marks as-is, and use
			two backslash ('\') characters where you want one:

			Spice_Exec_Dir: location you plan to install spice.

			Spice_Lib_Dir: location you plan to install the spice
				startup and data files.

			Spice_OptChar: command line option character
				(indicates whether you want to type
				"spice -r" or "spice /r").

			Def_Editor: location you plan to install spice

			AsciiRawFile: location you plan to install spice

			The last three options (not listed here) are not
			significant under MS-DOS, but should be left as is
			(blank).

		src/bin/cconf.c		Devices and analyses for "cspice"
		src/bin/bconf.c		Devices and analyses for "bspice"

			The initial segment of "cconf.c" and "bconf.c"
			are "#define" lines that determine which devices
			and analyses should be compiled in to the simulator.
			For devices, the line looks like "#define DEV_xxx"
			where "xxx" is the name of some device;  For
			analyses, the line looks like "#define AN_xxx".
			Names and descriptions of both analyses and devices
			are as listed below ("Devices and Analyses supported
			in Spice3f.2", at the end); see also the user's manual.
			A reasonable default is supplied with each.
			Do not modify anything below the list of '#define'
			lines.
		(more)

		src/include/os_msdos.h
			If you do not want the spice-supplied "hardcopy"
			routine for sending plots to an IBM Personal
			Graphics Printer (or equivalent), comment out the
			"#define WANT_PCHARDCOPY" line near the bottom.
			Some versions of MS-DOS can't normally send VGA
			graphics to a printer.

	Once these files have been edited, "cd" into the top directory
	(above "util\", "src\", and "conf\") and run "msc51.bat".  The
	script first sets compiler options in the environment and then
	procedes with the compile.  Since the environment may not have
	enough room, you may have to clear some unused environment
	variables before the build, otherwise the build could fail.
	Note that these options are only useful for MicroSoft C 5.1 or
	later.

	Compiler errors are written to the file "src\msc.out".  This
	includes many warnings because Spice3 was originally written in
	pre-ANSI C (or "K&R" C) under Unix.  Except for these (numerous)
	warnings, Spice3 should compile and run without trouble; see the
	next section on installing.

	Cleaning up under MS-DOS (after installing):

	To delete the original source tree from your hard disk, you may
	use the supplied script "util\delall.bat".  You must copy
	this script outside of the source tree before you run it or it
	will remove itself before finishing.  Run the script from the top
	directory of the source tree (above "util\").

---------- Compiling on the MacIntosh

	Since there is no scripting language on the Mac, you must do
	considerable work to build spice3 on a Mac.  The details
	are included in a seperate file, "notes/mac_port".

---------- Installing Spice3

	After spice3 and the associated programs have been created, you may
	test the program.  There are a few test inputs in the "examples"
	subdirectory.

	Because spice3 is not installed in it's final destination at this
	point, you should set the environment variable "SPICE_LIB_DIR" to
	the "lib" subdirectory to indicate the location of some startup files.

	Finally, you may install spice3 and associated components into
	a standard place.  Under Unix systems, the command "util/build
	system install" will do this automatically (again substitute
	your system name or type for "system").  Under MS-DOS or for
	the MacIntosh, the files must be copied explicitly.

	The executable programs are from the subdirectories "src/bin",
	as follows:
		spice3		UNIX only: the simulator.
		bspice		MS-DOS only: a batch mode simulator:
				"bspice < input.cir" generates the file
				"rawspice.raw", which is read by "nutmeg"
				(see below).
		cspice		MS-DOS only: a spice2 like interface for
				small runs (runs out of memory easily).
				Use "cspice < input.cir"; generates
				'asciiplots' for .plot lines.
		nutmeg		A stand-alone data analysis program;
				Spice3 without the simulation capability.
		help		A stand alone help browser.
		proc2mod	Converts process characterization files
				to Spice3 BSIM1 MOS model definitions.
		sconvert	Converts between ascii and binary spice
				data files (".raw" files).
		multidec	A utility for decomposing coupled lossy
				transmission lines into equivalent uncoupled
				lines.  Not available on MS-DOS (need the
				"getopt" library).

	The following startup/data files are installed from the "lib/"
	subdirectory:
		helpdir/spice.txt	on-line information for spice3.
		helpdir/spice.idx	index for spice.txt, generated with
						the program "bin/makeidx".

		scripts/spinit		spice/nutmeg commands executed at
						startup.
		scripts/setplot		A script for the command "setplot".

		news			a start up message of your choosing.
		mfbcap			graphics-terminal capability database
					(not required for MS-DOS).

	(Previous versions had a seperate help files for spice3 and
	nutmeg;  the current help file is now identicle to the "Spice3f4
	User's Manual", so there is no distinction).

	For the PC and MacIntosh, you must generate the ".idx" files
	yourself by running "makeidx spice.txt".  Unix "man" pages are
	also supplied for the programs spice, nutmeg, and sconvert, for
	the mfb database format (looks like termcap), and for the mfb
	library.  These are not installed automatically as they are
	old, unsupported, and may be out of date.

Devices and Analyses supported in Spice3f.4:
	For reference, the following is a list of all devices and their
	common abbreviation in Spice3:

		asrc:	arbitrary voltage/current source
		bjt:	bipolar junction transistor
		bsim1:	detailed MOS model
		bsim2:	detailed MOS model, revised version of bsim1
		cap:	capacitor
		cccs:	current-controlled current source
		ccvs:	current-controlled voltage source
		csw:	current controlled switch
		dio:	diode
		ltra:	lossy transmission line
		ind:	inductor
		isrc:	current source
		jfet:	Junction FET
		mes:	MES FET (GaAs)
		mos1:	MOS, simplest analytic model, fastest
		mos2:	MOS, middle complexity and accuracy
		mos3:	MOS, most complicated, most accurate
		mos6: 	MOS, new, fast analytic, short-channel
		res:	resistor
		sw:	switch
		tra:	lossless transmission line
		urc:	uniform RC line
		vccs:	voltage-controlled current source
		vcvs:	voltage-controlled voltage source
		vsrc:	voltage source

	The following is the corresponding list of analyses:
		op:	DC operating point
		dc:	DC transfer curve
		tf:	Small signal transfer function
		ac:	AC (frequency domain)
		tran:	transient
		pz:	pole-zero
		disto:	distortion
		noise:	noise
		sense:	sensitivity



Technical Problems

	Spice no longer has technical support from the
	University of California.  If you have problems with the
	media or questions about ording spice3, you may send email to:

		software@eecs.berkeley.edu

	or (via US Mail):

		EECS/ERL Industrial Support Office
		Attn: Spice Technical Question
		205 Cory Hall
		U.C. Berkeley
		Berkeley, CA   94720

	Patches for some previous version of spice3 are available via
	anonymous ftp from ic.berkeley.edu, in the subdirectory "pub/spice3/".
```
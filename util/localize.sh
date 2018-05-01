#! /bin/sh -
#
#	This shell script calls spice binaries indirectly after setting
#	some spice3-specific environment variables.  This way, you can
#	change the organization of the directories containing spice3
#	without re-compiling.
#
#	To use these for the program "xxx", move the actual executable to 
#	"xxx.bin", and copy this script to "xxx" (be sure this script is
#	executable).  Edit this script as necessary to set the listed
#	variables.

# Where the spice3 executable resides
SPICE_EXEC_DIR=~cad/bin

# Where spice3 support files reside
SPICE_LIB_DIR=~cad/lib/spice3

# For "rspice", the server name
# SPICE_HOST=localhost

# For mailing bugs
# SPICE_BUGADDR=cad@localhost

# Editor used by the "edit" command
# SPICE_EDITOR=/usr/ucb/vi

# Set to 1 if you want raw data files to be in ascii (to move across
# different types of systems.
# SPICE_ASCIIRAWFILE=0


# The following will be set automatically to the values shown; if you want
# to override these values, uncomment out the relevant line.
#
# SPICE_NEWS=$(SPICE_LIB_DIR)/news
# SPICE_MFBCAP=$(SPICE_LIB_DIR)/mfbcap
# SPICE_HELP=$(SPICE_LIB_DIR)/helpdir
# SPICE_SCRIPTS=$(SPICE_LIB_DIR)/scripts
# SPICE_PATH=$(SPICE_EXEC_DIR)/spice3
#

# Don't edit this line.
exec $0.bin

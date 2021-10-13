###############################################################################
# Author: Tudor Rus & Eric Rudzin
# Date: 2/15/2021
# Pledge: I pledge my honor that I have abided by the Stevens Honor System
# Description: a simple bash script to provide the basic functionality of a recycle bin
###############################################################################
#!/bin/bash

help_flag=0
list_flag=0
purge_flag=0
num_flags=0

readonly junk_dir=~/.junk

function help_menu {
  cat << HEREDOC
Usage: $(basename "$0") [-hlp] [list of files]
   -h: Display help.
   -l: List junked files.
   -p: Purge all files.
   [list of files] with no other arguments to junk those files.
HEREDOC
}

while getopts ":hlp" option ; do
	case "$option" in
		h)
			help_flag=1
			if [ $num_flags = 1 ] ; then
				echo "Error: Too many options enabled." >&2 
				help_menu
				exit 1
			fi
			num_flags=1
		;;
		l)
			list_flag=1
			if [ $num_flags = 1 ] ; then
				echo "Error: Too many options enabled." >&2 
				help_menu
				exit 1
			fi
			num_flags=1
			
		
		;;
		p) 
			purge_flag=1
			if [ $num_flags = 1 ] ; then
				echo "Error: Too many options enabled." >&2 
				help_menu
				exit 1
			fi
			num_flags=1
		;;
		?) 
			echo "Error: Unknown option '-$OPTARG'." >&2 
			help_menu          
			exit 1 
		;;    		
	esac
done


#checks if help flag is used
if [ $help_flag = 1 ] ; then
	help_menu
	exit 1
fi

#puts names of files in list
# shift "$((OPTIND-1))"
declare -a filenames
index=0
for f in "$@"; do
	if [ -f "$f" ] || [ -d "$f" ] ; then
		filenames[$index]="$f"
		(( ++index ))
	fi
done


# check if junk exists, otherwise makes it
if [ ! -d $junk_dir ] ; then
	mkdir $junk_dir
fi


#check case ./junk.sh with not flags or file names is given
if [  $num_flags = 0 ] ; then 
	if [ ${#filenames[*]} = 0 ] ; then
		help_menu
		exit 1
	 else
    		# delete given files
    		for f in "${filenames[@]}"; do
      			if [ ! -f "$f" ] && [ ! -d "$f" ]; then
        			printf "Warning: '%s' not found.\n" $f >&2
        			
      			else
        			mv $f $junk_dir;
        		fi
        	done
	fi
fi

# purge function
function purge {
    for file in $junk_dir; do
    	rm -rf $file
    done
}

if [ $purge_flag = 1 ] ; then
	purge
	exit 1
fi

if [ $list_flag = 1 ] ; then
	ls -lAF $junk_dir
	exit 1
fi


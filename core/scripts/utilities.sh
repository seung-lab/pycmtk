#!/bin/sh

##
##  Copyright 2007-2009 SRI International
##
##  This file is part of the Computational Morphometry Toolkit.
##
##  http://www.nitrc.org/projects/cmtk/
##
##  The Computational Morphometry Toolkit is free software: you can
##  redistribute it and/or modify it under the terms of the GNU General Public
##  License as published by the Free Software Foundation, either version 3 of
##  the License, or (at your option) any later version.
##
##  The Computational Morphometry Toolkit is distributed in the hope that it
##  will be useful, but WITHOUT ANY WARRANTY; without even the implied
##  warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU General Public License for more details.
##
##  You should have received a copy of the GNU General Public License along
##  with the Computational Morphometry Toolkit.  If not, see
##  <http://www.gnu.org/licenses/>.
##
##  $Revision$
##
##  $LastChangedDate$
##
##  $LastChangedBy$
##

##
## Helper functions
##

needs_update()
{
    local target=$1
    if test ! -e ${target}; then
	if test -e ${target}.gz; then
	    target=${target}.gz
	fi
    fi

    shift
    local sources="$*"
    for source in ${sources}; do
	if [ ! -e ${source} ]; then
	    if [ ! -e ${source}.gz ]; then
		echo "========================================================================"
		echo "MISSING SOURCE ${source}"
		echo "========================================================================"
		
		return 1
	    fi
	fi
	
	if [ -e ${source}.lock ]; then
	    echo "========================================================================"
	    echo "SOURCE LOCKED ${source}"
	    echo "========================================================================"
	    
	    return 1
	fi
    done

    local source

    if [ ! -e ${target} ]; then
	echo "========================================================================"
	echo "CREATE ${target}"
        echo "========================================================================"

	return 0
    fi

    for source in ${sources}; do
	if test ! -e ${source}; then
	    if test -e ${source}.gz; then
		source=${source}.gz
	    fi
	fi
	
	if test ${target} -ot ${source}; then
            echo "========================================================================"
	    echo "UPDATE ${target}"
            echo "DUE TO ${source}"
            echo "========================================================================"

	    return 0
	fi
    done

    return 1
}

# NFS-safe (hopefully) file locking.
lockfile_create()
{
    local lockfile=$1.lock
    local hostpid=`hostname`-$$

    if [ -e ${lockfile} ]; then
	# lockfile already exists, so clearly we were not the first
	return 1;
    fi

    mkdir -p `dirname ${lockfile}`
    echo ${hostpid} >> ${lockfile}

    if [ `head -n 1 ${lockfile}` != ${hostpid} ]; then
	# first one to write PID was not us
	return 1;
    fi

    trap "rm -f ${lockfile}; exit" INT TERM EXIT
    return 0;
}

lockfile_delete()
{
    local file=${1}
    local lockfile=${file}.lock

    if [ -f ${file} ]; then
	touch --no-create -r ${lockfile} ${file}
    else
	touch --no-create -r ${lockfile} ${file}.gz
    fi

    rm -f ${lockfile}
    trap - INT TERM EXIT
}

#
# Combine dependency checking with locking of target
#
needs_update_and_lock()
{
    if needs_update $*; then
	if lockfile_create $1; then
	    return 0
	fi
    fi
    return 1
}

#
# Given a .hdr file name, compress corresponding .img file.
#
gzip_hdr_img()
{
    for i in $*; do
	local img=`echo ${i} | sed 's/\.hdr/\.img/g'`
	gzip -9f ${img}
    done
}

#
# Echo a command line, then execute it.
#
echo_and_exec()
{
    echo $*
    $*
}

#
# Find file in list of paths
#
find_file()
{
    local file=$1
    shift

    for d in $*; do
	test -e ${d}/${file} && echo ${d}/${file} && return
    done

    echo "COULD NOT FIND: $file" > /dev/stderr
}

#!/bin/bash

if [ -z $1 ];
then
	echo "no tag"
	exit 1
fi

(cd /home/galqiwi/current_tasks/aicup_binaries && ./build_tag.sh $1)

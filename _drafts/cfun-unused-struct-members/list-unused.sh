#!/bin/sh
# find unused structure members
# run inside your source directory, is not recursive

# enumerate
spatch -quiet -sp_file enum-members.cocci *.[ch] | sed -ne 's/STRUCT: //p' | \
while read stru memb; do
	echo "Struct: $stru::$memb"
	# search structure/member pairs
	if ! coccigrep -t "struct $stru" -a "$memb" --vim *.[ch] > /dev/null; then
		echo "NOT USED: $stru::$memb"
	fi
done

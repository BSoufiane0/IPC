#!/bin/zsh

echo "at: "$(pwd)
find ./build -maxdepth 1 -type f -name 'osmpExecutable*' | while read file ;do
	echo "test: "${file}"\n"
	
	if ./build/osmp_run 20 -L /home/kali/log.txt -v 1 ${file};then
		echo "test succeded ...\n"
        else
		echo "test Failed ...\n"	       
	fi
	echo "=============================\n"
done

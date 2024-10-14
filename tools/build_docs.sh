#!/bin/bash

if ! type doxygen >/dev/null 2>&1; then
	echo "Building the documentation requires doxygen" >&2
	exit 1
fi

doxygen Doxyfile

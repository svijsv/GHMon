#!/usr/bin/python3
import subprocess
from datetime import datetime

try:
	version = subprocess.check_output(["git", "describe", "--tags", "--always"]).strip()
except:
	version = "0000000"

# Using datetime.today().ctime() will cause everything to be rebuilt every
# time because compiler flags change
print('-DPROGVERS="\\\"{}\\\"" -DBUILD_DATE="\\\"{}\\\""'.format(version, datetime.today().strftime("%a %b %d %Y")))

#!/usr/bin/python3
import subprocess
from datetime import datetime

try:
	version = subprocess.check_output(["git", "describe", "--tags", "--always"]).strip()
except:
	version = "0000000"

print('-DPROGNAME="\\\"{}\\\"" -DPROGVERS="\\\"{}\\\"" -DBUILD_DATE="\\\"{}\\\""'.format("GHMon", version, datetime.today().ctime()))

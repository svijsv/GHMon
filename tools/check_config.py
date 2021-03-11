#!/usr/bin/python3
#
#Check configuration files, copy in the defaults if none exist
#
import shutil

CFG_FILES = [ "config.h", "advanced.h", "sensors.c", "controllers.c", "tables.c" ]
CFG_DIR = "./config/"
TEMPLATE_DIR = "./config/templates/"

for fname in CFG_FILES:
	src_path = TEMPLATE_DIR + fname
	dst_path = CFG_DIR + fname

	try:
		f = open(dst_path, "r")
	except:
		print("Copying {} to {}".format(src_path, dst_path))
		shutil.copyfile(src_path, dst_path)
	else:
		#print("Configuration file {} exists".format(dst_path))
		f.close();

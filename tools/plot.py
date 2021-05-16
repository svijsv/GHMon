#!/usr/bin/python3
#
# Generate a graph based on log file data
# This doesn't presently allow graphing uptime or warnings fields
#
import sys
import argparse
import re
from matplotlib import pyplot, dates

TIME_FIELD = 0
HIGH_POINT_VAL = "H"
LOW_POINT_VAL  = "L"

class Field:
	name = ""
	num = 0
	min_y = LOW_POINT_VAL
	max_y = HIGH_POINT_VAL
	low = 0
	high = 0
	subplot = 0
	ypoints = []


def logtime_to_hours(log_time):
	if re.match(r"[0-9]+:[0-9][0-9]", log_time):
		t = log_time.split(":")
		return (float(t[0]) + (float(t[1]) / 60.0))
	elif re.match(r"[0-9]{4,4}\.[0-9]{2,2}\.[0-9]{2,2} [0-9]{2,2}:[0-9]{2,2}", log_time):
		# I'm pretty sure this doesn't work how I think it does but I don't
		# care enough to fix it right now; it's unlikely anyone will be using
		# dates anyway
		return (dates.datestr2num(log_time) * 24.0)
	else:
		return 0

parser = argparse.ArgumentParser(description="Generate a graph based on log file data")
parser.add_argument("--out-file",   "-o", help="Output to file (default is to display)", default="-", metavar="[outfile]");
parser.add_argument("--start-time", "-s", help="Start time (default is beginning of log)", default="-");
parser.add_argument("--end-time",   "-e", help="End time (default is end of log)", default="-");
parser.add_argument("--field",      "-F", help="Add field to graph ('name[...,nameN][,min_y[,max_y]]')", action="append", required=True);
parser.add_argument("in_files",           help="Input files", action="append");
param = parser.parse_args();

# Subplot indexes start at 1
subplot = 1
fields = []
for F in param.field:
	parts = F.split(",")

	min_y = LOW_POINT_VAL
	max_y = HIGH_POINT_VAL
	temp = []
	for p in parts:
		if not p.isdigit():
			f = Field()
			f.subplot = subplot
			f.ypoints = []
			f.name = p
			temp.append(f)
		elif min_y == LOW_POINT_VAL:
			min_y = int(p)
		else:
			max_y = int(p)
	for f in temp:
		f.min_y = min_y
		f.max_y = max_y

	for f in temp:
		fields.append(f)
	subplot += 1

if param.start_time != "-":
	start = logtime_to_hours(param.start_time)
else:
	start = 0
if param.end_time != "-":
	stop  = logtime_to_hours(param.end_time)
else:
	stop = 0

xpoints = []
use_line = False
first = 0
last = 0
for ifile_name in param.in_files:
	ifile = open(ifile_name)
	for line in ifile:
		line = line.strip()

		if re.match(r"# uptime", line):
			names = re.sub(r"\[!]", "", line)
			names = names.lstrip("# ").split("\t")
			for f in fields:
				f.num = 0
			i = 0
			for name in names:
				for f in fields:
					if f.name == name:
						f.num = i
				i += 1
			for f in fields:
				# Using 0 as the marker means we can't use uptime, but that
				# would require special handling later anyway
				if f.num == 0:
					exit("Couldn't find field '{}'".format(f.name))

		if line[0] == "#":
			continue

		values = line.split("\t")
		logtime = logtime_to_hours(values[TIME_FIELD])
		if (start == 0 or logtime >= start) and (stop == 0 or logtime <= stop):
			xpoints.append(logtime)
			for f in fields:
				value = values[f.num].replace("!", "")
				# min_y and max_y aren't known yet unless they were set on
				# the command line, but setting high/low to anything at all
				# will change their auto-detected values anyway
				if value == "(HIGH)":
					value = HIGH_POINT_VAL
				elif value == "(LOW)":
					value = LOW_POINT_VAL
				else:
					value = int(value)
					if value > f.high:
						f.high = value
					elif value < f.low:
						f.low = value
				f.ypoints.append(value)

i = 0
colors = [ 'b', 'g', 'r', 'c', 'm', 'y', 'k', 'w', ]
first_x = min(xpoints)
last_x  = max(xpoints)
xpoints = [x - first_x for x in xpoints]
#titles = []
legends = []
for f in fields:
	# When using the same subplot for more than one field this line will
	# issue a spurious deprecation warning; see:
	# https://stackoverflow.com/questions/46933824/matplotlib-adding-an-axes-using-the-same-arguments-as-a-previous-axes
	# https://github.com/matplotlib/matplotlib/issues/12513
	pyplot.subplot(len(fields), 1, f.subplot)
	# This assumes the subplots are processed in ascending order
	if len(legends) < f.subplot:
		#titles.append(f.name)
		legends.append([f.name])
	else:
		#titles[f.subplot-1] += ", "+f.name
		#titles[f.subplot-1] += "\n"+f.name
		legends[f.subplot-1].append(f.name)
	if param.start_time != "-":
		pyplot.xlabel("Hours ({}+x)".format(param.start_time))
	else:
		pyplot.xlabel("Hours")
	pyplot.grid(True)

	if f.max_y == HIGH_POINT_VAL:
		f.max_y = f.high
	if f.min_y == LOW_POINT_VAL:
		f.min_y = f.low
	for iy in range(len(f.ypoints)):
		if f.ypoints[iy] == HIGH_POINT_VAL:
			f.ypoints[iy] = f.max_y
		elif f.ypoints[iy] == LOW_POINT_VAL:
			f.ypoints[iy] = f.min_y

	pyplot.axis([0, last_x-first_x, f.min_y, f.max_y])
	pyplot.plot(xpoints, f.ypoints, "-"+colors[i])
	i += 1
	# Ignore color 'w', white-on-white won't show
	if i > 6:
		i = 0

i = 0
for l in legends:
	pyplot.subplot(len(fields), 1, i+1)
#	pyplot.title(titles[i], loc="left")
#	if len(l) > 1:
	pyplot.legend(l)
	i += 1

if param.out_file == "-":
	pyplot.show(block=True)
else:
	pyplot.savefig(param.out_file)


pyplot.close()

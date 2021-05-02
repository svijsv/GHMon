#!/usr/bin/python3
#
# Generate a thermistor voltage look-up table for GHMon
#
# Voltage values generated will only be valid if the ratio of the series
# resistor to the reference resistance in the actual circuit is the same
# as used for building the table.
#
import sys
import argparse
import math

#
# Global variables
#
C_TO_K = 273.15


#
# Default Settings
#
#Table parameters
MIN_T = -5.0
MAX_T = 50.0
STEPS = 10
#
#Thermistor parameters
T0   = 25.0
R0   = 20000.0
#BETA = ln(R0/R1)/((1/T0)-(1/T1))
BETA = 3950.0
#
#System parameters
SYSVCC   = 3300
SERIES_R = 20000.0
#
#Output parameters
COLUMNS = 4
SCALE   = 100


#
# Helper functions
#
def calc_RfromT(T):
	#T = 1/(1/T0 + 1/B * log(R/R0))
	#1/T = 1/T0 + 1/B * log(R/R0)
	#log(R/R0)/B = 1/T - 1/T0
	#log(R) - log(R0) = (1/T - 1/T0) * B
	#log(R) = (1/T - 1/T0) * B + log(R0)
	#R = exp((1/T - 1/T0) * B + log(R0))
	#R = exp((1/T - 1/T0) * B) * R0
	#return math.exp(((1.0/T) - (1.0/param.Tref)) * param.beta) * param.Rref
	return math.exp(((param.beta/T) - (param.beta/param.Tref))) * param.Rref

def calc_VfromR(R):
	#Vo = (R2 * Vs)/(R1 + R2)
	return (R * param.sysVcc) / (param.seriesR + R)

def calc_RfromV(V):
	#R2 = (Vo*R1)/(Vs-Vo)
	return (V * param.seriesR) / (param.sysVcc - V)
#
# Program body
#
parser = argparse.ArgumentParser(description="Generate a voltage-to-temperature look-up table for a thermistor")
parser.add_argument("--outfile", "-o", help="Output to file (default is stdout)", default="-", metavar="[filename]");
parser.add_argument("--columns", "-c", help="Number of table columns (default is 4)", default=COLUMNS, type=int);
parser.add_argument("--fahrenheit", "-F", help="Use Fahrenheit in table (default is Celsius)", action="store_true");
parser.add_argument("--Tref", help="Reference temperature in Celsius (default is 25C)", default=T0, type=float);
parser.add_argument("--Rref", help="Reference resistance in ohms (default is 20K)", default=R0, type=float);
parser.add_argument("--beta", help="The beta coefficient of the thermistor (default is 3950)", default=BETA, type=float);
parser.add_argument("--sysVcc", help="The system voltage in mV (default 3300)", default=SYSVCC, type=int);
parser.add_argument("--seriesR", help="The value in ohms of the resistor to be used in series with the thermistor (default is 20K)", default=SERIES_R, type=float);
parser.add_argument("--min", help="Minimum temperature in table (default is -5C)", default=MIN_T, type=float);
parser.add_argument("--max", help="Maximum temperature in table (default is 50C)", default=MAX_T, type=float);
parser.add_argument("--steps", help="Number of elements in table (default is 10)", default=STEPS, type=int);
parser.add_argument("--scale", help="Scale table entries by this amount (default is 100)", default=SCALE, type=int);
param = parser.parse_args();
param.Tref += C_TO_K
param.min += C_TO_K
param.max += C_TO_K

tab_min = calc_VfromR(calc_RfromT(param.min));
tab_max = calc_VfromR(calc_RfromT(param.max));
if tab_min > tab_max:
	tmp = tab_min
	tab_min = tab_max
	tab_max = tmp
#GHMon determines step size by integer division; rounding it here will result
#in a better reproduction at the cost of a range possibly slightly off from
#that requested.
step_size = round((tab_max - tab_min)/(param.steps-1));

tab_T = []
tab_V = []
#tab_R = []
for i in range(0, param.steps):
	V = tab_min + (step_size * i)
	R = calc_RfromV(V)

	#T = 1/(1/T0 + 1/B * log(R/R0))
	T = 1.0 / ((1.0 / param.Tref) + (math.log(R / param.Rref) / param.beta));
	T -= C_TO_K
	if param.fahrenheit:
		T = (T * 1.8) + 32.0

	tab_T.append(T)
	tab_V.append(V)
	#tab_R.append(R)
tab_min = tab_V[0];
tab_max = tab_V[param.steps-1];

if param.outfile != "-":
	outfile = open(param.outfile, "a")
else:
	outfile = sys.stdout

outfile.write("{ .min = %d, .max = %d, .Vref = %d, .table_multiplier = %d, .cflags = SENS_FLAG_VOLTS, .table = {\n" % (int(round(tab_min)), int(round(tab_max)), param.sysVcc, param.scale))
outfile.write("   //min: %.2fmV max: %.2fmV step: %.2fmV scale: %.2f\n" % (tab_min, tab_max, step_size, param.scale))
outfile.write("   //Tref: %.2fC Rref: %.2fR beta: %.2f\n" % (param.Tref, param.Rref, param.beta));
outfile.write("   //Vcc: %umV Rseries: %.02fR" % (param.sysVcc, param.seriesR))
for i in range(0, param.steps):
	if (i % param.columns) == 0:
		outfile.write("\n ")
	outfile.write(" % 6.0d, /*% 4.2fmV */" % (int(tab_T[i]*param.scale), tab_V[i]))

outfile.write("\n }\n},\n")

if outfile != sys.stdout:
	outfile.close()

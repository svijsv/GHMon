EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Device:Battery BT1
U 1 1 60445EF8
P 4550 5350
F 0 "BT1" V 4795 5350 50  0000 C CNN
F 1 "Battery" V 4704 5350 50  0000 C CNN
F 2 "" V 4550 5410 50  0001 C CNN
F 3 "~" V 4550 5410 50  0001 C CNN
	1    4550 5350
	0    -1   -1   0   
$EndComp
$Comp
L power:GND #PWR012
U 1 1 6044996B
P 4750 5350
F 0 "#PWR012" H 4750 5100 50  0001 C CNN
F 1 "GND" V 4755 5222 50  0000 R CNN
F 2 "" H 4750 5350 50  0001 C CNN
F 3 "" H 4750 5350 50  0001 C CNN
	1    4750 5350
	0    -1   -1   0   
$EndComp
$Comp
L power:GND #PWR013
U 1 1 60449E7F
P 4850 4600
F 0 "#PWR013" H 4850 4350 50  0001 C CNN
F 1 "GND" H 4855 4427 50  0000 C CNN
F 2 "" H 4850 4600 50  0001 C CNN
F 3 "" H 4850 4600 50  0001 C CNN
	1    4850 4600
	0    1    1    0   
$EndComp
Text GLabel 6050 3200 2    50   Input ~ 0
SENS_PWR
Text GLabel 4350 5350 0    50   Input ~ 0
BAT+
$Comp
L Device:R R8
U 1 1 6044AC04
P 8200 5150
F 0 "R8" H 8400 5100 50  0000 R CNN
F 1 "100K" H 8400 5200 50  0000 R CNN
F 2 "" V 8130 5150 50  0001 C CNN
F 3 "~" H 8200 5150 50  0001 C CNN
	1    8200 5150
	-1   0    0    1   
$EndComp
$Comp
L Device:R R12
U 1 1 6044BFB0
P 8350 5000
F 0 "R12" V 8143 5000 50  0000 C CNN
F 1 "100K" V 8234 5000 50  0000 C CNN
F 2 "" V 8280 5000 50  0001 C CNN
F 3 "~" H 8350 5000 50  0001 C CNN
	1    8350 5000
	0    1    1    0   
$EndComp
Wire Wire Line
	8200 5000 8050 5000
Connection ~ 8200 5000
$Comp
L power:GND #PWR017
U 1 1 6044C5E3
P 8200 5300
F 0 "#PWR017" H 8200 5050 50  0001 C CNN
F 1 "GND" H 8205 5127 50  0000 C CNN
F 2 "" H 8200 5300 50  0001 C CNN
F 3 "" H 8200 5300 50  0001 C CNN
	1    8200 5300
	1    0    0    -1  
$EndComp
Text GLabel 8500 5000 2    50   Input ~ 0
BAT+
Text GLabel 6050 4000 2    50   Input ~ 0
SENS_WATER
Text GLabel 6050 4100 2    50   Input ~ 0
SENS_TEMP_IN
Text GLabel 6050 4200 2    50   Input ~ 0
SENS_TEMP_OUT
Text GLabel 6050 4300 2    50   Input ~ 0
SENS_BAT
Text GLabel 4850 3300 0    50   Input ~ 0
SERIAL_TX
Text GLabel 4850 3400 0    50   Input ~ 0
SERIAL_RX
Text Notes 3700 3400 0    50   ~ 0
TX to adapter RX\nRX to adapter TX
Text GLabel 4850 2800 0    50   Input ~ 0
SPI_CS_SD
Text GLabel 4850 2900 0    50   Input ~ 0
SPI_SCK
Text GLabel 4850 3000 0    50   Input ~ 0
SPI_MISO
Text GLabel 4850 3100 0    50   Input ~ 0
SPI_MOSI
Text GLabel 4850 3800 0    50   Input ~ 0
LED_CTRL
Text GLabel 4850 3900 0    50   Input ~ 0
BUTTON
Text GLabel 4850 4400 0    50   Input ~ 0
FAN_CTRL
Text GLabel 4850 4300 0    50   Input ~ 0
WATER_CTRL
Text GLabel 2550 1450 2    50   Input ~ 0
SPI_CS_SD
Text GLabel 2550 1850 2    50   Input ~ 0
SPI_SCK
Text GLabel 2550 2050 2    50   Input ~ 0
SPI_MISO
Text GLabel 2550 1550 2    50   Input ~ 0
SPI_MOSI
Text GLabel 4850 4700 0    50   Input ~ 0
3v3
$Comp
L power:GND #PWR08
U 1 1 604547F8
P 2550 1650
F 0 "#PWR08" H 2550 1400 50  0001 C CNN
F 1 "GND" V 2555 1522 50  0000 R CNN
F 2 "" H 2550 1650 50  0001 C CNN
F 3 "" H 2550 1650 50  0001 C CNN
	1    2550 1650
	0    -1   1    0   
$EndComp
$Comp
L Device:R R5
U 1 1 60454DD8
P 2700 2150
F 0 "R5" V 2493 2150 50  0000 C CNN
F 1 "47K" V 2584 2150 50  0000 C CNN
F 2 "" V 2630 2150 50  0001 C CNN
F 3 "~" H 2700 2150 50  0001 C CNN
	1    2700 2150
	0    1    -1   0   
$EndComp
$Comp
L Device:R R6
U 1 1 6045540A
P 2700 1350
F 0 "R6" V 2815 1350 50  0000 C CNN
F 1 "47K" V 2906 1350 50  0000 C CNN
F 2 "" V 2630 1350 50  0001 C CNN
F 3 "~" H 2700 1350 50  0001 C CNN
	1    2700 1350
	0    1    -1   0   
$EndComp
$Comp
L power:GND #PWR07
U 1 1 6045B404
P 2550 1950
F 0 "#PWR07" H 2550 1700 50  0001 C CNN
F 1 "GND" V 2555 1822 50  0000 R CNN
F 2 "" H 2550 1950 50  0001 C CNN
F 3 "" H 2550 1950 50  0001 C CNN
	1    2550 1950
	0    -1   1    0   
$EndComp
Text Notes 900  2800 0    50   ~ 0
Converting a microSD adapter\nto a card reader is easy, but\nrequires either another\nvoltage regulator or a card\nthat sleeps well and doesn't\nuse a lot of power
$Comp
L Device:LED D1
U 1 1 6045F698
P 2200 3100
F 0 "D1" H 2193 3317 50  0000 C CNN
F 1 "LED" H 2193 3226 50  0000 C CNN
F 2 "" H 2200 3100 50  0001 C CNN
F 3 "~" H 2200 3100 50  0001 C CNN
	1    2200 3100
	1    0    0    -1  
$EndComp
Text GLabel 2350 3100 2    50   Input ~ 0
LED_CTRL
$Comp
L power:GND #PWR03
U 1 1 60460246
P 1750 3100
F 0 "#PWR03" H 1750 2850 50  0001 C CNN
F 1 "GND" V 1755 2972 50  0000 R CNN
F 2 "" H 1750 3100 50  0001 C CNN
F 3 "" H 1750 3100 50  0001 C CNN
	1    1750 3100
	0    1    1    0   
$EndComp
$Comp
L Device:R R1
U 1 1 60460AE0
P 1900 3100
F 0 "R1" V 1693 3100 50  0000 C CNN
F 1 "1K" V 1784 3100 50  0000 C CNN
F 2 "" V 1830 3100 50  0001 C CNN
F 3 "~" H 1900 3100 50  0001 C CNN
	1    1900 3100
	0    1    1    0   
$EndComp
$Comp
L Switch:SW_Push SW1
U 1 1 604623D4
P 2250 3600
F 0 "SW1" H 2250 3885 50  0000 C CNN
F 1 "SW_Push" H 2250 3794 50  0000 C CNN
F 2 "" H 2250 3800 50  0001 C CNN
F 3 "~" H 2250 3800 50  0001 C CNN
	1    2250 3600
	1    0    0    -1  
$EndComp
Text GLabel 2450 3600 2    50   Input ~ 0
BUTTON
Text GLabel 1750 3600 0    50   Input ~ 0
3v3
$Comp
L Device:R R2
U 1 1 604630E5
P 1900 3600
F 0 "R2" V 1693 3600 50  0000 C CNN
F 1 "1K" V 1784 3600 50  0000 C CNN
F 2 "" V 1830 3600 50  0001 C CNN
F 3 "~" H 1900 3600 50  0001 C CNN
	1    1900 3600
	0    1    1    0   
$EndComp
$Comp
L Relay:G5Q-1A K1
U 1 1 60465062
P 1800 4800
F 0 "K1" H 2130 4846 50  0000 L CNN
F 1 "G5Q-1A" H 2130 4755 50  0000 L CNN
F 2 "Relay_THT:Relay_SPST_Omron-G5Q-1A" H 2150 4750 50  0001 L CNN
F 3 "https://www.omron.com/ecb/products/pdf/en-g5q.pdf" H 1800 4800 50  0001 C CNN
	1    1800 4800
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR01
U 1 1 6046613E
P 1600 4500
F 0 "#PWR01" H 1600 4250 50  0001 C CNN
F 1 "GND" H 1605 4327 50  0000 C CNN
F 2 "" H 1600 4500 50  0001 C CNN
F 3 "" H 1600 4500 50  0001 C CNN
	1    1600 4500
	-1   0    0    1   
$EndComp
Text GLabel 1600 5100 3    50   Input ~ 0
WaterPump
$Comp
L Device:Q_NPN_CBE Q1
U 1 1 60468049
P 2100 4300
F 0 "Q1" H 2291 4254 50  0000 L CNN
F 1 "Q_NPN_CBE" H 1650 4450 50  0000 L CNN
F 2 "" H 2300 4400 50  0001 C CNN
F 3 "~" H 2100 4300 50  0001 C CNN
	1    2100 4300
	-1   0    0    1   
$EndComp
Text GLabel 2850 4300 2    50   Input ~ 0
WATER_CTRL
$Comp
L power:GND #PWR04
U 1 1 6046ABCD
P 2000 4000
F 0 "#PWR04" H 2000 3750 50  0001 C CNN
F 1 "GND" H 2005 3827 50  0000 C CNN
F 2 "" H 2000 4000 50  0001 C CNN
F 3 "" H 2000 4000 50  0001 C CNN
	1    2000 4000
	-1   0    0    1   
$EndComp
Text GLabel 2000 5100 3    50   Input ~ 0
BAT+
$Comp
L Relay:G5Q-1A K2
U 1 1 6046EE8F
P 1800 6750
F 0 "K2" H 2130 6796 50  0000 L CNN
F 1 "G5Q-1A" H 2130 6705 50  0000 L CNN
F 2 "Relay_THT:Relay_SPST_Omron-G5Q-1A" H 2150 6700 50  0001 L CNN
F 3 "https://www.omron.com/ecb/products/pdf/en-g5q.pdf" H 1800 6750 50  0001 C CNN
	1    1800 6750
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR02
U 1 1 6046EE95
P 1600 6450
F 0 "#PWR02" H 1600 6200 50  0001 C CNN
F 1 "GND" H 1605 6277 50  0000 C CNN
F 2 "" H 1600 6450 50  0001 C CNN
F 3 "" H 1600 6450 50  0001 C CNN
	1    1600 6450
	-1   0    0    1   
$EndComp
Text GLabel 1600 7050 3    50   Input ~ 0
Fan
$Comp
L Device:Q_NPN_CBE Q2
U 1 1 6046EE9C
P 2100 6250
F 0 "Q2" H 2291 6204 50  0000 L CNN
F 1 "Q_NPN_CBE" H 1650 6400 50  0000 L CNN
F 2 "" H 2300 6350 50  0001 C CNN
F 3 "~" H 2100 6250 50  0001 C CNN
	1    2100 6250
	-1   0    0    1   
$EndComp
Text GLabel 2900 6250 2    50   Input ~ 0
FAN_CTRL
$Comp
L power:GND #PWR05
U 1 1 6046EEA3
P 2000 5950
F 0 "#PWR05" H 2000 5700 50  0001 C CNN
F 1 "GND" H 2005 5777 50  0000 C CNN
F 2 "" H 2000 5950 50  0001 C CNN
F 3 "" H 2000 5950 50  0001 C CNN
	1    2000 5950
	-1   0    0    1   
$EndComp
Text GLabel 2000 7050 3    50   Input ~ 0
BAT+
$Comp
L Device:R R4
U 1 1 6047CDB4
P 2750 6250
F 0 "R4" V 2543 6250 50  0000 C CNN
F 1 "1K" V 2634 6250 50  0000 C CNN
F 2 "" V 2680 6250 50  0001 C CNN
F 3 "~" H 2750 6250 50  0001 C CNN
	1    2750 6250
	0    1    1    0   
$EndComp
$Comp
L Device:R R3
U 1 1 6047D832
P 2700 4300
F 0 "R3" V 2493 4300 50  0000 C CNN
F 1 "1K" V 2584 4300 50  0000 C CNN
F 2 "" V 2630 4300 50  0001 C CNN
F 3 "~" H 2700 4300 50  0001 C CNN
	1    2700 4300
	0    1    1    0   
$EndComp
Text GLabel 8050 5000 0    50   Input ~ 0
SENS_BAT
Text GLabel 8200 3900 0    50   Input ~ 0
SENS_TEMP_OUT
$Comp
L Device:Thermistor TH2
U 1 1 60481072
P 8200 4100
F 0 "TH2" H 8305 4146 50  0000 L CNN
F 1 "Thermistor" H 8305 4055 50  0000 L CNN
F 2 "" H 8200 4100 50  0001 C CNN
F 3 "~" H 8200 4100 50  0001 C CNN
	1    8200 4100
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR016
U 1 1 60481960
P 8200 4300
F 0 "#PWR016" H 8200 4050 50  0001 C CNN
F 1 "GND" H 8205 4127 50  0000 C CNN
F 2 "" H 8200 4300 50  0001 C CNN
F 3 "" H 8200 4300 50  0001 C CNN
	1    8200 4300
	1    0    0    -1  
$EndComp
Text GLabel 8500 3900 2    50   Input ~ 0
SENS_PWR
$Comp
L Device:R R11
U 1 1 6048236F
P 8350 3900
F 0 "R11" V 8143 3900 50  0000 C CNN
F 1 "[Match to TH2]" V 8234 3900 50  0000 C CNN
F 2 "" V 8280 3900 50  0001 C CNN
F 3 "~" H 8350 3900 50  0001 C CNN
	1    8350 3900
	0    1    1    0   
$EndComp
Text GLabel 8200 2800 0    50   Input ~ 0
SENS_TEMP_IN
$Comp
L Device:Thermistor TH1
U 1 1 60484B2D
P 8200 3000
F 0 "TH1" H 8305 3046 50  0000 L CNN
F 1 "Thermistor" H 8305 2955 50  0000 L CNN
F 2 "" H 8200 3000 50  0001 C CNN
F 3 "~" H 8200 3000 50  0001 C CNN
	1    8200 3000
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR015
U 1 1 60484B33
P 8200 3200
F 0 "#PWR015" H 8200 2950 50  0001 C CNN
F 1 "GND" H 8205 3027 50  0000 C CNN
F 2 "" H 8200 3200 50  0001 C CNN
F 3 "" H 8200 3200 50  0001 C CNN
	1    8200 3200
	1    0    0    -1  
$EndComp
Text GLabel 8500 2800 2    50   Input ~ 0
SENS_PWR
$Comp
L Device:R R10
U 1 1 60484B3A
P 8350 2800
F 0 "R10" V 8143 2800 50  0000 C CNN
F 1 "[Match to TH1]" V 8234 2800 50  0000 C CNN
F 2 "" V 8280 2800 50  0001 C CNN
F 3 "~" H 8350 2800 50  0001 C CNN
	1    8350 2800
	0    1    1    0   
$EndComp
$Comp
L Device:R_Variable HM1
U 1 1 60487425
P 8200 2000
F 0 "HM1" H 8328 2046 50  0000 L CNN
F 1 "Soil Hygrometer" H 8328 1955 50  0000 L CNN
F 2 "" V 8130 2000 50  0001 C CNN
F 3 "~" H 8200 2000 50  0001 C CNN
	1    8200 2000
	1    0    0    -1  
$EndComp
Text GLabel 8200 1850 0    50   Input ~ 0
SENS_WATER
$Comp
L power:GND #PWR014
U 1 1 60488711
P 8200 2150
F 0 "#PWR014" H 8200 1900 50  0001 C CNN
F 1 "GND" H 8205 1977 50  0000 C CNN
F 2 "" H 8200 2150 50  0001 C CNN
F 3 "" H 8200 2150 50  0001 C CNN
	1    8200 2150
	1    0    0    -1  
$EndComp
Text GLabel 8500 1850 2    50   Input ~ 0
SENS_PWR
$Comp
L Device:R R9
U 1 1 60488718
P 8350 1850
F 0 "R9" V 8143 1850 50  0000 C CNN
F 1 "[Match to HM1]" V 8234 1850 50  0000 C CNN
F 2 "" V 8280 1850 50  0001 C CNN
F 3 "~" H 8350 1850 50  0001 C CNN
	1    8350 1850
	0    1    1    0   
$EndComp
$Comp
L Regulator_Linear:AMS1117-3.3 U1
U 1 1 6048E684
P 3500 1250
F 0 "U1" H 3500 1399 50  0000 C CNN
F 1 "AMS1117-3.3" H 3500 1490 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-223-3_TabPin2" H 3500 1450 50  0001 C CNN
F 3 "http://www.advanced-monolithic.com/pdf/ds1117.pdf" H 3600 1000 50  0001 C CNN
	1    3500 1250
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR010
U 1 1 6048F42E
P 3500 950
F 0 "#PWR010" H 3500 700 50  0001 C CNN
F 1 "GND" H 3505 777 50  0000 C CNN
F 2 "" H 3500 950 50  0001 C CNN
F 3 "" H 3500 950 50  0001 C CNN
	1    3500 950 
	-1   0    0    1   
$EndComp
Text GLabel 4450 1250 2    50   Input ~ 0
BAT+
Wire Wire Line
	2550 1750 3200 1750
Wire Wire Line
	3200 1750 3200 1350
$Comp
L Device:Q_NPN_CBE Q3
U 1 1 60491319
P 3800 2200
F 0 "Q3" V 4128 2200 50  0000 C CNN
F 1 "Q_NPN_CBE" V 4037 2200 50  0000 C CNN
F 2 "" H 4000 2300 50  0001 C CNN
F 3 "~" H 3800 2200 50  0001 C CNN
	1    3800 2200
	0    1    -1   0   
$EndComp
$Comp
L Device:Q_PNP_EBC Q4
U 1 1 60495F58
P 4000 1350
F 0 "Q4" V 4328 1350 50  0000 C CNN
F 1 "Q_PNP_EBC" V 4237 1350 50  0000 C CNN
F 2 "" H 4200 1450 50  0001 C CNN
F 3 "~" H 4000 1350 50  0001 C CNN
	1    4000 1350
	0    -1   -1   0   
$EndComp
$Comp
L Device:R R7
U 1 1 60496B85
P 4300 1400
F 0 "R7" H 4230 1354 50  0000 R CNN
F 1 "100K" H 4230 1445 50  0000 R CNN
F 2 "" V 4230 1400 50  0001 C CNN
F 3 "~" H 4300 1400 50  0001 C CNN
	1    4300 1400
	-1   0    0    1   
$EndComp
Wire Wire Line
	4200 1250 4300 1250
Connection ~ 4300 1250
Wire Wire Line
	4300 1250 4450 1250
Wire Wire Line
	4000 1550 4300 1550
Wire Wire Line
	4000 1850 4000 2100
$Comp
L power:GND #PWR011
U 1 1 604998B2
P 3500 2350
F 0 "#PWR011" H 3500 2100 50  0001 C CNN
F 1 "GND" V 3505 2222 50  0000 R CNN
F 2 "" H 3500 2350 50  0001 C CNN
F 3 "" H 3500 2350 50  0001 C CNN
	1    3500 2350
	0    1    1    0   
$EndComp
Text GLabel 4850 4000 0    50   Input ~ 0
SD_PWR
Text GLabel 3800 2850 3    50   Input ~ 0
SD_PWR
$Comp
L Device:C C1
U 1 1 6049C1D6
P 8200 4850
F 0 "C1" H 8086 4896 50  0000 R CNN
F 1 "10nF" H 8086 4805 50  0000 R CNN
F 2 "" H 8238 4700 50  0001 C CNN
F 3 "~" H 8200 4850 50  0001 C CNN
	1    8200 4850
	1    0    0    -1  
$EndComp
Text Notes 4700 5650 0    50   ~ 0
BT1 should be 4-6V\nand can be any DC\npower source
$Comp
L STM32:bluepill-STM32F103C8 U2
U 1 1 6044A237
P 5550 3700
F 0 "U2" H 5450 4865 50  0000 C CNN
F 1 "bluepill-STM32F103C8" H 5450 4774 50  0000 C CNN
F 2 "" H 5550 3700 50  0001 C CNN
F 3 "" H 5550 3700 50  0001 C CNN
	1    5550 3700
	1    0    0    -1  
$EndComp
Wire Wire Line
	4350 5350 4350 4500
Wire Wire Line
	4350 4500 4850 4500
Text Notes 4250 1000 0    50   ~ 0
Q4 should expect\nto see peaks above\n200mA
$Comp
L Device:R R13
U 1 1 60454112
P 3800 2700
F 0 "R13" H 3870 2746 50  0000 L CNN
F 1 "1K" H 3870 2655 50  0000 L CNN
F 2 "" V 3730 2700 50  0001 C CNN
F 3 "~" H 3800 2700 50  0001 C CNN
	1    3800 2700
	1    0    0    -1  
$EndComp
$Comp
L Device:R R14
U 1 1 60460934
P 4000 1700
F 0 "R14" H 4070 1746 50  0000 L CNN
F 1 "1K" H 4070 1655 50  0000 L CNN
F 2 "" V 3930 1700 50  0001 C CNN
F 3 "~" H 4000 1700 50  0001 C CNN
	1    4000 1700
	1    0    0    -1  
$EndComp
Connection ~ 4000 1550
Text Notes 2400 5300 0    50   ~ 0
Q1 and Q2 should\nbe sized to the\ncurrent needs of\ntheir respective\nrelays
Wire Wire Line
	2850 2150 3200 2150
Connection ~ 3200 1350
Wire Wire Line
	3200 1350 3200 1250
Wire Wire Line
	2850 1350 3200 1350
Wire Wire Line
	3200 2150 3200 1750
Connection ~ 3200 1750
Text Notes 4650 1600 0    50   ~ 0
Connect to 3v3 if\nnot using a separate\nregulator
$Comp
L Connector:SD_Card J1
U 1 1 604581A2
P 1650 1750
F 0 "J1" H 1650 1085 50  0000 C CNN
F 1 "SD_Card" H 1650 1176 50  0000 C CNN
F 2 "" H 1650 1750 50  0001 C CNN
F 3 "http://portal.fciconnect.com/Comergent//fci/drawing/10067847.pdf" H 1650 1750 50  0001 C CNN
	1    1650 1750
	-1   0    0    -1  
$EndComp
$Comp
L Device:C C2
U 1 1 60482F1F
P 3350 1750
F 0 "C2" V 3510 1750 50  0000 C CNN
F 1 "47nF" V 3601 1750 50  0000 C CNN
F 2 "" H 3388 1600 50  0001 C CNN
F 3 "~" H 3350 1750 50  0001 C CNN
	1    3350 1750
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR06
U 1 1 60484B98
P 3500 1750
F 0 "#PWR06" H 3500 1500 50  0001 C CNN
F 1 "GND" V 3505 1622 50  0000 R CNN
F 2 "" H 3500 1750 50  0001 C CNN
F 3 "" H 3500 1750 50  0001 C CNN
	1    3500 1750
	0    -1   1    0   
$EndComp
Text Notes 9000 3300 0    50   ~ 0
The series resistors for\nsensors should be roughly\nin the middle of the range\nto be measured
Text Notes 900  1150 0    50   ~ 0
Care should be taken to feed the SD card\nmodule the voltage it expects both on Vcc\nand at the IO pins - some (like this jury-\nrigged adapter) only work with 3.3V MCUs,\nothers have integrated level shifters and\nonly work with 5V MCUs
$Comp
L Device:R R17
U 1 1 60906F86
P 3650 2550
F 0 "R17" V 3443 2550 50  0000 C CNN
F 1 "10K" V 3534 2550 50  0000 C CNN
F 2 "" V 3580 2550 50  0001 C CNN
F 3 "~" H 3650 2550 50  0001 C CNN
	1    3650 2550
	0    1    1    0   
$EndComp
Wire Wire Line
	3800 2550 3800 2400
Connection ~ 3800 2550
Wire Wire Line
	3500 2550 3500 2350
Wire Wire Line
	3500 2350 3500 2100
Wire Wire Line
	3500 2100 3600 2100
Connection ~ 3500 2350
$Comp
L Device:R R16
U 1 1 6090B236
P 2450 6100
F 0 "R16" H 2520 6146 50  0000 L CNN
F 1 "10K" H 2520 6055 50  0000 L CNN
F 2 "" V 2380 6100 50  0001 C CNN
F 3 "~" H 2450 6100 50  0001 C CNN
	1    2450 6100
	1    0    0    -1  
$EndComp
Wire Wire Line
	2450 5950 2000 5950
Wire Wire Line
	2000 5950 2000 6050
Connection ~ 2000 5950
Wire Wire Line
	2300 6250 2450 6250
Wire Wire Line
	2450 6250 2600 6250
Connection ~ 2450 6250
$Comp
L Device:R R15
U 1 1 6090EFF6
P 2400 4150
F 0 "R15" H 2470 4196 50  0000 L CNN
F 1 "10K" H 2470 4105 50  0000 L CNN
F 2 "" V 2330 4150 50  0001 C CNN
F 3 "~" H 2400 4150 50  0001 C CNN
	1    2400 4150
	1    0    0    -1  
$EndComp
Wire Wire Line
	2400 4000 2000 4000
Wire Wire Line
	2000 4100 2000 4000
Connection ~ 2000 4000
Wire Wire Line
	2300 4300 2400 4300
Wire Wire Line
	2400 4300 2550 4300
Connection ~ 2400 4300
Text Notes 2250 5750 0    50   ~ 0
The internal pulldowns can be\nenabled on the controlling pins\nin place of R15 and R16 on\nplatforms that support them
$EndSCHEMATC

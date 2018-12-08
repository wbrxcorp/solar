EESchema Schematic File Version 4
LIBS:esp485-32-breakout-cache
EELAYER 26 0
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
L Interface_UART:MAX3485 U1
U 1 1 5B923B04
P 2650 1900
F 0 "U1" H 2800 1850 60  0000 C CNN
F 1 "MAX3485" H 2650 2350 60  0000 C CNN
F 2 "Package_SO:SOIC-8_3.9x4.9mm_P1.27mm" H 2650 1900 60  0001 C CNN
F 3 "" H 2650 1900 60  0000 C CNN
	1    2650 1900
	1    0    0    -1  
$EndComp
$Comp
L Device:R R7
U 1 1 5B924085
P 2750 5150
F 0 "R7" V 2830 5150 50  0000 C CNN
F 1 "10K" V 2750 5150 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" V 2680 5150 50  0001 C CNN
F 3 "" H 2750 5150 50  0001 C CNN
	1    2750 5150
	0    1    1    0   
$EndComp
Text Notes 7950 7500 0    60   ~ 0
ESP485-32-BREAKOUT
$Comp
L power:+3.3V #PWR01
U 1 1 5B9252B0
P 9900 1400
F 0 "#PWR01" H 9900 1250 50  0001 C CNN
F 1 "+3.3V" H 9900 1540 50  0000 C CNN
F 2 "" H 9900 1400 50  0001 C CNN
F 3 "" H 9900 1400 50  0001 C CNN
	1    9900 1400
	1    0    0    -1  
$EndComp
$Comp
L power:PWR_FLAG #FLG05
U 1 1 5B926539
P 7400 1400
F 0 "#FLG05" H 7400 1475 50  0001 C CNN
F 1 "PWR_FLAG" H 7400 1550 50  0000 C CNN
F 2 "" H 7400 1400 50  0001 C CNN
F 3 "" H 7400 1400 50  0001 C CNN
	1    7400 1400
	1    0    0    -1  
$EndComp
Wire Wire Line
	7400 1400 8200 1400
$Comp
L Connector_Generic:Conn_01x08 J1
U 1 1 5B94B3AE
P 9600 3900
F 0 "J1" H 9680 3892 50  0000 L CNN
F 1 "Conn_01x08" H 9680 3801 50  0000 L CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x08_P2.54mm_Vertical" H 9600 3900 50  0001 C CNN
F 3 "~" H 9600 3900 50  0001 C CNN
	1    9600 3900
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x08 J2
U 1 1 5B94E662
P 9450 5200
F 0 "J2" H 9530 5192 50  0000 L CNN
F 1 "Conn_01x08" H 9530 5101 50  0000 L CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x08_P2.54mm_Vertical" H 9450 5200 50  0001 C CNN
F 3 "~" H 9450 5200 50  0001 C CNN
	1    9450 5200
	1    0    0    -1  
$EndComp
$Comp
L Regulator_Linear:AMS1117-3.3 U3
U 1 1 5BA6808A
P 9250 1400
F 0 "U3" H 9250 1642 50  0000 C CNN
F 1 "AMS1117-3.3" H 9250 1551 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-223-3_TabPin2" H 9250 1600 50  0001 C CNN
F 3 "http://www.advanced-monolithic.com/pdf/ds1117.pdf" H 9350 1150 50  0001 C CNN
	1    9250 1400
	1    0    0    -1  
$EndComp
Wire Wire Line
	8950 1400 8950 1650
$Comp
L power:GND #PWR0101
U 1 1 5BA7723D
P 9250 2250
F 0 "#PWR0101" H 9250 2000 50  0001 C CNN
F 1 "GND" H 9255 2077 50  0000 C CNN
F 2 "" H 9250 2250 50  0001 C CNN
F 3 "" H 9250 2250 50  0001 C CNN
	1    9250 2250
	1    0    0    -1  
$EndComp
Wire Wire Line
	9250 1700 9250 1900
Wire Wire Line
	9550 1400 9550 1600
$Comp
L Device:C C1
U 1 1 5BA86A4D
P 8350 1800
F 0 "C1" H 8465 1846 50  0000 L CNN
F 1 "C" H 8465 1755 50  0000 L CNN
F 2 "Capacitor_SMD:C_1206_3216Metric_Pad1.42x1.75mm_HandSolder" H 8388 1650 50  0001 C CNN
F 3 "~" H 8350 1800 50  0001 C CNN
	1    8350 1800
	1    0    0    -1  
$EndComp
$Comp
L Device:C C2
U 1 1 5BA86B48
P 8650 1800
F 0 "C2" H 8765 1846 50  0000 L CNN
F 1 "C" H 8765 1755 50  0000 L CNN
F 2 "Capacitor_SMD:C_1206_3216Metric_Pad1.42x1.75mm_HandSolder" H 8688 1650 50  0001 C CNN
F 3 "~" H 8650 1800 50  0001 C CNN
	1    8650 1800
	1    0    0    -1  
$EndComp
$Comp
L Device:C C3
U 1 1 5BA86C88
P 9750 1750
F 0 "C3" H 9865 1796 50  0000 L CNN
F 1 "C" H 9865 1705 50  0000 L CNN
F 2 "Capacitor_Tantalum_SMD:CP_EIA-7343-15_Kemet-W_Pad2.25x2.55mm_HandSolder" H 9788 1600 50  0001 C CNN
F 3 "~" H 9750 1750 50  0001 C CNN
	1    9750 1750
	1    0    0    -1  
$EndComp
$Comp
L Device:C C4
U 1 1 5BA86D43
P 10050 1750
F 0 "C4" H 10165 1796 50  0000 L CNN
F 1 "C" H 10165 1705 50  0000 L CNN
F 2 "Capacitor_SMD:C_1206_3216Metric_Pad1.42x1.75mm_HandSolder" H 10088 1600 50  0001 C CNN
F 3 "~" H 10050 1750 50  0001 C CNN
	1    10050 1750
	1    0    0    -1  
$EndComp
Wire Wire Line
	9750 1900 9250 1900
Connection ~ 9250 1900
Wire Wire Line
	9250 1900 9250 1950
Wire Wire Line
	10050 1900 9750 1900
Connection ~ 9750 1900
Wire Wire Line
	9750 1600 9550 1600
Wire Wire Line
	9750 1600 10050 1600
Connection ~ 9750 1600
Wire Wire Line
	8650 1650 8950 1650
Wire Wire Line
	8650 1650 8350 1650
Connection ~ 8650 1650
Wire Wire Line
	8350 1950 8650 1950
Wire Wire Line
	8650 1950 9250 1950
Connection ~ 8650 1950
Connection ~ 9250 1950
Connection ~ 8950 1400
Wire Wire Line
	9900 1400 9550 1400
Connection ~ 9550 1400
$Comp
L Device:C C5
U 1 1 5BA20D2D
P 2500 5400
F 0 "C5" H 2615 5446 50  0000 L CNN
F 1 "1uF" H 2615 5355 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 2538 5250 50  0001 C CNN
F 3 "~" H 2500 5400 50  0001 C CNN
	1    2500 5400
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0102
U 1 1 5BA33C46
P 2650 2750
F 0 "#PWR0102" H 2650 2500 50  0001 C CNN
F 1 "GND" H 2655 2577 50  0000 C CNN
F 2 "" H 2650 2750 50  0001 C CNN
F 3 "" H 2650 2750 50  0001 C CNN
	1    2650 2750
	1    0    0    -1  
$EndComp
Wire Wire Line
	2650 2500 2650 2750
$Comp
L power:VCC #PWR0103
U 1 1 5BA4E6FC
P 8200 1400
F 0 "#PWR0103" H 8200 1250 50  0001 C CNN
F 1 "VCC" H 8217 1573 50  0000 C CNN
F 2 "" H 8200 1400 50  0001 C CNN
F 3 "" H 8200 1400 50  0001 C CNN
	1    8200 1400
	1    0    0    -1  
$EndComp
Connection ~ 8200 1400
Wire Wire Line
	8200 1400 8950 1400
$Comp
L Device:R R8
U 1 1 5BDFB9CD
P 2150 3500
F 0 "R8" H 2220 3546 50  0000 L CNN
F 1 "0" H 2220 3455 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric" V 2080 3500 50  0001 C CNN
F 3 "~" H 2150 3500 50  0001 C CNN
	1    2150 3500
	0    1    1    0   
$EndComp
$Comp
L power:VCC #PWR0104
U 1 1 5BE88E53
P 9200 3600
F 0 "#PWR0104" H 9200 3450 50  0001 C CNN
F 1 "VCC" H 9217 3773 50  0000 C CNN
F 2 "" H 9200 3600 50  0001 C CNN
F 3 "" H 9200 3600 50  0001 C CNN
	1    9200 3600
	1    0    0    -1  
$EndComp
Wire Wire Line
	9400 3600 9200 3600
$Comp
L power:+3.3V #PWR0105
U 1 1 5BE8F31D
P 9050 3700
F 0 "#PWR0105" H 9050 3550 50  0001 C CNN
F 1 "+3.3V" H 9050 3840 50  0000 C CNN
F 2 "" H 9050 3700 50  0001 C CNN
F 3 "" H 9050 3700 50  0001 C CNN
	1    9050 3700
	1    0    0    -1  
$EndComp
Wire Wire Line
	9050 3700 9400 3700
Text GLabel 9300 3800 0    50   Input ~ 0
SCK
Wire Wire Line
	9300 3800 9400 3800
Text GLabel 9050 3900 0    50   Input ~ 0
MISO
Wire Wire Line
	9050 3900 9400 3900
Text GLabel 9300 4000 0    50   Input ~ 0
MOSI
Wire Wire Line
	9300 4000 9400 4000
Text GLabel 9050 4100 0    50   Input ~ 0
A
Wire Wire Line
	9050 4100 9400 4100
Text GLabel 9300 4200 0    50   Input ~ 0
B
Wire Wire Line
	9300 4200 9400 4200
Text GLabel 9050 4300 0    50   Input ~ 0
IO0
Wire Wire Line
	9050 4300 9400 4300
$Comp
L power:GND #PWR0106
U 1 1 5BEBD0B6
P 8350 4900
F 0 "#PWR0106" H 8350 4650 50  0001 C CNN
F 1 "GND" H 8350 4750 50  0000 C CNN
F 2 "" H 8350 4900 50  0001 C CNN
F 3 "" H 8350 4900 50  0001 C CNN
	1    8350 4900
	1    0    0    -1  
$EndComp
Wire Wire Line
	8350 4900 9250 4900
Text GLabel 9050 5000 0    50   Input ~ 0
IO26
Text GLabel 8750 5100 0    50   Input ~ 0
ADC
Text GLabel 9050 5200 0    50   Input ~ 0
RST
Text GLabel 8750 5300 0    50   Input ~ 0
SCL
Text GLabel 9050 5400 0    50   Input ~ 0
TXD
Text GLabel 8750 5500 0    50   Input ~ 0
RXD
Text GLabel 9050 5600 0    50   Input ~ 0
SDA
Wire Wire Line
	9050 5000 9250 5000
Wire Wire Line
	8750 5100 9250 5100
Wire Wire Line
	9250 5200 9050 5200
Wire Wire Line
	8750 5300 9250 5300
Wire Wire Line
	9250 5400 9050 5400
Wire Wire Line
	8750 5500 9250 5500
Wire Wire Line
	9050 5600 9250 5600
Text GLabel 2100 5150 0    50   Input ~ 0
RST
$Comp
L power:+3.3V #PWR0107
U 1 1 5BF1F614
P 3050 4950
F 0 "#PWR0107" H 3050 4800 50  0001 C CNN
F 1 "+3.3V" H 3050 5090 50  0000 C CNN
F 2 "" H 3050 4950 50  0001 C CNN
F 3 "" H 3050 4950 50  0001 C CNN
	1    3050 4950
	1    0    0    -1  
$EndComp
Wire Wire Line
	3050 5150 2900 5150
Wire Wire Line
	2600 5150 2500 5150
Wire Wire Line
	2500 5150 2500 5250
$Comp
L power:GND #PWR0108
U 1 1 5BF8012B
P 2500 5550
F 0 "#PWR0108" H 2500 5300 50  0001 C CNN
F 1 "GND" H 2500 5400 50  0000 C CNN
F 2 "" H 2500 5550 50  0001 C CNN
F 3 "" H 2500 5550 50  0001 C CNN
	1    2500 5550
	1    0    0    -1  
$EndComp
Text GLabel 1700 3500 0    50   Input ~ 0
IO2
Wire Wire Line
	1700 3500 2000 3500
Text GLabel 2650 3500 2    50   Input ~ 0
B
Wire Wire Line
	2300 3500 2650 3500
Text GLabel 1700 3900 0    50   Input ~ 0
IO5
Text GLabel 2650 3900 2    50   Input ~ 0
A
$Comp
L Device:R R4
U 1 1 5BFC93CC
P 2150 3900
F 0 "R4" H 2220 3946 50  0000 L CNN
F 1 "0" H 2220 3855 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric" V 2080 3900 50  0001 C CNN
F 3 "~" H 2150 3900 50  0001 C CNN
	1    2150 3900
	0    1    1    0   
$EndComp
Wire Wire Line
	1700 3900 2000 3900
Wire Wire Line
	2300 3900 2650 3900
Text GLabel 3400 1800 2    50   Input ~ 0
B
Wire Wire Line
	3050 1800 3400 1800
Text GLabel 3400 2100 2    50   Input ~ 0
A
Wire Wire Line
	3050 2100 3400 2100
Text GLabel 1450 2000 0    50   Input ~ 0
IO2
Text GLabel 1700 1900 0    50   Input ~ 0
IO5
Wire Wire Line
	1700 1900 2250 1900
Wire Wire Line
	1450 2000 2250 2000
$Comp
L power:+3.3V #PWR0110
U 1 1 5C0545BC
P 2650 1200
F 0 "#PWR0110" H 2650 1050 50  0001 C CNN
F 1 "+3.3V" H 2650 1340 50  0000 C CNN
F 2 "" H 2650 1200 50  0001 C CNN
F 3 "" H 2650 1200 50  0001 C CNN
	1    2650 1200
	1    0    0    -1  
$EndComp
Wire Wire Line
	2650 1200 2650 1400
Wire Wire Line
	3550 3300 4650 3300
Connection ~ 2500 5150
Wire Wire Line
	2500 5150 2100 5150
Text GLabel 7100 3850 2    50   Input ~ 0
SCK
Text GLabel 6800 3750 2    50   Input ~ 0
MISO
Text GLabel 7100 3150 2    50   Input ~ 0
MOSI
Text GLabel 6650 4950 2    50   Input ~ 0
IO2
Text GLabel 6800 4350 2    50   Input ~ 0
IO0
Text GLabel 6800 3950 2    50   Input ~ 0
IO5
Text GLabel 4000 3500 0    50   Input ~ 0
ADC
Text GLabel 4350 3400 0    50   Input ~ 0
RST
Text GLabel 7150 3250 2    50   Input ~ 0
SCL
Text GLabel 6800 3350 2    50   Input ~ 0
TXD
Text GLabel 7150 3450 2    50   Input ~ 0
RXD
Text GLabel 6800 3550 2    50   Input ~ 0
SDA
Wire Wire Line
	6800 3950 6500 3950
Wire Wire Line
	4650 3500 4000 3500
Wire Wire Line
	4350 3400 4650 3400
Wire Wire Line
	6500 3250 7150 3250
Wire Wire Line
	6500 3350 6800 3350
Wire Wire Line
	6500 3450 7150 3450
Wire Wire Line
	6500 3550 6800 3550
Wire Wire Line
	7100 3850 6500 3850
Wire Wire Line
	6800 3750 6500 3750
Wire Wire Line
	6500 3150 7100 3150
Wire Wire Line
	6050 4950 6650 4950
Wire Wire Line
	6800 4350 6500 4350
$Comp
L power:+3.3V #PWR0112
U 1 1 5BE7E69B
P 3550 3300
F 0 "#PWR0112" H 3550 3150 50  0001 C CNN
F 1 "+3.3V" H 3550 3440 50  0000 C CNN
F 2 "" H 3550 3300 50  0001 C CNN
F 3 "" H 3550 3300 50  0001 C CNN
	1    3550 3300
	1    0    0    -1  
$EndComp
Wire Wire Line
	9250 1950 9250 2200
$Comp
L power:PWR_FLAG #FLG0101
U 1 1 5BE7FEA9
P 8850 2200
F 0 "#FLG0101" H 8850 2275 50  0001 C CNN
F 1 "PWR_FLAG" H 8850 2350 50  0000 C CNN
F 2 "" H 8850 2200 50  0001 C CNN
F 3 "" H 8850 2200 50  0001 C CNN
	1    8850 2200
	1    0    0    -1  
$EndComp
Wire Wire Line
	8850 2200 9250 2200
Connection ~ 9250 2200
Wire Wire Line
	9250 2200 9250 2250
$Comp
L components:ESP32-WROOM U2
U 1 1 5B923A7D
P 5600 3900
F 0 "U2" H 5600 3800 50  0000 C CNN
F 1 "ESP-WROOM-32" H 5600 4350 50  0000 C CNN
F 2 "esp32:ESP32-WROOM" H 5500 3900 50  0001 C CNN
F 3 "" H 5500 3900 50  0001 C CNN
	1    5600 3900
	1    0    0    -1  
$EndComp
Wire Wire Line
	5150 4950 5150 5050
Wire Wire Line
	3050 4950 3050 5150
Text GLabel 4450 4200 0    50   Input ~ 0
IO26
Wire Wire Line
	4450 4200 4650 4200
NoConn ~ 6500 4250
NoConn ~ 5950 4950
NoConn ~ 5850 4950
NoConn ~ 5750 4950
NoConn ~ 5650 4950
NoConn ~ 5550 4950
NoConn ~ 5450 4950
NoConn ~ 5350 4950
NoConn ~ 5250 4950
NoConn ~ 4650 4500
NoConn ~ 4650 4400
NoConn ~ 4650 4300
NoConn ~ 4650 4100
NoConn ~ 4650 4000
NoConn ~ 4650 3900
NoConn ~ 4650 3800
NoConn ~ 4650 3700
NoConn ~ 4650 3600
Connection ~ 6500 4550
Wire Wire Line
	6500 4450 6500 4550
Wire Wire Line
	6500 4550 6500 5050
Wire Wire Line
	4650 4600 4650 5050
Wire Wire Line
	4650 5050 5150 5050
$Comp
L power:GND #PWR0109
U 1 1 5BE8AF88
P 6500 5050
F 0 "#PWR0109" H 6500 4800 50  0001 C CNN
F 1 "GND" H 6505 4877 50  0000 C CNN
F 2 "" H 6500 5050 50  0001 C CNN
F 3 "" H 6500 5050 50  0001 C CNN
	1    6500 5050
	1    0    0    -1  
$EndComp
Wire Wire Line
	5150 5050 6500 5050
Connection ~ 5150 5050
Connection ~ 6500 5050
Wire Wire Line
	2250 1800 1250 1800
Text Label 1300 1800 0    50   ~ 0
IO16
Wire Wire Line
	1500 2100 2250 2100
Text Label 1550 2100 0    50   ~ 0
IO17
Wire Wire Line
	6500 4050 7300 4050
Text Label 7150 4050 0    50   ~ 0
IO17
Wire Wire Line
	6500 4150 7000 4150
Text Label 6850 4150 0    50   ~ 0
IO16
$EndSCHEMATC

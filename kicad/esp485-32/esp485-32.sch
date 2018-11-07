EESchema Schematic File Version 4
LIBS:esp485-32-cache
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
P 2150 1550
F 0 "U1" H 2300 1500 60  0000 C CNN
F 1 "MAX3485" H 2150 2000 60  0000 C CNN
F 2 "Package_SO:SOIC-8_3.9x4.9mm_P1.27mm" H 2150 1550 60  0001 C CNN
F 3 "" H 2150 1550 60  0000 C CNN
	1    2150 1550
	1    0    0    -1  
$EndComp
$Comp
L Device:R R1
U 1 1 5B923C52
P 3550 3950
F 0 "R1" V 3630 3950 50  0000 C CNN
F 1 "10K" V 3550 3950 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" V 3480 3950 50  0001 C CNN
F 3 "" H 3550 3950 50  0001 C CNN
	1    3550 3950
	-1   0    0    1   
$EndComp
$Comp
L Device:R R2
U 1 1 5B923CFF
P 6550 4800
F 0 "R2" V 6630 4800 50  0000 C CNN
F 1 "10K" V 6550 4800 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" V 6480 4800 50  0001 C CNN
F 3 "" H 6550 4800 50  0001 C CNN
	1    6550 4800
	0    -1   -1   0   
$EndComp
$Comp
L Device:R R4
U 1 1 5B9244E1
P 2750 1850
F 0 "R4" V 2830 1850 50  0000 C CNN
F 1 "20K" V 2750 1850 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.15x1.40mm_HandSolder" V 2680 1850 50  0001 C CNN
F 3 "" H 2750 1850 50  0001 C CNN
	1    2750 1850
	0    -1   -1   0   
$EndComp
$Comp
L Device:R R5
U 1 1 5B9244E7
P 2850 1250
F 0 "R5" V 2930 1250 50  0000 C CNN
F 1 "20K" V 2850 1250 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.15x1.40mm_HandSolder" V 2780 1250 50  0001 C CNN
F 3 "" H 2850 1250 50  0001 C CNN
	1    2850 1250
	0    1    1    0   
$EndComp
Text Notes 7950 7500 0    60   ~ 0
ESP485
$Comp
L Device:R R10
U 1 1 5B924936
P 3350 1600
F 0 "R10" V 3430 1600 50  0000 C CNN
F 1 "100" V 3350 1600 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.15x1.40mm_HandSolder" V 3280 1600 50  0001 C CNN
F 3 "" H 3350 1600 50  0001 C CNN
	1    3350 1600
	-1   0    0    1   
$EndComp
$Comp
L power:+3.3V #PWR01
U 1 1 5B9252B0
P 9450 1050
F 0 "#PWR01" H 9450 900 50  0001 C CNN
F 1 "+3.3V" H 9450 1190 50  0000 C CNN
F 2 "" H 9450 1050 50  0001 C CNN
F 3 "" H 9450 1050 50  0001 C CNN
	1    9450 1050
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR03
U 1 1 5B92A51D
P 6000 5250
F 0 "#PWR03" H 6000 5000 50  0001 C CNN
F 1 "GND" H 6000 5100 50  0000 C CNN
F 2 "" H 6000 5250 50  0001 C CNN
F 3 "" H 6000 5250 50  0001 C CNN
	1    6000 5250
	1    0    0    -1  
$EndComp
Wire Wire Line
	1750 1550 1750 1650
Wire Wire Line
	1750 1750 1550 1750
$Comp
L power:PWR_FLAG #FLG05
U 1 1 5B926539
P 6950 1050
F 0 "#FLG05" H 6950 1125 50  0001 C CNN
F 1 "PWR_FLAG" H 6950 1200 50  0000 C CNN
F 2 "" H 6950 1050 50  0001 C CNN
F 3 "" H 6950 1050 50  0001 C CNN
	1    6950 1050
	1    0    0    -1  
$EndComp
Wire Wire Line
	6950 1050 7750 1050
$Comp
L Connector_Generic:Conn_01x08 J1
U 1 1 5B94B3AE
P 9350 2600
F 0 "J1" H 9430 2592 50  0000 L CNN
F 1 "Conn_01x08" H 9430 2501 50  0000 L CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x08_P2.54mm_Vertical" H 9350 2600 50  0001 C CNN
F 3 "~" H 9350 2600 50  0001 C CNN
	1    9350 2600
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x08 J2
U 1 1 5B94E662
P 9400 4150
F 0 "J2" H 9480 4142 50  0000 L CNN
F 1 "Conn_01x08" H 9480 4051 50  0000 L CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x08_P2.54mm_Vertical" H 9400 4150 50  0001 C CNN
F 3 "~" H 9400 4150 50  0001 C CNN
	1    9400 4150
	1    0    0    -1  
$EndComp
$Comp
L Regulator_Linear:AMS1117-3.3 U3
U 1 1 5BA6808A
P 8800 1050
F 0 "U3" H 8800 1292 50  0000 C CNN
F 1 "ADP3338-3.3" H 8800 1201 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-223-3_TabPin2" H 8800 1250 50  0001 C CNN
F 3 "http://www.advanced-monolithic.com/pdf/ds1117.pdf" H 8900 800 50  0001 C CNN
	1    8800 1050
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0101
U 1 1 5BA7723D
P 8800 1600
F 0 "#PWR0101" H 8800 1350 50  0001 C CNN
F 1 "GND" H 8805 1427 50  0000 C CNN
F 2 "" H 8800 1600 50  0001 C CNN
F 3 "" H 8800 1600 50  0001 C CNN
	1    8800 1600
	1    0    0    -1  
$EndComp
Wire Wire Line
	8800 1350 8800 1550
Wire Wire Line
	9300 1050 9300 1250
$Comp
L Device:C C1
U 1 1 5BA86A4D
P 7900 1450
F 0 "C1" H 8015 1496 50  0000 L CNN
F 1 "C" H 8015 1405 50  0000 L CNN
F 2 "Capacitor_SMD:C_1206_3216Metric_Pad1.42x1.75mm_HandSolder" H 7938 1300 50  0001 C CNN
F 3 "~" H 7900 1450 50  0001 C CNN
	1    7900 1450
	1    0    0    -1  
$EndComp
$Comp
L Device:C C2
U 1 1 5BA86B48
P 8200 1450
F 0 "C2" H 8315 1496 50  0000 L CNN
F 1 "C" H 8315 1405 50  0000 L CNN
F 2 "Capacitor_SMD:C_1206_3216Metric_Pad1.42x1.75mm_HandSolder" H 8238 1300 50  0001 C CNN
F 3 "~" H 8200 1450 50  0001 C CNN
	1    8200 1450
	1    0    0    -1  
$EndComp
$Comp
L Device:C C3
U 1 1 5BA86C88
P 9300 1400
F 0 "C3" H 9415 1446 50  0000 L CNN
F 1 "C" H 9415 1355 50  0000 L CNN
F 2 "Capacitor_SMD:C_1206_3216Metric_Pad1.42x1.75mm_HandSolder" H 9338 1250 50  0001 C CNN
F 3 "~" H 9300 1400 50  0001 C CNN
	1    9300 1400
	1    0    0    -1  
$EndComp
$Comp
L Device:C C4
U 1 1 5BA86D43
P 9600 1400
F 0 "C4" H 9715 1446 50  0000 L CNN
F 1 "C" H 9715 1355 50  0000 L CNN
F 2 "Capacitor_SMD:C_1206_3216Metric_Pad1.42x1.75mm_HandSolder" H 9638 1250 50  0001 C CNN
F 3 "~" H 9600 1400 50  0001 C CNN
	1    9600 1400
	1    0    0    -1  
$EndComp
Wire Wire Line
	9300 1550 8800 1550
Connection ~ 8800 1550
Wire Wire Line
	8800 1550 8800 1600
Wire Wire Line
	9600 1550 9300 1550
Connection ~ 9300 1550
Wire Wire Line
	9300 1250 9600 1250
Wire Wire Line
	8200 1300 8200 1050
Wire Wire Line
	8200 1300 7900 1300
Connection ~ 8200 1300
Wire Wire Line
	7900 1600 8200 1600
Wire Wire Line
	8200 1600 8800 1600
Connection ~ 8200 1600
Connection ~ 8800 1600
Wire Wire Line
	9450 1050 9300 1050
$Comp
L Device:C C5
U 1 1 5BA20D2D
P 2700 4250
F 0 "C5" H 2815 4296 50  0000 L CNN
F 1 "C" H 2815 4205 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 2738 4100 50  0001 C CNN
F 3 "~" H 2700 4250 50  0001 C CNN
	1    2700 4250
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0102
U 1 1 5BA33C46
P 2700 4550
F 0 "#PWR0102" H 2700 4300 50  0001 C CNN
F 1 "GND" H 2705 4377 50  0000 C CNN
F 2 "" H 2700 4550 50  0001 C CNN
F 3 "" H 2700 4550 50  0001 C CNN
	1    2700 4550
	1    0    0    -1  
$EndComp
Wire Wire Line
	2700 4400 2700 4550
Wire Wire Line
	3550 4100 2700 4100
$Comp
L power:VCC #PWR0103
U 1 1 5BA4E6FC
P 7750 1050
F 0 "#PWR0103" H 7750 900 50  0001 C CNN
F 1 "VCC" H 7767 1223 50  0000 C CNN
F 2 "" H 7750 1050 50  0001 C CNN
F 3 "" H 7750 1050 50  0001 C CNN
	1    7750 1050
	1    0    0    -1  
$EndComp
Connection ~ 7750 1050
Wire Wire Line
	7750 1050 8200 1050
Wire Wire Line
	1300 1650 1750 1650
Wire Wire Line
	1550 1750 1550 1450
Wire Wire Line
	1550 1450 1750 1450
$Comp
L Device:R R8
U 1 1 5BDFB9CD
P 5200 2350
F 0 "R8" H 5270 2396 50  0000 L CNN
F 1 "0" H 5270 2305 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.15x1.40mm_HandSolder" V 5130 2350 50  0001 C CNN
F 3 "~" H 5200 2350 50  0001 C CNN
	1    5200 2350
	0    1    1    0   
$EndComp
$Comp
L esp32:ESP32-WROOM U2
U 1 1 5BE3DB20
P 4950 4600
F 0 "U2" H 4925 5987 60  0000 C CNN
F 1 "ESP32-WROOM" H 4925 5881 60  0000 C CNN
F 2 "esp32:ESP32-WROOM" H 5300 5950 60  0001 C CNN
F 3 "" H 4500 5050 60  0001 C CNN
	1    4950 4600
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR0104
U 1 1 5BE77897
P 2150 1000
F 0 "#PWR0104" H 2150 850 50  0001 C CNN
F 1 "+3.3V" H 2150 1140 50  0000 C CNN
F 2 "" H 2150 1000 50  0001 C CNN
F 3 "" H 2150 1000 50  0001 C CNN
	1    2150 1000
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0105
U 1 1 5BE819BA
P 2150 2200
F 0 "#PWR0105" H 2150 1950 50  0001 C CNN
F 1 "GND" H 2155 2027 50  0000 C CNN
F 2 "" H 2150 2200 50  0001 C CNN
F 3 "" H 2150 2200 50  0001 C CNN
	1    2150 2200
	1    0    0    -1  
$EndComp
Text GLabel 1300 1650 0    50   Input ~ 0
IO2
Wire Wire Line
	1550 1450 1300 1450
Connection ~ 1550 1450
Text GLabel 1300 1450 0    50   Input ~ 0
IO0
Connection ~ 1750 1650
Wire Wire Line
	3100 1050 2150 1050
Wire Wire Line
	2150 1000 2150 1050
Connection ~ 2150 1050
Wire Wire Line
	2150 2150 2150 2200
Wire Wire Line
	3000 2150 2150 2150
Connection ~ 2150 2150
Wire Wire Line
	2550 1450 2700 1450
Wire Wire Line
	2700 1250 2700 1450
Connection ~ 2700 1450
Wire Wire Line
	2550 1750 2600 1750
Wire Wire Line
	3100 1050 3100 1850
Wire Wire Line
	3100 1850 2900 1850
Wire Wire Line
	3000 1250 3000 1450
Wire Wire Line
	2600 1850 2600 1750
Connection ~ 2600 1750
Wire Wire Line
	2600 1750 3350 1750
Wire Wire Line
	3000 1450 3000 2150
Connection ~ 9300 1050
Wire Wire Line
	9300 1050 9100 1050
Connection ~ 9300 1250
Connection ~ 8200 1050
Wire Wire Line
	8200 1050 8500 1050
$Comp
L power:+3.3V #PWR0106
U 1 1 5BFD1C7D
P 4000 3750
F 0 "#PWR0106" H 4000 3600 50  0001 C CNN
F 1 "+3.3V" H 4000 3890 50  0000 C CNN
F 2 "" H 4000 3750 50  0001 C CNN
F 3 "" H 4000 3750 50  0001 C CNN
	1    4000 3750
	1    0    0    -1  
$EndComp
Wire Wire Line
	4000 3750 4000 3800
Wire Wire Line
	4000 4100 3550 4100
$Comp
L power:VCC #PWR0107
U 1 1 5BFDE0A6
P 9050 2300
F 0 "#PWR0107" H 9050 2150 50  0001 C CNN
F 1 "VCC" H 9067 2473 50  0000 C CNN
F 2 "" H 9050 2300 50  0001 C CNN
F 3 "" H 9050 2300 50  0001 C CNN
	1    9050 2300
	1    0    0    -1  
$EndComp
Wire Wire Line
	9150 2300 9050 2300
Wire Wire Line
	5850 5150 5850 5250
Wire Wire Line
	5850 5250 6000 5250
Connection ~ 5850 5250
$Comp
L power:GND #PWR0108
U 1 1 5BFE474F
P 4000 5700
F 0 "#PWR0108" H 4000 5450 50  0001 C CNN
F 1 "GND" H 4000 5550 50  0000 C CNN
F 2 "" H 4000 5700 50  0001 C CNN
F 3 "" H 4000 5700 50  0001 C CNN
	1    4000 5700
	1    0    0    -1  
$EndComp
Text GLabel 2600 4100 0    50   Input ~ 0
RST
Wire Wire Line
	2700 4100 2600 4100
Connection ~ 2700 4100
Connection ~ 3550 4100
Wire Wire Line
	3550 3800 4000 3800
Connection ~ 4000 3800
Wire Wire Line
	4000 3800 4000 4000
Text GLabel 6550 5050 2    50   Input ~ 0
IO0
$Comp
L power:+3.3V #PWR0109
U 1 1 5BFEBD2B
P 6800 4800
F 0 "#PWR0109" H 6800 4650 50  0001 C CNN
F 1 "+3.3V" H 6800 4940 50  0000 C CNN
F 2 "" H 6800 4800 50  0001 C CNN
F 3 "" H 6800 4800 50  0001 C CNN
	1    6800 4800
	1    0    0    -1  
$EndComp
Wire Wire Line
	5850 5050 6400 5050
Wire Wire Line
	6400 4800 6400 5050
Connection ~ 6400 5050
Wire Wire Line
	6400 5050 6550 5050
$Comp
L power:+3.3V #PWR0110
U 1 1 5BFEFD1C
P 8900 3950
F 0 "#PWR0110" H 8900 3800 50  0001 C CNN
F 1 "+3.3V" H 8900 4090 50  0000 C CNN
F 2 "" H 8900 3950 50  0001 C CNN
F 3 "" H 8900 3950 50  0001 C CNN
	1    8900 3950
	1    0    0    -1  
$EndComp
Wire Wire Line
	9200 3950 8900 3950
Wire Wire Line
	9200 3850 9200 3500
$Comp
L power:GND #PWR0111
U 1 1 5BFF28E1
P 8900 3500
F 0 "#PWR0111" H 8900 3250 50  0001 C CNN
F 1 "GND" H 8900 3350 50  0000 C CNN
F 2 "" H 8900 3500 50  0001 C CNN
F 3 "" H 8900 3500 50  0001 C CNN
	1    8900 3500
	1    0    0    -1  
$EndComp
Wire Wire Line
	8900 3500 9200 3500
Wire Wire Line
	9200 4150 8800 4150
Text GLabel 8800 4150 0    50   Input ~ 0
RST
Wire Wire Line
	5400 5650 5400 5800
Text GLabel 5400 5800 3    50   Input ~ 0
IO2
Text GLabel 3750 4200 0    50   Input ~ 0
TOUT
Wire Wire Line
	3750 4200 4000 4200
Text GLabel 9050 4050 0    50   Input ~ 0
TOUT
Wire Wire Line
	9050 4050 9200 4050
Wire Wire Line
	9200 4250 9050 4250
Text GLabel 9050 4250 0    50   Input ~ 0
SCL
Text GLabel 8800 4550 0    50   Input ~ 0
SDA
Wire Wire Line
	9200 4550 8800 4550
Text GLabel 8800 4350 0    50   Input ~ 0
TXD
Wire Wire Line
	9200 4350 8800 4350
Text GLabel 9050 4450 0    50   Input ~ 0
RXD
Wire Wire Line
	9050 4450 9200 4450
Text GLabel 6100 4050 2    50   Input ~ 0
TXD
Text GLabel 6350 4150 2    50   Input ~ 0
RXD
Wire Wire Line
	5850 4050 6100 4050
Wire Wire Line
	5850 4150 6350 4150
Text GLabel 3600 1450 2    50   Input ~ 0
B
Text GLabel 3600 1750 2    50   Input ~ 0
A
Wire Wire Line
	3600 1450 3350 1450
Connection ~ 3350 1450
Wire Wire Line
	3600 1750 3350 1750
Connection ~ 3350 1750
Text GLabel 8750 3000 0    50   Input ~ 0
B
Wire Wire Line
	9150 3000 8750 3000
Text GLabel 9050 2900 0    50   Input ~ 0
A
Wire Wire Line
	9050 2900 9150 2900
Text GLabel 8750 2800 0    50   Input ~ 0
IO0
Wire Wire Line
	8750 2800 9150 2800
Text GLabel 9050 2700 0    50   Input ~ 0
SS
Text GLabel 8750 2600 0    50   Input ~ 0
MOSI
Text GLabel 9050 2500 0    50   Input ~ 0
MISO
Text GLabel 8750 2400 0    50   Input ~ 0
SCK
Wire Wire Line
	9150 2400 8750 2400
Wire Wire Line
	9050 2500 9150 2500
Wire Wire Line
	9150 2600 8750 2600
Wire Wire Line
	9050 2700 9150 2700
Text GLabel 6350 4550 2    50   Input ~ 0
SCK
Wire Wire Line
	5850 4550 6350 4550
Text GLabel 5950 4450 2    50   Input ~ 0
MISO
Wire Wire Line
	5950 4450 5850 4450
Text GLabel 5950 3850 2    50   Input ~ 0
MOSI
Wire Wire Line
	5850 3850 5950 3850
Text GLabel 5950 4650 2    50   Input ~ 0
SS
Wire Wire Line
	5950 4650 5850 4650
Wire Wire Line
	6700 4800 6800 4800
Text GLabel 6350 3950 2    50   Input ~ 0
SCL
Wire Wire Line
	6350 3950 5850 3950
Text GLabel 6100 4250 2    50   Input ~ 0
SDA
Wire Wire Line
	5850 4250 6100 4250
Text GLabel 4750 2350 0    50   Input ~ 0
IO2
Text GLabel 5800 2350 2    50   Input ~ 0
A
Wire Wire Line
	4750 2350 5050 2350
Wire Wire Line
	5350 2350 5800 2350
NoConn ~ 4000 4300
NoConn ~ 4000 4400
NoConn ~ 4000 4500
NoConn ~ 4000 4600
NoConn ~ 4000 4700
NoConn ~ 4000 4800
NoConn ~ 4000 4900
NoConn ~ 4000 5000
NoConn ~ 4000 5100
NoConn ~ 4000 5200
NoConn ~ 5850 4750
NoConn ~ 5850 4850
NoConn ~ 5850 4950
NoConn ~ 4500 5650
NoConn ~ 4600 5650
NoConn ~ 4700 5650
NoConn ~ 4800 5650
NoConn ~ 4900 5650
NoConn ~ 5000 5650
NoConn ~ 5100 5650
NoConn ~ 5200 5650
NoConn ~ 5300 5650
Wire Wire Line
	2700 1450 3350 1450
Wire Wire Line
	4000 5300 4000 5700
$EndSCHEMATC

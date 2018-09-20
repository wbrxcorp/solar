EESchema Schematic File Version 4
LIBS:esp485-12f-cache
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
L components:ESP-12F U2
U 1 1 5B923A7D
P 5800 4050
F 0 "U2" H 5800 3950 50  0000 C CNN
F 1 "ESP-12F" H 5800 4500 50  0000 C CNN
F 2 "ESP8266:ESP-12E_SMD_NO_UNUSED_PAD" H 5700 4050 50  0001 C CNN
F 3 "" H 5700 4050 50  0001 C CNN
	1    5800 4050
	1    0    0    -1  
$EndComp
$Comp
L Interface_UART:MAX3485 U1
U 1 1 5B923B04
P 3750 2500
F 0 "U1" H 3900 2450 60  0000 C CNN
F 1 "MAX3485" H 3750 2950 60  0000 C CNN
F 2 "Package_SO:SOIC-8_3.9x4.9mm_P1.27mm" H 3750 2500 60  0001 C CNN
F 3 "" H 3750 2500 60  0000 C CNN
	1    3750 2500
	1    0    0    -1  
$EndComp
$Comp
L Device:R R1
U 1 1 5B923C52
P 3150 3800
F 0 "R1" V 3230 3800 50  0000 C CNN
F 1 "10K" V 3150 3800 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.15x1.40mm_HandSolder" V 3080 3800 50  0001 C CNN
F 3 "" H 3150 3800 50  0001 C CNN
	1    3150 3800
	0    1    1    0   
$EndComp
$Comp
L Device:R R2
U 1 1 5B923CFF
P 2750 4250
F 0 "R2" V 2830 4250 50  0000 C CNN
F 1 "10K" V 2750 4250 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.15x1.40mm_HandSolder" V 2680 4250 50  0001 C CNN
F 3 "" H 2750 4250 50  0001 C CNN
	1    2750 4250
	0    1    1    0   
$EndComp
$Comp
L Device:R R3
U 1 1 5B923D91
P 2750 4400
F 0 "R3" V 2830 4400 50  0000 C CNN
F 1 "10K" V 2750 4400 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.15x1.40mm_HandSolder" V 2680 4400 50  0001 C CNN
F 3 "" H 2750 4400 50  0001 C CNN
	1    2750 4400
	0    1    1    0   
$EndComp
$Comp
L Device:R R7
U 1 1 5B924085
P 7250 2050
F 0 "R7" V 7330 2050 50  0000 C CNN
F 1 "10K" V 7250 2050 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.15x1.40mm_HandSolder" V 7180 2050 50  0001 C CNN
F 3 "" H 7250 2050 50  0001 C CNN
	1    7250 2050
	1    0    0    -1  
$EndComp
$Comp
L Device:R R9
U 1 1 5B92408B
P 4300 4200
F 0 "R9" V 4380 4200 50  0000 C CNN
F 1 "10K" V 4300 4200 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.15x1.40mm_HandSolder" V 4230 4200 50  0001 C CNN
F 3 "" H 4300 4200 50  0001 C CNN
	1    4300 4200
	0    1    1    0   
$EndComp
$Comp
L Device:R R4
U 1 1 5B9244E1
P 4600 2050
F 0 "R4" V 4680 2050 50  0000 C CNN
F 1 "20K" V 4600 2050 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.15x1.40mm_HandSolder" V 4530 2050 50  0001 C CNN
F 3 "" H 4600 2050 50  0001 C CNN
	1    4600 2050
	0    1    1    0   
$EndComp
$Comp
L Device:R R5
U 1 1 5B9244E7
P 4550 2650
F 0 "R5" V 4630 2650 50  0000 C CNN
F 1 "20K" V 4550 2650 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.15x1.40mm_HandSolder" V 4480 2650 50  0001 C CNN
F 3 "" H 4550 2650 50  0001 C CNN
	1    4550 2650
	0    1    1    0   
$EndComp
Text Notes 7950 7500 0    60   ~ 0
ESP485-12F
$Comp
L Device:R R10
U 1 1 5B924936
P 5050 2800
F 0 "R10" V 5130 2800 50  0000 C CNN
F 1 "100" V 5050 2800 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.15x1.40mm_HandSolder" V 4980 2800 50  0001 C CNN
F 3 "" H 5050 2800 50  0001 C CNN
	1    5050 2800
	0    1    1    0   
$EndComp
$Comp
L power:+3.3V #PWR06
U 1 1 5B9252B0
P 9450 1050
F 0 "#PWR06" H 9450 900 50  0001 C CNN
F 1 "+3.3V" H 9450 1190 50  0000 C CNN
F 2 "" H 9450 1050 50  0001 C CNN
F 3 "" H 9450 1050 50  0001 C CNN
	1    9450 1050
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR02
U 1 1 5B927F29
P 4150 4950
F 0 "#PWR02" H 4150 4700 50  0001 C CNN
F 1 "GND" H 4150 4800 50  0000 C CNN
F 2 "" H 4150 4950 50  0001 C CNN
F 3 "" H 4150 4950 50  0001 C CNN
	1    4150 4950
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR03
U 1 1 5B92A51D
P 7100 5250
F 0 "#PWR03" H 7100 5000 50  0001 C CNN
F 1 "GND" H 7100 5100 50  0000 C CNN
F 2 "" H 7100 5250 50  0001 C CNN
F 3 "" H 7100 5250 50  0001 C CNN
	1    7100 5250
	1    0    0    -1  
$EndComp
$Comp
L Device:R R8
U 1 1 5B92AAFC
P 6750 2050
F 0 "R8" V 6830 2050 50  0000 C CNN
F 1 "10K" V 6750 2050 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.15x1.40mm_HandSolder" V 6680 2050 50  0001 C CNN
F 3 "" H 6750 2050 50  0001 C CNN
	1    6750 2050
	1    0    0    -1  
$EndComp
Wire Wire Line
	4400 2200 4750 2200
Wire Wire Line
	4400 2500 5400 2500
Wire Wire Line
	4900 1900 4900 2200
Wire Wire Line
	4000 4250 2900 4250
Wire Wire Line
	3350 2500 3350 2600
Wire Wire Line
	3350 2400 2900 2450
Wire Wire Line
	4400 2500 4400 2650
Wire Wire Line
	4700 2650 4700 2300
Connection ~ 4700 2300
Wire Wire Line
	4450 2400 4450 2050
Wire Wire Line
	4750 2050 4750 2200
Connection ~ 4750 2200
Wire Wire Line
	4900 2400 4900 2800
Connection ~ 4450 2400
Wire Wire Line
	5400 3100 4150 3100
Wire Wire Line
	4150 3100 4150 4200
Connection ~ 4150 4200
Wire Wire Line
	4450 4200 4450 5150
Wire Wire Line
	7250 4150 6900 4150
Connection ~ 6750 1900
Wire Wire Line
	6750 2200 6750 3300
Wire Wire Line
	3350 2600 3300 2650
Wire Wire Line
	3300 2650 3300 3300
Wire Wire Line
	3300 3300 4700 3300
Wire Wire Line
	2900 4450 3050 4450
Wire Wire Line
	2900 4400 2900 4450
$Comp
L power:PWR_FLAG #FLG01
U 1 1 5B926509
P 3700 4800
F 0 "#FLG01" H 3700 4875 50  0001 C CNN
F 1 "PWR_FLAG" H 3700 4950 50  0000 C CNN
F 2 "" H 3700 4800 50  0001 C CNN
F 3 "" H 3700 4800 50  0001 C CNN
	1    3700 4800
	1    0    0    -1  
$EndComp
$Comp
L power:PWR_FLAG #FLG02
U 1 1 5B926539
P 6950 1050
F 0 "#FLG02" H 6950 1125 50  0001 C CNN
F 1 "PWR_FLAG" H 6950 1200 50  0000 C CNN
F 2 "" H 6950 1050 50  0001 C CNN
F 3 "" H 6950 1050 50  0001 C CNN
	1    6950 1050
	1    0    0    -1  
$EndComp
Wire Wire Line
	6950 1050 7750 1050
Wire Wire Line
	3700 4800 4150 4800
Connection ~ 4150 4800
Wire Wire Line
	2600 4250 2600 4400
Wire Wire Line
	4700 2300 5200 2300
Wire Wire Line
	4750 2200 4900 2200
Wire Wire Line
	4450 2400 4900 2400
Wire Wire Line
	4150 4800 4150 4850
Connection ~ 3350 2600
Connection ~ 2900 4250
Wire Wire Line
	4400 2200 4400 2000
Wire Wire Line
	4400 2000 3750 2000
Wire Wire Line
	4400 2500 4350 2500
Wire Wire Line
	4350 2500 4350 3050
Wire Wire Line
	4350 3050 3950 3050
Wire Wire Line
	3950 3050 3950 3100
Wire Wire Line
	3950 3100 3750 3100
Connection ~ 4400 2500
Wire Wire Line
	4250 2300 4250 2400
Wire Wire Line
	4250 2400 4150 2400
Wire Wire Line
	4250 2300 4700 2300
Wire Wire Line
	4300 2400 4300 2700
Wire Wire Line
	4300 2700 4150 2700
Wire Wire Line
	4300 2400 4450 2400
Wire Wire Line
	6750 1900 7250 1900
$Comp
L Switch:SW_Push SW1
U 1 1 5B9460EF
P 3250 4850
F 0 "SW1" H 3250 5135 50  0000 C CNN
F 1 "SW_Push" H 3250 5044 50  0000 C CNN
F 2 "tact:tactile_switch_smd_4x3mm_2p" H 3250 5050 50  0001 C CNN
F 3 "" H 3250 5050 50  0001 C CNN
	1    3250 4850
	1    0    0    -1  
$EndComp
Wire Wire Line
	3050 4850 3050 4450
Connection ~ 3050 4450
Wire Wire Line
	3450 4850 4150 4850
Connection ~ 4150 4850
Wire Wire Line
	4150 4850 4150 4950
$Comp
L Switch:SW_Push SW2
U 1 1 5B949D0E
P 6650 5550
F 0 "SW2" H 6650 5835 50  0000 C CNN
F 1 "SW_Push" H 6650 5744 50  0000 C CNN
F 2 "tact:tactile_switch_smd_4x3mm_2p" H 6650 5750 50  0001 C CNN
F 3 "" H 6650 5750 50  0001 C CNN
	1    6650 5550
	1    0    0    -1  
$EndComp
Wire Wire Line
	6850 5550 6850 5250
Wire Wire Line
	6850 5250 7000 5250
Wire Wire Line
	6450 5550 6450 4900
Wire Wire Line
	6450 4900 7300 4900
Wire Wire Line
	7300 4900 7300 4150
Wire Wire Line
	7300 4150 7250 4150
Connection ~ 7250 4150
Wire Wire Line
	7250 2200 7250 4150
Connection ~ 7250 1900
$Comp
L Regulator_Linear:AMS1117-3.3 U3
U 1 1 5BA6808A
P 8800 1050
F 0 "U3" H 8800 1292 50  0000 C CNN
F 1 "AMS1117-3.3" H 8800 1201 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-223-3_TabPin2" H 8800 1250 50  0001 C CNN
F 3 "http://www.advanced-monolithic.com/pdf/ds1117.pdf" H 8900 800 50  0001 C CNN
	1    8800 1050
	1    0    0    -1  
$EndComp
Wire Wire Line
	8500 1050 8500 1300
$Comp
L power:GND #PWR05
U 1 1 5BA7723D
P 8800 1600
F 0 "#PWR05" H 8800 1350 50  0001 C CNN
F 1 "GND" H 8805 1427 50  0000 C CNN
F 2 "" H 8800 1600 50  0001 C CNN
F 3 "" H 8800 1600 50  0001 C CNN
	1    8800 1600
	1    0    0    -1  
$EndComp
Wire Wire Line
	8800 1350 8800 1550
Wire Wire Line
	9100 1050 9100 1250
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
L Device:CP C3
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
L Device:CP C4
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
	9300 1250 9100 1250
Connection ~ 9100 1250
Wire Wire Line
	9100 1250 9100 1900
Wire Wire Line
	9300 1250 9600 1250
Connection ~ 9300 1250
Wire Wire Line
	8200 1300 8500 1300
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
	4900 1900 6750 1900
Connection ~ 8500 1050
Wire Wire Line
	9450 1050 9100 1050
Connection ~ 9100 1050
$Comp
L Device:C C5
U 1 1 5BA20D2D
P 1800 4250
F 0 "C5" H 1915 4296 50  0000 L CNN
F 1 "C" H 1915 4205 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 1838 4100 50  0001 C CNN
F 3 "~" H 1800 4250 50  0001 C CNN
	1    1800 4250
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR01
U 1 1 5BA33C46
P 1800 4650
F 0 "#PWR01" H 1800 4400 50  0001 C CNN
F 1 "GND" H 1805 4477 50  0000 C CNN
F 2 "" H 1800 4650 50  0001 C CNN
F 3 "" H 1800 4650 50  0001 C CNN
	1    1800 4650
	1    0    0    -1  
$EndComp
Wire Wire Line
	1800 4400 1800 4650
$Comp
L power:VCC #PWR04
U 1 1 5BA4E6FC
P 7750 1050
F 0 "#PWR04" H 7750 900 50  0001 C CNN
F 1 "VCC" H 7767 1223 50  0000 C CNN
F 2 "" H 7750 1050 50  0001 C CNN
F 3 "" H 7750 1050 50  0001 C CNN
	1    7750 1050
	1    0    0    -1  
$EndComp
Connection ~ 7750 1050
Wire Wire Line
	7750 1050 8500 1050
Wire Wire Line
	7250 1900 9100 1900
Wire Wire Line
	3050 4450 4600 4450
Wire Wire Line
	2900 2450 2900 4250
Wire Wire Line
	4150 4200 4150 4800
Wire Wire Line
	4900 3750 4600 3750
Wire Wire Line
	4600 3750 4600 4450
Wire Wire Line
	3350 2700 3200 2700
Wire Wire Line
	3200 2700 3200 3200
Wire Wire Line
	3200 3200 6900 3200
Wire Wire Line
	6900 3200 6900 4150
Connection ~ 6900 4150
Wire Wire Line
	6900 4150 6700 4150
NoConn ~ 4900 3850
Connection ~ 2600 4250
Connection ~ 4900 1900
Wire Wire Line
	2600 1900 4900 1900
Wire Wire Line
	1800 3950 3400 3950
Wire Wire Line
	2600 1900 2600 3800
Wire Wire Line
	3300 3800 3400 3800
Wire Wire Line
	3400 3800 3400 3950
Connection ~ 3400 3950
Wire Wire Line
	3400 3950 4900 3950
Wire Wire Line
	3000 3800 2600 3800
Connection ~ 2600 3800
Wire Wire Line
	2600 3800 2600 4050
Wire Wire Line
	1800 3950 1800 4100
Wire Wire Line
	4450 5150 6300 5150
Wire Wire Line
	6300 5150 6300 4800
Wire Wire Line
	6300 4800 6800 4800
Wire Wire Line
	6800 4800 6800 4350
Wire Wire Line
	6800 4350 6700 4350
Wire Wire Line
	4000 4250 4000 4700
Wire Wire Line
	4000 4700 4800 4700
Wire Wire Line
	4800 4700 4800 5300
Wire Wire Line
	4800 5300 6200 5300
Wire Wire Line
	6200 5300 6200 4700
Wire Wire Line
	6200 4700 6900 4700
Wire Wire Line
	6900 4700 6900 4250
Wire Wire Line
	6900 4250 6700 4250
Wire Wire Line
	4900 4050 4700 4050
Wire Wire Line
	4700 4050 4700 3300
Connection ~ 4700 3300
Wire Wire Line
	4700 3300 6750 3300
NoConn ~ 4900 4150
NoConn ~ 4900 4350
NoConn ~ 4900 4250
NoConn ~ 6700 3950
NoConn ~ 6700 4050
Wire Wire Line
	6700 4450 7000 4450
Wire Wire Line
	7000 4450 7000 5250
Connection ~ 7000 5250
Wire Wire Line
	7000 5250 7100 5250
Wire Wire Line
	4900 4450 4500 4400
Wire Wire Line
	4500 4400 4500 4050
Wire Wire Line
	4500 4050 2600 4050
Connection ~ 2600 4050
Wire Wire Line
	2600 4050 2600 4250
$Comp
L Connector_Generic:Conn_01x06 J1
U 1 1 5BA89674
P 6300 2500
F 0 "J1" H 6380 2492 50  0000 L CNN
F 1 "Conn_01x04" H 6380 2401 50  0000 L CNN
F 2 "Connector_JST:JST_XH_S06B-XH-A-1_1x06_P2.50mm_Horizontal" H 6300 2500 50  0001 C CNN
F 3 "~" H 6300 2500 50  0001 C CNN
	1    6300 2500
	1    0    0    1   
$EndComp
Wire Wire Line
	7750 1050 7750 1200
Wire Wire Line
	7750 1200 6000 1200
Wire Wire Line
	6700 3750 6700 2900
Wire Wire Line
	6700 2900 5900 2900
Wire Wire Line
	5900 2900 5900 2200
Wire Wire Line
	6700 3850 6800 3850
Wire Wire Line
	6800 3850 6800 3000
Wire Wire Line
	6800 3000 5800 3000
Wire Wire Line
	5800 3000 5800 2700
Text Label 6700 3500 0    50   ~ 0
TXD
Text Label 6800 3700 0    50   ~ 0
RXD
Text Label 6750 4150 0    50   ~ 0
IO0
Text Label 6800 4250 0    50   ~ 0
IO2
Text Label 6750 4350 0    50   ~ 0
IO15
Text Label 4800 4050 0    50   ~ 0
IO16
Text Label 4800 3750 0    50   ~ 0
RESET
Text Label 4200 2400 0    50   ~ 0
B
Text Label 4250 2700 0    50   ~ 0
A
Text Label 4750 3950 0    50   ~ 0
EN
Wire Wire Line
	6000 2300 6100 2300
Wire Wire Line
	6000 1200 6000 2300
Wire Wire Line
	5900 2200 6100 2200
Wire Wire Line
	5200 2300 5600 2300
Wire Wire Line
	5600 2300 5600 2400
Wire Wire Line
	5600 2400 6100 2400
Connection ~ 5200 2300
Wire Wire Line
	5800 2700 6100 2700
Wire Wire Line
	4900 2400 5500 2400
Wire Wire Line
	5500 2400 5500 2500
Wire Wire Line
	5500 2500 6100 2500
Connection ~ 4900 2400
Wire Wire Line
	5200 2300 5200 2800
Wire Wire Line
	6100 2600 5400 2600
Wire Wire Line
	5400 2500 5400 2600
Connection ~ 5400 2600
Wire Wire Line
	5400 2600 5400 3100
$EndSCHEMATC

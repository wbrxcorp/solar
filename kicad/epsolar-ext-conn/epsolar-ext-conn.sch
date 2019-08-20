EESchema Schematic File Version 4
LIBS:epsolar-ext-conn-cache
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
L Connector_Generic:Conn_01x08 J11
U 1 1 5CBB0546
P 3450 3900
F 0 "J11" H 3368 4217 50  0000 C CNN
F 1 "CONN" H 3368 4126 50  0000 C CNN
F 2 "lib:8p_cable_holes" H 3450 3900 50  0001 C CNN
F 3 "~" H 3450 3900 50  0001 C CNN
	1    3450 3900
	-1   0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x08 J14
U 1 1 5CBB1510
P 6050 5550
F 0 "J14" H 5968 6067 50  0000 C CNN
F 1 "USBSERIAL" H 5968 5976 50  0000 C CNN
F 2 "lib:usbserial" H 6050 5550 50  0001 C CNN
F 3 "~" H 6050 5550 50  0001 C CNN
	1    6050 5550
	1    0    0    -1  
$EndComp
Wire Wire Line
	5450 5250 5850 5250
Wire Wire Line
	5450 5450 5850 5450
Wire Wire Line
	5450 5550 5850 5550
Wire Wire Line
	5450 5750 5850 5750
Text Label 5600 5250 0    50   ~ 0
RESET
Text Label 5650 5450 0    50   ~ 0
RXD
Text Label 5650 5550 0    50   ~ 0
TXD
Text Label 5600 5350 0    50   ~ 0
VBUS
Text Label 5650 5750 0    50   ~ 0
IO0
$Comp
L Connector:RJ45 J9
U 1 1 5CBB3D57
P 1250 5700
F 0 "J9" H 1307 6367 50  0000 C CNN
F 1 "RJ45" H 1307 6276 50  0000 C CNN
F 2 "lib:RJ45" V 1250 5725 50  0001 C CNN
F 3 "~" V 1250 5725 50  0001 C CNN
	1    1250 5700
	1    0    0    -1  
$EndComp
Wire Wire Line
	1650 5300 1650 5400
Wire Wire Line
	1650 5500 1650 5600
Wire Wire Line
	1650 5700 1650 5800
Wire Wire Line
	1650 5900 1650 6000
Text Label 1650 6000 0    50   ~ 0
VCTRLR
Text Label 1650 5800 0    50   ~ 0
B
Wire Wire Line
	1650 5300 1850 5300
$Comp
L power:GND #PWR0109
U 1 1 5CBBE809
P 1850 5300
F 0 "#PWR0109" H 1850 5050 50  0001 C CNN
F 1 "GND" H 1855 5127 50  0000 C CNN
F 2 "" H 1850 5300 50  0001 C CNN
F 3 "" H 1850 5300 50  0001 C CNN
	1    1850 5300
	1    0    0    -1  
$EndComp
Text Label 1650 5600 0    50   ~ 0
A
$Comp
L Switch:SW_SPDT SW3
U 1 1 5D58070A
P 7350 1300
F 0 "SW3" H 7350 1585 50  0000 C CNN
F 1 "SW_SPDT" H 7350 1494 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical" H 7350 1300 50  0001 C CNN
F 3 "~" H 7350 1300 50  0001 C CNN
	1    7350 1300
	1    0    0    -1  
$EndComp
Wire Wire Line
	7550 1200 7900 1200
Wire Wire Line
	7550 1400 7900 1400
Text Label 7600 1200 0    50   ~ 0
VCTRLR
Text Label 7650 1400 0    50   ~ 0
VBUS
Wire Wire Line
	7150 1300 6850 1300
Text Label 7150 1300 0    50   ~ 0
VIN
Wire Wire Line
	3000 5700 1650 5700
Connection ~ 1650 5700
Wire Wire Line
	2550 5600 1650 5600
Connection ~ 1650 5600
Connection ~ 1650 5300
Wire Wire Line
	5100 5650 5850 5650
$Comp
L power:GND #PWR0107
U 1 1 5CBBD5B0
P 5100 6150
F 0 "#PWR0107" H 5100 5900 50  0001 C CNN
F 1 "GND" H 5105 5977 50  0000 C CNN
F 2 "" H 5100 6150 50  0001 C CNN
F 3 "" H 5100 6150 50  0001 C CNN
	1    5100 6150
	1    0    0    -1  
$EndComp
Wire Wire Line
	5100 5650 5100 5950
Wire Wire Line
	3650 3600 4100 3600
Wire Wire Line
	3650 3700 4100 3700
Wire Wire Line
	3650 3900 4100 3900
Wire Wire Line
	3650 4000 4100 4000
Wire Wire Line
	3650 4100 4100 4100
Wire Wire Line
	3650 4200 4100 4200
Wire Wire Line
	3650 4300 4100 4300
Text Label 3650 3600 0    50   ~ 0
IO0
Text Label 3650 3700 0    50   ~ 0
RXD
Text Label 3650 3900 0    50   ~ 0
A
Text Label 3650 4000 0    50   ~ 0
B
Text Label 3650 4100 0    50   ~ 0
VIN
Text Label 3650 4200 0    50   ~ 0
TXD
Text Label 3650 4300 0    50   ~ 0
RESET
Wire Wire Line
	4450 3800 3650 3800
$Comp
L power:GND #PWR0101
U 1 1 5D60A8F7
P 4450 4300
F 0 "#PWR0101" H 4450 4050 50  0001 C CNN
F 1 "GND" H 4455 4127 50  0000 C CNN
F 2 "" H 4450 4300 50  0001 C CNN
F 3 "" H 4450 4300 50  0001 C CNN
	1    4450 4300
	1    0    0    -1  
$EndComp
Wire Wire Line
	4450 3800 4450 4300
Wire Wire Line
	4400 5350 5850 5350
$Comp
L Connector:USB_B_Micro J1
U 1 1 5D60E8E9
P 4100 5550
F 0 "J1" H 4157 6017 50  0000 C CNN
F 1 "USB_B_Micro" H 4157 5926 50  0000 C CNN
F 2 "lib:USB_Micro-B_Horizontal" H 4250 5500 50  0001 C CNN
F 3 "~" H 4250 5500 50  0001 C CNN
	1    4100 5550
	1    0    0    -1  
$EndComp
NoConn ~ 4400 5750
Wire Wire Line
	4000 5950 4100 5950
Wire Wire Line
	4100 5950 5100 5950
Connection ~ 4100 5950
Connection ~ 5100 5950
Wire Wire Line
	5100 5950 5100 6150
Wire Wire Line
	4400 5550 5300 5550
Wire Wire Line
	5300 5550 5300 5850
Wire Wire Line
	5300 5850 5850 5850
Wire Wire Line
	4400 5650 4900 5650
Wire Wire Line
	4900 5650 4900 5750
Wire Wire Line
	4900 5750 5200 5750
Wire Wire Line
	5200 5750 5200 5950
Wire Wire Line
	5200 5950 5850 5950
Text Label 5300 5850 0    50   ~ 0
D+
Text Label 5200 5950 0    50   ~ 0
D-
$EndSCHEMATC

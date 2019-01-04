EESchema Schematic File Version 4
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
L Connector:RJ45 U1
U 1 1 5B9329F5
P 3400 5200
F 0 "U1" H 3878 5497 60  0000 L CNN
F 1 "RJ45" H 3878 5391 60  0000 L CNN
F 2 "Connector_RJ:RJ45_Amphenol_54602-x08_Horizontal" H 3450 5200 60  0001 C CNN
F 3 "" H 3450 5200 60  0001 C CNN
	1    3400 5200
	1    0    0    1   
$EndComp
$Comp
L Connector_Generic:Conn_01x04 J1
U 1 1 5B932CE2
P 5300 5300
F 0 "J1" H 5380 5292 50  0000 L CNN
F 1 "Conn_01x04" H 5380 5201 50  0000 L CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Vertical" H 5300 5300 50  0001 C CNN
F 3 "~" H 5300 5300 50  0001 C CNN
	1    5300 5300
	1    0    0    -1  
$EndComp
Wire Wire Line
	3800 5000 4200 5000
Wire Wire Line
	4200 5000 4200 5200
Wire Wire Line
	4200 5200 4600 5200
Wire Wire Line
	3800 5500 4600 5500
Wire Wire Line
	3800 5400 4500 5400
Wire Wire Line
	3950 5300 4350 5300
Wire Wire Line
	3800 4900 3800 5000
Connection ~ 3800 5000
Wire Wire Line
	3800 5100 3800 5200
Wire Wire Line
	3800 5200 3950 5200
Wire Wire Line
	3950 5200 3950 5300
Connection ~ 3800 5200
Wire Wire Line
	3800 5300 3800 5400
Connection ~ 3800 5400
Wire Wire Line
	3800 5500 3800 5600
Connection ~ 3800 5500
$Comp
L power:GND #PWR0101
U 1 1 5C00AFBF
P 4600 5600
F 0 "#PWR0101" H 4600 5350 50  0001 C CNN
F 1 "GND" H 4605 5427 50  0000 C CNN
F 2 "" H 4600 5600 50  0001 C CNN
F 3 "" H 4600 5600 50  0001 C CNN
	1    4600 5600
	1    0    0    -1  
$EndComp
Wire Wire Line
	4600 5500 4600 5600
Connection ~ 4600 5500
Wire Wire Line
	4600 5500 4750 5500
$Comp
L power:VCC #PWR0102
U 1 1 5C00B115
P 4600 5050
F 0 "#PWR0102" H 4600 4900 50  0001 C CNN
F 1 "VCC" H 4617 5223 50  0000 C CNN
F 2 "" H 4600 5050 50  0001 C CNN
F 3 "" H 4600 5050 50  0001 C CNN
	1    4600 5050
	1    0    0    -1  
$EndComp
Wire Wire Line
	4600 5050 4600 5200
Connection ~ 4600 5200
Wire Wire Line
	4600 5200 5100 5200
Text Label 4900 5400 0    50   ~ 0
A
Text Label 4800 5300 0    50   ~ 0
B
$Comp
L Connector:RJ45 U2
U 1 1 5C29B2FB
P 3400 6150
F 0 "U2" H 3878 6447 60  0000 L CNN
F 1 "RJ45" H 3878 6341 60  0000 L CNN
F 2 "Connector_RJ:RJ45_Amphenol_54602-x08_Horizontal" H 3450 6150 60  0001 C CNN
F 3 "" H 3450 6150 60  0001 C CNN
	1    3400 6150
	1    0    0    1   
$EndComp
Wire Wire Line
	4200 5200 4200 5850
Wire Wire Line
	4200 5850 3800 5850
Connection ~ 4200 5200
Wire Wire Line
	3800 5850 3800 5950
Connection ~ 3800 5850
Wire Wire Line
	4350 5300 4350 6050
Wire Wire Line
	4350 6050 3800 6050
Connection ~ 4350 5300
Wire Wire Line
	4350 5300 5100 5300
Wire Wire Line
	3800 6050 3800 6150
Connection ~ 3800 6050
Wire Wire Line
	4500 5400 4500 6250
Wire Wire Line
	4500 6250 3800 6250
Connection ~ 4500 5400
Wire Wire Line
	4500 5400 5100 5400
Wire Wire Line
	3800 6250 3800 6350
Connection ~ 3800 6250
Wire Wire Line
	4750 5500 4750 6450
Wire Wire Line
	4750 6450 3800 6450
Connection ~ 4750 5500
Wire Wire Line
	4750 5500 5100 5500
Connection ~ 3800 6450
Wire Wire Line
	3800 6450 3800 6550
$EndSCHEMATC

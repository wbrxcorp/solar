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
L Connector:RJ12 U1
U 1 1 5B9329F5
P 3400 5200
F 0 "U1" H 3400 5750 60  0000 L CNN
F 1 "RJ12" H 3350 4800 60  0000 L CNN
F 2 "Connector_RJ:RJ12_Amphenol_54601" H 3450 5200 60  0001 C CNN
F 3 "" H 3450 5200 60  0001 C CNN
	1    3400 5200
	1    0    0    1   
$EndComp
$Comp
L Connector_Generic:Conn_01x07 J1
U 1 1 5B932CE2
P 5300 5300
F 0 "J1" H 5380 5292 50  0000 L CNN
F 1 "Conn_01x07" H 5380 5201 50  0000 L CNN
F 2 "Connector_PinHeader_2.00mm:PinHeader_1x07_P2.00mm_Vertical" H 5300 5300 50  0001 C CNN
F 3 "~" H 5300 5300 50  0001 C CNN
	1    5300 5300
	1    0    0    -1  
$EndComp
Wire Wire Line
	3800 5200 5100 5200
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
$Comp
L power:VCC #PWR0102
U 1 1 5C00B115
P 4200 4750
F 0 "#PWR0102" H 4200 4600 50  0001 C CNN
F 1 "VCC" H 4217 4923 50  0000 C CNN
F 2 "" H 4200 4750 50  0001 C CNN
F 3 "" H 4200 4750 50  0001 C CNN
	1    4200 4750
	1    0    0    -1  
$EndComp
Wire Wire Line
	5100 5100 4200 5100
Wire Wire Line
	4600 5400 3800 5400
Wire Wire Line
	4600 5400 4600 5600
Wire Wire Line
	5100 5400 4600 5400
Connection ~ 4600 5400
Wire Wire Line
	3800 5300 5100 5300
Wire Wire Line
	4200 4750 4200 5100
Connection ~ 4200 5100
Wire Wire Line
	4200 5100 3800 5100
Wire Wire Line
	5100 5400 5100 5500
Connection ~ 5100 5400
Wire Wire Line
	3800 5000 5100 5000
Wire Wire Line
	3800 5500 4900 5500
Wire Wire Line
	4900 5500 4900 5600
Wire Wire Line
	4900 5600 5100 5600
Text Label 4550 5000 0    50   ~ 0
RESET
Text Label 4550 5100 0    50   ~ 0
VCC
Text Label 4550 5200 0    50   ~ 0
TXD
Text Label 4550 5300 0    50   ~ 0
RXD
Text Label 4200 5500 0    50   ~ 0
IO0
$EndSCHEMATC

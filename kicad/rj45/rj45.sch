EESchema Schematic File Version 4
LIBS:rj45-cache
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
L components:7810-8P8C U1
U 1 1 5B9329F5
P 4450 4650
F 0 "U1" H 4928 4947 60  0000 L CNN
F 1 "7810-8P8C" H 4928 4841 60  0000 L CNN
F 2 "7810-8P8C:7810-8P8C" H 4500 4650 60  0001 C CNN
F 3 "" H 4500 4650 60  0001 C CNN
	1    4450 4650
	1    0    0    -1  
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
	4100 4850 4200 4850
Wire Wire Line
	4200 4850 4200 5200
Wire Wire Line
	4200 5200 5100 5200
Connection ~ 4200 4850
Wire Wire Line
	4300 4850 4400 4850
Wire Wire Line
	4400 4850 4400 5300
Wire Wire Line
	4400 5300 5100 5300
Connection ~ 4400 4850
Wire Wire Line
	4500 4850 4600 4850
Wire Wire Line
	4600 4850 4600 5400
Wire Wire Line
	4600 5400 5100 5400
Connection ~ 4600 4850
Wire Wire Line
	4700 4850 4800 4850
Wire Wire Line
	4800 4850 4800 5500
Wire Wire Line
	4800 5500 5100 5500
Connection ~ 4800 4850
$EndSCHEMATC

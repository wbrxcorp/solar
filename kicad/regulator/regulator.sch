EESchema Schematic File Version 4
LIBS:regulator-cache
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
L power:GND #PWR0101
U 1 1 5C41249C
P 5200 4850
F 0 "#PWR0101" H 5200 4600 50  0001 C CNN
F 1 "GND" H 5205 4677 50  0000 C CNN
F 2 "" H 5200 4850 50  0001 C CNN
F 3 "" H 5200 4850 50  0001 C CNN
	1    5200 4850
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x02 J1
U 1 1 5C4124B5
P 6300 4200
F 0 "J1" H 6380 4192 50  0000 L CNN
F 1 "Conn_01x02" H 6380 4101 50  0000 L CNN
F 2 "TerminalBlock:TerminalBlock_bornier-2_P5.08mm" H 6300 4200 50  0001 C CNN
F 3 "~" H 6300 4200 50  0001 C CNN
	1    6300 4200
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x02 J2
U 1 1 5C4124D6
P 6300 4650
F 0 "J2" H 6380 4642 50  0000 L CNN
F 1 "Conn_01x02" H 6380 4551 50  0000 L CNN
F 2 "TerminalBlock:TerminalBlock_bornier-2_P5.08mm" H 6300 4650 50  0001 C CNN
F 3 "~" H 6300 4650 50  0001 C CNN
	1    6300 4650
	1    0    0    -1  
$EndComp
Wire Wire Line
	6100 4750 5200 4750
Wire Wire Line
	5200 4750 5200 4850
Wire Wire Line
	6100 4300 5200 4300
Wire Wire Line
	5200 4300 5200 4750
Connection ~ 5200 4750
NoConn ~ 6100 4200
NoConn ~ 6100 4650
$EndSCHEMATC

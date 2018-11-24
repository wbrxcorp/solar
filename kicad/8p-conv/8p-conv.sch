EESchema Schematic File Version 4
LIBS:8p-conv-cache
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
L Connector_Generic:Conn_01x08 J1
U 1 1 5BF90DDF
P 5100 2650
F 0 "J1" V 5317 2596 50  0000 C CNN
F 1 "Conn_01x08" V 5226 2596 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x08_P2.54mm_Vertical" H 5100 2650 50  0001 C CNN
F 3 "~" H 5100 2650 50  0001 C CNN
	1    5100 2650
	0    -1   -1   0   
$EndComp
$Comp
L Connector_Generic:Conn_01x08 J2
U 1 1 5BF90F0B
P 5100 3700
F 0 "J2" V 5224 3646 50  0000 C CNN
F 1 "Conn_01x08" V 5315 3646 50  0000 C CNN
F 2 "Connector_JST:JST_PH_S8B-PH-K_1x08_P2.00mm_Horizontal" H 5100 3700 50  0001 C CNN
F 3 "~" H 5100 3700 50  0001 C CNN
	1    5100 3700
	0    -1   1    0   
$EndComp
Wire Wire Line
	4900 3250 4900 3500
Wire Wire Line
	4800 2850 4800 3500
Text Label 5500 3450 1    50   ~ 0
RST
Text Label 5400 3450 1    50   ~ 0
TXD
Text Label 5300 3450 1    50   ~ 0
VIN
Text Label 5200 3400 1    50   ~ 0
B
Text Label 5100 3400 1    50   ~ 0
A
Text Label 5000 3450 1    50   ~ 0
GND
Text Label 4900 3450 1    50   ~ 0
RXD
Text Label 4800 3100 1    50   ~ 0
IO0
Wire Wire Line
	5200 3200 4900 3200
Wire Wire Line
	4900 3200 4900 2850
Wire Wire Line
	5200 3200 5200 3500
Wire Wire Line
	5100 3100 5000 3100
Wire Wire Line
	5000 3100 5000 2850
Wire Wire Line
	5100 3100 5100 3500
Wire Wire Line
	5300 3050 5100 3050
Wire Wire Line
	5100 3050 5100 2850
Wire Wire Line
	5300 3050 5300 3500
Wire Wire Line
	5000 3150 5200 3150
Wire Wire Line
	5200 3150 5200 2850
Wire Wire Line
	5000 3150 5000 3500
Wire Wire Line
	5500 3000 5300 3000
Wire Wire Line
	5300 3000 5300 2850
Wire Wire Line
	5500 3000 5500 3500
Wire Wire Line
	5400 2850 5400 3500
Wire Wire Line
	4900 3250 5450 3250
Wire Wire Line
	5450 3250 5450 2850
Wire Wire Line
	5450 2850 5500 2850
$EndSCHEMATC

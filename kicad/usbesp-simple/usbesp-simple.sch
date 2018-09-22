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
L Connector:USB_B_Micro J1
U 1 1 5BA66B76
P 3700 3500
F 0 "J1" H 3755 3967 50  0000 C CNN
F 1 "USB_B_Micro" H 3755 3876 50  0000 C CNN
F 2 "Connector_USB:USB_Micro-B_Molex_47346-0001" H 3850 3450 50  0001 C CNN
F 3 "~" H 3850 3450 50  0001 C CNN
	1    3700 3500
	1    0    0    -1  
$EndComp
$Comp
L Device:C C1
U 1 1 5BA5A511
P 4300 2650
F 0 "C1" H 4415 2696 50  0000 L CNN
F 1 "0.1uF" H 4415 2605 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 4338 2500 50  0001 C CNN
F 3 "~" H 4300 2650 50  0001 C CNN
	1    4300 2650
	1    0    0    -1  
$EndComp
$Comp
L Device:C C2
U 1 1 5BA5A5A4
P 4750 2650
F 0 "C2" H 4865 2696 50  0000 L CNN
F 1 "10uF" H 4865 2605 50  0000 L CNN
F 2 "Capacitor_SMD:C_1206_3216Metric_Pad1.42x1.75mm_HandSolder" H 4788 2500 50  0001 C CNN
F 3 "~" H 4750 2650 50  0001 C CNN
	1    4750 2650
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0101
U 1 1 5BA5AF2D
P 5450 4800
F 0 "#PWR0101" H 5450 4550 50  0001 C CNN
F 1 "GND" H 5455 4627 50  0000 C CNN
F 2 "" H 5450 4800 50  0001 C CNN
F 3 "" H 5450 4800 50  0001 C CNN
	1    5450 4800
	1    0    0    -1  
$EndComp
$Comp
L Interface_USB:FT230XS U1
U 1 1 5BA5EC2D
P 6250 3700
F 0 "U1" H 6250 4600 50  0000 C CNN
F 1 "FT230XS" H 6250 4500 50  0000 C CNN
F 2 "Package_SO:SSOP-16_3.9x4.9mm_P0.635mm" H 6250 3700 50  0001 C CNN
F 3 "http://www.ftdichip.com/Products/ICs/FT232RL.htm" H 6250 3700 50  0001 C CNN
	1    6250 3700
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x02 J3
U 1 1 5BA5F229
P 8150 4100
F 0 "J3" H 8230 4142 50  0000 L CNN
F 1 "Conn_01x02" H 8230 4051 50  0000 L CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x02_P2.54mm_Vertical" H 8150 4100 50  0001 C CNN
F 3 "~" H 8150 4100 50  0001 C CNN
	1    8150 4100
	1    0    0    -1  
$EndComp
Wire Wire Line
	6350 3000 6350 2900
$Comp
L Device:C C3
U 1 1 5BA5F7DD
P 4850 3550
F 0 "C3" H 4965 3596 50  0000 L CNN
F 1 "0.1uF" H 4965 3505 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 4888 3400 50  0001 C CNN
F 3 "~" H 4850 3550 50  0001 C CNN
	1    4850 3550
	1    0    0    -1  
$EndComp
Wire Wire Line
	4850 3300 5550 3300
Wire Wire Line
	4000 3300 4000 2400
Wire Wire Line
	4000 2400 4300 2400
Wire Wire Line
	4300 2500 4300 2400
Connection ~ 4300 2400
Wire Wire Line
	4300 2400 4750 2400
Wire Wire Line
	4750 2500 4750 2400
Connection ~ 4750 2400
Wire Wire Line
	4750 2400 6150 2400
Wire Wire Line
	4300 2800 4550 2800
Wire Wire Line
	4850 3700 4850 3900
Wire Wire Line
	4000 3500 4200 3500
Wire Wire Line
	4200 3500 4200 3700
NoConn ~ 4000 3700
Wire Wire Line
	3600 3900 3700 3900
Wire Wire Line
	3700 3900 4850 3900
Connection ~ 3700 3900
Connection ~ 4850 3900
Wire Wire Line
	4850 3900 4850 4800
Wire Wire Line
	4850 4800 5450 4800
Text Label 4000 2400 0    50   ~ 0
VBUS
Wire Wire Line
	6950 3400 7500 3400
Wire Wire Line
	7500 3650 7950 3650
Wire Wire Line
	6950 3300 7400 3300
Wire Wire Line
	7400 3300 7400 4200
Wire Wire Line
	7400 4200 7950 4200
Text Label 7150 3300 0    50   ~ 0
TXD
Text Label 7150 3400 0    50   ~ 0
RXD
Wire Wire Line
	5450 4700 5450 4800
Connection ~ 5450 4800
Wire Wire Line
	7300 4100 7300 4700
Wire Wire Line
	7500 3400 7500 3650
Connection ~ 5450 4700
Wire Wire Line
	6150 3000 6150 2400
Text Label 4350 3700 0    50   ~ 0
USBDP
Text Label 4500 3600 0    50   ~ 0
USBDM
Wire Wire Line
	5450 4700 7300 4700
Wire Wire Line
	4000 3600 5550 3600
Wire Wire Line
	5550 2900 6350 2900
Wire Wire Line
	5550 2900 5550 3300
Connection ~ 5550 3300
Wire Wire Line
	4850 3300 4850 3400
Wire Wire Line
	4200 3700 5550 3700
NoConn ~ 6950 3500
NoConn ~ 6950 3600
NoConn ~ 6950 3800
NoConn ~ 6950 3900
NoConn ~ 6950 4000
NoConn ~ 6950 4100
Wire Wire Line
	5450 4400 5450 4600
Wire Wire Line
	5450 4400 6150 4400
Wire Wire Line
	6350 4400 6150 4400
Connection ~ 6150 4400
Wire Wire Line
	4550 2800 4550 4600
Wire Wire Line
	4550 4600 5450 4600
Connection ~ 4550 2800
Wire Wire Line
	4550 2800 4750 2800
Connection ~ 5450 4600
Wire Wire Line
	5450 4600 5450 4700
Wire Wire Line
	7300 4100 7950 4100
NoConn ~ 5550 3900
$Comp
L Connector_Generic:Conn_01x02 J2
U 1 1 5BAA5695
P 8150 3550
F 0 "J2" H 8230 3542 50  0000 L CNN
F 1 "Conn_01x02" H 8230 3451 50  0000 L CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x02_P2.54mm_Vertical" H 8150 3550 50  0001 C CNN
F 3 "~" H 8150 3550 50  0001 C CNN
	1    8150 3550
	1    0    0    -1  
$EndComp
Wire Wire Line
	6150 2400 7950 2400
Wire Wire Line
	7950 2400 7950 3550
Connection ~ 6150 2400
Text Label 5150 3300 0    50   ~ 0
3V3
$EndSCHEMATC

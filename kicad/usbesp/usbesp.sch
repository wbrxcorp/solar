EESchema Schematic File Version 4
LIBS:usbesp-cache
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
L Transistor_BJT:BC818 Q1
U 1 1 5BA6719F
P 7550 5350
F 0 "Q1" H 7741 5396 50  0000 L CNN
F 1 "S9013" H 7741 5305 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 7750 5275 50  0001 L CIN
F 3 "https://www.fairchildsemi.com/datasheets/BC/BC818.pdf" H 7550 5350 50  0001 L CNN
	1    7550 5350
	1    0    0    -1  
$EndComp
$Comp
L Transistor_BJT:BC818 Q2
U 1 1 5BA6726F
P 7550 5800
F 0 "Q2" H 7741 5846 50  0000 L CNN
F 1 "S9013" H 7741 5755 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 7750 5725 50  0001 L CIN
F 3 "https://www.fairchildsemi.com/datasheets/BC/BC818.pdf" H 7550 5800 50  0001 L CNN
	1    7550 5800
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
L Connector_Generic:Conn_01x03 J2
U 1 1 5BA5AB51
P 8150 3500
F 0 "J2" H 8230 3542 50  0000 L CNN
F 1 "Conn_01x06" H 8230 3451 50  0000 L CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical" H 8150 3500 50  0001 C CNN
F 3 "~" H 8150 3500 50  0001 C CNN
	1    8150 3500
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
L Interface_USB:FT232RL U1
U 1 1 5BA5EC2D
P 6250 3700
F 0 "U1" H 6250 4878 50  0000 C CNN
F 1 "FT232RL" H 6250 4787 50  0000 C CNN
F 2 "Package_SO:SSOP-28_5.3x10.2mm_P0.65mm" H 6250 3700 50  0001 C CNN
F 3 "http://www.ftdichip.com/Products/ICs/FT232RL.htm" H 6250 3700 50  0001 C CNN
	1    6250 3700
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x03 J3
U 1 1 5BA5F229
P 8150 4100
F 0 "J3" H 8230 4142 50  0000 L CNN
F 1 "Conn_01x06" H 8230 4051 50  0000 L CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical" H 8150 4100 50  0001 C CNN
F 3 "~" H 8150 4100 50  0001 C CNN
	1    8150 4100
	1    0    0    -1  
$EndComp
Wire Wire Line
	6150 2700 6150 2650
Wire Wire Line
	6150 2650 5400 2650
Wire Wire Line
	5400 3000 5450 3000
$Comp
L Device:C C3
U 1 1 5BA5F7DD
P 4850 3150
F 0 "C3" H 4965 3196 50  0000 L CNN
F 1 "0.1uF" H 4965 3105 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 4888 3000 50  0001 C CNN
F 3 "~" H 4850 3150 50  0001 C CNN
	1    4850 3150
	1    0    0    -1  
$EndComp
Wire Wire Line
	4850 3000 5400 3000
Connection ~ 5400 3000
Wire Wire Line
	4850 3300 4850 3700
Text Label 5150 3000 0    50   ~ 0
3v3
Wire Wire Line
	4000 3300 4000 2400
Wire Wire Line
	4000 2400 4300 2400
Wire Wire Line
	7900 2400 7900 3400
Wire Wire Line
	7900 3400 7950 3400
Wire Wire Line
	5400 2950 5400 3000
Wire Wire Line
	5400 2650 5400 3000
Wire Wire Line
	4300 2500 4300 2400
Connection ~ 4300 2400
Wire Wire Line
	4300 2400 4750 2400
Wire Wire Line
	4750 2500 4750 2400
Connection ~ 4750 2400
Wire Wire Line
	4750 2400 6350 2400
Wire Wire Line
	4300 2800 4750 2800
Wire Wire Line
	4750 2800 4750 3700
Wire Wire Line
	4750 3700 4850 3700
Connection ~ 4750 2800
Connection ~ 4850 3700
Wire Wire Line
	4850 3700 4850 3900
Wire Wire Line
	4000 3500 4200 3500
Wire Wire Line
	4200 3500 4200 3300
Wire Wire Line
	4200 3300 5450 3300
Wire Wire Line
	4000 3600 4250 3600
Wire Wire Line
	4250 3600 4250 3400
Wire Wire Line
	4250 3400 5450 3400
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
	7050 3100 7500 3100
Wire Wire Line
	7500 4100 7950 4100
Wire Wire Line
	7050 3000 7400 3000
Wire Wire Line
	7400 3000 7400 4200
Wire Wire Line
	7400 4200 7950 4200
Text Label 7150 3000 0    50   ~ 0
TXD
Text Label 7150 3100 0    50   ~ 0
RXD
Wire Wire Line
	6050 4700 6250 4700
Wire Wire Line
	6250 4700 6350 4700
Connection ~ 6250 4700
Wire Wire Line
	6350 4700 6450 4700
Connection ~ 6350 4700
Wire Wire Line
	6050 4700 5450 4700
Wire Wire Line
	5450 4700 5450 4800
Connection ~ 6050 4700
Connection ~ 5450 4800
Wire Wire Line
	7300 4000 7300 4700
Wire Wire Line
	7300 4700 6450 4700
Connection ~ 6450 4700
Wire Wire Line
	7300 4000 7500 4000
Wire Wire Line
	7500 4000 7950 4000
Wire Wire Line
	7500 3100 7500 4100
NoConn ~ 5450 3700
NoConn ~ 5450 3900
NoConn ~ 5450 4100
NoConn ~ 7050 3300
NoConn ~ 7050 3500
NoConn ~ 7050 3600
NoConn ~ 7050 3700
NoConn ~ 7050 4000
NoConn ~ 7050 4100
NoConn ~ 7050 4200
NoConn ~ 7050 4300
NoConn ~ 7050 4400
Wire Wire Line
	5450 4400 5450 4700
Connection ~ 5450 4700
Wire Wire Line
	6350 2700 6350 2400
Connection ~ 6350 2400
Wire Wire Line
	6350 2400 7900 2400
$Comp
L Device:R R1
U 1 1 5BA71853
P 7000 5350
F 0 "R1" V 7207 5350 50  0000 C CNN
F 1 "10K" V 7116 5350 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" V 6930 5350 50  0001 C CNN
F 3 "~" H 7000 5350 50  0001 C CNN
	1    7000 5350
	0    -1   -1   0   
$EndComp
$Comp
L Device:R R2
U 1 1 5BA71955
P 7000 5800
F 0 "R2" V 7207 5800 50  0000 C CNN
F 1 "10K" V 7116 5800 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" V 6930 5800 50  0001 C CNN
F 3 "~" H 7000 5800 50  0001 C CNN
	1    7000 5800
	0    -1   -1   0   
$EndComp
Wire Wire Line
	7050 3200 7150 3200
Wire Wire Line
	7150 3200 7150 5000
Wire Wire Line
	7150 5000 6700 5000
Wire Wire Line
	6700 5000 6700 5350
Wire Wire Line
	6700 5350 6800 5350
Wire Wire Line
	7050 3400 7200 3400
Wire Wire Line
	7200 3400 7200 4900
Wire Wire Line
	7200 4900 6600 4900
Wire Wire Line
	6600 4900 6600 5550
Wire Wire Line
	6600 5800 6850 5800
Text Label 7100 3200 0    50   ~ 0
RTS
Text Label 7200 3400 0    50   ~ 0
DTR
Wire Wire Line
	7650 5150 7650 3600
Wire Wire Line
	7650 5600 7850 5600
Wire Wire Line
	7850 5600 7850 3500
Wire Wire Line
	7850 3500 7950 3500
Wire Wire Line
	7650 5550 6600 5550
Wire Wire Line
	7150 5800 7350 5800
Wire Wire Line
	7650 6000 6800 6000
Wire Wire Line
	6800 6000 6800 5350
Text Label 7650 3750 0    50   ~ 0
IO0
Text Label 7850 3700 0    50   ~ 0
RESET
Wire Wire Line
	7650 3600 7950 3600
Text Label 4350 3300 0    50   ~ 0
USBDP
Text Label 4400 3400 0    50   ~ 0
USBDM
Wire Wire Line
	7150 5350 7350 5350
Connection ~ 6800 5350
Wire Wire Line
	6800 5350 6850 5350
Connection ~ 6600 5550
Wire Wire Line
	6600 5550 6600 5800
$EndSCHEMATC

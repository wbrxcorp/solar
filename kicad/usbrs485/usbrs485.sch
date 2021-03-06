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
F 2 "Connector_USB:USB_Micro-B_Amphenol_10103594-0001LF_Horizontal" H 3850 3450 50  0001 C CNN
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
	7050 3000 7400 3000
Wire Wire Line
	7400 3000 7400 3850
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
	7500 3100 7500 3550
NoConn ~ 5450 3700
NoConn ~ 5450 3900
NoConn ~ 5450 4100
NoConn ~ 7050 3300
NoConn ~ 7050 3500
NoConn ~ 7050 3600
NoConn ~ 7050 3700
NoConn ~ 7050 4000
NoConn ~ 7050 4100
NoConn ~ 7050 4300
NoConn ~ 7050 4400
Wire Wire Line
	5450 4400 5450 4700
Connection ~ 5450 4700
Wire Wire Line
	6350 2700 6350 2400
Text Label 4350 3300 0    50   ~ 0
USBDP
Text Label 4400 3400 0    50   ~ 0
USBDM
$Comp
L Connector:RJ45 J2
U 1 1 5C2EBD98
P 9500 3650
F 0 "J2" H 9170 3654 50  0000 R CNN
F 1 "RJ45" H 9170 3745 50  0000 R CNN
F 2 "Connector_RJ:RJ45_Amphenol_54602-x08_Horizontal" V 9500 3675 50  0001 C CNN
F 3 "~" V 9500 3675 50  0001 C CNN
	1    9500 3650
	-1   0    0    1   
$EndComp
NoConn ~ 9100 3350
NoConn ~ 9100 3450
Wire Wire Line
	9100 3550 9100 3650
Wire Wire Line
	9100 3950 9100 4050
$Comp
L power:GND #PWR0102
U 1 1 5C2F4564
P 9100 4250
F 0 "#PWR0102" H 9100 4000 50  0001 C CNN
F 1 "GND" H 9105 4077 50  0000 C CNN
F 2 "" H 9100 4250 50  0001 C CNN
F 3 "" H 9100 4250 50  0001 C CNN
	1    9100 4250
	1    0    0    -1  
$EndComp
NoConn ~ 7050 3200
$Comp
L Interface_UART:MAX3485 U2
U 1 1 5C300CA3
P 8150 3650
F 0 "U2" H 8150 4328 50  0000 C CNN
F 1 "MAX3485" H 8150 4237 50  0000 C CNN
F 2 "Package_SO:SOIC-8_3.9x4.9mm_P1.27mm" H 8150 2950 50  0001 C CNN
F 3 "https://datasheets.maximintegrated.com/en/ds/MAX3483-MAX3491.pdf" H 8150 3700 50  0001 C CNN
	1    8150 3650
	1    0    0    -1  
$EndComp
Connection ~ 9100 3550
Wire Wire Line
	8550 3850 8600 3850
Wire Wire Line
	9100 3850 9100 3750
Connection ~ 9100 3850
Wire Wire Line
	7750 3850 7400 3850
Wire Wire Line
	7750 3550 7500 3550
Wire Wire Line
	7050 4200 7600 4200
Wire Wire Line
	7600 4200 7600 3750
Wire Wire Line
	7600 3750 7750 3750
Wire Wire Line
	7750 3650 7750 3750
Connection ~ 7750 3750
Wire Wire Line
	6150 2650 8150 2650
Wire Wire Line
	8150 2650 8150 3150
Connection ~ 6150 2650
Wire Wire Line
	8150 4250 8950 4250
Wire Wire Line
	9100 4250 9100 4050
Connection ~ 9100 4250
Connection ~ 9100 4050
Text Label 8800 3550 0    50   ~ 0
B
Text Label 8800 3850 0    50   ~ 0
A
$Comp
L Device:R R1
U 1 1 5C309C79
P 8600 2800
F 0 "R1" H 8670 2846 50  0000 L CNN
F 1 "20K" H 8670 2755 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.15x1.40mm_HandSolder" V 8530 2800 50  0001 C CNN
F 3 "~" H 8600 2800 50  0001 C CNN
	1    8600 2800
	1    0    0    -1  
$EndComp
Wire Wire Line
	8600 2650 8150 2650
Connection ~ 8150 2650
Wire Wire Line
	8600 2950 8600 3850
Connection ~ 8600 3850
Wire Wire Line
	8600 3850 9100 3850
$Comp
L Device:R R2
U 1 1 5C30BD19
P 8950 4100
F 0 "R2" H 9020 4146 50  0000 L CNN
F 1 "20K" H 9020 4055 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.15x1.40mm_HandSolder" V 8880 4100 50  0001 C CNN
F 3 "~" H 8950 4100 50  0001 C CNN
	1    8950 4100
	1    0    0    -1  
$EndComp
Connection ~ 8950 4250
Wire Wire Line
	8950 4250 9100 4250
Wire Wire Line
	8950 3950 8950 3550
Wire Wire Line
	8550 3550 8950 3550
Connection ~ 8950 3550
Wire Wire Line
	8950 3550 9100 3550
NoConn ~ 7050 3400
Text Label 7250 4200 0    50   ~ 0
DE
$EndSCHEMATC

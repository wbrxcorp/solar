EESchema Schematic File Version 4
LIBS:power-injector-cache
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
Wire Wire Line
	6800 2300 6800 2050
Wire Wire Line
	6800 2600 6800 2850
Wire Wire Line
	8150 2050 8150 2300
Wire Wire Line
	8150 2600 8150 2850
$Comp
L Connector_Generic:Conn_01x03 J10
U 1 1 5CBBD280
P 5500 5300
F 0 "J10" V 5464 5480 50  0000 L CNN
F 1 "DCDC" V 5373 5480 50  0000 L CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical" H 5500 5300 50  0001 C CNN
F 3 "~" H 5500 5300 50  0001 C CNN
	1    5500 5300
	0    -1   -1   0   
$EndComp
$Comp
L Connector:RJ45 J3
U 1 1 5CBB3D57
P 4300 4200
F 0 "J3" H 4357 4867 50  0000 C CNN
F 1 "RJ45" H 4357 4776 50  0000 C CNN
F 2 "lib:RJ45" V 4300 4225 50  0001 C CNN
F 3 "~" V 4300 4225 50  0001 C CNN
	1    4300 4200
	1    0    0    -1  
$EndComp
Wire Wire Line
	4700 3800 4700 3900
Wire Wire Line
	4700 4000 4700 4100
Wire Wire Line
	4700 4200 4700 4300
Wire Wire Line
	4700 4400 4700 4500
Text Label 4700 4500 0    50   ~ 0
VCTRLR
Text Label 4700 4300 0    50   ~ 0
B
Wire Wire Line
	4700 3800 4900 3800
$Comp
L power:GND #PWR0109
U 1 1 5CBBE809
P 4900 3800
F 0 "#PWR0109" H 4900 3550 50  0001 C CNN
F 1 "GND" H 4905 3627 50  0000 C CNN
F 2 "" H 4900 3800 50  0001 C CNN
F 3 "" H 4900 3800 50  0001 C CNN
	1    4900 3800
	1    0    0    -1  
$EndComp
Text Label 4700 4100 0    50   ~ 0
A
$Comp
L Regulator_Linear:AMS1117-3.3 U2
U 1 1 5D56F2FA
P 7450 2050
F 0 "U2" H 7450 2292 50  0000 C CNN
F 1 "AMS1117-1.2" H 7450 2201 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-223-3_TabPin2" H 7450 2250 50  0001 C CNN
F 3 "http://www.advanced-monolithic.com/pdf/ds1117.pdf" H 7550 1800 50  0001 C CNN
	1    7450 2050
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_SPDT SW1
U 1 1 5D58070A
P 4050 5400
F 0 "SW1" H 4050 5685 50  0000 C CNN
F 1 "SW_SPDT" H 4050 5594 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical" H 4050 5400 50  0001 C CNN
F 3 "~" H 4050 5400 50  0001 C CNN
	1    4050 5400
	1    0    0    -1  
$EndComp
Wire Wire Line
	4250 5300 4600 5300
Text Label 4300 5300 0    50   ~ 0
VBATT_AUTO
Text Label 4350 5500 0    50   ~ 0
VBATT
Wire Wire Line
	3850 5400 3550 5400
Text Label 3550 5400 0    50   ~ 0
VBATT_IN
Wire Wire Line
	7150 2050 6800 2050
Text Label 6850 2050 0    50   ~ 0
VCTRLR
Wire Wire Line
	7750 2050 8150 2050
Text Label 7950 2050 0    50   ~ 0
1V2
$Comp
L power:GND #PWR0110
U 1 1 5D666BBA
P 7450 3000
F 0 "#PWR0110" H 7450 2750 50  0001 C CNN
F 1 "GND" H 7455 2827 50  0000 C CNN
F 2 "" H 7450 3000 50  0001 C CNN
F 3 "" H 7450 3000 50  0001 C CNN
	1    7450 3000
	1    0    0    -1  
$EndComp
$Comp
L Device:C C3
U 1 1 5D6C0A80
P 6800 2450
F 0 "C3" H 6915 2496 50  0000 L CNN
F 1 "0.1uF" H 6915 2405 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 6838 2300 50  0001 C CNN
F 3 "~" H 6800 2450 50  0001 C CNN
	1    6800 2450
	1    0    0    -1  
$EndComp
$Comp
L Device:C C4
U 1 1 5D6C1767
P 8150 2450
F 0 "C4" H 8265 2496 50  0000 L CNN
F 1 "0.1uF" H 8265 2405 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 8188 2300 50  0001 C CNN
F 3 "~" H 8150 2450 50  0001 C CNN
	1    8150 2450
	1    0    0    -1  
$EndComp
Wire Wire Line
	8150 2850 7450 2850
Wire Wire Line
	7450 2350 7450 2850
Connection ~ 7450 2850
Wire Wire Line
	7450 2850 7450 3000
Wire Wire Line
	7450 2850 6800 2850
Wire Wire Line
	6050 4200 4700 4200
Connection ~ 4700 4200
Wire Wire Line
	6050 4100 4700 4100
Connection ~ 4700 4100
Connection ~ 4700 3800
$Comp
L Connector:RJ45 J2
U 1 1 5DCE163B
P 6450 4200
F 0 "J2" H 6507 4867 50  0000 C CNN
F 1 "RJ45" H 6507 4776 50  0000 C CNN
F 2 "lib:RJ45" V 6450 4225 50  0001 C CNN
F 3 "~" V 6450 4225 50  0001 C CNN
	1    6450 4200
	-1   0    0    -1  
$EndComp
$Comp
L power:GND #PWR0103
U 1 1 5D7FC7AB
P 5500 6000
F 0 "#PWR0103" H 5500 5750 50  0001 C CNN
F 1 "GND" H 5505 5827 50  0000 C CNN
F 2 "" H 5500 6000 50  0001 C CNN
F 3 "" H 5500 6000 50  0001 C CNN
	1    5500 6000
	1    0    0    -1  
$EndComp
Wire Wire Line
	4900 3800 6050 3800
Connection ~ 4900 3800
Wire Wire Line
	6050 3800 6050 3900
Connection ~ 6050 3800
Wire Wire Line
	6050 4000 6050 4100
Connection ~ 6050 4100
Wire Wire Line
	6050 4200 6050 4300
Connection ~ 6050 4200
$Comp
L Connector:Barrel_Jack_Switch J1
U 1 1 5DD41354
P 3250 5500
F 0 "J1" H 3307 5817 50  0000 C CNN
F 1 "Barrel_Jack_Switch" H 3307 5726 50  0000 C CNN
F 2 "Connector_BarrelJack:BarrelJack_Horizontal" H 3300 5460 50  0001 C CNN
F 3 "~" H 3300 5460 50  0001 C CNN
	1    3250 5500
	1    0    0    -1  
$EndComp
Wire Wire Line
	3550 5500 3550 5600
$Comp
L Relay_SolidState:TLP222A U1
U 1 1 5DD4CCCC
P 9200 2150
F 0 "U1" H 9200 2475 50  0000 C CNN
F 1 "TLP222A" H 9200 2384 50  0000 C CNN
F 2 "Package_DIP:DIP-4_W7.62mm" H 9000 1950 50  0001 L CIN
F 3 "https://toshiba.semicon-storage.com/info/docget.jsp?did=17036&prodName=TLP222A" H 9150 2150 50  0001 L CNN
	1    9200 2150
	1    0    0    -1  
$EndComp
Wire Wire Line
	8150 2050 8900 2050
Connection ~ 8150 2050
Wire Wire Line
	8900 2250 8900 2850
Wire Wire Line
	8900 2850 8150 2850
Connection ~ 8150 2850
Wire Wire Line
	9500 2050 9900 2050
Text Label 9550 2250 0    50   ~ 0
VBATT_AUTO
Wire Wire Line
	9500 2250 9900 2250
Text Label 9600 2050 0    50   ~ 0
VBATT
Wire Wire Line
	5500 5500 5500 5800
Wire Wire Line
	4250 5500 5400 5500
Wire Wire Line
	3550 5600 3550 5800
Wire Wire Line
	3550 5800 5500 5800
Connection ~ 3550 5600
Connection ~ 5500 5800
Wire Wire Line
	5500 5800 5500 6000
Wire Wire Line
	6050 4400 6050 4500
Wire Wire Line
	6050 4500 6050 5500
Wire Wire Line
	6050 5500 5600 5500
Connection ~ 6050 4500
Text Label 6050 5050 0    50   ~ 0
5V
$EndSCHEMATC

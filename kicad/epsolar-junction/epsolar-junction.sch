EESchema Schematic File Version 4
LIBS:epsolar-junction-cache
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
	2950 1500 2650 1500
Text Label 2650 1500 0    50   ~ 0
3V3
Wire Wire Line
	4900 1800 5150 1800
Text Label 4900 1800 0    50   ~ 0
RESET
Wire Wire Line
	4900 2100 5150 2100
Wire Wire Line
	4900 2200 5150 2200
Text Label 4900 2100 0    50   ~ 0
TXD
Text Label 4900 2200 0    50   ~ 0
RXD
Wire Wire Line
	4900 1900 5150 1900
Text Label 4900 1900 0    50   ~ 0
SCL
Wire Wire Line
	4900 2300 5150 2300
Text Label 4900 2300 0    50   ~ 0
SDA
Wire Wire Line
	2950 2200 2650 2200
Text Label 2650 2200 0    50   ~ 0
IO0
Wire Wire Line
	2950 1700 2650 1700
Text Label 2650 1700 0    50   ~ 0
EDG_SW
Wire Wire Line
	2950 1800 2650 1800
Text Label 2650 1800 0    50   ~ 0
EDG_LED
$Comp
L Device:CP C1
U 1 1 5CBB73B0
P 6800 2450
F 0 "C1" H 6918 2496 50  0000 L CNN
F 1 "100uF" H 6918 2405 50  0000 L CNN
F 2 "Capacitor_THT:CP_Radial_D5.0mm_P2.50mm" H 6838 2300 50  0001 C CNN
F 3 "~" H 6800 2450 50  0001 C CNN
	1    6800 2450
	1    0    0    -1  
$EndComp
$Comp
L Device:CP C2
U 1 1 5CBB80F2
P 8150 2450
F 0 "C2" H 8268 2496 50  0000 L CNN
F 1 "1000uF" H 8268 2405 50  0000 L CNN
F 2 "Capacitor_THT:CP_Radial_D8.0mm_P3.50mm" H 8188 2300 50  0001 C CNN
F 3 "~" H 8150 2450 50  0001 C CNN
	1    8150 2450
	1    0    0    -1  
$EndComp
Wire Wire Line
	6800 2300 6800 2050
Text Label 6800 2200 0    50   ~ 0
VIN
Wire Wire Line
	6800 2600 6800 2850
Wire Wire Line
	8150 2050 8150 2300
Text Label 8150 2200 0    50   ~ 0
3V3
Wire Wire Line
	8150 2600 8150 2850
$Comp
L Connector_Generic:Conn_01x04 J11
U 1 1 5CBB0546
P 8450 3900
F 0 "J11" H 8368 4217 50  0000 C CNN
F 1 "DISPLAY" H 8368 4126 50  0000 C CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x04_P2.54mm_Vertical" H 8450 3900 50  0001 C CNN
F 3 "~" H 8450 3900 50  0001 C CNN
	1    8450 3900
	-1   0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x04 J12
U 1 1 5CBB1A42
P 8450 4600
F 0 "J12" H 8368 4917 50  0000 C CNN
F 1 "I2C" H 8368 4826 50  0000 C CNN
F 2 "Connector_JST:JST_XH_S4B-XH-A_1x04_P2.50mm_Horizontal" H 8450 4600 50  0001 C CNN
F 3 "~" H 8450 4600 50  0001 C CNN
	1    8450 4600
	-1   0    0    -1  
$EndComp
Wire Wire Line
	8650 3900 8900 3900
Wire Wire Line
	8650 4000 8900 4000
Wire Wire Line
	8650 4100 8900 4100
Wire Wire Line
	8650 4500 8900 4500
Wire Wire Line
	8650 4700 8900 4700
Wire Wire Line
	8650 4800 8900 4800
Text Label 8750 4500 0    50   ~ 0
3V3
$Comp
L power:GND #PWR0104
U 1 1 5CBB8014
P 9100 4600
F 0 "#PWR0104" H 9100 4350 50  0001 C CNN
F 1 "GND" H 9105 4427 50  0000 C CNN
F 2 "" H 9100 4600 50  0001 C CNN
F 3 "" H 9100 4600 50  0001 C CNN
	1    9100 4600
	1    0    0    -1  
$EndComp
Wire Wire Line
	8650 4600 9100 4600
Text Label 8750 4700 0    50   ~ 0
SCL
Text Label 8750 4800 0    50   ~ 0
SDA
$Comp
L Connector_Generic:Conn_01x03 J10
U 1 1 5CBBD280
P 7300 4550
F 0 "J10" V 7264 4730 50  0000 L CNN
F 1 "EDG" V 7173 4730 50  0000 L CNN
F 2 "Connector_JST:JST_XH_S3B-XH-A_1x03_P2.50mm_Horizontal" H 7300 4550 50  0001 C CNN
F 3 "~" H 7300 4550 50  0001 C CNN
	1    7300 4550
	-1   0    0    -1  
$EndComp
Wire Wire Line
	7500 4450 7900 4450
Wire Wire Line
	7500 4550 7900 4550
Wire Wire Line
	7500 4650 7900 4650
$Comp
L power:GND #PWR0105
U 1 1 5CBC14C2
P 7900 4650
F 0 "#PWR0105" H 7900 4400 50  0001 C CNN
F 1 "GND" H 7905 4477 50  0000 C CNN
F 2 "" H 7900 4650 50  0001 C CNN
F 3 "" H 7900 4650 50  0001 C CNN
	1    7900 4650
	1    0    0    -1  
$EndComp
Text Label 7600 4450 0    50   ~ 0
EDG_SW
Text Label 7600 4550 0    50   ~ 0
EDG_LED
Wire Wire Line
	9100 4600 9100 3800
Wire Wire Line
	9100 3800 8650 3800
Connection ~ 9100 4600
Text Label 8750 3900 0    50   ~ 0
3V3
Text Label 8750 4000 0    50   ~ 0
SCL
Text Label 8750 4100 0    50   ~ 0
SDA
$Comp
L Switch:SW_Push SW1
U 1 1 5CBC6079
P 9550 3000
F 0 "SW1" H 9550 3285 50  0000 C CNN
F 1 "SW_Push" H 9550 3194 50  0000 C CNN
F 2 "lib:tactile_switch_smd_4x3mm_2p" H 9550 3200 50  0001 C CNN
F 3 "~" H 9550 3200 50  0001 C CNN
	1    9550 3000
	1    0    0    -1  
$EndComp
Wire Wire Line
	9350 3000 9100 3000
Wire Wire Line
	9750 3350 10000 3350
$Comp
L power:GND #PWR0106
U 1 1 5CBCB33F
P 10000 3350
F 0 "#PWR0106" H 10000 3100 50  0001 C CNN
F 1 "GND" H 10005 3177 50  0000 C CNN
F 2 "" H 10000 3350 50  0001 C CNN
F 3 "" H 10000 3350 50  0001 C CNN
	1    10000 3350
	1    0    0    -1  
$EndComp
Text Label 9100 3000 0    50   ~ 0
RESET
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
L Connector:USB_B_Micro J13
U 1 1 5CBD3C27
P 4550 5450
F 0 "J13" H 4607 5917 50  0000 C CNN
F 1 "USB_B_Micro" H 4607 5826 50  0000 C CNN
F 2 "lib:USB_Micro-B_Horizontal" H 4700 5400 50  0001 C CNN
F 3 "~" H 4700 5400 50  0001 C CNN
	1    4550 5450
	1    0    0    -1  
$EndComp
Wire Wire Line
	5850 5350 5000 5350
Wire Wire Line
	5000 5350 5000 5250
Wire Wire Line
	5000 5250 4850 5250
NoConn ~ 4850 5650
Wire Wire Line
	4450 5850 4550 5850
Wire Wire Line
	4550 5850 5100 5850
Wire Wire Line
	5100 5850 5100 6150
Connection ~ 4550 5850
Wire Wire Line
	5100 5650 5100 5850
Wire Wire Line
	5100 5650 5850 5650
Connection ~ 5100 5850
Wire Wire Line
	4850 5450 5250 5450
Wire Wire Line
	5250 5450 5250 5850
Wire Wire Line
	5250 5850 5850 5850
Wire Wire Line
	4850 5550 5150 5550
Wire Wire Line
	5150 5550 5150 5950
Wire Wire Line
	5150 5950 5850 5950
Text Label 5650 5850 0    50   ~ 0
D+
Text Label 5650 5950 0    50   ~ 0
D-
Text Label 2650 1900 0    50   ~ 0
GPIO
$Comp
L Connector_Generic:Conn_01x02 J8
U 1 1 5CBB774B
P 1800 1900
F 0 "J8" H 1718 2117 50  0000 C CNN
F 1 "Conn_01x02" H 1718 2026 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x02_P2.54mm_Horizontal" H 1800 1900 50  0001 C CNN
F 3 "~" H 1800 1900 50  0001 C CNN
	1    1800 1900
	-1   0    0    -1  
$EndComp
Wire Wire Line
	2000 2000 2000 2200
$Comp
L power:GND #PWR0108
U 1 1 5CBBBE92
P 2000 2200
F 0 "#PWR0108" H 2000 1950 50  0001 C CNN
F 1 "GND" H 2005 2027 50  0000 C CNN
F 2 "" H 2000 2200 50  0001 C CNN
F 3 "" H 2000 2200 50  0001 C CNN
	1    2000 2200
	1    0    0    -1  
$EndComp
Wire Wire Line
	2000 1900 2950 1900
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
$Comp
L Switch:SW_Push SW2
U 1 1 5CBC0C02
P 9550 3350
F 0 "SW2" H 9550 3635 50  0000 C CNN
F 1 "SW_Push" H 9550 3544 50  0000 C CNN
F 2 "lib:tactile_switch_smd_4x3mm_2p" H 9550 3550 50  0001 C CNN
F 3 "~" H 9550 3550 50  0001 C CNN
	1    9550 3350
	1    0    0    -1  
$EndComp
Text Label 1650 5600 0    50   ~ 0
A
$Comp
L esp-wroom-02:ESP-13-WROOM-02 U1
U 1 1 5D5548E5
P 3950 1800
F 0 "U1" H 3925 2565 50  0000 C CNN
F 1 "ESP-13-WROOM-02" H 3925 2474 50  0000 C CNN
F 2 "lib:ESP-13-WROOM-02" H 3850 1800 50  0001 C CNN
F 3 "" H 3850 1800 50  0001 C CNN
	1    3950 1800
	1    0    0    -1  
$EndComp
$Comp
L Regulator_Linear:AMS1117-3.3 U2
U 1 1 5D56F2FA
P 7450 2050
F 0 "U2" H 7450 2292 50  0000 C CNN
F 1 "AMS1117-3.3" H 7450 2201 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-223-3_TabPin2" H 7450 2250 50  0001 C CNN
F 3 "http://www.advanced-monolithic.com/pdf/ds1117.pdf" H 7550 1800 50  0001 C CNN
	1    7450 2050
	1    0    0    -1  
$EndComp
$Comp
L Interface_UART:MAX3485 U3
U 1 1 5D5758B2
P 1850 4050
F 0 "U3" H 1850 4731 50  0000 C CNN
F 1 "MAX3485" H 1850 4640 50  0000 C CNN
F 2 "Package_SO:SOIC-8_3.9x4.9mm_P1.27mm" H 1850 3350 50  0001 C CNN
F 3 "https://datasheets.maximintegrated.com/en/ds/MAX3483-MAX3491.pdf" H 1850 4100 50  0001 C CNN
	1    1850 4050
	1    0    0    -1  
$EndComp
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
	7150 2050 6800 2050
Text Label 7150 2050 0    50   ~ 0
VIN
Wire Wire Line
	7750 2050 8150 2050
Text Label 7950 2050 0    50   ~ 0
3V3
Text Label 2100 3550 0    50   ~ 0
3V3
Wire Wire Line
	2250 4250 2400 4250
Text Label 2250 3950 0    50   ~ 0
B
Text Label 2250 4250 0    50   ~ 0
A
Wire Wire Line
	3950 2700 3950 2850
$Comp
L power:GND #PWR0101
U 1 1 5D5D73A5
P 3950 2900
F 0 "#PWR0101" H 3950 2650 50  0001 C CNN
F 1 "GND" H 3955 2727 50  0000 C CNN
F 2 "" H 3950 2900 50  0001 C CNN
F 3 "" H 3950 2900 50  0001 C CNN
	1    3950 2900
	1    0    0    -1  
$EndComp
Wire Wire Line
	5500 1500 5500 2000
$Comp
L power:GND #PWR0102
U 1 1 5D5F101A
P 5500 2100
F 0 "#PWR0102" H 5500 1850 50  0001 C CNN
F 1 "GND" H 5505 1927 50  0000 C CNN
F 2 "" H 5500 2100 50  0001 C CNN
F 3 "" H 5500 2100 50  0001 C CNN
	1    5500 2100
	1    0    0    -1  
$EndComp
Wire Wire Line
	4900 1500 5500 1500
Wire Wire Line
	2950 2300 2950 2850
Wire Wire Line
	2950 2850 3950 2850
Connection ~ 3950 2850
Wire Wire Line
	3950 2850 3950 2900
Wire Wire Line
	4900 2000 5500 2000
Connection ~ 5500 2000
Wire Wire Line
	5500 2000 5500 2100
Wire Wire Line
	4900 1600 5150 1600
Text Label 4900 1600 0    50   ~ 0
WAKE
Wire Wire Line
	4900 1700 5150 1700
Text Label 4900 1700 0    50   ~ 0
TOUT
Text Label 2650 1600 0    50   ~ 0
EN
Wire Wire Line
	2950 1600 2650 1600
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
Wire Wire Line
	1450 4050 1000 4050
Wire Wire Line
	1450 4150 1000 4150
Text Label 1000 4150 0    50   ~ 0
IO15
Text Label 1000 4050 0    50   ~ 0
IO2
Wire Wire Line
	1450 3950 900  3950
Wire Wire Line
	900  3950 900  4250
Wire Wire Line
	900  4250 1450 4250
Text Label 1000 3950 0    50   ~ 0
IO0
Wire Wire Line
	2950 2000 2650 2000
Text Label 2650 2000 0    50   ~ 0
IO15
Text Label 2650 2100 0    50   ~ 0
IO2
Wire Wire Line
	2650 2100 2950 2100
$Comp
L Device:C C3
U 1 1 5D6C0A80
P 6300 2450
F 0 "C3" H 6415 2496 50  0000 L CNN
F 1 "0.1uF" H 6415 2405 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 6338 2300 50  0001 C CNN
F 3 "~" H 6300 2450 50  0001 C CNN
	1    6300 2450
	1    0    0    -1  
$EndComp
$Comp
L Device:C C4
U 1 1 5D6C1767
P 8700 2450
F 0 "C4" H 8815 2496 50  0000 L CNN
F 1 "0.1uF" H 8815 2405 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 8738 2300 50  0001 C CNN
F 3 "~" H 8700 2450 50  0001 C CNN
	1    8700 2450
	1    0    0    -1  
$EndComp
Wire Wire Line
	8700 2300 8150 2300
Connection ~ 8150 2300
Wire Wire Line
	8700 2600 8150 2600
Connection ~ 8150 2600
Wire Wire Line
	6300 2300 6800 2300
Connection ~ 6800 2300
Wire Wire Line
	6300 2600 6800 2600
Connection ~ 6800 2600
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
	1850 4650 1850 4800
Wire Wire Line
	3000 3950 3000 4800
Wire Wire Line
	3000 5700 1650 5700
Wire Wire Line
	2250 3950 2400 3950
Connection ~ 1650 5700
Wire Wire Line
	2550 4250 2550 5600
Wire Wire Line
	2550 5600 1650 5600
Connection ~ 1650 5600
Connection ~ 1650 5300
Connection ~ 1850 5300
$Comp
L Device:R R2
U 1 1 5D753278
P 2550 3700
F 0 "R2" H 2620 3746 50  0000 L CNN
F 1 "20K" H 2620 3655 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.15x1.40mm_HandSolder" V 2480 3700 50  0001 C CNN
F 3 "~" H 2550 3700 50  0001 C CNN
	1    2550 3700
	1    0    0    -1  
$EndComp
$Comp
L Device:R R1
U 1 1 5D753768
P 2000 4800
F 0 "R1" V 2207 4800 50  0000 C CNN
F 1 "20K" V 2116 4800 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.15x1.40mm_HandSolder" V 1930 4800 50  0001 C CNN
F 3 "~" H 2000 4800 50  0001 C CNN
	1    2000 4800
	0    -1   -1   0   
$EndComp
Wire Wire Line
	2550 3550 1850 3550
Wire Wire Line
	2550 3850 2550 4250
Connection ~ 2550 4250
Connection ~ 1850 4800
Wire Wire Line
	1850 4800 1850 5300
Wire Wire Line
	2150 4800 3000 4800
Connection ~ 3000 4800
Wire Wire Line
	3000 4800 3000 5700
$Comp
L Device:R R3
U 1 1 5D76D3D6
P 2400 4100
F 0 "R3" H 2330 4054 50  0000 R CNN
F 1 "200" H 2330 4145 50  0000 R CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.15x1.40mm_HandSolder" V 2330 4100 50  0001 C CNN
F 3 "~" H 2400 4100 50  0001 C CNN
	1    2400 4100
	-1   0    0    1   
$EndComp
Connection ~ 2400 3950
Wire Wire Line
	2400 3950 3000 3950
Connection ~ 2400 4250
Wire Wire Line
	2400 4250 2550 4250
$Comp
L Device:R R4
U 1 1 5D794CB7
P 4450 3650
F 0 "R4" H 4520 3696 50  0000 L CNN
F 1 "10K" H 4520 3605 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" V 4380 3650 50  0001 C CNN
F 3 "~" H 4450 3650 50  0001 C CNN
	1    4450 3650
	1    0    0    -1  
$EndComp
$Comp
L Device:R R5
U 1 1 5D79513E
P 4700 3650
F 0 "R5" H 4770 3696 50  0000 L CNN
F 1 "10K" H 4770 3605 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" V 4630 3650 50  0001 C CNN
F 3 "~" H 4700 3650 50  0001 C CNN
	1    4700 3650
	1    0    0    -1  
$EndComp
$Comp
L Device:R R6
U 1 1 5D79588E
P 4950 3650
F 0 "R6" H 5020 3696 50  0000 L CNN
F 1 "10K" H 5020 3605 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" V 4880 3650 50  0001 C CNN
F 3 "~" H 4950 3650 50  0001 C CNN
	1    4950 3650
	1    0    0    -1  
$EndComp
$Comp
L Device:R R7
U 1 1 5D795F2B
P 5200 3650
F 0 "R7" H 5270 3696 50  0000 L CNN
F 1 "10K" H 5270 3605 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" V 5130 3650 50  0001 C CNN
F 3 "~" H 5200 3650 50  0001 C CNN
	1    5200 3650
	1    0    0    -1  
$EndComp
$Comp
L Device:R R9
U 1 1 5D797AF7
P 5700 3650
F 0 "R9" H 5770 3696 50  0000 L CNN
F 1 "10K" H 5770 3605 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" V 5630 3650 50  0001 C CNN
F 3 "~" H 5700 3650 50  0001 C CNN
	1    5700 3650
	1    0    0    -1  
$EndComp
Wire Wire Line
	4450 3500 4700 3500
Connection ~ 4700 3500
Wire Wire Line
	4700 3500 4950 3500
Wire Wire Line
	4950 4050 4950 3800
Wire Wire Line
	4950 3500 5200 3500
Connection ~ 4950 3500
$Comp
L Device:R R8
U 1 1 5D7B0F49
P 5450 3650
F 0 "R8" H 5520 3696 50  0000 L CNN
F 1 "470" H 5520 3605 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.15x1.40mm_HandSolder" V 5380 3650 50  0001 C CNN
F 3 "~" H 5450 3650 50  0001 C CNN
	1    5450 3650
	1    0    0    -1  
$EndComp
Wire Wire Line
	4700 4050 4700 3800
Wire Wire Line
	4450 4050 4450 3800
Text Label 4450 4050 0    50   ~ 0
EN
Text Label 4700 4050 0    50   ~ 0
IO0
Text Label 4950 4050 0    50   ~ 0
IO2
Wire Wire Line
	5200 3800 5200 4050
Text Label 5200 4050 0    50   ~ 0
RESET
Wire Wire Line
	5700 3500 5700 3200
Text Label 5700 3300 0    50   ~ 0
IO15
Text Label 4200 3500 0    50   ~ 0
3V3
$Comp
L Device:C C5
U 1 1 5D7F337F
P 4450 4200
F 0 "C5" H 4565 4246 50  0000 L CNN
F 1 "0.1uF" H 4565 4155 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 4488 4050 50  0001 C CNN
F 3 "~" H 4450 4200 50  0001 C CNN
	1    4450 4200
	1    0    0    -1  
$EndComp
$Comp
L Device:C C6
U 1 1 5D7F3B87
P 5200 4200
F 0 "C6" H 5315 4246 50  0000 L CNN
F 1 "0.1uF" H 5315 4155 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 5238 4050 50  0001 C CNN
F 3 "~" H 5200 4200 50  0001 C CNN
	1    5200 4200
	1    0    0    -1  
$EndComp
Wire Wire Line
	5200 4350 4800 4350
Wire Wire Line
	4800 4350 4800 4450
Connection ~ 4800 4350
Wire Wire Line
	4800 4350 4450 4350
$Comp
L power:GND #PWR0103
U 1 1 5D7FC7AB
P 4800 4450
F 0 "#PWR0103" H 4800 4200 50  0001 C CNN
F 1 "GND" H 4805 4277 50  0000 C CNN
F 2 "" H 4800 4450 50  0001 C CNN
F 3 "" H 4800 4450 50  0001 C CNN
	1    4800 4450
	1    0    0    -1  
$EndComp
Wire Wire Line
	5450 3500 5450 3200
Text Label 5450 3300 0    50   ~ 0
WAKE
Wire Wire Line
	5450 3800 5200 3800
Connection ~ 5200 3800
Wire Wire Line
	5700 3800 5700 4350
Wire Wire Line
	5700 4350 5200 4350
Connection ~ 5200 4350
Wire Wire Line
	4450 3500 4200 3500
Connection ~ 4450 3500
NoConn ~ 5150 1700
Wire Wire Line
	9350 3350 9100 3350
Text Label 9100 3350 0    50   ~ 0
IO0
Wire Wire Line
	9750 3000 10000 3000
Wire Wire Line
	10000 3000 10000 3350
Connection ~ 10000 3350
$EndSCHEMATC

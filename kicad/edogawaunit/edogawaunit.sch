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
L Relay_SolidState:TLP222A-2 U1
U 1 1 5BD18370
P 4250 3500
F 0 "U1" H 4250 3825 50  0000 C CNN
F 1 "TLP222A-2" H 4250 3734 50  0000 C CNN
F 2 "Package_DIP:DIP-8_W7.62mm" H 4050 3300 50  0001 L CIN
F 3 "https://toshiba.semicon-storage.com/info/docget.jsp?did=17036&prodName=TLP222A" H 4200 3500 50  0001 L CNN
	1    4250 3500
	1    0    0    -1  
$EndComp
$Comp
L Relay_SolidState:TLP222A-2 U1
U 2 1 5BD183ED
P 4250 4050
F 0 "U1" H 4250 3733 50  0000 C CNN
F 1 "TLP222A-2" H 4250 3824 50  0000 C CNN
F 2 "Package_DIP:DIP-8_W7.62mm" H 4050 3850 50  0001 L CIN
F 3 "https://toshiba.semicon-storage.com/info/docget.jsp?did=17036&prodName=TLP222A" H 4200 4050 50  0001 L CNN
	2    4250 4050
	-1   0    0    1   
$EndComp
$Comp
L Connector_Generic:Conn_01x02 J3
U 1 1 5BD1860D
P 5200 4100
F 0 "J3" H 5119 3775 50  0000 C CNN
F 1 "Conn_01x02" H 5119 3866 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x02_P2.54mm_Vertical" H 5200 4100 50  0001 C CNN
F 3 "~" H 5200 4100 50  0001 C CNN
	1    5200 4100
	1    0    0    1   
$EndComp
$Comp
L Connector_Generic:Conn_01x02 J2
U 1 1 5BD1865C
P 5200 3400
F 0 "J2" H 5280 3392 50  0000 L CNN
F 1 "Conn_01x02" H 5280 3301 50  0000 L CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x02_P2.54mm_Vertical" H 5200 3400 50  0001 C CNN
F 3 "~" H 5200 3400 50  0001 C CNN
	1    5200 3400
	1    0    0    -1  
$EndComp
$Comp
L Device:LED D1
U 1 1 5BD18731
P 4800 4450
F 0 "D1" H 4791 4666 50  0000 C CNN
F 1 "LED" H 4791 4575 50  0000 C CNN
F 2 "LED_THT:LED_D3.0mm" H 4800 4450 50  0001 C CNN
F 3 "~" H 4800 4450 50  0001 C CNN
	1    4800 4450
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x03 J1
U 1 1 5BD1888C
P 3050 3750
F 0 "J1" H 2970 4067 50  0000 C CNN
F 1 "Conn_01x03" H 2970 3976 50  0000 C CNN
F 2 "Connector_JST:JST_XH_B03B-XH-A_1x03_P2.50mm_Vertical" H 3050 3750 50  0001 C CNN
F 3 "~" H 3050 3750 50  0001 C CNN
	1    3050 3750
	-1   0    0    -1  
$EndComp
$Comp
L Device:R R1
U 1 1 5BD189D8
P 3600 3400
F 0 "R1" V 3393 3400 50  0000 C CNN
F 1 "330" V 3484 3400 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.15x1.40mm_HandSolder" V 3530 3400 50  0001 C CNN
F 3 "~" H 3600 3400 50  0001 C CNN
	1    3600 3400
	0    1    1    0   
$EndComp
Wire Wire Line
	3250 3650 3250 3400
Wire Wire Line
	3250 3400 3450 3400
Wire Wire Line
	3750 3400 3950 3400
Wire Wire Line
	3950 3600 3950 3850
Wire Wire Line
	3950 3850 3300 3850
$Comp
L power:GND #PWR0101
U 1 1 5BD18D3B
P 3300 4350
F 0 "#PWR0101" H 3300 4100 50  0001 C CNN
F 1 "GND" H 3305 4177 50  0000 C CNN
F 2 "" H 3300 4350 50  0001 C CNN
F 3 "" H 3300 4350 50  0001 C CNN
	1    3300 4350
	1    0    0    -1  
$EndComp
Wire Wire Line
	3300 4350 3300 4300
Connection ~ 3300 3850
Wire Wire Line
	3300 3850 3250 3850
Wire Wire Line
	3250 3750 3600 3750
Wire Wire Line
	3600 3750 3600 3950
Wire Wire Line
	3600 3950 3950 3950
Wire Wire Line
	3950 4150 3300 4150
Connection ~ 3300 4150
Wire Wire Line
	3300 4150 3300 3850
Wire Wire Line
	5000 4100 5000 4450
Wire Wire Line
	5000 4450 4950 4450
Wire Wire Line
	4650 4450 4650 4150
Wire Wire Line
	4650 4150 4550 4150
Wire Wire Line
	4550 3950 5000 3950
Wire Wire Line
	5000 3950 5000 4000
Wire Wire Line
	4550 3400 5000 3400
Wire Wire Line
	4550 3600 4600 3600
Wire Wire Line
	5000 3600 5000 3500
$Comp
L Switch:SW_Push SW1
U 1 1 5BD1A2EA
P 4800 3150
F 0 "SW1" H 4800 3435 50  0000 C CNN
F 1 "SW_Push" H 4800 3344 50  0000 C CNN
F 2 "Button_Switch_THT:SW_PUSH_6mm" H 4800 3350 50  0001 C CNN
F 3 "" H 4800 3350 50  0001 C CNN
	1    4800 3150
	1    0    0    -1  
$EndComp
Wire Wire Line
	5000 3400 5000 3150
Connection ~ 5000 3400
Wire Wire Line
	4600 3150 4600 3600
Connection ~ 4600 3600
Wire Wire Line
	4600 3600 5000 3600
$Comp
L power:PWR_FLAG #FLG0101
U 1 1 5BD1AA99
P 3150 4300
F 0 "#FLG0101" H 3150 4375 50  0001 C CNN
F 1 "PWR_FLAG" H 3150 4474 50  0000 C CNN
F 2 "" H 3150 4300 50  0001 C CNN
F 3 "~" H 3150 4300 50  0001 C CNN
	1    3150 4300
	1    0    0    -1  
$EndComp
Wire Wire Line
	3150 4300 3300 4300
Connection ~ 3300 4300
Wire Wire Line
	3300 4300 3300 4150
Text Label 3350 3400 0    50   ~ 0
SW_OUT
Text Label 3400 3750 0    50   ~ 0
LED_IN
Text Label 5000 4300 0    50   ~ 0
LEDPLUS
Text Label 4650 3950 0    50   ~ 0
LEDMINUS
Text Label 4800 3400 0    50   ~ 0
SW1
Text Label 4800 3600 0    50   ~ 0
SW2
$EndSCHEMATC

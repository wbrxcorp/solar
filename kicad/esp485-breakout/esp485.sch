EESchema Schematic File Version 4
LIBS:esp485-cache
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
L components:ESP-13-WROOM-02 U2
U 1 1 5B923A7D
P 5800 4050
F 0 "U2" H 5800 3950 50  0000 C CNN
F 1 "ESP-WROOM-02" H 5800 4500 50  0000 C CNN
F 2 "ESP8266:ESP-13-WROOM-02" H 5700 4050 50  0001 C CNN
F 3 "" H 5700 4050 50  0001 C CNN
	1    5800 4050
	1    0    0    -1  
$EndComp
$Comp
L Interface_UART:MAX3485 U1
U 1 1 5B923B04
P 3750 2500
F 0 "U1" H 3900 2450 60  0000 C CNN
F 1 "MAX3485" H 3750 2950 60  0000 C CNN
F 2 "Housings_SOIC:SOIC-8_3.9x4.9mm_Pitch1.27mm" H 3750 2500 60  0001 C CNN
F 3 "" H 3750 2500 60  0000 C CNN
	1    3750 2500
	1    0    0    -1  
$EndComp
$Comp
L Device:R R1
U 1 1 5B923C52
P 2750 3850
F 0 "R1" V 2830 3850 50  0000 C CNN
F 1 "10K" V 2750 3850 50  0000 C CNN
F 2 "Resistors_SMD:R_0805_HandSoldering" V 2680 3850 50  0001 C CNN
F 3 "" H 2750 3850 50  0001 C CNN
	1    2750 3850
	0    1    1    0   
$EndComp
$Comp
L Device:R R2
U 1 1 5B923CFF
P 2750 4250
F 0 "R2" V 2830 4250 50  0000 C CNN
F 1 "10K" V 2750 4250 50  0000 C CNN
F 2 "Resistors_SMD:R_0805_HandSoldering" V 2680 4250 50  0001 C CNN
F 3 "" H 2750 4250 50  0001 C CNN
	1    2750 4250
	0    1    1    0   
$EndComp
$Comp
L Device:R R3
U 1 1 5B923D91
P 2750 4400
F 0 "R3" V 2830 4400 50  0000 C CNN
F 1 "10K" V 2750 4400 50  0000 C CNN
F 2 "Resistors_SMD:R_0805_HandSoldering" V 2680 4400 50  0001 C CNN
F 3 "" H 2750 4400 50  0001 C CNN
	1    2750 4400
	0    1    1    0   
$EndComp
$Comp
L Device:R R7
U 1 1 5B924085
P 7250 2050
F 0 "R7" V 7330 2050 50  0000 C CNN
F 1 "10K" V 7250 2050 50  0000 C CNN
F 2 "Resistors_SMD:R_0805_HandSoldering" V 7180 2050 50  0001 C CNN
F 3 "" H 7250 2050 50  0001 C CNN
	1    7250 2050
	1    0    0    -1  
$EndComp
$Comp
L Device:R R9
U 1 1 5B92408B
P 4300 4200
F 0 "R9" V 4380 4200 50  0000 C CNN
F 1 "10K" V 4300 4200 50  0000 C CNN
F 2 "Resistors_SMD:R_0805_HandSoldering" V 4230 4200 50  0001 C CNN
F 3 "" H 4300 4200 50  0001 C CNN
	1    4300 4200
	0    1    1    0   
$EndComp
$Comp
L Device:R R4
U 1 1 5B9244E1
P 4600 2050
F 0 "R4" V 4680 2050 50  0000 C CNN
F 1 "20K" V 4600 2050 50  0000 C CNN
F 2 "Resistors_SMD:R_0805_HandSoldering" V 4530 2050 50  0001 C CNN
F 3 "" H 4600 2050 50  0001 C CNN
	1    4600 2050
	0    1    1    0   
$EndComp
$Comp
L Device:R R5
U 1 1 5B9244E7
P 4550 2650
F 0 "R5" V 4630 2650 50  0000 C CNN
F 1 "20K" V 4550 2650 50  0000 C CNN
F 2 "Resistors_SMD:R_0805_HandSoldering" V 4480 2650 50  0001 C CNN
F 3 "" H 4550 2650 50  0001 C CNN
	1    4550 2650
	0    1    1    0   
$EndComp
Text Notes 7950 7500 0    60   ~ 0
ESP485
$Comp
L Device:R R10
U 1 1 5B924936
P 5050 2800
F 0 "R10" V 5130 2800 50  0000 C CNN
F 1 "100" V 5050 2800 50  0000 C CNN
F 2 "Resistors_SMD:R_0805_HandSoldering" V 4980 2800 50  0001 C CNN
F 3 "" H 5050 2800 50  0001 C CNN
	1    5050 2800
	0    1    1    0   
$EndComp
$Comp
L power:+3.3V #PWR01
U 1 1 5B9252B0
P 4900 1900
F 0 "#PWR01" H 4900 1750 50  0001 C CNN
F 1 "+3.3V" H 4900 2040 50  0000 C CNN
F 2 "" H 4900 1900 50  0001 C CNN
F 3 "" H 4900 1900 50  0001 C CNN
	1    4900 1900
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR02
U 1 1 5B927F29
P 4150 4950
F 0 "#PWR02" H 4150 4700 50  0001 C CNN
F 1 "GND" H 4150 4800 50  0000 C CNN
F 2 "" H 4150 4950 50  0001 C CNN
F 3 "" H 4150 4950 50  0001 C CNN
	1    4150 4950
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR03
U 1 1 5B92A51D
P 7100 5250
F 0 "#PWR03" H 7100 5000 50  0001 C CNN
F 1 "GND" H 7100 5100 50  0000 C CNN
F 2 "" H 7100 5250 50  0001 C CNN
F 3 "" H 7100 5250 50  0001 C CNN
	1    7100 5250
	1    0    0    -1  
$EndComp
$Comp
L Device:R R8
U 1 1 5B92AAFC
P 6750 2050
F 0 "R8" V 6830 2050 50  0000 C CNN
F 1 "10K" V 6750 2050 50  0000 C CNN
F 2 "Resistors_SMD:R_0805_HandSoldering" V 6680 2050 50  0001 C CNN
F 3 "" H 6750 2050 50  0001 C CNN
	1    6750 2050
	1    0    0    -1  
$EndComp
Wire Wire Line
	4400 2200 4750 2200
Wire Wire Line
	4400 2500 5400 2500
Wire Wire Line
	4900 1900 4900 2200
Wire Wire Line
	4800 4350 2900 4250
Wire Wire Line
	4800 3850 2900 3850
Wire Wire Line
	3350 2500 3350 2600
Wire Wire Line
	2600 1900 2600 3700
Connection ~ 2600 4250
Wire Wire Line
	2600 1900 4900 1900
Connection ~ 2600 3850
Wire Wire Line
	4800 3750 2600 3700
Connection ~ 2600 3700
Wire Wire Line
	3350 2400 2900 2450
Wire Wire Line
	3350 2700 3150 2750
Wire Wire Line
	3150 2750 3150 4100
Wire Wire Line
	3150 4100 2900 4400
Wire Wire Line
	4400 2500 4400 2650
Wire Wire Line
	4700 2650 4700 2300
Connection ~ 4700 2300
Wire Wire Line
	4450 2400 4450 2050
Wire Wire Line
	4750 2050 4750 2200
Connection ~ 4750 2200
Connection ~ 4900 2200
Wire Wire Line
	4900 2400 4900 2800
Connection ~ 4900 2400
Connection ~ 4450 2400
Wire Wire Line
	5200 2800 5200 2300
Connection ~ 5200 2300
Wire Wire Line
	5400 2500 5400 3100
Wire Wire Line
	5400 3100 4150 3100
Wire Wire Line
	4150 3100 4150 4200
Wire Wire Line
	4150 4950 5800 4950
Wire Wire Line
	4800 4550 4150 4550
Connection ~ 4150 4550
Connection ~ 4900 1900
Connection ~ 4150 4200
Wire Wire Line
	4450 4200 4800 4250
Wire Wire Line
	7250 2200 7250 3550
Wire Wire Line
	7250 4050 6750 4050
Wire Wire Line
	7100 3750 7100 4250
Wire Wire Line
	6750 4250 7100 4250
Connection ~ 7100 4250
Wire Wire Line
	7700 4450 6750 4450
Wire Wire Line
	6750 4350 7750 4350
Wire Wire Line
	9200 3650 8050 4750
Wire Wire Line
	8050 4750 7100 4750
Connection ~ 7100 4750
Connection ~ 6750 1900
Wire Wire Line
	6750 2200 6750 3300
Wire Wire Line
	3350 2600 3300 2650
Wire Wire Line
	3300 2650 3300 3300
Wire Wire Line
	3300 3300 6750 3300
Connection ~ 6750 3300
Wire Wire Line
	6750 4150 7650 4150
Wire Wire Line
	7650 4150 7650 4000
Wire Wire Line
	7650 4000 9200 3950
Wire Wire Line
	6750 4550 7800 4550
Wire Wire Line
	7800 4550 7800 4100
Wire Wire Line
	7800 4100 9200 4250
Wire Wire Line
	2900 4450 3050 4450
Wire Wire Line
	2900 4400 2900 4450
Wire Wire Line
	9200 3850 8550 3550
Wire Wire Line
	8550 3550 7250 3550
Connection ~ 7250 3550
Wire Wire Line
	4800 3950 4700 3950
Wire Wire Line
	4700 3950 4700 3200
Wire Wire Line
	4800 4050 4550 4050
Wire Wire Line
	4550 4050 4550 3400
Wire Wire Line
	4550 3400 9200 2200
Wire Wire Line
	4800 4150 4550 4150
Wire Wire Line
	4550 4150 4550 5200
Wire Wire Line
	4550 5200 8750 5150
Wire Wire Line
	8700 2550 8600 4200
Wire Wire Line
	8600 4200 8600 5100
Wire Wire Line
	8600 5100 4450 5100
Wire Wire Line
	4450 5100 4450 4200
$Comp
L power:PWR_FLAG #FLG04
U 1 1 5B926509
P 3700 4800
F 0 "#FLG04" H 3700 4875 50  0001 C CNN
F 1 "PWR_FLAG" H 3700 4950 50  0000 C CNN
F 2 "" H 3700 4800 50  0001 C CNN
F 3 "" H 3700 4800 50  0001 C CNN
	1    3700 4800
	1    0    0    -1  
$EndComp
$Comp
L power:PWR_FLAG #FLG05
U 1 1 5B926539
P 6000 1700
F 0 "#FLG05" H 6000 1775 50  0001 C CNN
F 1 "PWR_FLAG" H 6000 1850 50  0000 C CNN
F 2 "" H 6000 1700 50  0001 C CNN
F 3 "" H 6000 1700 50  0001 C CNN
	1    6000 1700
	1    0    0    -1  
$EndComp
Wire Wire Line
	6000 1700 6000 1900
Connection ~ 6000 1900
Wire Wire Line
	3700 4800 4150 4800
Connection ~ 4150 4800
Wire Wire Line
	7750 4350 7750 3800
Wire Wire Line
	7750 3800 9200 4050
Wire Wire Line
	7700 4450 7700 3900
Wire Wire Line
	7700 3900 9200 4150
Wire Wire Line
	2600 4250 2600 4400
Wire Wire Line
	2600 3850 2600 4250
Wire Wire Line
	2600 3700 2600 3850
Wire Wire Line
	4700 2300 5200 2300
Wire Wire Line
	4750 2200 4900 2200
Wire Wire Line
	4900 2200 9200 2000
Wire Wire Line
	4900 2400 9200 2600
Wire Wire Line
	4450 2400 4900 2400
Wire Wire Line
	5200 2300 9200 2500
Wire Wire Line
	4150 4550 4150 4800
Wire Wire Line
	4900 1900 6000 1900
Wire Wire Line
	4150 4200 4150 4550
Wire Wire Line
	7100 4250 7100 4750
Wire Wire Line
	6750 3300 6750 3850
Wire Wire Line
	7250 3550 7250 4050
Wire Wire Line
	6000 1900 6750 1900
Wire Wire Line
	4150 4800 4150 4850
Connection ~ 3350 2600
Connection ~ 2900 4250
Wire Wire Line
	4400 2200 4400 2000
Wire Wire Line
	4400 2000 3750 2000
Wire Wire Line
	4400 2500 4350 2500
Wire Wire Line
	4350 2500 4350 3050
Wire Wire Line
	4350 3050 3950 3050
Wire Wire Line
	3950 3050 3950 3100
Wire Wire Line
	3950 3100 3750 3100
Connection ~ 4400 2500
Wire Wire Line
	4250 2300 4250 2400
Wire Wire Line
	4250 2400 4150 2400
Wire Wire Line
	4250 2300 4700 2300
Wire Wire Line
	4300 2400 4300 2700
Wire Wire Line
	4300 2700 4150 2700
Wire Wire Line
	4300 2400 4450 2400
Wire Wire Line
	2900 2450 2900 4250
Wire Wire Line
	6750 3750 7100 3750
Wire Wire Line
	6750 1900 7250 1900
$Comp
L Connector_Generic:Conn_01x07 J1
U 1 1 5B94B3AE
P 9400 2300
F 0 "J1" H 9480 2292 50  0000 L CNN
F 1 "Conn_01x07" H 9480 2201 50  0000 L CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x07_Pitch2.54mm" H 9400 2300 50  0001 C CNN
F 3 "~" H 9400 2300 50  0001 C CNN
	1    9400 2300
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x07 J2
U 1 1 5B94E662
P 9400 3950
F 0 "J2" H 9480 3942 50  0000 L CNN
F 1 "Conn_01x07" H 9480 3851 50  0000 L CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x07_Pitch2.54mm" H 9400 3950 50  0001 C CNN
F 3 "~" H 9400 3950 50  0001 C CNN
	1    9400 3950
	1    0    0    -1  
$EndComp
Wire Wire Line
	4700 3200 9200 2100
Wire Wire Line
	8800 5150 8750 5150
Wire Wire Line
	8800 5150 9000 2450
Wire Wire Line
	9000 2450 9200 2300
Wire Wire Line
	8700 2550 9200 2400
Wire Wire Line
	6750 3950 9200 3750
$Comp
L Switch:SW_Push SW1
U 1 1 5B9460EF
P 3250 4850
F 0 "SW1" H 3250 5135 50  0000 C CNN
F 1 "SW_Push" H 3250 5044 50  0000 C CNN
F 2 "tact:tactile_switch_smd_4x3mm_2p" H 3250 5050 50  0001 C CNN
F 3 "" H 3250 5050 50  0001 C CNN
	1    3250 4850
	1    0    0    -1  
$EndComp
Wire Wire Line
	3050 4850 3050 4450
Connection ~ 3050 4450
Wire Wire Line
	3450 4850 4150 4850
Connection ~ 4150 4850
Wire Wire Line
	4150 4850 4150 4950
$Comp
L Switch:SW_Push SW2
U 1 1 5B949D0E
P 6650 5550
F 0 "SW2" H 6650 5835 50  0000 C CNN
F 1 "SW_Push" H 6650 5744 50  0000 C CNN
F 2 "tact:tactile_switch_smd_4x3mm_2p" H 6650 5750 50  0001 C CNN
F 3 "" H 6650 5750 50  0001 C CNN
	1    6650 5550
	1    0    0    -1  
$EndComp
Wire Wire Line
	3050 4450 4800 4450
Wire Wire Line
	6850 5550 6850 5250
Wire Wire Line
	6850 5250 7100 5250
Connection ~ 7100 5250
Wire Wire Line
	6450 5550 6450 4900
Wire Wire Line
	6450 4900 7300 4900
Wire Wire Line
	7300 4900 7300 4050
Wire Wire Line
	7300 4050 7250 4050
Connection ~ 7250 4050
Wire Wire Line
	7100 4750 7100 5250
$EndSCHEMATC
EESchema Schematic File Version 4
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
$Comp
L Connector:Conn_01x04_Female J2
U 1 1 61749C09
P 6150 2800
F 0 "J2" H 6178 2776 50  0000 L CNN
F 1 "GPS (UART)" H 6178 2685 50  0000 L CNN
F 2 "Connector_PinHeader_1.00mm:PinHeader_1x04_P1.00mm_Horizontal" H 6150 2800 50  0001 C CNN
F 3 "~" H 6150 2800 50  0001 C CNN
	1    6150 2800
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x04_Female J1
U 1 1 61749D72
P 6150 2100
F 0 "J1" H 6178 2076 50  0000 L CNN
F 1 "IMU (I2C)" H 6178 1985 50  0000 L CNN
F 2 "Connector_PinHeader_1.00mm:PinHeader_1x04_P1.00mm_Horizontal" H 6150 2100 50  0001 C CNN
F 3 "~" H 6150 2100 50  0001 C CNN
	1    6150 2100
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x06_Female J4
U 1 1 6174B27E
P 6150 4650
F 0 "J4" H 6178 4626 50  0000 L CNN
F 1 "SD (SPI)" H 6178 4535 50  0000 L CNN
F 2 "Connector_PinHeader_1.00mm:PinHeader_1x06_P1.00mm_Horizontal" H 6150 4650 50  0001 C CNN
F 3 "~" H 6150 4650 50  0001 C CNN
	1    6150 4650
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x04_Female J3
U 1 1 6174C279
P 6150 3450
F 0 "J3" H 6178 3426 50  0000 L CNN
F 1 "BT (UART)" H 6178 3335 50  0000 L CNN
F 2 "Connector_PinHeader_1.00mm:PinHeader_1x04_P1.00mm_Horizontal" H 6150 3450 50  0001 C CNN
F 3 "~" H 6150 3450 50  0001 C CNN
	1    6150 3450
	1    0    0    -1  
$EndComp
Wire Wire Line
	5250 2900 5250 3800
Wire Wire Line
	2950 3800 2950 3600
Wire Wire Line
	5350 3000 5350 3900
Wire Wire Line
	3850 3600 3850 4550
Wire Wire Line
	3550 3600 3550 4450
Wire Wire Line
	3650 3600 3650 4650
Wire Wire Line
	3750 4750 3750 3600
Wire Wire Line
	4400 2000 4400 2700
Wire Wire Line
	4250 3000 4400 3000
Wire Wire Line
	4400 3000 4400 3450
Wire Wire Line
	4400 2700 4400 3000
Connection ~ 4400 2700
Connection ~ 4400 3000
Wire Wire Line
	4400 2000 5950 2000
Wire Wire Line
	4400 2700 5950 2700
Wire Wire Line
	4400 3450 5950 3450
Connection ~ 4400 3450
Wire Wire Line
	4400 3450 4400 4950
Wire Wire Line
	3550 4450 5950 4450
Wire Wire Line
	3850 4550 5950 4550
Wire Wire Line
	3650 4650 5950 4650
Wire Wire Line
	3750 4750 5950 4750
Wire Wire Line
	4400 4950 5950 4950
Wire Wire Line
	2850 3900 2850 3600
Wire Wire Line
	2150 2100 4500 2100
Wire Wire Line
	4500 4850 5950 4850
Connection ~ 4500 3350
Wire Wire Line
	4500 3350 4500 4850
Connection ~ 4500 2800
Wire Wire Line
	4500 2800 4500 3350
Connection ~ 4500 2100
Wire Wire Line
	4500 2100 4500 2800
Wire Wire Line
	4500 2100 5950 2100
Wire Wire Line
	4500 3350 5950 3350
Wire Wire Line
	4500 2800 5950 2800
Wire Wire Line
	2150 2100 2150 2900
$Comp
L MCU_Module:Arduino_UNO_R3 A1
U 1 1 61745C35
P 3150 3100
F 0 "A1" V 3196 1956 50  0000 R CNN
F 1 "Arduino_UNO_R3" V 3105 1956 50  0000 R CNN
F 2 "Module:Arduino_UNO_R3" H 3150 3100 50  0001 C CIN
F 3 "https://www.arduino.cc/en/Main/arduinoBoardUno" H 3150 3100 50  0001 C CNN
	1    3150 3100
	0    -1   -1   0   
$EndComp
Wire Wire Line
	5750 3550 5950 3550
Wire Wire Line
	5850 3650 5950 3650
Wire Wire Line
	3150 4150 5750 4150
Wire Wire Line
	3050 4250 5850 4250
Wire Wire Line
	5250 2900 5950 2900
Wire Wire Line
	5350 3000 5950 3000
Wire Wire Line
	2950 3800 5250 3800
Wire Wire Line
	2850 3900 5350 3900
Wire Wire Line
	3650 2600 3650 2450
Wire Wire Line
	4950 2350 4950 2200
Wire Wire Line
	5050 2450 5050 2300
Wire Wire Line
	3550 2350 3550 2600
Wire Wire Line
	5850 3650 5850 4250
Wire Wire Line
	5750 3550 5750 4150
Wire Wire Line
	3150 3600 3150 4150
Wire Wire Line
	3050 3600 3050 4250
Wire Wire Line
	3550 2350 4950 2350
Wire Wire Line
	3650 2450 5050 2450
Wire Wire Line
	4950 2200 5950 2200
Wire Wire Line
	5050 2300 5950 2300
Text Notes 5800 2000 0    50   ~ 0
GND\n
Text Notes 5800 2700 0    50   ~ 0
GND\n
Text Notes 5800 3450 0    50   ~ 0
GND\n
Text Notes 5800 4950 0    50   ~ 0
GND\n
Text Notes 5600 2100 0    50   ~ 0
VCC +5V\n
Text Notes 5600 2800 0    50   ~ 0
VCC +5V\n
Text Notes 5600 3350 0    50   ~ 0
VCC +5V\n
Text Notes 5650 4850 0    50   ~ 0
VCC +5V\n
Text Notes 5800 2200 0    50   ~ 0
SDA\n
Text Notes 5800 2300 0    50   ~ 0
SCL
Text Notes 5850 2900 0    50   ~ 0
TX\n
Text Notes 5850 3550 0    50   ~ 0
TX\n
Text Notes 5850 3650 0    50   ~ 0
RX
Text Notes 5850 3000 0    50   ~ 0
RX
Text Notes 5800 4550 0    50   ~ 0
SCK
Text Notes 5800 4650 0    50   ~ 0
MOSI
Text Notes 5800 4750 0    50   ~ 0
MISO
Text Notes 5850 4450 0    50   ~ 0
~CS
Text Notes 2750 2400 0    50   ~ 0
ECEN 4013 \nGROUP 1\nREV 1\n
$EndSCHEMATC

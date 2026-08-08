# Badminton Court Automation System

A team project for automating badminton court lighting and ventilation using ESP32, MQTT, and a Raspberry Pi server.

## System Overview

The system connects the badminton court booking system with ESP32 devices.

Web Application -> MariaDB -> Raspberry Pi -> Python -> MQTT / Mosquitto -> ESP32 -> Court Lights / Fans

## My Contributions

- Created the MariaDB database for storing booking information.
- Set up the Raspberry Pi server.
- Developed the Python program to check booking status.
- Implemented MQTT communication between the server and ESP32.
- Programmed the ESP32 to control 10 court lights and 5 fans.
- Integrated three DHT11 sensors for additional temperature-based fan control.

## Technologies

- Python
- C/C++
- ESP32
- MQTT
- Mosquitto
- MariaDB
- Linux
- Nginx
- DHT11

# Smart Home Fire and Gas Detection and fire extinguisher System

Here is the list of components for your project:-

ESP32 Microcontroller,
MQ-135 Gas Sensor,
Flame Sensor (IR Flame Sensor),
DC Water Pump (Fire Extinguisher),
Active Buzzer,
5V Relay Module,
I2C LCD 16x2 Display,
5V Adapter or USB Power Supply,
12V Power Supply (for water pump),
Jumper Wires and Connectors,
Breadboard

Circuit Connections:-

Gas Sensor: VCC to 3.3V, GND to GND, Analog OUT to GPIO34 (or any other analog pin) 

Flame Sensor: VCC to 3.3V, GND to GND, Digital OUT to GPIO35

Buzzer: Positive (+) to GPIO26, Negative (-) to GND
Relay Module (for Water Pump): Signal IN to GPIO27, VCC to 5V, GND to GND

I2C LCD: SDA to GPIO21, SCL to GPIO22, VCC to 3.3V, GND to GND

# M5StickS3 MQTT Counter

ESPHome firmware for the M5Stack M5StickS3 with the M5Stack Dual Button Unit using MQTT synchronized counters, PMIC power management, and BMI270 shake wake support.

Repository: https://github.com/slochewie/M5StickS3-MQTT-Counter

---

# Features

- ESPHome firmware for M5StickS3
- M5Stack Dual Button Unit support
- MQTT synchronized distributed counters
- Custom display component
- Custom M5PM1 power management component
- True PMIC hardware shutdown
- BMI270 shake wake support
- USB-aware power management
- Battery optimized runtime behavior
- OTA updates via ESPHome web server
- Battery voltage and percentage reporting
- Multi-device synchronized counters
- Long-press dual-button counter reset

---

# Hardware

## Required

- M5Stack M5StickS3
- M5Stack Dual Button Unit

## Optional

- Li-ion battery
- MQTT broker
- Node-RED
- Home Assistant

---

# MQTT Architecture

Multiple M5StickS3 devices can act as synchronized physical counters using MQTT topics.

text M5StickS3 Devices         |         v     MQTT Broker         |         v      Node-RED 

Each device:
- publishes increment/decrement/reset commands
- subscribes to counter state updates
- automatically stays synchronized with all other devices

---

# PMIC Power Management

This project includes a custom ESPHome external component for direct M5PM1 PMIC management.

Features include:

- true PMIC hardware power-off
- inactivity timeout handling
- display dimming and suspension
- USB-aware runtime behavior
- battery-aware shutdown logic
- WiFi shutdown before PMIC power-off
- Grove/5V rail monitoring
- battery voltage monitoring

## Runtime Behavior

### USB Connected

- device remains powered
- WiFi remains connected
- MQTT remains connected
- display suspends after inactivity
- button press restores display

### Battery Powered

- display dims after inactivity
- display suspends
- WiFi disconnects
- PMIC performs true hardware shutdown
- shake wake restores the device

---

# BMI270 Shake Wake

This project uses the SparkFun BMI270 Arduino Library for IMU wake configuration.

Features include:

- configurable ANY_MOTION threshold
- configurable motion duration filtering
- PMIC wake interrupt routing
- shake/motion wake from PMIC shutdown

Motion tuning values are stored in secrets.yaml:

yaml motion_threshold: "0x400" motion_duration: "0x64" 

---

# ESPHome Features

- ESPHome external components
- MQTT-only architecture
- no Home Assistant API required
- OTA updates through ESPHome web server
- custom display rendering
- battery percentage calculation
- configurable secrets-based deployment

---

# Example Use Cases

- occupancy counters
- venue capacity counters
- distributed synchronized tally counters
- event attendance tracking
- MQTT physical control panels

---

# Known Limitations

- battery percentage is estimated from voltage curve
- IMU shake sensitivity may still require tuning
- Grove port currently uses the 5V boost rail
- current display component is optimized for functionality over animation performance
- PMIC shutdown behavior may vary slightly while USB power is connected
- Dual Button Unit currently powered through Grove 5V

---

# Future Improvements

- common/shared YAML package structure
- multiple selectable counters on-device
- Remote-RED integration
- Zigbee scene integration
- 3.3V Dual Button power optimization
- additional wake gesture filtering
- improved display sleep behavior

---

# Setup

## ESPHome

Install ESPHome and clone this repository.

Copy:

text secrets.example.yaml 

to:

text secrets.yaml 

Then configure:
- WiFi
- MQTT credentials
- device name
- motion tuning values

Compile and upload with ESPHome.

---


# License

MIT License

Copyright (c) 2026

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
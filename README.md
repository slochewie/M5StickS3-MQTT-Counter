# M5StickS3 MQTT Counter

ESPHome firmware for the **M5Stack M5StickS3** with the **M5Stack Dual Button Unit** using MQTT synchronized counters, custom display handling, M5PM1 PMIC power management, BMI270 shake wake, USB-aware runtime behavior, and Quiet Charging Mode.

Repository: <https://github.com/slochewie/M5StickS3-MQTT-Counter>

---

## Features

- ESPHome firmware for M5StickS3
- M5Stack Dual Button Unit support
- MQTT synchronized distributed counters
- Custom display component
- Custom M5PM1 power management component
- True PMIC hardware shutdown
- BMI270 shake wake support
- USB-aware power management
- Quiet Charging Mode using Key2
- Optional PMIC boost / 5V rail disable during Quiet Charging Mode
- Battery optimized runtime behavior
- OTA updates through ESPHome web server
- Battery voltage and estimated battery percentage reporting
- Multi-device synchronized counters
- Long-press dual-button counter reset

---

## Hardware

### Required

- M5Stack M5StickS3
- M5Stack Dual Button Unit

### Optional

- Li-ion battery
- MQTT broker
- Node-RED
- Home Assistant

---

## Button Behavior

| Button | Action |
|---|---|
| Key1 | Request latest counter state over MQTT |
| Key2 | Toggle Quiet Charging Mode |
| Dual Button Blue | Increment counter |
| Dual Button Red | Decrement counter |
| Dual Button Blue + Red long press | Reset counter |

---

## MQTT Architecture

Multiple M5StickS3 devices can act as synchronized physical counters using MQTT topics.

```text
M5StickS3 Devices
        |
        v
   MQTT Broker
        |
        v
    Node-RED
```

Each device:

- publishes increment, decrement, and reset commands
- subscribes to counter state updates
- automatically stays synchronized with all other devices

---

## PMIC Power Management

This project includes a custom ESPHome external component for direct **M5PM1 PMIC** management.

Features include:

- true PMIC hardware power-off
- inactivity timeout handling
- display dimming and suspension
- USB-aware runtime behavior
- battery-aware shutdown logic
- WiFi shutdown before PMIC power-off
- Grove / 5V rail monitoring
- battery voltage monitoring
- PMIC boost / 5V rail control

---

## Runtime Behavior

### USB Connected

When USB power is detected using VIN:

- device remains powered
- WiFi remains connected
- MQTT remains connected
- display suspends after inactivity
- button press restores display
- PMIC shutdown is skipped so OTA/web access remains available

### Battery Powered

When USB/VIN is not present:

- display dims after inactivity
- display suspends
- WiFi disconnects
- PMIC performs true hardware shutdown
- BMI270 shake wake restores the device

---

## Quiet Charging Mode

Quiet Charging Mode is toggled with **Key2**.

Quiet Charging Mode is intended for charging or idle storage when you want the device to appear off and reduce activity without relying on PMIC shutdown while USB power is connected.

When entering Quiet Charging Mode:

- inactivity timer is stopped
- display is suspended
- backlight is turned off
- WiFi is disabled
- PMIC boost / 5V rail can be disabled

When exiting Quiet Charging Mode:

- PMIC boost / 5V rail is re-enabled
- WiFi is enabled
- display is resumed
- backlight is restored
- device requests the latest counter state over MQTT
- inactivity timer is restarted

### 5V Rail / Boost Behavior

The M5PM1 boost converter generates the 5V rail from the battery. This rail can power the Grove port and connected accessories.

Quiet Charging Mode can disable boost with:

```cpp
id(stick_power).disable_boost();
```

and re-enable it with:

```cpp
id(stick_power).enable_boost();
```

This may reduce idle power consumption, but if the Dual Button Unit is powered from the Grove 5V pin, disabling boost may also remove power from the Dual Button Unit until boost is re-enabled.

If the Dual Button Unit is later wired to 3.3V instead of Grove 5V, disabling boost becomes more useful for battery/runtime optimization.

---

## BMI270 Shake Wake

This project uses the **SparkFun BMI270 Arduino Library** for IMU wake configuration.

Features include:

- configurable ANY_MOTION threshold
- configurable motion duration filtering
- PMIC wake interrupt routing
- shake/motion wake from PMIC shutdown

Motion tuning values are stored in `secrets.yaml`:

```yaml
motion_threshold: "0x400"
motion_duration: "0x64"
```

Higher values reduce accidental wakeups.

---

## ESPHome Features

- ESPHome external components
- MQTT-only architecture
- no Home Assistant API required
- OTA updates through ESPHome web server
- custom display rendering
- battery percentage calculation
- configurable secrets-based deployment

---

## Example Use Cases

- occupancy counters
- venue capacity counters
- distributed synchronized tally counters
- event attendance tracking
- MQTT physical control panels

---

## Known Limitations

- battery percentage is estimated from voltage curve
- IMU shake sensitivity may still require tuning
- Grove port normally uses the 5V boost rail
- disabling boost may affect Grove-powered accessories
- current display component is optimized for functionality over animation performance
- PMIC shutdown behavior may vary while USB power is connected
- Dual Button Unit behavior while boost is disabled depends on how the unit is powered

---

## Future Improvements

- common/shared YAML package structure
- multiple selectable counters on-device
- Remote-RED integration
- Zigbee scene integration
- 3.3V Dual Button power optimization
- additional wake gesture filtering
- improved display sleep behavior
- more precise battery state-of-charge calculation

---

## Setup

Install ESPHome and clone this repository.

Copy:

```text
secrets.example.yaml
```

to:

```text
secrets.yaml
```

Then configure:

- WiFi
- MQTT credentials
- device name
- MQTT topics
- motion tuning values

Compile and upload with ESPHome.

---

## License

MIT License

Copyright (c) 2026

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include <Wire.h>
#include <M5PM1.h>
#include <M5Unified.h>
#include <SparkFun_BMI270_Arduino_Library.h>

namespace esphome {
namespace m5sticks3_power {

class M5StickS3Power : public PollingComponent {
 public:
  M5StickS3Power() : PollingComponent(60000) {}

  void setup() override;
  void update() override;

  void set_sda_pin(int pin) { sda_pin_ = pin; }
  void set_scl_pin(int pin) { scl_pin_ = pin; }
  void set_address(uint8_t address) { address_ = address; }
  void set_imu_address(uint8_t address) { imu_address_ = address; }
  void set_motion_threshold(uint16_t threshold) { motion_threshold_ = threshold; }
  void set_motion_duration(uint16_t duration) { motion_duration_ = duration; }

  void set_battery_sensor(sensor::Sensor *s) { battery_sensor_ = s; }
  void set_input_sensor(sensor::Sensor *s) { input_sensor_ = s; }
  void set_five_volt_sensor(sensor::Sensor *s) { five_volt_sensor_ = s; }

  void enter_pmic_shutdown();
  bool imu_test_read();

 protected:
  bool init_pmic_();
  bool init_m5_();
  bool init_bmi270_();
  bool configure_bmi270_motion_wake_();
  bool resync_pmic_();

  bool pmic_ready_{false};
  bool m5_ready_{false};
  bool bmi270_ready_{false};

  M5PM1 pm1_;
  BMI270 imu_;

  int sda_pin_{47};
  int scl_pin_{48};
  uint8_t address_{0x6E};
  uint8_t imu_address_{0x68};

  // BMI270 ANY_MOTION tuning.
  // threshold LSB is roughly 0.48 mg. duration LSB is roughly 20 ms.
  uint16_t motion_threshold_{0xC0};
  uint16_t motion_duration_{0x19};

  sensor::Sensor *battery_sensor_{nullptr};
  sensor::Sensor *input_sensor_{nullptr};
  sensor::Sensor *five_volt_sensor_{nullptr};
};

}  // namespace m5sticks3_power
}  // namespace esphome

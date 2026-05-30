#include "m5sticks3_power.h"
#include "esphome/core/log.h"

namespace esphome {
namespace m5sticks3_power {

static const char *TAG = "m5sticks3_power";

bool M5StickS3Power::init_pmic_() {
  Wire.begin(sda_pin_, scl_pin_, 100000);
  delay(20);

  m5pm1_err_t err = pm1_.begin(&Wire, address_, sda_pin_, scl_pin_, 100000);
  if (err != M5PM1_OK) {
    ESP_LOGE(TAG, "PMIC init failed: %d", err);
    return false;
  }

  delay(20);
  pm1_.setDcdcEnable(true);
  delay(20);
  pm1_.setLdoEnable(true);
  delay(20);
  pm1_.setChargeEnable(true);
  delay(20);
  pm1_.setBoostEnable(true);
  boost_enabled_ = true;
  delay(20);

  // Proven LCD/audio rail wake sequence from working StickS3 YAML.
  pm1_.gpioSetFunc(M5PM1_GPIO_NUM_2, M5PM1_GPIO_FUNC_GPIO);
  pm1_.gpioSetMode(M5PM1_GPIO_NUM_2, M5PM1_GPIO_MODE_OUTPUT);
  pm1_.gpioSetDrive(M5PM1_GPIO_NUM_2, M5PM1_GPIO_DRIVE_PUSHPULL);
  pm1_.gpioSetOutput(M5PM1_GPIO_NUM_2, false);
  delay(100);

  pmic_ready_ = true;
  ESP_LOGI(TAG, "PMIC init complete");
  return true;
}

bool M5StickS3Power::resync_pmic_() {
  Wire.end();
  delay(20);

  Wire.begin(sda_pin_, scl_pin_, 100000);
  delay(20);

  m5pm1_err_t err = pm1_.begin(&Wire, address_, sda_pin_, scl_pin_, 100000);
  if (err != M5PM1_OK) {
    ESP_LOGE(TAG, "PMIC re-init failed: %d", err);
    return false;
  }

  pmic_ready_ = true;
  delay(50);
  return true;
}

bool M5StickS3Power::init_m5_() {
  if (m5_ready_) return true;

  auto cfg = M5.config();
  cfg.serial_baudrate = 0;
  M5.begin(cfg);
  delay(100);

  m5_ready_ = true;
  ESP_LOGI(TAG, "M5Unified begin complete");
  return true;
}

bool M5StickS3Power::init_bmi270_() {
  if (bmi270_ready_) return true;

  // StickS3 IMU is expected at 0x68 on the same I2C bus.
  int8_t ret = imu_.beginI2C(imu_address_, Wire);
  if (ret != BMI2_OK) {
    ESP_LOGE(TAG, "SparkFun BMI270 beginI2C failed: %d", ret);
    return false;
  }

  bmi270_ready_ = true;
  ESP_LOGI(TAG, "SparkFun BMI270 beginI2C complete");
  return true;
}

void M5StickS3Power::setup() {
  init_pmic_();
}

void M5StickS3Power::update() {
  // M5PM1 can put its I2C interface to sleep. Re-sync before sensor reads,
  // and do one throwaway read before the real read to wake the ADC/read path.
  if (!resync_pmic_()) {
    ESP_LOGW(TAG, "Skipping PMIC sensor update: re-sync failed");
    return;
  }

  uint16_t mv = 0;
  m5pm1_err_t err;

  if (battery_sensor_ != nullptr) {
    pm1_.readVbat(&mv);
    delay(20);
    err = pm1_.readVbat(&mv);
    if (err == M5PM1_OK) {
      battery_sensor_->publish_state(mv / 1000.0f);
    } else {
      ESP_LOGW(TAG, "readVbat failed after re-sync: err=%d", err);
    }
  }

  if (input_sensor_ != nullptr) {
    pm1_.readVin(&mv);
    delay(20);
    err = pm1_.readVin(&mv);
    if (err == M5PM1_OK) {
      input_sensor_->publish_state(mv / 1000.0f);
    } else {
      ESP_LOGW(TAG, "readVin failed after re-sync: err=%d", err);
    }
  }

  if (five_volt_sensor_ != nullptr) {
    pm1_.read5VInOut(&mv);
    delay(20);
    err = pm1_.read5VInOut(&mv);
    if (err == M5PM1_OK) {
      five_volt_sensor_->publish_state(mv / 1000.0f);
    } else {
      ESP_LOGW(TAG, "read5VInOut failed after re-sync: err=%d", err);
    }
  }
}

bool M5StickS3Power::imu_test_read() {
  ESP_LOGW(TAG, "Starting M5Unified IMU read test");

  if (!init_m5_()) {
    return false;
  }

  M5.update();

  if (!M5.Imu.isEnabled()) {
    ESP_LOGE(TAG, "M5Unified reports IMU is NOT enabled");
    return false;
  }

  float ax = 0.0f;
  float ay = 0.0f;
  float az = 0.0f;

  if (!M5.Imu.getAccel(&ax, &ay, &az)) {
    ESP_LOGE(TAG, "M5.Imu.getAccel() failed");
    return false;
  }

  ESP_LOGW(TAG, "M5Unified IMU accel: ax=%.3f ay=%.3f az=%.3f", ax, ay, az);
  return true;
}

bool M5StickS3Power::configure_bmi270_motion_wake_() {
  ESP_LOGW(TAG, "Configuring BMI270 ANY_MOTION wake interrupt");

  if (!init_bmi270_()) {
    ESP_LOGE(TAG, "Cannot configure BMI270 wake: BMI270 init failed");
    return false;
  }

  int8_t ret = BMI2_OK;

  ret |= imu_.disableFeature(BMI2_ANY_MOTION);
  delay(20);

  bmi2_sens_config motion_cfg = {};
  motion_cfg.type = BMI2_ANY_MOTION;

  ret |= imu_.getConfig(&motion_cfg);

  motion_cfg.cfg.any_motion.threshold = motion_threshold_;
  motion_cfg.cfg.any_motion.duration = motion_duration_;
  motion_cfg.cfg.any_motion.select_x = BMI2_ENABLE;
  motion_cfg.cfg.any_motion.select_y = BMI2_ENABLE;
  motion_cfg.cfg.any_motion.select_z = BMI2_ENABLE;

  ret |= imu_.setConfig(motion_cfg);

  bmi2_int_pin_config int_pin_cfg = {};
  int_pin_cfg.pin_type = BMI2_INT1;
  int_pin_cfg.int_latch = BMI2_INT_NON_LATCH;
  int_pin_cfg.pin_cfg[0].lvl = BMI2_INT_ACTIVE_LOW;
  int_pin_cfg.pin_cfg[0].od = BMI2_INT_PUSH_PULL;
  int_pin_cfg.pin_cfg[0].output_en = BMI2_INT_OUTPUT_ENABLE;
  int_pin_cfg.pin_cfg[0].input_en = BMI2_INT_INPUT_DISABLE;

  ret |= imu_.setInterruptPinConfig(int_pin_cfg);
  ret |= imu_.mapInterruptToPin(BMI2_ANY_MOTION_INT, BMI2_INT1);
  ret |= imu_.enableFeature(BMI2_ANY_MOTION);

  if (ret != BMI2_OK) {
    ESP_LOGE(TAG, "BMI270 ANY_MOTION config failed: %d", ret);
    return false;
  }

  ESP_LOGW(TAG, "BMI270 ANY_MOTION configured threshold=0x%X duration=0x%X",
           motion_threshold_, motion_duration_);
  return true;
}

void M5StickS3Power::enable_boost() {
  ESP_LOGW(TAG, "Enabling PMIC boost / 5V rail");

  if (!resync_pmic_()) {
    ESP_LOGE(TAG, "Cannot enable boost: PMIC re-sync failed");
    return;
  }

  pm1_.setBoostEnable(true);
  boost_enabled_ = true;
  delay(50);
}

void M5StickS3Power::disable_boost() {
  ESP_LOGW(TAG, "Disabling PMIC boost / 5V rail");

  if (!resync_pmic_()) {
    ESP_LOGE(TAG, "Cannot disable boost: PMIC re-sync failed");
    return;
  }

  pm1_.setBoostEnable(false);
  boost_enabled_ = false;
  delay(50);
}

bool M5StickS3Power::is_boost_enabled() {
  return boost_enabled_;
}

void M5StickS3Power::enter_pmic_shutdown() {
  ESP_LOGW(TAG, "Preparing PMIC power-off with BMI270 shake wake");

  if (!resync_pmic_()) {
    ESP_LOGE(TAG, "Cannot enter PMIC power-off: PMIC re-sync failed");
    return;
  }

  if (!configure_bmi270_motion_wake_()) {
    ESP_LOGE(TAG, "BMI270 wake setup failed; falling back to PWR-button-only power-off");
  }

  ESP_LOGW(TAG, "Clearing PMIC IRQ flags");
  pm1_.irqClearGpioAll();
  pm1_.irqClearSysAll();
  pm1_.irqClearBtnAll();
  delay(50);

  // StickS3 IMU INT1 is connected to M5PM1 PYG4/GPIO4.
  pm1_.gpioSetMode(M5PM1_GPIO_NUM_4, M5PM1_GPIO_MODE_INPUT);
  pm1_.gpioSetPull(M5PM1_GPIO_NUM_4, M5PM1_GPIO_PULL_UP);
  pm1_.gpioSetWakeEnable(M5PM1_GPIO_NUM_4, true);
  pm1_.gpioSetWakeEdge(M5PM1_GPIO_NUM_4, M5PM1_GPIO_WAKE_FALLING);
  delay(50);

  // Keep L1/IMU rail alive so the BMI270 can assert INT1 while the rest
  // of the system is off.
  pm1_.setLdoEnable(true);
  delay(20);
  pm1_.ldoSetPowerHold(true);
  delay(20);
  pm1_.setLedEnLevel(true);
  delay(20);

  // Turn off LCD/audio rail.
  pm1_.gpioSetOutput(M5PM1_GPIO_NUM_2, true);
  delay(50);

  ESP_LOGW(TAG, "Requesting PMIC timer power-off in 1 second. Shake/move should wake.");
  delay(250);

  m5pm1_err_t err = pm1_.timerSet(1, M5PM1_TIM_ACTION_POWEROFF);
  if (err != M5PM1_OK) {
    ESP_LOGE(TAG, "PMIC timer power-off request failed: %d", err);
    return;
  }

  delay(3000);
  ESP_LOGE(TAG, "Still alive after PMIC timer power-off request");
}

}  // namespace m5sticks3_power
}  // namespace esphome
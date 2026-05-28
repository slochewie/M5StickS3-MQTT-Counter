from esphome import pins
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

AUTO_LOAD = ["sensor"]

m5sticks3_power_ns = cg.esphome_ns.namespace("m5sticks3_power")
M5StickS3Power = m5sticks3_power_ns.class_("M5StickS3Power", cg.PollingComponent)

CONF_SDA = "sda"
CONF_SCL = "scl"
CONF_ADDRESS = "address"
CONF_IMU_ADDRESS = "imu_address"
CONF_MOTION_THRESHOLD = "motion_threshold"
CONF_MOTION_DURATION = "motion_duration"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(M5StickS3Power),

    cv.Required(CONF_SDA): pins.gpio_output_pin_schema,
    cv.Required(CONF_SCL): pins.gpio_output_pin_schema,

    cv.Optional(CONF_ADDRESS, default=0x6E): cv.hex_int,
    cv.Optional(CONF_IMU_ADDRESS, default=0x68): cv.hex_int,

    # BMI270 ANY_MOTION tuning.
    # Higher threshold/duration = less sensitive shake wake.
    cv.Optional(CONF_MOTION_THRESHOLD, default=0xC0): cv.hex_int,
    cv.Optional(CONF_MOTION_DURATION, default=0x19): cv.hex_int,
}).extend(cv.polling_component_schema("60s"))


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    cg.add(var.set_sda_pin(config[CONF_SDA]["number"]))
    cg.add(var.set_scl_pin(config[CONF_SCL]["number"]))
    cg.add(var.set_address(config[CONF_ADDRESS]))
    cg.add(var.set_imu_address(config[CONF_IMU_ADDRESS]))
    cg.add(var.set_motion_threshold(config[CONF_MOTION_THRESHOLD]))
    cg.add(var.set_motion_duration(config[CONF_MOTION_DURATION]))

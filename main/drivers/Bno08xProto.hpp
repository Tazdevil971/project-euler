#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>

namespace euler::bno08x {

static inline void write_u16(std::span<uint8_t> buf, size_t off,
                             uint16_t value) {
    buf[off + 0] = uint8_t(value);
    buf[off + 1] = uint8_t(value >> 8);
}

static inline void write_u32(std::span<uint8_t> buf, size_t off,
                             uint32_t value) {
    buf[off + 0] = uint8_t(value);
    buf[off + 1] = uint8_t(value >> 8);
    buf[off + 2] = uint8_t(value >> 16);
    buf[off + 3] = uint8_t(value >> 24);
}

static inline uint16_t read_u16(std::span<const uint8_t> buf, size_t off) {
    return uint16_t(buf[off + 0] | (buf[off + 1] << 8));
}

static inline int16_t read_i16(std::span<const uint8_t> buf, size_t off) {
    return static_cast<int16_t>(read_u16(buf, off));
}

static inline float read_f16(std::span<const uint8_t> buf, size_t off, int q) {
    return float(read_i16(buf, off) / float(1 << q));
}

static inline uint32_t read_u32(std::span<const uint8_t> buf, size_t off) {
    return uint32_t(buf[off + 0] | (buf[off + 1] << 8) | (buf[off + 2] << 16) |
                    (buf[off + 3] << 24));
}

static inline int32_t read_i32(std::span<const uint8_t> buf, size_t off) {
    return static_cast<int32_t>(read_u32(buf, off));
}

static inline float read_f32(std::span<const uint8_t> buf, size_t off, int q) {
    return float(read_i32(buf, off) / float(1 << q));
}

struct Header {
    uint16_t len;
    uint8_t chan;
    uint8_t seq;

    static constexpr size_t SIZE = 4;

    static constexpr void write(Header value, std::span<uint8_t> buf) {
        write_u16(buf, 0, value.len);
        buf[2] = value.chan;
        buf[3] = value.seq;
    }

    static constexpr Header read(std::span<const uint8_t> buf) {
        return {.len = read_u16(buf, 0), .chan = buf[2], .seq = buf[3]};
    }
};

namespace channels {
static constexpr uint8_t SHTP_COMMAND = 0;
static constexpr uint8_t EXECUTABLE = 1;
static constexpr uint8_t SH2_CONTROL = 2;
static constexpr uint8_t INPUT_SENSOR_REPORTS = 3;
static constexpr uint8_t WAKE_INPUT_SENSOR_REPORTS = 4;
static constexpr uint8_t GYRO_ROTATION_VECTOR = 5;
}  // namespace channels

namespace report_id {
static constexpr uint8_t GET_FEATURE_REQUEST = 0xfe;
static constexpr uint8_t SET_FEATURE_COMMAND = 0xfd;
static constexpr uint8_t GET_FEATURE_RESPONSE = 0xfc;
static constexpr uint8_t BASE_TIMESTAMP = 0xfb;
static constexpr uint8_t TIMESTAMP_REBASE = 0xfa;
static constexpr uint8_t PRODUCT_ID_REQUEST = 0xf9;
static constexpr uint8_t PRODUCT_ID_RESPONSE = 0xf8;
static constexpr uint8_t FRS_WRITE_REQUEST = 0xf7;
static constexpr uint8_t FRS_WRITE_DATA = 0xf6;
static constexpr uint8_t FRS_WRITE_RESPONSE = 0xf5;
static constexpr uint8_t FRS_READ_REQUEST = 0xf4;
static constexpr uint8_t FRS_REQ_RESPONSE = 0xf3;
static constexpr uint8_t COMMAND_REQUEST = 0xf2;
static constexpr uint8_t COMMAND_RESPONSE = 0xf1;
static constexpr uint8_t ACCELEROMETER = 0x01;
static constexpr uint8_t GYROSCOPE = 0x02;
static constexpr uint8_t MAGNETIC_FIELD = 0x03;
static constexpr uint8_t LINEAR_ACCELERATION = 0x04;
static constexpr uint8_t ROTATION_VECTOR = 0x05;
static constexpr uint8_t GRAVITY = 0x06;
static constexpr uint8_t UNCALIBRATED_GYROSCOPE = 0x07;
static constexpr uint8_t GAME_ROTATION_VECTOR = 0x08;
static constexpr uint8_t GEOMAGNETIC_ROTATION_VECTOR = 0x09;
static constexpr uint8_t PRESSURE = 0x0a;
static constexpr uint8_t AMBIENT_LIGHT = 0x0b;
static constexpr uint8_t HUMIDITY = 0x0c;
static constexpr uint8_t PROXIMITY = 0x0d;
static constexpr uint8_t TEMPERATURE = 0x0e;
static constexpr uint8_t UNCALIBRATED_MAGNETIC_FIELD = 0x0e;
static constexpr uint8_t TAP_DETECTOR = 0x10;
static constexpr uint8_t STEP_COUNTER = 0x11;
static constexpr uint8_t SIGNIFICANT_MOTION = 0x12;
static constexpr uint8_t STABILITY_CLASSIFIER = 0x13;
static constexpr uint8_t RAW_ACCELEROMETER = 0x14;
static constexpr uint8_t RAW_GYROSCOPE = 0x15;
static constexpr uint8_t RAW_MAGNETOMETER = 0x16;
static constexpr uint8_t SAR = 0x17;
static constexpr uint8_t STEP_DETECTOR = 0x18;
static constexpr uint8_t SHAKE_DETECTOR = 0x19;
static constexpr uint8_t FLIP_DETECTOR = 0x1a;
static constexpr uint8_t PICKUP_DETECTOR = 0x1b;
static constexpr uint8_t STABILITY_DETECTOR = 0x1c;
static constexpr uint8_t PERSONAL_ACTIVITY_DETECTOR = 0x1e;
static constexpr uint8_t SLEEP_DETECTOR = 0x1f;
static constexpr uint8_t TILT_DETECTOR = 0x20;
static constexpr uint8_t POCKET_DETECTOR = 0x21;
static constexpr uint8_t CIRCLE_DETECTOR = 0x22;
static constexpr uint8_t HEART_RATE_MONITOR = 0x23;
static constexpr uint8_t ARVR_STABILIZED_ROTATION_VECTOR = 0x28;
static constexpr uint8_t ARVR_STABILIZED_GAME_ROTATION_VECTOR = 0x29;
}  // namespace report_id

struct SetFeatureCommand {
    // TODO: Unsupported
    // uint16_t change_sensitivity;

    uint8_t feature_report_id;

    bool wake_up_enable;
    bool always_on_enable;

    // Report interval in microseconds
    uint32_t report_interval;
    // Batch interval in microseconds
    uint32_t batch_interval;
    // Sensor specific configuration word
    uint32_t config_word;

    static constexpr size_t SIZE = Header::SIZE + 17;

    static constexpr void write(Header header, SetFeatureCommand value,
                                std::span<uint8_t> buf) {
        Header::write(header, buf);
        buf[4] = report_id::SET_FEATURE_COMMAND;
        buf[5] = value.feature_report_id;
        buf[6] = (value.wake_up_enable ? (1 << 2) : 0) |
                 (value.always_on_enable ? (1 << 3) : 0);
        buf[7] = 0;
        buf[8] = 0;
        write_u32(buf, 9, value.report_interval);
        write_u32(buf, 13, value.batch_interval);
        write_u32(buf, 17, value.config_word);
    }
};

struct BaseTimestampReference {
    int32_t base_delta;

    static constexpr size_t SIZE = 5;

    static constexpr BaseTimestampReference read(std::span<const uint8_t> buf) {
        assert(buf[0] == report_id::BASE_TIMESTAMP);
        return {.base_delta = read_i32(buf, 1)};
    }
};

struct RebaseTimestampReference {
    int32_t rebase_delta;

    static constexpr size_t SIZE = 5;

    static constexpr RebaseTimestampReference read(
        std::span<const uint8_t> buf) {
        assert(buf[0] == report_id::TIMESTAMP_REBASE);
        return {.rebase_delta = read_i32(buf, 1)};
    }
};

struct SensorReportCommon {
    uint8_t seq;
    enum class Status {
        Unreliable,
        AccuracyLow,
        AccuracyMedium,
        AccuracyHigh
    } status;
    uint16_t delay;

    static constexpr size_t SIZE = 4;

    static constexpr SensorReportCommon read(std::span<const uint8_t> buf) {
        Status status = Status::Unreliable;
        switch (buf[2] & 0b11) {
            case 0:
                status = Status::Unreliable;
                break;
            case 1:
                status = Status::AccuracyLow;
                break;
            case 2:
                status = Status::AccuracyMedium;
                break;
            case 3:
                status = Status::AccuracyHigh;
                break;
        }

        return {.seq = buf[1],
                .status = status,
                .delay = uint16_t(((buf[2] & 0b1111'1100) << 6) | buf[3])};
    }
};

struct ARVRStabilizedRotationVector {
    SensorReportCommon common;
    float x, y, z, w;
    float accuracy;

    static constexpr size_t SIZE = SensorReportCommon::SIZE + 10;

    static constexpr ARVRStabilizedRotationVector read(
        std::span<const uint8_t> buf) {
        assert(buf[0] == report_id::ARVR_STABILIZED_ROTATION_VECTOR);

        return {.common = SensorReportCommon::read(buf),
                .x = read_f16(buf, 4, 14),
                .y = read_f16(buf, 6, 14),
                .z = read_f16(buf, 8, 14),
                .w = read_f16(buf, 10, 14),
                .accuracy = read_f16(buf, 12, 12)};
    }
};

}  // namespace euler::bno08x
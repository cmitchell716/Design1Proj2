#pragma once
#ifndef MPU9250_H
#define MPU9250_H

#include <Wire.h>

#include "./MPU9250RegisterMap.h"

enum class ACCEL_FS_SEL {
    A2G,
    A4G,
    A8G,
    A16G
};
enum class GYRO_FS_SEL {
    G250DPS,
    G500DPS,
    G1000DPS,
    G2000DPS
};
enum class MAG_OUTPUT_BITS {
    M14BITS,
    M16BITS
};

enum class FIFO_SAMPLE_RATE : uint8_t {
    SMPL_1000HZ,
    SMPL_500HZ,
    SMPL_333HZ,
    SMPL_250HZ,
    SMPL_200HZ,
    SMPL_167HZ,
    SMPL_143HZ,
    SMPL_125HZ,
};

enum class GYRO_DLPF_CFG : uint8_t {
    DLPF_250HZ,
    DLPF_184HZ,
    DLPF_92HZ,
    DLPF_41HZ,
    DLPF_20HZ,
    DLPF_10HZ,
    DLPF_5HZ,
    DLPF_3600HZ,
};

enum class ACCEL_DLPF_CFG : uint8_t {
    DLPF_218HZ_0,
    DLPF_218HZ_1,
    DLPF_99HZ,
    DLPF_45HZ,
    DLPF_21HZ,
    DLPF_10HZ,
    DLPF_5HZ,
    DLPF_420HZ,
};

static constexpr uint8_t MPU9250_WHOAMI_DEFAULT_VALUE {0x71};
static constexpr uint8_t MPU9255_WHOAMI_DEFAULT_VALUE {0x73};
static constexpr uint8_t MPU6500_WHOAMI_DEFAULT_VALUE {0x70};

struct MPU9250Setting {
    ACCEL_FS_SEL accel_fs_sel {ACCEL_FS_SEL::A16G};
    GYRO_FS_SEL gyro_fs_sel {GYRO_FS_SEL::G2000DPS};
    MAG_OUTPUT_BITS mag_output_bits {MAG_OUTPUT_BITS::M16BITS};
    FIFO_SAMPLE_RATE fifo_sample_rate {FIFO_SAMPLE_RATE::SMPL_200HZ};
    uint8_t gyro_fchoice {0x03};
    GYRO_DLPF_CFG gyro_dlpf_cfg {GYRO_DLPF_CFG::DLPF_41HZ};
    uint8_t accel_fchoice {0x01};
    ACCEL_DLPF_CFG accel_dlpf_cfg {ACCEL_DLPF_CFG::DLPF_45HZ};
};

template <typename WireType>
class MPU9250_ {
    static constexpr uint8_t MPU9250_DEFAULT_ADDRESS {0x68};  // Device address when ADO = 0
    static constexpr uint8_t AK8963_ADDRESS {0x0C};           //  Address of magnetometer
    static constexpr uint8_t AK8963_WHOAMI_DEFAULT_VALUE {0x48};
    uint8_t mpu_i2c_addr {MPU9250_DEFAULT_ADDRESS};

    // settings
    MPU9250Setting setting;
    // TODO: this should be configured!!
    static constexpr uint8_t MAG_MODE {0x06};  // 0x02 for 8 Hz, 0x06 for 100 Hz continuous magnetometer data read
    float acc_resolution {0.f};                // scale resolutions per LSB for the sensors
    float gyro_resolution {0.f};               // scale resolutions per LSB for the sensors
    float mag_resolution {0.f};                // scale resolutions per LSB for the sensors

    // Calibration Parameters
    float acc_bias[3] {0., 0., 0.};   // acc calibration value in ACCEL_FS_SEL: 2g
    float gyro_bias[3] {0., 0., 0.};  // gyro calibration value in GYRO_FS_SEL: 250dps
    float mag_bias_factory[3] {0., 0., 0.};
    float mag_bias[3] {0., 0., 0.};  // mag calibration value in MAG_OUTPUT_BITS: 16BITS
    float mag_scale[3] {1., 1., 1.};
    float magnetic_declination = -7.51;  // Japan, 24th June


    // IMU Data
    float a[3] {0.f, 0.f, 0.f};
    float g[3] {0.f, 0.f, 0.f};
    float m[3] {0.f, 0.f, 0.f};
    size_t n_filter_iter {1};

    // Other settings
    bool has_connected {false};
    bool b_ahrs {true};
    bool b_verbose {false};

    // I2C
    WireType* wire;
    uint8_t i2c_err_;

public:
    static constexpr uint16_t CALIB_GYRO_SENSITIVITY {131};     // LSB/degrees/sec
    static constexpr uint16_t CALIB_ACCEL_SENSITIVITY {16384};  // LSB/g

    bool setup(const uint8_t addr, const MPU9250Setting& mpu_setting = MPU9250Setting(), WireType& w = Wire) {
        // addr should be valid for MPU
        if ((addr < MPU9250_DEFAULT_ADDRESS) || (addr > MPU9250_DEFAULT_ADDRESS + 7)) {
            Serial.print("I2C address 0x");
            Serial.print(addr, HEX);
            Serial.println(" is not valid for MPU. Please check your I2C address.");
            return false;
        }
        mpu_i2c_addr = addr;
        setting = mpu_setting;
        wire = &w;

        if (isConnectedMPU9250()) {
            initMPU9250();
            if (isConnectedAK8963())
                initAK8963();
            else {
                if (b_verbose)
                    Serial.println("Could not connect to AK8963");
                has_connected = false;
                return false;
            }
        } else {
            if (b_verbose)
                Serial.println("Could not connect to MPU9250");
            has_connected = false;
            return false;
        }
        has_connected = true;
        return true;
    }

    void sleep(bool b) {
        byte c = read_byte(mpu_i2c_addr, PWR_MGMT_1);  // read the value, change sleep bit to match b, write byte back to register
        if (b) {
            c = c | 0x40;  // sets the sleep bit
        } else {
            c = c & 0xBF;  // mask 1011111 keeps all the previous bits
        }
        write_byte(mpu_i2c_addr, PWR_MGMT_1, c);
    }

    void verbose(const bool b) {
        b_verbose = b;
    }

    void ahrs(const bool b) {
        b_ahrs = b;
    }

    void calibrateAccelGyro() {
        calibrate_acc_gyro_impl();
    }

    void calibrateMag() {
        calibrate_mag_impl();
    }

    bool isConnected() {
        has_connected = isConnectedMPU9250() && isConnectedAK8963();
        return has_connected;
    }

    bool isConnectedMPU9250() {
        byte c = read_byte(mpu_i2c_addr, WHO_AM_I_MPU9250);
        if (b_verbose) {
            Serial.print("MPU9250 WHO AM I = ");
            Serial.println(c, HEX);
        }
        bool b = (c == MPU9250_WHOAMI_DEFAULT_VALUE);
        b |= (c == MPU9255_WHOAMI_DEFAULT_VALUE);
        b |= (c == MPU6500_WHOAMI_DEFAULT_VALUE);
        return b;
    }

    bool isConnectedAK8963() {
        byte c = read_byte(AK8963_ADDRESS, AK8963_WHO_AM_I);
        if (b_verbose) {
            Serial.print("AK8963 WHO AM I = ");
            Serial.println(c, HEX);
        }
        return (c == AK8963_WHOAMI_DEFAULT_VALUE);
    }

    bool isSleeping() {
        byte c = read_byte(mpu_i2c_addr, PWR_MGMT_1);
        return (c & 0x40) == 0x40;
    }

    bool available() {
        return has_connected && (read_byte(mpu_i2c_addr, INT_STATUS) & 0x01);
    }

    bool update() {
        if (!available()) return false;

        update_accel_gyro();
        update_mag();
        return true;
    }

    float getAcc(const uint8_t i) const { return (i < 3) ? a[i] : 0.f; }
    float getGyro(const uint8_t i) const { return (i < 3) ? g[i] : 0.f; }
    float getMag(const uint8_t i) const { return (i < 3) ? m[i] : 0.f; }

    float getAccX() const { return a[0]; }
    float getAccY() const { return a[1]; }
    float getAccZ() const { return a[2]; }
    float getGyroX() const { return g[0]; }
    float getGyroY() const { return g[1]; }
    float getGyroZ() const { return g[2]; }
    float getMagX() const { return m[0]; }
    float getMagY() const { return m[1]; }
    float getMagZ() const { return m[2]; }

    float getAccBias(const uint8_t i) const { return (i < 3) ? acc_bias[i] : 0.f; }
    float getGyroBias(const uint8_t i) const { return (i < 3) ? gyro_bias[i] : 0.f; }
    float getMagBias(const uint8_t i) const { return (i < 3) ? mag_bias[i] : 0.f; }
    float getMagScale(const uint8_t i) const { return (i < 3) ? mag_scale[i] : 0.f; }

    float getAccBiasX() const { return acc_bias[0]; }
    float getAccBiasY() const { return acc_bias[1]; }
    float getAccBiasZ() const { return acc_bias[2]; }
    float getGyroBiasX() const { return gyro_bias[0]; }
    float getGyroBiasY() const { return gyro_bias[1]; }
    float getGyroBiasZ() const { return gyro_bias[2]; }
    float getMagBiasX() const { return mag_bias[0]; }
    float getMagBiasY() const { return mag_bias[1]; }
    float getMagBiasZ() const { return mag_bias[2]; }
    float getMagScaleX() const { return mag_scale[0]; }
    float getMagScaleY() const { return mag_scale[1]; }
    float getMagScaleZ() const { return mag_scale[2]; }


    void setAccBias(const float x, const float y, const float z) {
        acc_bias[0] = x;
        acc_bias[1] = y;
        acc_bias[2] = z;
        write_accel_offset();
    }
    void setGyroBias(const float x, const float y, const float z) {
        gyro_bias[0] = x;
        gyro_bias[1] = y;
        gyro_bias[2] = z;
        write_gyro_offset();
    }
    void setMagBias(const float x, const float y, const float z) {
        mag_bias[0] = x;
        mag_bias[1] = y;
        mag_bias[2] = z;
    }
    void setMagScale(const float x, const float y, const float z) {
        mag_scale[0] = x;
        mag_scale[1] = y;
        mag_scale[2] = z;
    }
    void setMagneticDeclination(const float d) { magnetic_declination = d; }

    void setFilterIterations(const size_t n) {
        if (n > 0) n_filter_iter = n;
    }

    

private:
    void initMPU9250() {
        acc_resolution = get_acc_resolution(setting.accel_fs_sel);
        gyro_resolution = get_gyro_resolution(setting.gyro_fs_sel);
        mag_resolution = get_mag_resolution(setting.mag_output_bits);

        // reset device
        write_byte(mpu_i2c_addr, PWR_MGMT_1, 0x80);  // Write a one to bit 7 reset bit; toggle reset device
        delay(100);

        // wake up device
        write_byte(mpu_i2c_addr, PWR_MGMT_1, 0x00);  // Clear sleep mode bit (6), enable all sensors
        delay(100);                                  // Wait for all registers to reset

        // get stable time source
        write_byte(mpu_i2c_addr, PWR_MGMT_1, 0x01);  // Auto select clock source to be PLL gyroscope reference if ready else
        delay(200);

        // Configure Gyro and Thermometer
        // Disable FSYNC and set thermometer and gyro bandwidth to 41 and 42 Hz, respectively;
        // minimum delay time for this setting is 5.9 ms, which means sensor fusion update rates cannot
        // be higher than 1 / 0.0059 = 170 Hz
        // GYRO_DLPF_CFG = bits 2:0 = 011; this limits the sample rate to 1000 Hz for both
        // With the MPU9250, it is possible to get gyro sample rates of 32 kHz (!), 8 kHz, or 1 kHz
        uint8_t mpu_config = (uint8_t)setting.gyro_dlpf_cfg;
        write_byte(mpu_i2c_addr, MPU_CONFIG, mpu_config);

        // Set sample rate = gyroscope output rate/(1 + SMPLRT_DIV)
        uint8_t sample_rate = (uint8_t)setting.fifo_sample_rate;
        write_byte(mpu_i2c_addr, SMPLRT_DIV, sample_rate);  // Use a 200 Hz rate; a rate consistent with the filter update rate
                                                            // determined inset in CONFIG above

        // Set gyroscope full scale range
        // Range selects FS_SEL and GFS_SEL are 0 - 3, so 2-bit values are left-shifted into positions 4:3
        uint8_t c = read_byte(mpu_i2c_addr, GYRO_CONFIG);  // get current GYRO_CONFIG register value
        c = c & ~0xE0;                                     // Clear self-test bits [7:5]
        c = c & ~0x03;                                     // Clear Fchoice bits [1:0]
        c = c & ~0x18;                                     // Clear GYRO_FS_SEL bits [4:3]
        c = c | (uint8_t(setting.gyro_fs_sel) << 3);       // Set full scale range for the gyro
        c = c | (uint8_t(~setting.gyro_fchoice) & 0x03);   // Set Fchoice for the gyro
        write_byte(mpu_i2c_addr, GYRO_CONFIG, c);          // Write new GYRO_CONFIG value to register

        // Set accelerometer full-scale range configuration
        c = read_byte(mpu_i2c_addr, ACCEL_CONFIG);     // get current ACCEL_CONFIG register value
        c = c & ~0xE0;                                 // Clear self-test bits [7:5]
        c = c & ~0x18;                                 // Clear ACCEL_FS_SEL bits [4:3]
        c = c | (uint8_t(setting.accel_fs_sel) << 3);  // Set full scale range for the accelerometer
        write_byte(mpu_i2c_addr, ACCEL_CONFIG, c);     // Write new ACCEL_CONFIG register value

        // Set accelerometer sample rate configuration
        // It is possible to get a 4 kHz sample rate from the accelerometer by choosing 1 for
        // accel_fchoice_b bit [3]; in this case the bandwidth is 1.13 kHz
        c = read_byte(mpu_i2c_addr, ACCEL_CONFIG2);        // get current ACCEL_CONFIG2 register value
        c = c & ~0x0F;                                     // Clear accel_fchoice_b (bit 3) and A_DLPFG (bits [2:0])
        c = c | (~(setting.accel_fchoice << 3) & 0x08);    // Set accel_fchoice_b to 1
        c = c | (uint8_t(setting.accel_dlpf_cfg) & 0x07);  // Set accelerometer rate to 1 kHz and bandwidth to 41 Hz
        write_byte(mpu_i2c_addr, ACCEL_CONFIG2, c);        // Write new ACCEL_CONFIG2 register value

        // The accelerometer, gyro, and thermometer are set to 1 kHz sample rates,
        // but all these rates are further reduced by a factor of 5 to 200 Hz because of the SMPLRT_DIV setting

        // Configure Interrupts and Bypass Enable
        // Set interrupt pin active high, push-pull, hold interrupt pin level HIGH until interrupt cleared,
        // clear on read of INT_STATUS, and enable I2C_BYPASS_EN so additional chips
        // can join the I2C bus and all can be controlled by the Arduino as master
        write_byte(mpu_i2c_addr, INT_PIN_CFG, 0x22);
        write_byte(mpu_i2c_addr, INT_ENABLE, 0x01);  // Enable data ready (bit 0) interrupt
        delay(100);
    }

    void initAK8963() {
        // First extract the factory calibration for each magnetometer axis
        uint8_t raw_data[3];                            // x/y/z gyro calibration data stored here
        write_byte(AK8963_ADDRESS, AK8963_CNTL, 0x00);  // Power down magnetometer
        delay(10);
        write_byte(AK8963_ADDRESS, AK8963_CNTL, 0x0F);  // Enter Fuse ROM access mode
        delay(10);
        read_bytes(AK8963_ADDRESS, AK8963_ASAX, 3, &raw_data[0]);      // Read the x-, y-, and z-axis calibration values
        mag_bias_factory[0] = (float)(raw_data[0] - 128) / 256. + 1.;  // Return x-axis sensitivity adjustment values, etc.
        mag_bias_factory[1] = (float)(raw_data[1] - 128) / 256. + 1.;
        mag_bias_factory[2] = (float)(raw_data[2] - 128) / 256. + 1.;
        write_byte(AK8963_ADDRESS, AK8963_CNTL, 0x00);  // Power down magnetometer
        delay(10);
        // Configure the magnetometer for continuous read and highest resolution
        // set Mscale bit 4 to 1 (0) to enable 16 (14) bit resolution in CNTL register,
        // and enable continuous mode data acquisition MAG_MODE (bits [3:0]), 0010 for 8 Hz and 0110 for 100 Hz sample rates
        write_byte(AK8963_ADDRESS, AK8963_CNTL, (uint8_t)setting.mag_output_bits << 4 | MAG_MODE);  // Set magnetometer data resolution and sample ODR
        delay(10);

        if (b_verbose) {
            Serial.println("Mag Factory Calibration Values: ");
            Serial.print("X-Axis sensitivity offset value ");
            Serial.println(mag_bias_factory[0], 2);
            Serial.print("Y-Axis sensitivity offset value ");
            Serial.println(mag_bias_factory[1], 2);
            Serial.print("Z-Axis sensitivity offset value ");
            Serial.println(mag_bias_factory[2], 2);
        }
    }

public:
    void update_accel_gyro() {
        int16_t raw_acc_gyro_data[7];        // used to read all 14 bytes at once from the MPU9250 accel/gyro
        read_accel_gyro(raw_acc_gyro_data);  // INT cleared on any read

        // Now we'll calculate the accleration value into actual g's
        a[0] = (float)raw_acc_gyro_data[0] * acc_resolution;  // get actual g value, this depends on scale being set
        a[1] = (float)raw_acc_gyro_data[1] * acc_resolution;
        a[2] = (float)raw_acc_gyro_data[2] * acc_resolution;

        // Calculate the gyro value into actual degrees per second
        g[0] = (float)raw_acc_gyro_data[4] * gyro_resolution;  // get actual gyro value, this depends on scale being set
        g[1] = (float)raw_acc_gyro_data[5] * gyro_resolution;
        g[2] = (float)raw_acc_gyro_data[6] * gyro_resolution;
    }

private:
    void read_accel_gyro(int16_t* destination) {
        uint8_t raw_data[14];                                                 // x/y/z accel register data stored here
        read_bytes(mpu_i2c_addr, ACCEL_XOUT_H, 14, &raw_data[0]);             // Read the 14 raw data registers into data array
        destination[0] = ((int16_t)raw_data[0] << 8) | (int16_t)raw_data[1];  // Turn the MSB and LSB into a signed 16-bit value
        destination[1] = ((int16_t)raw_data[2] << 8) | (int16_t)raw_data[3];
        destination[2] = ((int16_t)raw_data[4] << 8) | (int16_t)raw_data[5];
        destination[3] = ((int16_t)raw_data[6] << 8) | (int16_t)raw_data[7];
        destination[4] = ((int16_t)raw_data[8] << 8) | (int16_t)raw_data[9];
        destination[5] = ((int16_t)raw_data[10] << 8) | (int16_t)raw_data[11];
        destination[6] = ((int16_t)raw_data[12] << 8) | (int16_t)raw_data[13];
    }

public:
    void update_mag() {
        int16_t mag_count[3] = {0, 0, 0};  // Stores the 16-bit signed magnetometer sensor output

        // Read the x/y/z adc values
        if (read_mag(mag_count)) {
            // Calculate the magnetometer values in milliGauss
            // Include factory calibration per data sheet and user environmental corrections
            // mag_bias is calcurated in 16BITS
            float bias_to_current_bits = mag_resolution / get_mag_resolution(MAG_OUTPUT_BITS::M16BITS);
            m[0] = (float)(mag_count[0] * mag_resolution * mag_bias_factory[0] - mag_bias[0] * bias_to_current_bits) * mag_scale[0];  // get actual magnetometer value, this depends on scale being set
            m[1] = (float)(mag_count[1] * mag_resolution * mag_bias_factory[1] - mag_bias[1] * bias_to_current_bits) * mag_scale[1];
            m[2] = (float)(mag_count[2] * mag_resolution * mag_bias_factory[2] - mag_bias[2] * bias_to_current_bits) * mag_scale[2];
        }
    }

private:
    bool read_mag(int16_t* destination) {
        const uint8_t st1 = read_byte(AK8963_ADDRESS, AK8963_ST1);
        if (st1 & 0x01) {                                                    // wait for magnetometer data ready bit to be set
            uint8_t raw_data[7];                                             // x/y/z gyro register data, ST2 register stored here, must read ST2 at end of data acquisition
            read_bytes(AK8963_ADDRESS, AK8963_XOUT_L, 7, &raw_data[0]);      // Read the six raw data and ST2 registers sequentially into data array
            if (MAG_MODE == 0x02 || MAG_MODE == 0x04 || MAG_MODE == 0x06) {  // continuous or external trigger read mode
                if ((st1 & 0x02) != 0)                                       // check if data is not skipped
                    return false;                                            // this should be after data reading to clear DRDY register
            }

            uint8_t c = raw_data[6];                                         // End data read by reading ST2 register
            if (!(c & 0x08)) {                                               // Check if magnetic sensor overflow set, if not then report data
                destination[0] = ((int16_t)raw_data[1] << 8) | raw_data[0];  // Turn the MSB and LSB into a signed 16-bit value
                destination[1] = ((int16_t)raw_data[3] << 8) | raw_data[2];  // Data stored as little Endian
                destination[2] = ((int16_t)raw_data[5] << 8) | raw_data[4];
                return true;
            }
        }
        return false;
    }


    // Function which accumulates gyro and accelerometer data after device initialization. It calculates the average
    // of the at-rest readings and then loads the resulting offsets into accelerometer and gyro bias registers.
    // ACCEL_FS_SEL: 2g (maximum sensitivity)
    // GYRO_FS_SEL: 250dps (maximum sensitivity)
    void calibrate_acc_gyro_impl() {
        set_acc_gyro_to_calibration();
        collect_acc_gyro_data_to(acc_bias, gyro_bias);
        write_accel_offset();
        write_gyro_offset();
        delay(100);
        initMPU9250();
        delay(1000);
    }

    void set_acc_gyro_to_calibration() {
        // reset device
        write_byte(mpu_i2c_addr, PWR_MGMT_1, 0x80);  // Write a one to bit 7 reset bit; toggle reset device
        delay(100);

        // get stable time source; Auto select clock source to be PLL gyroscope reference if ready
        // else use the internal oscillator, bits 2:0 = 001
        write_byte(mpu_i2c_addr, PWR_MGMT_1, 0x01);
        write_byte(mpu_i2c_addr, PWR_MGMT_2, 0x00);
        delay(200);

        // Configure device for bias calculation
        write_byte(mpu_i2c_addr, INT_ENABLE, 0x00);    // Disable all interrupts
        write_byte(mpu_i2c_addr, FIFO_EN, 0x00);       // Disable FIFO
        write_byte(mpu_i2c_addr, PWR_MGMT_1, 0x00);    // Turn on internal clock source
        write_byte(mpu_i2c_addr, I2C_MST_CTRL, 0x00);  // Disable I2C master
        write_byte(mpu_i2c_addr, USER_CTRL, 0x00);     // Disable FIFO and I2C master modes
        write_byte(mpu_i2c_addr, USER_CTRL, 0x0C);     // Reset FIFO and DMP
        delay(15);

        // Configure MPU6050 gyro and accelerometer for bias calculation
        write_byte(mpu_i2c_addr, MPU_CONFIG, 0x01);    // Set low-pass filter to 188 Hz
        write_byte(mpu_i2c_addr, SMPLRT_DIV, 0x00);    // Set sample rate to 1 kHz
        write_byte(mpu_i2c_addr, GYRO_CONFIG, 0x00);   // Set gyro full-scale to 250 degrees per second, maximum sensitivity
        write_byte(mpu_i2c_addr, ACCEL_CONFIG, 0x00);  // Set accelerometer full-scale to 2 g, maximum sensitivity

        // Configure FIFO to capture accelerometer and gyro data for bias calculation
        write_byte(mpu_i2c_addr, USER_CTRL, 0x40);  // Enable FIFO
        write_byte(mpu_i2c_addr, FIFO_EN, 0x78);    // Enable gyro and accelerometer sensors for FIFO  (max size 512 bytes in MPU-9150)
        delay(40);                                  // accumulate 40 samples in 40 milliseconds = 480 bytes
    }

    void collect_acc_gyro_data_to(float* a_bias, float* g_bias) {
        // At end of sample accumulation, turn off FIFO sensor read
        uint8_t data[12];                                    // data array to hold accelerometer and gyro x, y, z, data
        write_byte(mpu_i2c_addr, FIFO_EN, 0x00);             // Disable gyro and accelerometer sensors for FIFO
        read_bytes(mpu_i2c_addr, FIFO_COUNTH, 2, &data[0]);  // read FIFO sample count
        uint16_t fifo_count = ((uint16_t)data[0] << 8) | data[1];
        uint16_t packet_count = fifo_count / 12;  // How many sets of full gyro and accelerometer data for averaging

        for (uint16_t ii = 0; ii < packet_count; ii++) {
            int16_t accel_temp[3] = {0, 0, 0}, gyro_temp[3] = {0, 0, 0};
            read_bytes(mpu_i2c_addr, FIFO_R_W, 12, &data[0]);              // read data for averaging
            accel_temp[0] = (int16_t)(((int16_t)data[0] << 8) | data[1]);  // Form signed 16-bit integer for each sample in FIFO
            accel_temp[1] = (int16_t)(((int16_t)data[2] << 8) | data[3]);
            accel_temp[2] = (int16_t)(((int16_t)data[4] << 8) | data[5]);
            gyro_temp[0] = (int16_t)(((int16_t)data[6] << 8) | data[7]);
            gyro_temp[1] = (int16_t)(((int16_t)data[8] << 8) | data[9]);
            gyro_temp[2] = (int16_t)(((int16_t)data[10] << 8) | data[11]);

            a_bias[0] += (float)accel_temp[0];  // Sum individual signed 16-bit biases to get accumulated signed 32-bit biases
            a_bias[1] += (float)accel_temp[1];
            a_bias[2] += (float)accel_temp[2];
            g_bias[0] += (float)gyro_temp[0];
            g_bias[1] += (float)gyro_temp[1];
            g_bias[2] += (float)gyro_temp[2];
        }
        a_bias[0] /= (float)packet_count;  // Normalize sums to get average count biases
        a_bias[1] /= (float)packet_count;
        a_bias[2] /= (float)packet_count;
        g_bias[0] /= (float)packet_count;
        g_bias[1] /= (float)packet_count;
        g_bias[2] /= (float)packet_count;

        if (a_bias[2] > 0L) {
            a_bias[2] -= (float)CALIB_ACCEL_SENSITIVITY;
        }  // Remove gravity from the z-axis accelerometer bias calculation
        else {
            a_bias[2] += (float)CALIB_ACCEL_SENSITIVITY;
        }
    }

    void write_accel_offset() {
        // Construct the accelerometer biases for push to the hardware accelerometer bias registers. These registers contain
        // factory trim values which must be added to the calculated accelerometer biases; on boot up these registers will hold
        // non-zero values. In addition, bit 0 of the lower byte must be preserved since it is used for temperature
        // compensation calculations. Accelerometer bias registers expect bias input as 2048 LSB per g, so that
        // the accelerometer biases calculated above must be divided by 8.

        uint8_t read_data[2] = {0};
        int16_t acc_bias_reg[3] = {0, 0, 0};                      // A place to hold the factory accelerometer trim biases
        read_bytes(mpu_i2c_addr, XA_OFFSET_H, 2, &read_data[0]);  // Read factory accelerometer trim values
        acc_bias_reg[0] = ((int16_t)read_data[0] << 8) | read_data[1];
        read_bytes(mpu_i2c_addr, YA_OFFSET_H, 2, &read_data[0]);
        acc_bias_reg[1] = ((int16_t)read_data[0] << 8) | read_data[1];
        read_bytes(mpu_i2c_addr, ZA_OFFSET_H, 2, &read_data[0]);
        acc_bias_reg[2] = ((int16_t)read_data[0] << 8) | read_data[1];

        int16_t mask_bit[3] = {1, 1, 1};  // Define array to hold mask bit for each accelerometer bias axis
        for (int i = 0; i < 3; i++) {
            if (acc_bias_reg[i] % 2) {
                mask_bit[i] = 0;
            }
            acc_bias_reg[i] -= (int16_t)acc_bias[i] >> 3;  // Subtract calculated averaged accelerometer bias scaled to 2048 LSB/g
            if (mask_bit[i]) {
                acc_bias_reg[i] = acc_bias_reg[i] & ~mask_bit[i];  // Preserve temperature compensation bit
            } else {
                acc_bias_reg[i] = acc_bias_reg[i] | 0x0001;  // Preserve temperature compensation bit
            }
        }

        uint8_t write_data[6] = {0};
        write_data[0] = (acc_bias_reg[0] >> 8) & 0xFF;
        write_data[1] = (acc_bias_reg[0]) & 0xFF;
        write_data[2] = (acc_bias_reg[1] >> 8) & 0xFF;
        write_data[3] = (acc_bias_reg[1]) & 0xFF;
        write_data[4] = (acc_bias_reg[2] >> 8) & 0xFF;
        write_data[5] = (acc_bias_reg[2]) & 0xFF;

        // Push accelerometer biases to hardware registers
        write_byte(mpu_i2c_addr, XA_OFFSET_H, write_data[0]);
        write_byte(mpu_i2c_addr, XA_OFFSET_L, write_data[1]);
        write_byte(mpu_i2c_addr, YA_OFFSET_H, write_data[2]);
        write_byte(mpu_i2c_addr, YA_OFFSET_L, write_data[3]);
        write_byte(mpu_i2c_addr, ZA_OFFSET_H, write_data[4]);
        write_byte(mpu_i2c_addr, ZA_OFFSET_L, write_data[5]);
    }

    void write_gyro_offset() {
        // Construct the gyro biases for push to the hardware gyro bias registers, which are reset to zero upon device startup
        uint8_t gyro_offset_data[6] {0};
        gyro_offset_data[0] = (-(int16_t)gyro_bias[0] / 4 >> 8) & 0xFF;  // Divide by 4 to get 32.9 LSB per deg/s to conform to expected bias input format
        gyro_offset_data[1] = (-(int16_t)gyro_bias[0] / 4) & 0xFF;       // Biases are additive, so change sign on calculated average gyro biases
        gyro_offset_data[2] = (-(int16_t)gyro_bias[1] / 4 >> 8) & 0xFF;
        gyro_offset_data[3] = (-(int16_t)gyro_bias[1] / 4) & 0xFF;
        gyro_offset_data[4] = (-(int16_t)gyro_bias[2] / 4 >> 8) & 0xFF;
        gyro_offset_data[5] = (-(int16_t)gyro_bias[2] / 4) & 0xFF;

        // Push gyro biases to hardware registers
        write_byte(mpu_i2c_addr, XG_OFFSET_H, gyro_offset_data[0]);
        write_byte(mpu_i2c_addr, XG_OFFSET_L, gyro_offset_data[1]);
        write_byte(mpu_i2c_addr, YG_OFFSET_H, gyro_offset_data[2]);
        write_byte(mpu_i2c_addr, YG_OFFSET_L, gyro_offset_data[3]);
        write_byte(mpu_i2c_addr, ZG_OFFSET_H, gyro_offset_data[4]);
        write_byte(mpu_i2c_addr, ZG_OFFSET_L, gyro_offset_data[5]);
    }

    // mag calibration is executed in MAG_OUTPUT_BITS: 16BITS
    void calibrate_mag_impl() {
        // set MAG_OUTPUT_BITS to maximum to calibrate
        MAG_OUTPUT_BITS mag_output_bits_cache = setting.mag_output_bits;
        setting.mag_output_bits = MAG_OUTPUT_BITS::M16BITS;
        initAK8963();
        collect_mag_data_to(mag_bias, mag_scale);

        if (b_verbose) {
            Serial.println("Mag Calibration done!");

            Serial.println("AK8963 mag biases (mG)");
            Serial.print(mag_bias[0]);
            Serial.print(", ");
            Serial.print(mag_bias[1]);
            Serial.print(", ");
            Serial.print(mag_bias[2]);
            Serial.println();
            Serial.println("AK8963 mag scale (mG)");
            Serial.print(mag_scale[0]);
            Serial.print(", ");
            Serial.print(mag_scale[1]);
            Serial.print(", ");
            Serial.print(mag_scale[2]);
            Serial.println();
        }

        // restore MAG_OUTPUT_BITS
        setting.mag_output_bits = mag_output_bits_cache;
        initAK8963();
    }

    void collect_mag_data_to(float* m_bias, float* m_scale) {
        if (b_verbose)
            Serial.println("Mag Calibration: Wave device in a figure eight until done!");
        delay(4000);

        // shoot for ~fifteen seconds of mag data
        uint16_t sample_count = 0;
        if (MAG_MODE == 0x02)
            sample_count = 128;     // at 8 Hz ODR, new mag data is available every 125 ms
        else if (MAG_MODE == 0x06)  // in this library, fixed to 100Hz
            sample_count = 1500;    // at 100 Hz ODR, new mag data is available every 10 ms

        int32_t bias[3] = {0, 0, 0}, scale[3] = {0, 0, 0};
        int16_t mag_max[3] = {-32767, -32767, -32767};
        int16_t mag_min[3] = {32767, 32767, 32767};
        int16_t mag_temp[3] = {0, 0, 0};
        for (uint16_t ii = 0; ii < sample_count; ii++) {
            read_mag(mag_temp);  // Read the mag data
            for (int jj = 0; jj < 3; jj++) {
                if (mag_temp[jj] > mag_max[jj]) mag_max[jj] = mag_temp[jj];
                if (mag_temp[jj] < mag_min[jj]) mag_min[jj] = mag_temp[jj];
            }
            if (MAG_MODE == 0x02) delay(135);  // at 8 Hz ODR, new mag data is available every 125 ms
            if (MAG_MODE == 0x06) delay(12);   // at 100 Hz ODR, new mag data is available every 10 ms
        }

        if (b_verbose) {
            Serial.println("mag x min/max:");
            Serial.println(mag_min[0]);
            Serial.println(mag_max[0]);
            Serial.println("mag y min/max:");
            Serial.println(mag_min[1]);
            Serial.println(mag_max[1]);
            Serial.println("mag z min/max:");
            Serial.println(mag_min[2]);
            Serial.println(mag_max[2]);
        }

        // Get hard iron correction
        bias[0] = (mag_max[0] + mag_min[0]) / 2;  // get average x mag bias in counts
        bias[1] = (mag_max[1] + mag_min[1]) / 2;  // get average y mag bias in counts
        bias[2] = (mag_max[2] + mag_min[2]) / 2;  // get average z mag bias in counts

        float bias_resolution = get_mag_resolution(MAG_OUTPUT_BITS::M16BITS);
        m_bias[0] = (float)bias[0] * bias_resolution * mag_bias_factory[0];  // save mag biases in G for main program
        m_bias[1] = (float)bias[1] * bias_resolution * mag_bias_factory[1];
        m_bias[2] = (float)bias[2] * bias_resolution * mag_bias_factory[2];

        // Get soft iron correction estimate
        //*** multiplication by mag_bias_factory added in accordance with the following comment
        //*** https://github.com/kriswiner/MPU9250/issues/456#issue-836657973
        scale[0] = (float)(mag_max[0] - mag_min[0]) * mag_bias_factory[0] / 2;  // get average x axis max chord length in counts
        scale[1] = (float)(mag_max[1] - mag_min[1]) * mag_bias_factory[1] / 2;  // get average y axis max chord length in counts
        scale[2] = (float)(mag_max[2] - mag_min[2]) * mag_bias_factory[2] / 2;  // get average z axis max chord length in counts

        float avg_rad = scale[0] + scale[1] + scale[2];
        avg_rad /= 3.0;

        m_scale[0] = avg_rad / ((float)scale[0]);
        m_scale[1] = avg_rad / ((float)scale[1]);
        m_scale[2] = avg_rad / ((float)scale[2]);
    }

   

    float get_acc_resolution(const ACCEL_FS_SEL accel_af_sel) const {
        switch (accel_af_sel) {
            // Possible accelerometer scales (and their register bit settings) are:
            // 2 Gs (00), 4 Gs (01), 8 Gs (10), and 16 Gs  (11).
            // Here's a bit of an algorith to calculate DPS/(ADC tick) based on that 2-bit value:
            case ACCEL_FS_SEL::A2G:
                return 2.0 / 32768.0;
            case ACCEL_FS_SEL::A4G:
                return 4.0 / 32768.0;
            case ACCEL_FS_SEL::A8G:
                return 8.0 / 32768.0;
            case ACCEL_FS_SEL::A16G:
                return 16.0 / 32768.0;
            default:
                return 0.;
        }
    }

    float get_gyro_resolution(const GYRO_FS_SEL gyro_fs_sel) const {
        switch (gyro_fs_sel) {
            // Possible gyro scales (and their register bit settings) are:
            // 250 DPS (00), 500 DPS (01), 1000 DPS (10), and 2000 DPS  (11).
            // Here's a bit of an algorith to calculate DPS/(ADC tick) based on that 2-bit value:
            case GYRO_FS_SEL::G250DPS:
                return 250.0 / 32768.0;
            case GYRO_FS_SEL::G500DPS:
                return 500.0 / 32768.0;
            case GYRO_FS_SEL::G1000DPS:
                return 1000.0 / 32768.0;
            case GYRO_FS_SEL::G2000DPS:
                return 2000.0 / 32768.0;
            default:
                return 0.;
        }
    }

    float get_mag_resolution(const MAG_OUTPUT_BITS mag_output_bits) const {
        switch (mag_output_bits) {
            // Possible magnetometer scales (and their register bit settings) are:
            // 14 bit resolution (0) and 16 bit resolution (1)
            // Proper scale to return milliGauss
            case MAG_OUTPUT_BITS::M14BITS:
                return 10. * 4912. / 8190.0;
            case MAG_OUTPUT_BITS::M16BITS:
                return 10. * 4912. / 32760.0;
            default:
                return 0.;
        }
    }

    void write_byte(uint8_t address, uint8_t subAddress, uint8_t data) {
        wire->beginTransmission(address);    // Initialize the Tx buffer
        wire->write(subAddress);             // Put slave register address in Tx buffer
        wire->write(data);                   // Put data in Tx buffer
        i2c_err_ = wire->endTransmission();  // Send the Tx buffer
        if (i2c_err_) print_i2c_error();
    }

    uint8_t read_byte(uint8_t address, uint8_t subAddress) {
        uint8_t data = 0;                         // `data` will store the register data
        wire->beginTransmission(address);         // Initialize the Tx buffer
        wire->write(subAddress);                  // Put slave register address in Tx buffer
        i2c_err_ = wire->endTransmission(false);  // Send the Tx buffer, but send a restart to keep connection alive
        if (i2c_err_) print_i2c_error();
        wire->requestFrom(address, (size_t)1);       // Read one byte from slave register address
        if (wire->available()) data = wire->read();  // Fill Rx buffer with result
        return data;                                 // Return data read from slave register
    }

    void read_bytes(uint8_t address, uint8_t subAddress, uint8_t count, uint8_t* dest) {
        wire->beginTransmission(address);         // Initialize the Tx buffer
        wire->write(subAddress);                  // Put slave register address in Tx buffer
        i2c_err_ = wire->endTransmission(false);  // Send the Tx buffer, but send a restart to keep connection alive
        if (i2c_err_) print_i2c_error();
        uint8_t i = 0;
        wire->requestFrom(address, count);  // Read bytes from slave register address
        while (wire->available()) {
            dest[i++] = wire->read();
        }  // Put read results in the Rx buffer
    }

    void print_i2c_error() {
        if (i2c_err_ == 7) return;  // to avoid stickbreaker-i2c branch's error code
        Serial.print("I2C ERROR CODE : ");
        Serial.println(i2c_err_);
    }
};

using MPU9250 = MPU9250_<TwoWire>;

#endif  // MPU9250_H


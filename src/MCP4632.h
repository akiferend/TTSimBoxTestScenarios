//TODO:
//Add presistent memory to save the last resistance values 

#ifndef MCP4632_H
#define MCP4632_H

#include <Arduino.h>

// Eğer main veya build flags içinde tanımlanmadıysa varsayılan I2C adresleri
#ifndef DGTLPOT_ADDR_A
#define DGTLPOT_ADDR_A 0x2D
#endif

#ifndef DGTLPOT_ADDR_B
#define DGTLPOT_ADDR_B 0x2E
#endif

#ifndef DGTLPOT_ADDR_C
#define DGTLPOT_ADDR_C 0x2F
#endif

// Interpolasyon Kalibrasyon Noktası Yapısı
struct CalibrationPoint {
    float targetKohm;   // Kullanıcının kodda istediği direnç (kOhm)
    float actualKohm;   // Çip pinde tam o değeri versin diye girilmesi gereken ayarlı kOhm
};

// Varsayılan Kalibrasyon Tablosu Makrosu (Tek satır - syntax hatasını imkansız kılar)
#ifndef DGTLPOT_CALIBRATION_TABLE
#define DGTLPOT_CALIBRATION_TABLE { {0.0f, 0.0f}, {10.0f, 10.5f}, {30.0f, 32.0f}, {40.0f, 42.0f}, {45.0f, 45.0f}, {50.0f, 55.5f} }
#endif

// Toplam aktif şematik kanal sayısı (3'ten 8'e kadar 6 adet)
#define MAX_POT_CHANNELS 6

// 128-Step Resistance Enum (0 to 50k Ohm)
enum ResistanceStep {
    R_0_KOHM = 0,
    R_0_391_KOHM = 1,
    R_0_781_KOHM = 2,
    R_1_172_KOHM = 3,
    R_1_563_KOHM = 4,
    R_1_953_KOHM = 5,
    R_2_344_KOHM = 6,
    R_2_734_KOHM = 7,
    R_3_125_KOHM = 8,
    R_3_516_KOHM = 9,
    R_3_906_KOHM = 10,
    R_4_297_KOHM = 11,
    R_4_688_KOHM = 12,
    R_5_078_KOHM = 13,
    R_5_469_KOHM = 14,
    R_5_859_KOHM = 15,
    R_6_250_KOHM = 16,
    R_6_641_KOHM = 17,
    R_7_031_KOHM = 18,
    R_7_422_KOHM = 19,
    R_7_813_KOHM = 20,
    R_8_203_KOHM = 21,
    R_8_594_KOHM = 22,
    R_8_984_KOHM = 23,
    R_9_375_KOHM = 24,
    R_9_766_KOHM = 25,
    R_10_156_KOHM = 26,
    R_10_547_KOHM = 27,
    R_10_938_KOHM = 28,
    R_11_328_KOHM = 29,
    R_11_719_KOHM = 30,
    R_12_109_KOHM = 31,
    R_12_500_KOHM = 32,
    R_12_891_KOHM = 33,
    R_13_281_KOHM = 34,
    R_13_672_KOHM = 35,
    R_14_063_KOHM = 36,
    R_14_453_KOHM = 37,
    R_14_844_KOHM = 38,
    R_15_234_KOHM = 39,
    R_15_625_KOHM = 40,
    R_16_016_KOHM = 41,
    R_16_406_KOHM = 42,
    R_16_797_KOHM = 43,
    R_17_188_KOHM = 44,
    R_17_578_KOHM = 45,
    R_17_969_KOHM = 46,
    R_18_359_KOHM = 47,
    R_18_750_KOHM = 48,
    R_19_141_KOHM = 49,
    R_19_531_KOHM = 50,
    R_19_922_KOHM = 51,
    R_20_313_KOHM = 52,
    R_20_703_KOHM = 53,
    R_21_094_KOHM = 54,
    R_21_484_KOHM = 55,
    R_21_875_KOHM = 56,
    R_22_266_KOHM = 57,
    R_22_656_KOHM = 58,
    R_23_047_KOHM = 59,
    R_23_438_KOHM = 60,
    R_23_828_KOHM = 61,
    R_24_219_KOHM = 62,
    R_24_609_KOHM = 63,
    R_25_000_KOHM = 64,
    R_25_391_KOHM = 65,
    R_25_781_KOHM = 66,
    R_26_172_KOHM = 67,
    R_26_563_KOHM = 68,
    R_26_953_KOHM = 69,
    R_27_344_KOHM = 70,
    R_27_734_KOHM = 71,
    R_28_125_KOHM = 72,
    R_28_516_KOHM = 73,
    R_28_906_KOHM = 74,
    R_29_297_KOHM = 75,
    R_29_688_KOHM = 76,
    R_30_078_KOHM = 77,
    R_30_469_KOHM = 78,
    R_30_859_KOHM = 79,
    R_31_250_KOHM = 80,
    R_31_641_KOHM = 81,
    R_32_031_KOHM = 82,
    R_32_422_KOHM = 83,
    R_32_813_KOHM = 84,
    R_33_203_KOHM = 85,
    R_33_594_KOHM = 86,
    R_33_984_KOHM = 87,
    R_34_375_KOHM = 88,
    R_34_766_KOHM = 89,
    R_35_156_KOHM = 90,
    R_35_547_KOHM = 91,
    R_35_938_KOHM = 92,
    R_36_328_KOHM = 93,
    R_36_719_KOHM = 94,
    R_37_109_KOHM = 95,
    R_37_500_KOHM = 96,
    R_37_891_KOHM = 97,
    R_38_281_KOHM = 98,
    R_38_672_KOHM = 99,
    R_39_063_KOHM = 100,
    R_39_453_KOHM = 101,
    R_39_844_KOHM = 102,
    R_40_234_KOHM = 103,
    R_40_625_KOHM = 104,
    R_41_016_KOHM = 105,
    R_41_406_KOHM = 106,
    R_41_797_KOHM = 107,
    R_42_188_KOHM = 108,
    R_42_578_KOHM = 109,
    R_42_969_KOHM = 110,
    R_43_359_KOHM = 111,
    R_43_750_KOHM = 112,
    R_44_141_KOHM = 113,
    R_44_531_KOHM = 114,
    R_49_219_KOHM = 126,
    R_49_609_KOHM = 127,
    R_50_000_KOHM = 128
};

// Şematik pinlerine karşılık gelen kanallar
enum DGTLPOT_Channel {
    DGTLPOT_3, // 0
    DGTLPOT_4, // 1
    DGTLPOT_5, // 2
    DGTLPOT_6, // 3
    DGTLPOT_7, // 4
    DGTLPOT_8  // 5
};

class MCP4632 {
private:
    byte _deviceAddr;
    const byte CMD_WIPER0 = 0x00;
    const byte CMD_WIPER1 = 0x10;
    const float TOTAL_RESISTANCE = 50.0f; // 50 kOhm
    const int MAX_STEPS = 128;

public:
    MCP4632(byte deviceAddr);
    void setStep(byte wiperChannel, byte stepValue);
    void setResistance(byte wiperChannel, float kohm);
    void setPreset(byte wiperChannel, ResistanceStep step);
};

// Çoklu Sürücü Yöneticisi Sınıfı
class DGTLPOT {
private:
    MCP4632 _potA;
    MCP4632 _potB;
    MCP4632 _potC;

    // Non-blocking fade durum yapısı
    struct FadeState {
        bool active = false;
        float currentKohm;
        float endKohm;
        float stepSizeKohm;
        unsigned long delayMs;
        unsigned long lastUpdateTime;
        float direction;
    };

    FadeState _channelFades[MAX_POT_CHANNELS];

public:
    DGTLPOT();

    void setChanelPreset(DGTLPOT_Channel channel, ResistanceStep preset);
    void setChanelResistance(DGTLPOT_Channel channel, float kohm);

    void startChanelFade(DGTLPOT_Channel channel, float startKohm, float endKohm, float stepSizeKohm, unsigned long delayMs);
    void startChanelFadeByDuration(DGTLPOT_Channel channel, float startKohm, float endKohm, float stepSizeKohm, unsigned long totalDurationMs);
    void updateFades();

    void setChanelTemperatureNTC10K(DGTLPOT_Channel channel, float celcius);
    void runHardwareTest(float ch3_kohm, float ch4_kohm, float ch5_kohm, float ch6_kohm, float ch7_kohm, float ch8_kohm);

    void clearAll();
};

#endif
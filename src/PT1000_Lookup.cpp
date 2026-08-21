#include "PT1000_Lookup.h"

// DIN EN 60751 PT1000 Tablosu (Görseldeki resmi verilere göre)
const PT1000Point PT1000Lookup::table[] = {
    { -70,  723.30f }, { -60,  763.30f }, { -50,  803.10f }, { -40,  842.70f },
    { -30,  882.20f }, { -20,  921.60f }, { -10,  960.90f }, {   0, 1000.00f },
    {  10, 1039.00f }, {  20, 1077.90f }, {  30, 1116.70f }, {  40, 1155.40f },
    {  50, 1194.00f }, {  60, 1232.40f }, {  70, 1270.80f }, {  80, 1309.00f },
    {  90, 1347.10f }, { 100, 1385.10f }, { 110, 1422.90f }, { 120, 1460.60f },
    { 126, 1483.30f }, { 130, 1498.30f }, { 140, 1535.80f }, { 150, 1573.30f },
    { 160, 1610.50f }, { 170, 1647.70f }, { 180, 1684.80f }, { 190, 1721.70f },
    { 200, 1758.60f }, { 250, 1941.00f }, { 300, 2120.50f }, { 350, 2297.20f },
    { 400, 2470.90f }, { 450, 2641.80f }, { 500, 2809.80f }
};

const int PT1000Lookup::tableSize = sizeof(PT1000Lookup::table) / sizeof(PT1000Point);

float PT1000Lookup::temperatureToOhm(float celsius) {
    if (tableSize == 0) return 1000.00f;

    if (celsius <= table[0].tempC) return table[0].ohm;
    if (celsius >= table[tableSize - 1].tempC) return table[tableSize - 1].ohm;

    for (int i = 0; i < tableSize - 1; i++) {
        float t0 = table[i].tempC;
        float t1 = table[i + 1].tempC;

        if (celsius >= t0 && celsius <= t1) {
            float r0 = table[i].ohm;
            float r1 = table[i + 1].ohm;
            // Lineer İnterpolasyon
            return r0 + (celsius - t0) * (r1 - r0) / (t1 - t0);
        }
    }
    return table[tableSize - 1].ohm;
}

float PT1000Lookup::ohmToTemperature(float ohm) {
    if (tableSize == 0) return 0.0f;

    if (ohm <= table[0].ohm) return table[0].tempC;
    if (ohm >= table[tableSize - 1].ohm) return table[tableSize - 1].tempC;

    for (int i = 0; i < tableSize - 1; i++) {
        float r0 = table[i].ohm;
        float r1 = table[i + 1].ohm;

        if (ohm >= r0 && ohm <= r1) {
            float t0 = table[i].tempC;
            float t1 = table[i + 1].tempC;
            // Ters Lineer İnterpolasyon
            return t0 + (ohm - r0) * (t1 - t0) / (r1 - r0);
        }
    }
    return table[tableSize - 1].tempC;
}
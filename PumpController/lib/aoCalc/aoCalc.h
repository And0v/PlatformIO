#ifndef _ao_calc_h
#define _ao_calc_h

#define CALC_STATE_OK   0x00
#define CALC_STATE_INIT 0x01
#define CALC_STATE_LOW  0x02
#define CALC_STATE_HIGH 0x04

typedef struct{
    word port;
    word state;
    word smooth;
    int16_t adc0;
    int16_t adc;
    int32_t adcSum;
} CalcDef;

typedef struct{
    word smooth;
    word moveUp;
    word moveDown;
    int16_t value;
    int16_t prevValue;
    int32_t valueSum;
    int16_t valueAvg;
    int16_t divUp;
    int16_t divDown;
} StaticticsDef;


bool addCalcValue(CalcDef *adcDef, int16_t value, int16_t min, int16_t max);
void calcStatistics(CalcDef &, StaticticsDef &, bool);

#endif // _ao_calc_h
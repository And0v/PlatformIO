#include "aoFuzzy.h"
#include <Arduino.h>

float pertinences[20];
float inputValues[3];
float outputValues[2];

// Creating the FuzzySet to compond FuzzyInput distance
const FuzzySet iClose PROGMEM = {-3, -1, 1, 3, &pertinences[0]};
const FuzzySet iBig PROGMEM = {-20, -10, 10, 20, &pertinences[1]};

const pFuzzySet dst[] PROGMEM = {&iClose, &iBig};

const FuzzySet iSlow PROGMEM = {1, 10, 10, 20, &pertinences[2]};
const FuzzySet iQuick PROGMEM = {45, 60, 70, 70, &pertinences[3]};
const pFuzzySet inpSpeed[] PROGMEM = {&iSlow, &iQuick};

const FuzzySet iCold PROGMEM = {10, 10, 30, 50, &pertinences[4]};
const FuzzySet iGood PROGMEM = {40, 50, 80, 85, &pertinences[5]};
const FuzzySet iHot PROGMEM = {85, 95, 95, 95, &pertinences[6]};

const pFuzzySet temperature[] PROGMEM = {&iCold, &iGood, &iHot};
/* I N P U T S */
const FuzzySets inputs[] PROGMEM = {
    {.currValue = &inputValues[0], .size = 2, .sets = dst},
    {.currValue = &inputValues[1], .size = 3, .sets = inpSpeed},
    {.currValue = &inputValues[3], .size = 3, .sets = temperature},
    {.currValue = NULL, .size = 0, .sets = NULL}

};

const FuzzySet oMinimum PROGMEM = {0, 2, 5, 10, &pertinences[7]};
const FuzzySet oAverage PROGMEM = {10, 10, 20, 20, &pertinences[8]};
const FuzzySet oMaximum PROGMEM = {30, 50, 80, 100, &pertinences[9]};

const pFuzzySet risk[] PROGMEM = {&oMinimum, &oAverage, &oMaximum};

const FuzzySet oSlow PROGMEM = {1, 10, 10, 20, &pertinences[14]};
const FuzzySet oNormal PROGMEM = {15, 30, 30, 50, &pertinences[15]};
const FuzzySet oQuick PROGMEM = {45, 60, 70, 70, &pertinences[16]};
const pFuzzySet outSpeed[] PROGMEM = {&oStoped, &oSlow, &oNormal, &oQuick};

/* O U T P U T S */
const FuzzySets outputs[] PROGMEM = {
    {.currValue = &outputValues[0], .size = 3, .sets = risk},
    {.currValue = &outputValues[1], .size = 4, .sets = outSpeed},
    {.currValue = NULL, .size = 0, .sets = NULL}};
/*******************************
 * R U L E 1                    *
 ********************************/
const FuzzyRuleAntecedent distanceCloseAndSpeedQuick PROGMEM = {
    .mode = MODE_FS_FS,
    .op = OP_AND,
    .u1 = {.set1 = &iClose},
    .u2 = {.set2 = &iQuick}};
const FuzzyRuleAntecedent temperatureCold PROGMEM = {
    .mode = MODE_FS, .op = OP_NONE, .u1 = {.set1 = &iCold}};
const FuzzyRuleAntecedent ifDistanceCloseAndSpeedQuickOrTemperatureCold
    PROGMEM = {.mode = MODE_FRA_FRA,
               .op = OP_OR,
               .u1 = {.rAnt1 = &distanceCloseAndSpeedQuick},
               .u2 = {.rAnt2 = &temperatureCold}};
const pFuzzySet riskMaximumAndSpeedSlow[] PROGMEM = {&oMaximum, &oSlow};
FuzzyComposition riskMaximumAndSpeedSlowCompositions[] = {{0, 0, 0}, {0, 0, 0}};

const FuzzyRuleConsequent thenRisMaximumAndSpeedSlow PROGMEM = {
    .size = 2,
    .sets = riskMaximumAndSpeedSlow,
    .composition = riskMaximumAndSpeedSlowCompositions};

const FuzzyRule rule1 PROGMEM = {
    .antecedent = &ifDistanceCloseAndSpeedQuickOrTemperatureCold,
    .consequent = &thenRisMaximumAndSpeedSlow};
/*******************************
 * R U L E 2                    *
 ********************************/
const FuzzyRuleAntecedent distanceSafeAndSpeedNormal PROGMEM = {
    .mode = MODE_FS_FS,
    .op = OP_AND,
    .u1 = {.set1 = &iSafe},
    .u2 = {.set2 = &iNormal}};
const FuzzyRuleAntecedent temperatureGood PROGMEM = {
    .mode = MODE_FS, .op = OP_NONE, .u1 = {.set1 = &iGood}};
const FuzzyRuleAntecedent ifDistanceSafeAndSpeedNormalOrTemperatureGood
    PROGMEM = {.mode = MODE_FRA_FRA,
               .op = OP_OR,
               .u1 = {.rAnt1 = &distanceSafeAndSpeedNormal},
               .u2 = {.rAnt2 = &temperatureGood}};
const pFuzzySet riskAverageAndSpeedNormal[] PROGMEM = {&oAverage, &oNormal};
FuzzyComposition riskAverageAndSpeedNormalCompositions[] = {{0, 0, 0},
                                                            {0, 0, 0}};

const FuzzyRuleConsequent thenRiskAverageAndSpeedNormal PROGMEM = {
    .size = 2,
    .sets = riskAverageAndSpeedNormal,
    .composition = riskAverageAndSpeedNormalCompositions};

const FuzzyRule rule2 PROGMEM = {
    .antecedent = &ifDistanceSafeAndSpeedNormalOrTemperatureGood,
    .consequent = &thenRiskAverageAndSpeedNormal};

/*******************************
 * R U L E 3                    *
 ********************************/
const FuzzyRuleAntecedent distanceBigAndSpeedNormal PROGMEM = {
    .mode = MODE_FS_FS,
    .op = OP_AND,
    .u1 = {.set1 = &iBig},
    .u2 = {.set2 = &iSlow}};
const FuzzyRuleAntecedent temperatureHot PROGMEM = {
    .mode = MODE_FS, .op = OP_NONE, .u1 = {.set1 = &iHot}};
const FuzzyRuleAntecedent ifDistanceBigAndSpeedSlowOrTemperatureHot PROGMEM = {
    .mode = MODE_FRA_FRA,
    .op = OP_OR,
    .u1 = {.rAnt1 = &distanceBigAndSpeedNormal},
    .u2 = {.rAnt2 = &temperatureHot}};
const pFuzzySet riskMinimumSpeedQuick[] PROGMEM = {&oMinimum, &oQuick};
FuzzyComposition riskMinimumSpeedQuickCompositions[] = {{0, 0, 0}, {0, 0, 0}};

const FuzzyRuleConsequent thenRiskMinimumSpeedQuick PROGMEM = {
    .size = 2,
    .sets = riskMinimumSpeedQuick,
    .composition = riskMinimumSpeedQuickCompositions};

const FuzzyRule rule3 PROGMEM = {.antecedent =
                                     &ifDistanceBigAndSpeedSlowOrTemperatureHot,
                                 .consequent = &thenRiskMinimumSpeedQuick};

const pFuzzyRule rules[] PROGMEM = {&rule1, &rule2, &rule3, NULL};

FuzzyAO fuzzy;

float output;

void setup() {
  Serial.begin(115200);
  Serial.println("Setup...");
  //  int rulesCount = sizeof(FuzzyAO::rules)/sizeof(FuzzyRule);
  fuzzy.begin(inputs, outputs, rules);
  Serial.println("...Done!");
  fuzzy.fuzzify();
}

float getTemp() { return 1; }
void setPower(float put) {}

float pTemp = nan;

void loop() {
  float temp = getTemp();
  float dist = setpoint - temp;
  if (pTemp == nan) {
    dist
  }

  // Step 5 - Report inputs value, passing its ID and value
  fuzzy.setInput(0, dist);
  fuzzy.setInput(1, dist);
  fuzzy.setInput(2, temp);
  // Step 6 - Exe the fuzzification
  fuzzy.fuzzify();
  // Step 7 - Exe the desfuzzyfica??o for each output, passing its ID
  float output = fuzzy.defuzzify();

  setPower(output);
}

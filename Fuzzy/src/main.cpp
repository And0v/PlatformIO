#include <Arduino.h>
#include "aoFuzzy.h"

float pertinences[20];

// Creating the FuzzySet to compond FuzzyInput distance
const FuzzySet iClose PROGMEM = {0, 20, 20, 40, &pertinences[0]}; // Small distance
const FuzzySet iSafe  PROGMEM = {30, 50, 50, 70, &pertinences[1]}; // Safe distance
const FuzzySet iBig PROGMEM = {60, 80, 80, 80, &pertinences[2]}; // Big distance
const pFuzzySet dst[] PROGMEM = {&iClose, &iSafe, &iBig};

const FuzzySet iStoped PROGMEM = {0, 0, 0, 0, &pertinences[3]};
const FuzzySet iSlow PROGMEM = {1, 10, 10, 20, &pertinences[4]};
const FuzzySet iNormal PROGMEM = {15, 30, 30, 50, &pertinences[5]};
const FuzzySet iQuick PROGMEM = {45, 60, 70, 70, &pertinences[6]};
const pFuzzySet inpSpeed[] PROGMEM = {&iStoped, &iSlow, &iNormal, &iQuick};


const FuzzySet iCold PROGMEM = {5, 10, 15, 20, &pertinences[7]};
const FuzzySet iGood PROGMEM = {15, 18, 23, 25, &pertinences[8]};
const FuzzySet iHot PROGMEM = {24, 25, 30, 30, &pertinences[9]};

const pFuzzySet temperature[] PROGMEM = {&iCold, &iGood, &iHot};
/* I N P U T S */
const FuzzySets inputs[] PROGMEM = {
  {.size = 3, .sets = dst},
  {.size = 4, .sets = inpSpeed},
  {.size = 3, .sets = temperature}
};

const FuzzySet oMinimum PROGMEM = {0, 20, 20, 40, &pertinences[10]};
const FuzzySet oAverage PROGMEM = {30, 50, 50, 70, &pertinences[11]};
const FuzzySet oMaximum PROGMEM = {60, 80, 80, 100, &pertinences[12]};

const pFuzzySet risk[] PROGMEM = {&oMinimum, &oAverage, &oMaximum};

const FuzzySet oStoped PROGMEM = {0, 0, 0, 0, &pertinences[13]};
const FuzzySet oSlow PROGMEM = {1, 10, 10, 20, &pertinences[14]};
const FuzzySet oNormal PROGMEM = {15, 30, 30, 50, &pertinences[15]};
const FuzzySet oQuick PROGMEM = {45, 60, 70, 70, &pertinences[16]};
const pFuzzySet outSpeed[] PROGMEM = {&oStoped, &oSlow, &oNormal, &oQuick};

/* O U T P U T S */
const FuzzySets  outputs[] PROGMEM = {
  {.size = 3, .sets = risk},
  {.size = 4, .sets = outSpeed}
};
/*******************************
* R U L E 1                    *
********************************/
const FuzzyRuleAntecedent distanceCloseAndSpeedQuick PROGMEM = {
    .mode = MODE_FS_FS, .op = OP_AND, .u1 = {.set1 = &iClose}, .u2 = {.set2=&iQuick}
};
const FuzzyRuleAntecedent temperatureCold PROGMEM = {
  .mode = MODE_FS, .op = OP_NONE, .u1 = {.set1 = &iCold}
};
const FuzzyRuleAntecedent ifDistanceCloseAndSpeedQuickOrTemperatureCold PROGMEM = {
    .mode = MODE_FRA_FRA, .op = OP_OR,
    .u1 = {.rAnt1 = &distanceCloseAndSpeedQuick},
    .u2 = {.rAnt2 = &temperatureCold}
};
const pFuzzySet riskMaximumAndSpeedSlow[] PROGMEM = {&oMaximum, &oSlow};

const FuzzyRuleConsequent thenRisMaximumAndSpeedSlow PROGMEM = {
  .size = 2, .sets = riskMaximumAndSpeedSlow
};
const FuzzyRule rule1 PROGMEM = {
  .antecedent = &ifDistanceCloseAndSpeedQuickOrTemperatureCold,
  .consequent = &thenRisMaximumAndSpeedSlow
};
/*******************************
* R U L E 2                    *
********************************/
const FuzzyRuleAntecedent distanceSafeAndSpeedNormal PROGMEM = {
    .mode = MODE_FS_FS, .op = OP_AND, .u1 = {.set1 = &iSafe}, .u2 = {.set2=&iNormal}
};
const FuzzyRuleAntecedent temperatureGood PROGMEM = {
  .mode = MODE_FS, .op = OP_NONE, .u1 = {.set1 = &iGood}
};
const FuzzyRuleAntecedent ifDistanceSafeAndSpeedNormalOrTemperatureGood PROGMEM = {
    .mode = MODE_FRA_FRA, .op = OP_OR,
    .u1 = {.rAnt1 = &distanceSafeAndSpeedNormal},
    .u2 = {.rAnt2 = &temperatureGood}
};
const pFuzzySet riskAverageAndSpeedNormal[] PROGMEM = {&oAverage, &oNormal};

const FuzzyRuleConsequent thenRiskAverageAndSpeedNormal PROGMEM = {
  .size = 2, .sets = riskAverageAndSpeedNormal
};
const FuzzyRule rule2 PROGMEM = {
  .antecedent = &ifDistanceSafeAndSpeedNormalOrTemperatureGood,
  .consequent = &thenRiskAverageAndSpeedNormal
};

/*******************************
* R U L E 3                    *
********************************/
const FuzzyRuleAntecedent distanceBigAndSpeedNormal PROGMEM = {
    .mode = MODE_FS_FS, .op = OP_AND, .u1 = {.set1 = &iBig}, .u2 = {.set2=&iSlow}
};
const FuzzyRuleAntecedent temperatureHot PROGMEM = {
  .mode = MODE_FS, .op = OP_NONE, .u1 = {.set1 = &iHot}
};
const FuzzyRuleAntecedent ifDistanceBigAndSpeedSlowOrTemperatureHot PROGMEM = {
    .mode = MODE_FRA_FRA, .op = OP_OR,
    .u1 = {.rAnt1 = &distanceBigAndSpeedNormal},
    .u2 = {.rAnt2 = &temperatureHot}
};
const pFuzzySet riskMinimumSpeedQuick[] PROGMEM = {&oMinimum, &oQuick};

const FuzzyRuleConsequent thenRiskMinimumSpeedQuick PROGMEM = {
  .size = 2, .sets = riskMinimumSpeedQuick
};
const FuzzyRule rule3 PROGMEM = {
  .antecedent = &ifDistanceBigAndSpeedSlowOrTemperatureHot,
  .consequent = &thenRiskMinimumSpeedQuick
};


const pFuzzyRule rules[] PROGMEM = {
  &rule1, &rule2, &rule3
};

FuzzyAO fuzzy;

float input, output;

void setup() {

  Serial.begin(115200);
  Serial.println("Setup...");
//  int rulesCount = sizeof(FuzzyAO::rules)/sizeof(FuzzyRule);
  fuzzy.begin(inputs, outputs, rules);
  Serial.println("...Done!");
}

float getTemp(){
  return 1;
}
void setPower(float put){

}


void loop() {
  float dist = getTemp();

 // Step 5 - Report inputs value, passing its ID and value
 //  fuzzy.setInput(dist);
  // Step 6 - Exe the fuzzification
//  fuzzy.fuzzify();
  // Step 7 - Exe the desfuzzyfica??o for each output, passing its ID
//  float output = fuzzy.defuzzify();

  setPower(output);
}

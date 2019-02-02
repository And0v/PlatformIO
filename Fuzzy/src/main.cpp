#include <Arduino.h>
#include "aoFuzzy.h"

// Creating the FuzzySet to compond FuzzyInput distance
const FuzzySet iClose PROGMEM = {0, 20, 20, 40}; // Small distance
const FuzzySet iSafe  PROGMEM = {30, 50, 50, 70}; // Safe distance
const FuzzySet iBig PROGMEM = {60, 80, 80, 80}; // Big distance
const pFuzzySet dst[] PROGMEM = {&iClose, &iSafe, &iBig};

const FuzzySet iStoped PROGMEM = {0, 0, 0, 0};
const FuzzySet iSlow PROGMEM = {1, 10, 10, 20};
const FuzzySet iNormal PROGMEM = {15, 30, 30, 50};
const FuzzySet iQuick PROGMEM = {45, 60, 70, 70};
const pFuzzySet inpSpeed[] PROGMEM = {&iStoped, &iSlow, &iNormal, &iQuick};


const FuzzySet iCold PROGMEM = {5, 10, 15, 20};
const FuzzySet iGood PROGMEM = {15, 18, 23, 25};
const FuzzySet iHot PROGMEM = {24, 25, 30, 30};

const pFuzzySet temperature[] PROGMEM = {&iCold, &iGood, &iHot};
/* I N P U T S */
const FuzzySets FuzzyAO::inputs[] PROGMEM = {
  {.size = 3, .sets = dst},
  {.size = 4, .sets = inpSpeed},
  {.size = 3, .sets = temperature}
};

const FuzzySet oMinimum PROGMEM = {0, 20, 20, 40};
const FuzzySet oAverage PROGMEM = {30, 50, 50, 70};
const FuzzySet oMaximum PROGMEM = {60, 80, 80, 100};

const pFuzzySet risk[] PROGMEM = {&oMinimum, &oAverage, &oMaximum};

const FuzzySet oStoped PROGMEM = {0, 0, 0, 0};
const FuzzySet oSlow PROGMEM = {1, 10, 10, 20};
const FuzzySet oNormal PROGMEM = {15, 30, 30, 50};
const FuzzySet oQuick PROGMEM = {45, 60, 70, 70};
const pFuzzySet outSpeed[] PROGMEM = {&oStoped, &oSlow, &oNormal, &oQuick};

/* O U T P U T S */
const FuzzySets  FuzzyAO::outputs[] PROGMEM = {
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


const pFuzzyRule FuzzyAO::rules[] PROGMEM = {
  &rule1, &rule2, &rule3
};

FuzzyAO fuzzy;

float input, output;

void setup() {
  int rulesCount = sizeof(FuzzyAO::rules)/sizeof(FuzzyRule);
  fuzzy.begin(rulesCount);
}

float getTemp(){
  return 1;
}
void setPower(float put){

}


void loop() {
  float dist = getTemp();

 // Step 5 - Report inputs value, passing its ID and value
  fuzzy.setInput(dist);
  // Step 6 - Exe the fuzzification
  fuzzy.fuzzify();
  // Step 7 - Exe the desfuzzyfica??o for each output, passing its ID
  float output = fuzzy.defuzzify();

  setPower(output);
}

#include <Arduino.h>
#include "aoFuzzy.h"

// Creating the FuzzySet to compond FuzzyInput distance
const FuzzySet small = {0, 20, 20, 40}; // Small distance
const FuzzySet safe  = {30, 50, 50, 70}; // Safe distance
const FuzzySet big  = {60, 80, 80, 80}; // Big distance

// Creating FuzzySet to compond FuzzyOutput velocity
const FuzzySet slow PROGMEM = {0, 10, 10, 20}; // Slow velocity
const FuzzySet average PROGMEM = {10, 20, 30, 40}; // Average velocity
const FuzzySet fast PROGMEM = {30, 40, 40, 50}; // Fast velocity

const FuzzyRule rule1 PROGMEM = {&small, &slow};
const FuzzyRule rule2 PROGMEM = {&safe, &average};
const FuzzyRule rule3 PROGMEM  = {&big, &fast};

const FuzzyRule FuzzyAO::rules[] PROGMEM = {
  rule1,
  rule2,
  rule3,
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

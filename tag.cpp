
#include <FuzzyComposition.h>
#include <Fuzzy.h>
#include <FuzzyRuleConsequent.h>
#include <FuzzyOutput.h>
#include <FuzzyInput.h>
#include <FuzzyIO.h>
#include <FuzzySet.h>
#include <FuzzyRuleAntecedent.h>

// Step 1 -  Instantiating an object library
Fuzzy* fuzzy = new Fuzzy();

void setup(){
 Serial.begin(9600);

 // Step 2 - Creating a FuzzyInput distance
 FuzzyInput* distance = new FuzzyInput(1);// With its ID in param

 // Creating the FuzzySet to compond FuzzyInput distance
 FuzzySet* small = new FuzzySet(0, 20, 20, 40); // Small distance
 distance->addFuzzySet(small); // Add FuzzySet small to distance
 FuzzySet* safe = new FuzzySet(30, 50, 50, 70); // Safe distance
 distance->addFuzzySet(safe); // Add FuzzySet safe to distance
 FuzzySet* big = new FuzzySet(60, 80, 80, 80); // Big distance
 distance->addFuzzySet(big); // Add FuzzySet big to distance

 fuzzy->addFuzzyInput(distance); // Add FuzzyInput to Fuzzy object

 // Passo 3 - Creating FuzzyOutput velocity
 FuzzyOutput* velocity = new FuzzyOutput(1);// With its ID in param

 // Creating FuzzySet to compond FuzzyOutput velocity
 FuzzySet* slow = new FuzzySet(0, 10, 10, 20); // Slow velocity
 velocity->addFuzzySet(slow); // Add FuzzySet slow to velocity
 FuzzySet* average = new FuzzySet(10, 20, 30, 40); // Average velocity
 velocity->addFuzzySet(average); // Add FuzzySet average to velocity
 FuzzySet* fast = new FuzzySet(30, 40, 40, 50); // Fast velocity
 velocity->addFuzzySet(fast); // Add FuzzySet fast to velocity

 fuzzy->addFuzzyOutput(velocity); // Add FuzzyOutput to Fuzzy object

 //Passo 4 - Assembly the Fuzzy rules
 // FuzzyRule "IF distance = samll THEN velocity = slow"
 FuzzyRuleAntecedent* ifDistanceSmall = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ifDistanceSmall->joinSingle(small); // Adding corresponding FuzzySet to Antecedent object
 FuzzyRuleConsequent* thenVelocitySlow = new FuzzyRuleConsequent(); // Instantiating a Consequent to expression
 thenVelocitySlow->addOutput(slow);// Adding corresponding FuzzySet to Consequent object
 // Instantiating a FuzzyRule object
 FuzzyRule* fuzzyRule01 = new FuzzyRule(1, ifDistanceSmall, thenVelocitySlow); // Passing the Antecedent and the Consequent of expression
 
 fuzzy->addFuzzyRule(fuzzyRule01); // Adding FuzzyRule to Fuzzy object
 
 // FuzzyRule "IF distance = safe THEN velocity = normal"
 FuzzyRuleAntecedent* ifDistanceSafe = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ifDistanceSafe->joinSingle(safe); // Adding corresponding FuzzySet to Antecedent object
 FuzzyRuleConsequent* thenVelocityAverage = new FuzzyRuleConsequent(); // Instantiating a Consequent to expression
 thenVelocityAverage->addOutput(average); // Adding corresponding FuzzySet to Consequent object
 // Instantiating a FuzzyRule object
 FuzzyRule* fuzzyRule02 = new FuzzyRule(2, ifDistanceSafe, thenVelocityAverage); // Passing the Antecedent and the Consequent of expression
 
 fuzzy->addFuzzyRule(fuzzyRule02); // Adding FuzzyRule to Fuzzy object
 
 // FuzzyRule "IF distance = big THEN velocity = fast"
 FuzzyRuleAntecedent* ifDistanceBig = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ifDistanceBig->joinSingle(big); // Adding corresponding FuzzySet to Antecedent object
 FuzzyRuleConsequent* thenVelocityFast = new FuzzyRuleConsequent(); // Instantiating a Consequent to expression
 thenVelocityFast->addOutput(fast);// Adding corresponding FuzzySet to Consequent object
 // Instantiating a FuzzyRule object
 FuzzyRule* fuzzyRule03 = new FuzzyRule(3, ifDistanceBig, thenVelocityFast); // Passing the Antecedent and the Consequent of expression
 
 fuzzy->addFuzzyRule(fuzzyRule03); // Adding FuzzyRule to Fuzzy object
}

void loop(){
 float dist = getDistanceFromSonar(); 
 
// Step 5 - Report inputs value, passing its ID and value
fuzzy->setInput(1, dist); 
 // Step 6 - Exe the fuzzification
 fuzzy->fuzzify(); 
 // Step 7 - Exe the desfuzzyfica??o for each output, passing its ID
 float output = fuzzy->defuzzify(1);

 setRobotSpeed(output);

 delay(100);
}
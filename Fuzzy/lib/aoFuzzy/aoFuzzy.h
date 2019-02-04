#include "Arduino.h"

#ifndef _ao_fuzzy_h
#define  _ao_fuzzy_h


typedef struct {
  float a;
  float b;
  float c;
  float d;
  float * pertinence;
} FuzzySet, * pFuzzySet;


typedef struct{
  const word size;
  const pFuzzySet *sets;
} FuzzySets;


typedef struct FuzzyRuleAntecedentS{
  const byte mode;
  const byte op;
  union{
    const FuzzySet * set1;
    const struct FuzzyRuleAntecedentS * rAnt1;
  } u1;
  union u2{
    const FuzzySet * set2;
    const struct FuzzyRuleAntecedentS * rAnt2;
  } u2;
} FuzzyRuleAntecedent;

typedef FuzzySets FuzzyRuleConsequent;

typedef struct {
  const FuzzyRuleAntecedent * antecedent;
  const FuzzyRuleConsequent * consequent;
} FuzzyRule, * pFuzzyRule;

// CONSTANTES
#define OP_NONE 0
#define OP_AND 1
#define OP_OR 2
#define MODE_FS 1
#define MODE_FS_FS 2
#define MODE_FS_FRA 3
#define MODE_FRA_FRA 4

class FuzzyAO
{
  private:
    word _rulesCount;
    pFuzzyRule *_rules;
    word _inputsCount;
    FuzzySets *_inputs;
    word _outputsCount;
    FuzzySets *_outputs;

    float crispInput;
  protected:
    void setCrispInput(float crispInput);
    float getCrispInput();
    void resetFuzzySets(void);
    void calculateInputPertinences(void);
    float calculatePertinence(const FuzzySet * fs, float crispValue);

  public:
    FuzzyAO();
    void begin(FuzzySets *inputs,  FuzzySets * outputs,  pFuzzyRule * rules);

    void setInput(float dist);
    bool fuzzify(void);
    float defuzzify(void);

};


#endif // _ao_fuzzy_h

#include "Arduino.h"

#ifndef _ao_fuzzy_h
#define _ao_fuzzy_h

typedef struct {
  float a;
  float b;
  float c;
  float d;
  float *pertinence;
} FuzzySet, *pFuzzySet;

typedef struct {
  float *currValue;
  const word size;
  const pFuzzySet *sets;
} FuzzySets;

typedef struct FuzzyRuleAntecedentS {
  const byte mode;
  const byte op;
  union {
    const FuzzySet *set1;
    const struct FuzzyRuleAntecedentS *rAnt1;
  } u1;
  union u2 {
    const FuzzySet *set2;
    const struct FuzzyRuleAntecedentS *rAnt2;
  } u2;
} FuzzyRuleAntecedent;

typedef struct {
  const word size;
  const pFuzzySet *sets;
  FuzzyComposition *composition;
} FuzzyRuleConsequent;

typedef struct {
  float a;
  float b;
  float v;

} FuzzyComposition, *pFuzzyComposition;

typedef struct {
  const FuzzyRuleAntecedent *antecedent;
  const FuzzyRuleConsequent *consequent;
  bool *fired;
} FuzzyRule, *pFuzzyRule;

// CONSTANTES
#define OP_NONE 0
#define OP_AND 1
#define OP_OR 2
#define MODE_FS 1
#define MODE_FS_FS 2
#define MODE_FS_FRA 3
#define MODE_FRA_FS 4
#define MODE_FRA_FRA 5

class FuzzyAO {
private:
  word _rulesCount;
  pFuzzyRule *_ruleAux;
  word _inputsCount;
  FuzzySets *_inputAux;
  word _outputsCount;
  FuzzySets *_outputAux;

  float crispInput;

protected:
  void setCrispInput(float crispInput);
  float getCrispInput();
  void resetFuzzySets(FuzzySets *sets);
  void resetFuzzySet(FuzzySet *set);
  void calculateFuzzySetPertinences(FuzzySets *sets);
  float calculatePertinence(FuzzySet *fs, float crispValue);

  bool ruleEvaluateExpression(FuzzyRule *rule);
  bool consequentEvaluate(FuzzyRuleConsequent *consequent, float power);
  float antecedentEvaluate(FuzzyRuleAntecedent *antecedent);

  float getPertinence(FuzzySet *set);
  void setPertinence(FuzzySet *set, float pr);

  void truncateFuzzySets(FuzzySets *sets);

public:
  FuzzyAO();
  void begin(FuzzySets *inputs, FuzzySets *outputs, pFuzzyRule *rules);

  void setInput(float dist);
  bool fuzzify(void);
  float defuzzify(void);
};

#endif // _ao_fuzzy_h

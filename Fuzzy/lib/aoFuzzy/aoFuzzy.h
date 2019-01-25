#include "Arduino.h"

#ifndef _ao_fuzzy_h
#define  _ao_fuzzy_h


typedef struct {
  float a;
  float b;
  float c;
  float d;
} FuzzySet;

typedef struct {
  const FuzzySet * Antecedent;
  const FuzzySet * Consequent;
} FuzzyRule;

typedef struct {
  float input;
  float output;
} Pertinence;




class FuzzyAO
{
  private:
    float crispInput;
    float _output;
    word _rulesCount;
    Pertinence _pertinences[];

  protected:
    void setCrispInput(float crispInput);
    float getCrispInput();
    void resetFuzzySets(void);
    void calculateInputPertinences(void);
    float calculatePertinence(const FuzzySet * fs, float crispValue);

  public:
    static const FuzzyRule rules[];
    FuzzyAO();
    void begin(word rc);

    void setInput(float dist);
    bool fuzzify(void);
    float defuzzify(void);

};


#endif // _ao_fuzzy_h

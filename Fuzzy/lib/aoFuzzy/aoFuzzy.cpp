#include "aoFuzzy.h"



FuzzyAO::FuzzyAO(){
}
void FuzzyAO::begin(word rc){
  _rulesCount = rc;
}


void FuzzyAO::setCrispInput(float crispInput){
    this->crispInput = crispInput;
}

float FuzzyAO::getCrispInput(){
    return this->crispInput;
}


void FuzzyAO::setInput( float crispValue){
  setCrispInput(crispValue);
}

void FuzzyAO::resetFuzzySets(void)
{
  for(byte i = 0; i < _rulesCount;++i){
    _pertinences[i].input = 0.0f;
    _pertinences[i].output = 0.0f;
  }
}

float FuzzyAO::calculatePertinence(const FuzzySet * fs, float crispValue)
{
  float slope;
  float pertinence;

    if (crispValue < fs->a){
        if (fs->a == fs->b && fs->b != fs->c && fs->c != fs->d){
            pertinence = 1.0;
        }else{
            pertinence = 0.0;
        }
    }else if (crispValue >= fs->a && crispValue < fs->b){
        slope = 1.0 / (fs->b - fs->a);
        pertinence = slope * (crispValue - fs->b) + 1.0;
    }else if (crispValue >= fs->b && crispValue <= fs->c){
        pertinence = 1.0;
    }else if (crispValue > fs->c && crispValue <= fs->d){
        slope = 1.0 / (fs->c - fs->d);
        pertinence = slope * (crispValue - fs->c) + 1.0;
    }else if (crispValue > fs->d){
        if (fs->c == fs->d && fs->c != fs->b && fs->b != fs->a){
          pertinence = 1.0;
        }else{
          pertinence = 0.0;
        }
    }
    return pertinence;
}



void FuzzyAO::calculateInputPertinences(void)
{
  for(byte i = 0; i < _rulesCount;++i){
    _pertinences[i].input = calculatePertinence(rules[i].Antecedent, this->crispInput);
  }
}


bool FuzzyAO::fuzzify(){
  resetFuzzySets();
  calculateInputPertinences();
/*
  evaluateExpression();

    // Truncado os conjuntos de saída
    fuzzyOutputAux = this->fuzzyOutputs;
    while(fuzzyOutputAux != NULL){
        fuzzyOutputAux->fuzzyOutput->truncate();
        fuzzyOutputAux = fuzzyOutputAux->next;
    }
*/
    return true;
}


float FuzzyAO::defuzzify(void)
{
  return 0;
}

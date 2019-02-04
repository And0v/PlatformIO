#include "aoFuzzy.h"



FuzzyAO::FuzzyAO(){
}
void FuzzyAO::begin(FuzzySets *inputs,  FuzzySets * outputs,  pFuzzyRule * rules)
{
  _inputs = inputs;
  _outputs = outputs;
  _rules = rules;
  _rulesCount = sizeof(&_rules)/sizeof(FuzzyRule);
  _inputsCount = sizeof(&_inputs)/sizeof(FuzzyRule);
  _outputsCount = sizeof(&_outputs)/sizeof(FuzzyRule);

  Serial.print("inputs ");
  Serial.println(_inputsCount);
  Serial.print("outputs ");
  Serial.println(_outputsCount);
  Serial.print("rules ");
  Serial.println(_rulesCount);
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
    // _pertinences[i].input = 0.0f;
    // _pertinences[i].output = 0.0f;
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

}


bool FuzzyAO::fuzzify(){
  // fuzzyInputArray* fuzzyInputAux;
  // fuzzyOutputArray *fuzzyOutputAux;
  //
  // fuzzyInputAux = this->fuzzyInputs;
  // while(fuzzyInputAux != NULL){
  //     fuzzyInputAux->fuzzyInput->resetFuzzySets();
  //     fuzzyInputAux = fuzzyInputAux->next;
  // }
  //
  // fuzzyOutputAux = this->fuzzyOutputs;
  // while(fuzzyOutputAux != NULL){
  //     fuzzyOutputAux->fuzzyOutput->resetFuzzySets();
  //     fuzzyOutputAux = fuzzyOutputAux->next;
  // }
  //
  // // Calculando a pertinência de todos os FuzzyInputs
  // fuzzyInputAux = this->fuzzyInputs;
  // while(fuzzyInputAux != NULL){
  //     fuzzyInputAux->fuzzyInput->calculateFuzzySetPertinences();
  //     fuzzyInputAux = fuzzyInputAux->next;
  // }
  //
  // // Avaliando quais regras foram disparadas
  // fuzzyRuleArray* fuzzyRuleAux;
  // fuzzyRuleAux = this->_rules;
  // // Calculando as pertinências de totos os FuzzyInputs
  // while(fuzzyRuleAux != NULL){
  //     fuzzyRuleAux->fuzzyRule->evaluateExpression();
  //     fuzzyRuleAux = fuzzyRuleAux->next;
  // }
  //
  // // Truncado os conjuntos de saída
  // fuzzyOutputAux = this->fuzzyOutputs;
  // while(fuzzyOutputAux != NULL){
  //     fuzzyOutputAux->fuzzyOutput->truncate();
  //     fuzzyOutputAux = fuzzyOutputAux->next;
  // }

  return true;
}


float FuzzyAO::defuzzify(void)
{
  return 0;
}

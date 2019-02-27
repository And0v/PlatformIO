#include "aoFuzzy.h"

FuzzyAO::FuzzyAO() {}
void FuzzyAO::begin(FuzzySets *inputs, FuzzySets *outputs, pFuzzyRule *rules) {
  _inputAux = inputs;
  _outputAux = outputs;
  _ruleAux = rules;

  FuzzySets sets = {.currValue = NULL, .size = 0};
  _inputsCount = 0;
  do {
    memcpy_P(&sets, &inputs[_inputsCount], sizeof(FuzzySets));
    if ((sets.size != 0) && (sets.sets != NULL)) {
      ++_inputsCount;
    }
  } while ((sets.size != 0) && (sets.sets != NULL) && (_inputsCount < 10));

  _outputsCount = 0;
  do {
    memcpy_P(&sets, &outputs[_outputsCount], sizeof(FuzzySets));
    if ((sets.size != 0) && (sets.sets != NULL)) {
      ++_outputsCount;
    }
  } while ((sets.size != 0) && (sets.sets != NULL) && (_outputsCount < 10));

  _rulesCount = 0;
  pFuzzyRule pRule = NULL;
  do {
    memcpy_P(&pRule, &rules[_rulesCount], sizeof(pFuzzyRule));
    if (pRule != NULL) {
      ++_rulesCount;
    }
  } while ((pRule != NULL) && (_rulesCount < 10));
}

void FuzzyAO::setCrispInput(float crispInput) { this->crispInput = crispInput; }

float FuzzyAO::getCrispInput() { return this->crispInput; }

void FuzzyAO::setInput(float crispValue) { setCrispInput(crispValue); }

void FuzzyAO::resetFuzzySet(FuzzySet *set) {
  FuzzySet fs;
  Serial.print("  resetFuzzySet: ");
  memcpy_P(&fs, set, sizeof(FuzzySet));
  Serial.print("    a ");
  Serial.print(fs.a);
  Serial.print(", b ");
  Serial.print(fs.b);
  Serial.print(", c ");
  Serial.print(fs.c);
  Serial.print(", d ");
  Serial.print(fs.d);
  Serial.print(", p ");
  Serial.println((word)fs.pertinence, HEX);
  *(fs.pertinence) = 0;
}

void FuzzyAO::resetFuzzySets(FuzzySets *sets) {
  Serial.println("resetFuzzySets:");
  FuzzySets fss = {};
  while (true) {
    memcpy_P(&fss, sets, sizeof(FuzzySets));
    if ((fss.size == 0) || (fss.sets == NULL)) {
      break;
    }
    Serial.print("  ");
    Serial.print((word)fss.size);
    Serial.print(", ptr ");
    Serial.println((word)fss.sets, HEX);

    sets++;
    word ptrs[fss.size];
    memcpy_P(&ptrs, fss.sets, sizeof(word) * fss.size);
    for (byte i = 0; i < fss.size; ++i) {
      Serial.print(" ");
      Serial.print(ptrs[i], HEX);
      resetFuzzySet((FuzzySet *)ptrs[i]);
    }
    Serial.println();
  }
}

float FuzzyAO::calculatePertinence(FuzzySet *set, float crispValue) {
  float slope;
  float pertinence;
  FuzzySet fs;
  memcpy_P(&fs, set, sizeof(FuzzySet));

  if (crispValue < fs.a) {
    if (fs.a == fs.b &&
        ((fs.b != fs.c && fs.c != fs.d) || (fs.b == fs.c && fs.c != fs.d))) {
      pertinence = 1.0;
    } else {
      pertinence = 0.0;
    }
  } else if (crispValue >= fs.a && crispValue < fs.b) {
    slope = 1.0 / (fs.b - fs.a);
    pertinence = slope * (crispValue - fs.b) + 1.0;
  } else if (crispValue >= fs.b && crispValue <= fs.c) {
    pertinence = 1.0;
  } else if (crispValue > fs.c && crispValue <= fs.d) {
    slope = 1.0 / (fs.c - fs.d);
    pertinence = slope * (crispValue - fs.c) + 1.0;
  } else if (crispValue > fs.d) {
    if (fs.c == fs.d && fs.c != fs.b && fs.b != fs.a) {
      pertinence = 1.0;
    } else {
      pertinence = 0.0;
    }
  }
  *(fs.pertinence) = pertinence;
  return pertinence;
}

void FuzzyAO::calculateFuzzySetPertinences(FuzzySets *sets) {
  FuzzySets fss = {};
  while (true) {
    memcpy_P(&fss, sets, sizeof(FuzzySets));
    if ((fss.size == 0) || (fss.sets == NULL)) {
      break;
    }
    sets++;
    word ptrs[fss.size];
    memcpy_P(&ptrs, fss.sets, sizeof(word) * fss.size);
    for (byte i = 0; i < fss.size; ++i) {
      calculatePertinence((FuzzySet *)ptrs[i], *(fss.currValue));
    }
  }
}

float FuzzyAO::getPertinence(FuzzySet *set) {
  FuzzySet fs;
  memcpy_P(&fs, set, sizeof(FuzzySet));
  return *(fs.pertinence);
}

void FuzzyAO::setPertinence(FuzzySet *set, float pr) {
  FuzzySet fs;
  memcpy_P(&fs, set, sizeof(FuzzySet));
  *(fs.pertinence) = pr;
}

float FuzzyAO::antecedentEvaluate(FuzzyRuleAntecedent *antecedent) {
  FuzzyRuleAntecedent fa = {};
  memcpy_P(&fa, antecedent, sizeof(FuzzyRuleAntecedent));
  float pr1 = 0.0;
  float pr2 = 0.0;
  switch (fa.mode) {
  case MODE_FS:
    return getPertinence(fa.u1.set1);
    break;
  case MODE_FS_FS:
    pr1 = getPertinence(fa.u1.set1);
    pr2 = getPertinence(fa.u2.set2);
    break;
  case MODE_FS_FRA:
    pr1 = getPertinence(fa.u1.set1);
    pr2 = antecedentEvaluate(fa.u2.rAnt2);
    break;
  case MODE_FRA_FS:
    pr1 = antecedentEvaluate(fa.u1.rAnt1);
    pr2 = getPertinence(fa.u2.set2);
    break;
  case MODE_FRA_FRA:
    pr1 = antecedentEvaluate(fa.u1.rAnt1);
    pr2 = antecedentEvaluate(fa.u2.rAnt2);
    break;
  default:
    return 0.0;
  }
  switch (fa.op) {
  case OP_AND:
    if ((pr1 > 0.0) and (pr2 > 0.0)) {
      if (pr1 < pr2) {
        return pr1;
      } else {
        return pr2;
      }
    } else {
      return 0.0;
    }
    break;
  case OP_OR:
    if (pr1 > 0.0 or pr2 > 0.0) {
      if (pr1 > pr2) {
        return pr1;
      } else {
        return pr2;
      }
    } else {
      return 0.0;
    }
    break;
  default:
    return 0.0;
  }

  return 0.0;
}
bool FuzzyAO::consequentEvaluate(FuzzyRuleConsequent *consequent, float power) {
  FuzzyRuleConsequent frc = {};
  memcpy_P(&frc, consequent, sizeof(FuzzyRuleConsequent));
  word ptrs[frc.size];
  memcpy_P(&ptrs, frc.sets, sizeof(word) * frc.size);
  for (byte i = 0; i < frc.size; ++i) {
    setPertinence((FuzzySet *)ptrs[i], power);
  }
  return true;
}

bool FuzzyAO::ruleEvaluateExpression(FuzzyRule *rule) {
  bool fired = false;
  if (rule->antecedent != NULL) {
    float powerOfAntecedent = antecedentEvaluate(rule->antecedent);
    fired = (powerOfAntecedent > 0.0);
    if (rule->fired != NULL) {
      *(rule->fired) = fired;
    }
    if (rule->consequent != NULL) {
      consequentEvaluate(rule->consequent, powerOfAntecedent);
    }
  }
}

void FuzzyAO::truncateFuzzySets(FuzzySets *sets) {
  FuzzySets fss = {};
  while (true) {
    memcpy_P(&fss, sets, sizeof(FuzzySets));
    if ((fss.size == 0) || (fss.sets == NULL)) {
      break;
    }
    sets++;
    word ptrs[fss.size];
    memcpy_P(&ptrs, fss.sets, sizeof(word) * fss.size);
    for (byte i = 0; i < fss.size; ++i) {
      // truncateFuzzySet((FuzzySet *)ptrs[i]);
    }
  }
}

bool FuzzyAO::fuzzify() {

  Serial.println("resetFuzzySets !!!");
  resetFuzzySets(_inputAux);
  resetFuzzySets(_outputAux);

  Serial.println("calculateFuzzySetPertinences !!!!");

  calculateFuzzySetPertinences(_inputAux);

  FuzzyRule fr = {};
  for (byte r = 0; r < _rulesCount; ++r) {
    memcpy_P(&fr, &_ruleAux[r], sizeof(FuzzyRule));
    ruleEvaluateExpression(&fr);
  }

  truncateFuzzySets(_outputAux);

  return true;
}

float FuzzyAO::defuzzify(void) { return 0; }

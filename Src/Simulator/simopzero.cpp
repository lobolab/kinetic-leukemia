// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#include "simopzero.h"

namespace LoboLab {

SimOpZero::SimOpZero(double *t)
  : to_(t) {
}

SimOpZero::~SimOpZero() {
}

void SimOpZero::compute() const {
  *to_ = 0;
}

}
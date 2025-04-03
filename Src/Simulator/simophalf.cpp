// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#include "simophalf.h"

namespace LoboLab {

  SimOpHalf::SimOpHalf(double *t)
      : to_(t) {
  }

  SimOpHalf::~SimOpHalf() {
  }

  void SimOpHalf::compute() const {
    *to_ = 0.5;
  }

}

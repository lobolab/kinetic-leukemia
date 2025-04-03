// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#pragma once

#include "simop.h"

namespace LoboLab {

class SimOpHalf : public SimOp {
  public:
    explicit SimOpHalf(double *to);
    ~SimOpHalf();

    void compute() const;

  private:
    double *to_;
  };

} // namespace LoboLab
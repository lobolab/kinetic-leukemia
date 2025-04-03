// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#pragma once

namespace LoboLab {

class SimOp {
 public:
  SimOp();
  virtual ~SimOp();

  virtual void compute() const = 0;
};

} // namespace LoboLab

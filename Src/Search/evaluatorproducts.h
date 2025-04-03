// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#pragma once

#include "search.h"
#include "Simulator/simulator.h"
#include "Simulator/simstate.h"
#include "Experiment/phenotype.h"

namespace LoboLab {

class Model;

class EvaluatorProducts {

 public:
  explicit EvaluatorProducts(const Search &search);
  ~EvaluatorProducts();

  const Search &search() const {return search_;}

  void loadModel(const Model &model);
  double evaluate(const Model &model, double maxError);
  QHash<int, double> createErrorTable(const Model &model, double maxError);
  double calcDistance(const SimState &state, const QHash<int, int> &labelsInd, 
                      const Experiment& exp) const;
  double calcExperimentError(const Experiment &exp);
  double calcDistance(const SimState &state, const Phenotype &phenotype) const;

 private:
  const Search &search_;
  Simulator simulator_;

  double localDistErrorThreshold_;
  double expDistErrorThreshold_;
  double globalDistErrorThreshold_;
};

} // namespace LoboLab

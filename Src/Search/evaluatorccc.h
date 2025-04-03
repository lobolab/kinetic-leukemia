// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#pragma once

#include "search.h"
#include "Simulator/simulator.h"
#include "Simulator/simstate.h"

namespace LoboLab {

class Model;

class EvaluatorCCC {

 public:
  explicit EvaluatorCCC(const Search &search);
  ~EvaluatorCCC();

  const Search &search() const {return search_;}

  void loadModel(const Model &model);
  double errorEvaluate(const Model &model, double maxError);
  QHash<int, double> createErrorTable(const Model &model, double maxError);
  double rSquaredTrainEvaluate(const Model &model, double maxError, bool adjustForFitness = true);
  double rSquaredTestEvaluate(const Model &model, double maxError, bool adjustForFitness = true);
  double calcExperimentError(const Experiment &exp);
  double calcDistance(const SimState &state, const Phenotype &phenotype) const;

 private:
   double calcRsquared(QList<double> &expArray, QList<double> &simArray);
   double calcRmse(QList<double> &expArray, QList<double> &simArray);

  const Search &search_;
  Simulator simulator_;
  
  double localDistErrorThreshold_;
  double expDistErrorThreshold_;
  double globalDistErrorThreshold_;
};

} // namespace LoboLab

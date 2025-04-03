// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#include "evaluatorproducts.h"
#include "searchparams.h"
#include "Simulator/simparams.h"
#include "Experiment/experiment.h"
#include "Experiment/phenotype.h"
#include "Experiment/product.h"
#include "Model/model.h"
#include "DB/db.h"
#include "Common/log.h"
#include "Common/mathalgo.h"
#include <qmath.h>

namespace LoboLab {

EvaluatorProducts::EvaluatorProducts(const Search &search) 
  : search_(search),
    simulator_(search) {
  localDistErrorThreshold_ = search_.simParams()->localDistErrThreshold();
  expDistErrorThreshold_ = search_.simParams()->expDistErrThreshold();
  globalDistErrorThreshold_ = search_.simParams()->globalDistErrThreshold();
}

EvaluatorProducts::~EvaluatorProducts() {
}

void EvaluatorProducts::loadModel(const Model &model) {
  simulator_.loadModel(&model);
}

QHash<int, double> EvaluatorProducts::createErrorTable(const Model &model, double maxError) {

  loadModel(model);
  QHash<int, double> errorTable;
  double error = 0;
  int nExperiments = search_.nExperiments();
  int i = 0;
  while (i < nExperiments && error <= maxError) {
    double experimentError = calcExperimentError(*search_.experiment(i));

    experimentError = MathAlgo::max(0.0, experimentError - expDistErrorThreshold_);
    errorTable[search_.experiment(i)->id()] = experimentError;
    error += experimentError / nExperiments;
    ++i;
  }

  return errorTable;
}

double EvaluatorProducts::evaluate(const Model &model, double maxError) {
  loadModel(model);
  double error = 0.0;
  int nExperiments = search_.nExperiments();
  int i = 0;
  while (i < nExperiments && (error - globalDistErrorThreshold_) <= maxError) {
    double experimentError = calcExperimentError(*search_.experiment(i));
    if (experimentError < 0.0) return experimentError;  // Error in the simulator

    experimentError = std::max(0.0, experimentError - expDistErrorThreshold_);
    error += experimentError / nExperiments;
    ++i;
  }

  error = std::max(0.0, error - globalDistErrorThreshold_);

  return error;

}

double EvaluatorProducts::calcExperimentError(const Experiment &exp) {

  double simulationError = 0;
  int n = exp.nPhenotypes();
  double change = 0.0;
  simulator_.loadExperiment(&exp);
  simulator_.initialize();
  double t = simulator_.time();
  int nDists = 0;
  for (int i = 0; i < n; ++i) {
    change = 0.0;
    Phenotype* phenotype = exp.phenotype(i); // Ordered by Time. First time can be 0.
    if (phenotype->product()->type() == 2 && phenotype->time() > 0) {
      double nextTimePeriod = phenotype->time() - t;
      if (nextTimePeriod > 0.0) {
        change = simulator_.simulate(nextTimePeriod);
        if (change < 0.0)
          return change;  // Error in the simulator
        t = phenotype->time();
      }
      double phenotypeDistance = calcDistance(
        simulator_.simulatedState(), *phenotype);

      simulationError += phenotypeDistance;// +change;
      nDists++;
    }
  }

  simulationError = sqrt(simulationError / nDists);

  return simulationError;
}

// This is used in the UI
double EvaluatorProducts::calcDistance(const SimState &state, 
                                       const QHash<int, int> &labels2Ind, 
                                       const Experiment& exp) const {
  double dist = 0;
  int nPhenotypes = exp.nPhenotypes();
  for (int i=0; i < nPhenotypes; ++i) {
    Phenotype* phenotype = exp.phenotype(i);
    int index = labels2Ind.value(phenotype->product()->id(), -1);
    
    if (index > -1) {
      double a = state.product(index);
      double b = phenotype->concentration();

      double absSub = fabs(a - b) - localDistErrorThreshold_;

     if (absSub > 0)
      dist += pow(absSub,2);

    } else { // the phenotype product does not exist in the state
      dist += 10;
    }
  }

  return dist / nPhenotypes;
}

// This is used in the search
double EvaluatorProducts::calcDistance(const SimState &state,
                                       const Phenotype &phenotype) const {
  double dist = 0;
  int index = simulator_.labels2Ind().value(phenotype.product()->id(), -1);
  if (index > -1) {
    double a = state.product(index);
    double b = phenotype.concentration();

    double absSub = fabs(a - b) - localDistErrorThreshold_;
    if (absSub > 0)
      dist += pow(absSub, 2);
  }
  else { // the phenotype product does not exist in the state
    dist = 10;
  }
 
  return dist ;
}

}

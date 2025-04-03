// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#include "evaluatorccc.h"
#include "searchparams.h"
#include "Simulator/simparams.h"
#include "Experiment/experiment.h"
#include "Experiment/phenotype.h"
#include "Experiment/product.h"
#include "Model/model.h"

#include "DB/db.h"
#include "Common/log.h"
#include "Common/mathalgo.h"

namespace LoboLab {

  EvaluatorCCC::EvaluatorCCC(const Search &search)
    : search_(search),
    simulator_(search) {
    localDistErrorThreshold_ = search_.simParams()->localDistErrThreshold();
    expDistErrorThreshold_ = search_.simParams()->expDistErrThreshold();
    globalDistErrorThreshold_ = search_.simParams()->globalDistErrThreshold();
  }

  EvaluatorCCC::~EvaluatorCCC() {
  }

  void EvaluatorCCC::loadModel(const Model &model) {
    simulator_.loadModel(&model);
  }

  double EvaluatorCCC::errorEvaluate(const Model &model, double maxError) {
    loadModel(model);
    double error = 0.0;
    int nExperiments = search_.nExpPreds();
    int i = 0;
    while (i < nExperiments && (error - globalDistErrorThreshold_) <= maxError) {
      double experimentError = calcExperimentError(*search_.expPred(i));
      if (experimentError < 0.0) return experimentError;  // Error in the simulator

      experimentError = std::max(0.0, experimentError - expDistErrorThreshold_);
      error += experimentError / nExperiments;
      ++i;
    }

    error = std::max(0.0, error - globalDistErrorThreshold_);

    return error;
  }

  QHash<int, double> EvaluatorCCC::createErrorTable(const Model &model, double maxError) {

    loadModel(model);
    QHash<int, double> errorTable;
    double error = 0;
    int nExperiments = search_.nExpPreds();
    int i = 0;
    while (i < nExperiments && error <= maxError) {
      double experimentError = calcExperimentError(*search_.expPred(i));

      errorTable[search_.expPred(i)->id()] = experimentError;
      error += experimentError / nExperiments;

      ++i;
    }


    return errorTable;
  }

  double EvaluatorCCC::calcExperimentError(const Experiment &exp) {

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

  double EvaluatorCCC::calcDistance(const SimState &state,
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

    return dist;
  }

  double EvaluatorCCC::rSquaredTrainEvaluate(const Model &model, double maxError, bool adjustForFitness) {
    loadModel(model);
    double sumRsquared = 0;
    double sumRmse = 0;
    int nExperiments = search_.nExperiments();
    int n = 0;

    for (int i = 0; i < nExperiments; i++) {
      Experiment *exp = search_.experiment(i);
      QList<double> expValues;
      QList<double> simValues;

      int n = exp->nPhenotypes();
      double change = 0.0;
      simulator_.loadExperiment(exp);
      simulator_.initialize();
      double t = simulator_.time();

      for (int i = 0; i < n; ++i) {
        change = 0.0;
        Phenotype* phenotype = exp->phenotype(i); // Ordered by Time. First time can be 0.
        if (phenotype->product()->type() == 2 && phenotype->time() > 0) {
          double nextTimePeriod = phenotype->time() - t;
          if (nextTimePeriod > 0.0) {
            change = simulator_.simulate(nextTimePeriod);
            if (change < 0.0)
              return change;  // Error in the simulator
            t = phenotype->time();
          }
          int index = simulator_.labels2Ind().value(phenotype->product()->id(), -1);
          if (index > -1) {
            simValues.append(simulator_.simulatedState().product(index));
            expValues.append(phenotype->concentration());
          }
        }
      }
    
      sumRmse += calcRmse(expValues, simValues);
    }
    return sumRmse / nExperiments;
  }

  double EvaluatorCCC::rSquaredTestEvaluate(const Model &model, double maxError, bool adjustForFitness) {
    loadModel(model);
    double sumRsquared = 0;
    double sumRmse = 0;
    int nExperiments = search_.nExpPreds();
    int n = 0;

    for (int i = 0; i < nExperiments; i++) {
      Experiment *exp = search_.expPred(i);
      QList<double> expValues;
      QList<double> simValues;

      int n = exp->nPhenotypes();
      double change = 0.0;
      simulator_.loadExperiment(exp);
      simulator_.initialize();
      double t = simulator_.time();

      for (int i = 0; i < n; ++i) {
        change = 0.0;
        Phenotype* phenotype = exp->phenotype(i); // Ordered by Time. First time can be 0.
        if (phenotype->product()->type() == 2 && phenotype->time() > 0) {
          double nextTimePeriod = phenotype->time() - t;
          if (nextTimePeriod > 0.0) {
            change = simulator_.simulate(nextTimePeriod);
            if (change < 0.0)
              return change;  // Error in the simulator
            t = phenotype->time();
          }
          int index = simulator_.labels2Ind().value(phenotype->product()->id(), -1);
          if (index > -1) {
            simValues.append(simulator_.simulatedState().product(index));
            expValues.append(phenotype->concentration());
          }
        }
      }
     
      sumRmse += calcRmse(expValues, simValues);
    }
    return sumRmse / nExperiments;
  }

  double EvaluatorCCC::calcRsquared(QList<double> &expArray, QList<double> &simArray) {
    double sumExp = 0.0, sumSim = 0.0, sumExpSim = 0.0;
    double ssExp = 0.0, ssSim = 0.0, ssRes = 0.0, ssTot = 0.0; // square sums
    double meanExp = 0;
    double rSquared = 0.0;
    int n = expArray.size();

    for (int i = 0; i < expArray.size(); i++) {
      sumExp += expArray[i];
    }
    meanExp = sumExp / expArray.size();

    for (int i = 0; i < expArray.size() && i < simArray.size(); i++) {
      ssRes += pow((expArray[i]- simArray[i]), 2);
      ssTot += pow((expArray[i] - meanExp), 2);
    }

    // Check if SSTot is zero
    if (ssTot == 0) {
      if (ssRes == 0) {
        return 1.0; // Perfect fit
      }
      else {
        return 0.0; // No variability to explain, but predictions are not matching
      }
    }

    rSquared = 1 - (ssRes / ssTot);

    return rSquared;
  }

  double EvaluatorCCC::calcRmse(QList<double> &expArray, QList<double> &simArray) {
    double ssRes = 0.0; 
    double rmse = 0.0;
    int n = expArray.size() + 1;

    for (int i = 0; i < expArray.size() && i < simArray.size(); i++) {
      double absSub = fabs(expArray[i] - simArray[i]);
      if (absSub > 0)
        ssRes += pow(absSub, 2);
    }

    rmse = sqrt(ssRes/ n);

    return rmse;
  }
}



  

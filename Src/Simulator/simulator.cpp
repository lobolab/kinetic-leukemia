// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#include "simulator.h"

#include "Experiment/experiment.h"
#include "Experiment/phenotype.h"
#include "Experiment/product.h"

#include "Search/search.h"

namespace LoboLab {

  Simulator::Simulator(const Search &search)
    : search_(search), experiment_(NULL), model_(NULL), t_(0.0), nextPhenotype_(0) {
  }

  Simulator::~Simulator() {
  }

  void Simulator::loadModel(const Model *model, bool includeAllFeatures) {
    model_ = model;
    outputLabels_ = search_.outputLabels();
    modelSimulator_.loadModel(*model_, includeAllFeatures);
    initialState_.initialize(*model_, modelSimulator_.productLabels(), outputLabels_);
  }

  void Simulator::reloadModel(const Model *model, bool includeAllFeatures) {
    model_ = model;
    modelSimulator_.loadModel(*model_, includeAllFeatures);
  }

  void Simulator::loadExperiment(const Experiment *exp) {
    experiment_ = exp;
  }

  void Simulator::initialize() {
    double initTime = -1;
    if (model_ && experiment_) {
      modelSimulator_.reset();
      simulatedState_ = initialState_;
      t_ = simulatedState_.setProdConc(experiment_, &modelSimulator_, outputLabels_);
      nextPhenotype_ = 0;
    }
  }

  double Simulator::simulateWithoutPhenotypes(double timePeriod, bool rateCheck) {
    t_ = t_ + timePeriod;
    return modelSimulator_.simulate(timePeriod, simulatedState_, rateCheck);
  }


  double Simulator::simulate(double timePeriod, bool rateCheck) {
    double change = 0.0;
    double lastT = t_ + timePeriod;
    while (t_ < lastT) {
      if (nextPhenotype_ < experiment_->phenotypes().length()) {
        Phenotype* phenotype = experiment_->phenotype(nextPhenotype_); // Ordered by Time. First time can be 0.
        if (phenotype->time() > lastT) {
          change = modelSimulator_.simulate(lastT - t_, simulatedState_, rateCheck);
          t_ = lastT;
        } else {
          double tSpan = phenotype->time() - t_;
          if (tSpan > 0.0) {
            change = modelSimulator_.simulate(tSpan, simulatedState_, rateCheck);
            t_ = phenotype->time();
          }

          if (phenotype->product()->type() == 0)
            modelSimulator_.setProdRate(phenotype->product()->label(), phenotype->constRate());
          else if (phenotype->product()->type() == 1)
            applyPhenotypeConc(phenotype);
          else if (phenotype->product()->type() == 2)
            modelSimulator_.setProdRate(phenotype->product()->label(), phenotype->constRate());

          ++nextPhenotype_;
        }
      } else {
        change = modelSimulator_.simulate(lastT - t_, simulatedState_, rateCheck);
        t_ = lastT;
      }

      if (change < 0.0)
        return change;  // Error in the simulator
    }

    return change;
  }
}
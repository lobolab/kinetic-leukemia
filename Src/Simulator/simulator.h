// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#pragma once

#include "modelsimulator.h"
#include "simstate.h"

namespace LoboLab {

  class Experiment;
  class Search;

  class Simulator {
  public:
    Simulator(const Search &search);
    ~Simulator();

    inline double time() const { return t_; }

    inline const Model *model() const { return model_; }
    inline const Experiment *experiment() const { return experiment_; }
    inline const Search *search() const { return &search_; }

    inline const SimState &initialState() const { return initialState_; }
    inline const SimState &simulatedState() const { return simulatedState_; }
    inline const QList<int> &productLabels() const { return modelSimulator_.productLabels(); }
    inline const QHash<int, int> &labels2Ind() const {
      return modelSimulator_.labels2Ind();
    }

    inline int nProducts() const { return modelSimulator_.nProducts(); }
    inline int productLabel(int i) const { return modelSimulator_.productLabel(i); }

    inline void blockProdProd(int label) {
      modelSimulator_.blockProductProduction(label);
    }

    inline void applyPhenotypeConc(const Phenotype *phen) { 
      simulatedState_.setProdConc(phen, &modelSimulator_);
    }

    void loadModel(const Model *model, bool includeAllFeatures = false); // Reset state
    void reloadModel(const Model *model, bool includeAllFeatures = false); // Do not reset state
    void loadExperiment(const Experiment *exp);
    void initialize();

    double simulateWithoutPhenotypes(double timePeriod, bool rateCheck = true);
    double simulate(double timePeriod, bool rateCheck = true);

  private:


    const Search &search_;
    const Model *model_;
    const Experiment *experiment_;

    double t_;
    int nextPhenotype_;

    ModelSimulator modelSimulator_;

    SimState initialState_;
    SimState simulatedState_;
    QList<int> outputLabels_;
  };

} // namespace LoboLab

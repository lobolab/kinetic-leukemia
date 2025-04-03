// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#include "simstate.h"

#include "Common/log.h"
#include "Model/modelprod.h"
#include "Experiment/product.h"
#include "Experiment/phenotype.h"
#include "Experiment/experiment.h"


namespace LoboLab {

  SimState::SimState()
    : nProducts_(0), nProductsAllocated_(0), products_(NULL) {
  }

  SimState::~SimState() {
    delete[] products_;
  }

  SimState::SimState(const SimState &source) {
    adjustProductsUsed(source.nProducts_);
    for (int i = 0; i < nProducts_; ++i)
      products_[i] = source.products_[i];
  }

  SimState &SimState::operator=(const SimState &source) {
    adjustProductsUsed(source.nProducts_);
    for (int i = 0; i < nProducts_; ++i)
      products_[i] = source.products_[i];

    return *this;
  }

  // Add products if there is less than nProducts
  void SimState::adjustProductsUsed(int nProducts) {
    nProducts_ = nProducts;
    if (nProducts_ > nProductsAllocated_) {
      delete[] products_;
      products_ = new double[nProducts_];
      nProductsAllocated_ = nProducts_;
    }
  }

  // Initialize the products according to the model
  void SimState::initialize(const Model &model, const QList<int> &labels,
    QList<int> outputLabels) {
    outputLabels_ = outputLabels;
    int nProductsUsed = labels.size();
    int nProducts = nProductsUsed;
    adjustProductsUsed(nProducts);

    // Initialize the products
    for (int i = 0; i < nProducts; ++i)
      products_[i] = model.prodWithLabel(labels[i])->init();
  }

  void SimState::clearProducts() {
    for (int i = 0; i<nProducts_; ++i)
      products_[i] = 0;
  }

  // Set initial concentrations from experimental data
  double SimState::setProdConc(const Experiment *exp,
    const ModelSimulator *modelsim, QList<int> outputLabels) {

    int nProductsUsed = modelsim->nProducts();

    int nPhenotypes = exp->nPhenotypes();
    int i = 0;

    double initTime = exp->phenotype(i)->time();
    double initConc = exp->phenotype(i)->concentration();
    products_[i] = initConc;
    ++i;

    while (i < nPhenotypes && exp->phenotype(i)->time() == initTime) {
      setProdConc(exp->phenotype(i), modelsim);
      ++i;
    }

    return initTime;
  }

  void SimState::setProdConc(const Phenotype *phen,
    const ModelSimulator *modelsim) {
    int index = modelsim->labels2Ind().value(phen->product()->id(), -1);
    if (index > -1) {
      products_[index] = phen->concentration();
    }
  }

  void SimState::setProdConc(int prod, double conc) {
    products_[prod] = conc;
  }

}
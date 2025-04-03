// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#pragma once

#include "Common/mathalgo.h"
#include "Model/model.h"
#include "Simulator/modelsimulator.h"

#include <QHash>
#include <QPoint>
#include <QRect>
#include <QVector>

namespace LoboLab {

class Phenotype;
class Experiment;

class SimState {
 public:
  SimState();
  SimState(const SimState &source);
  SimState &operator=(const SimState &source);
  ~SimState();

  void initialize(const Model &model, const QList<int> &labels, QList<int> outputLabels);

  inline double &product(int i) { return products_[i]; }
  inline const double &product(int i) const { return products_[i]; }
  inline double *products() { return products_; }
  inline int nProducts() const { return nProducts_; }
  inline const QList<int> &outputLabels() const { return outputLabels_; }
  
  void clearProducts();
  
  double setProdConc(const Experiment *exp, const ModelSimulator *modelsim, QList<int> outputLabels);
  void setProdConc(const Phenotype *phen, const ModelSimulator *modelsim);
  void setProdConc(int prod, double conc);

 private:
  void adjustProductsUsed(int nProducts);
  int nProducts_; // products_ can contain more than necessary
  int nProductsAllocated_; // size of products_
  double *products_;
  QList<int> outputLabels_;
  QHash<int, double> outLabels2Conc_;
  int totalInd_;
};

} // namespace LoboLab

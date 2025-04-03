// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#pragma once

#include <QList>
#include <QHash>
#include <QSize>

#include "Common/mathalgo.h"
#include "Experiment/experiment.h"

namespace LoboLab {

class SimState;
class SimOp;
class Model;
class ModelProd;
class ModelLink;

class ModelSimulator {
 public:
  ModelSimulator();
  ~ModelSimulator();
  
  void loadModel(const Model &model, bool includeAllFeatures = false);
  double simulate(double tSpan, SimState &state, bool rateCheck = true);

  void setProdRate(int label, double rate);
  void blockProductProduction(int label);
  void applyDegradationFactor(int label, double factor);

  void reset();
  void resetProductProductionBlocks();
  void resetDegradationFactors();


  inline int nProducts() const { return nProducts_; }
  inline const QList<int> &productLabels() const { return labels_; }
  inline int productLabel(int i) const { return labels_[i]; }
  inline const QHash<int, int> &labels2Ind() const { return labels2Ind_; }

  void clearAll();
  void clearLabels();
  void clearProducts();
  void clearOps();
  void deleteOps();

 private:
  ModelSimulator(const ModelSimulator &source);
  ModelSimulator &operator=(const ModelSimulator &source);
  QList<SimOp*> createProductOps(int p, const QList<ModelLink*> &orLinks,
    const QList<ModelLink*>& andLinks);
  SimOp *createHillOpForLink(ModelLink *link, double *to) const;
  double integrate(const double*);
  void calcRates(double *rates);
  double checkSuccess(double errRat);

  QList<int> labels_;
  QHash<int, int> labels2Ind_;
  QMap<int, int> expProductInfo_;
  QList<int> outInterProductIds_;

  double h_;
  int nProducts_;
  int nAllocatedProducts_;
  int nConstRateProducts_;
  int nIntermediateProducts_;
  int nOutputProducts_;

  double *oldConcs_;
  double *regul_;
  double regulTemp_;

  double *productions_;
  double *limits_;
  double *constRates_;
  double *degradations_;
  double *degradationFactors_;

  double *rates1_;
  double *rates2_;
  double *rates3_;
  double *rates4_;
  double *rates5_;
  double *rates6_;
  double *rates7_;
  double *rates8_;
  double *rates9_;
  double *rates10_;

  double errold_;
  bool success_;
  
  
  int nOps_;
  SimOp **ops_;
  int nAllocatedOps_;

  QList<int> outputLabels_;

  // Constants
  static const double b1, b6, b7, b8, b9, b10, b11, b12, bhh1, bhh2, bhh3,
    er1, er6, er7, er8, er9, er10, er11, er12,
    a21, a31, a32, a41, a43, a51, a53, a54, a61, a64, a65, a71, a74, a75, a76,
    a81, a84, a85, a86, a87, a91, a94, a95, a96, a97, a98, a101, a104, a105,
    a106, a107, a108, a109, a111, a114, a115, a116, a117, a118, a119, a1110,
    a121, a124, a125, a126, a127, a128, a129, a1210, a1211, a141, a147, a148,
    a149, a1410, a1411, a1412, a1413, a151, a156, a157, a158, a1511, a1512,
    a1513, a1514, a161, a166, a167, a168, a169, a1613, a1614, a1615,
    aTol, rTol, hini, hmin, cmin, cmax, erroldini, erroldmin, beta, alpha, safe,
    minscale, maxscale;
};

} // namespace LoboLab

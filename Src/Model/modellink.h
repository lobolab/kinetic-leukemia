// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#pragma once

#include <QTextStream>

namespace LoboLab {

class ModelLink {
 public:
   ModelLink(int regulator = 0, int regulated = 0, int isAndReg = -1);
   ModelLink(int regulator, int regulated, double lim, double disConst, double hillCoef, bool isAndReg, bool isPositive);
  virtual ~ModelLink();
  ModelLink(const ModelLink &source);

  inline int regulatorProdLabel() const { return regulator_; }
  inline int regulatedProdLabel() const { return regulated_; }
  inline double disConst() const { return disConst_; }
  inline double hillCoef() const { return hillCoef_; }
  inline bool isAndReg() const { return isAndReg_; }
  inline bool isPos() const { return isPositive_; }

  inline void setRegulator(int regulatorLabel) {regulator_ = regulatorLabel;}
  inline void setRegulated(int regulatedLabel) {regulated_ = regulatedLabel;}
  inline void setDisConst(double newDis) { disConst_ = newDis; }
  inline void setHillCoef(double newHill) { hillCoef_ = newHill; }
  inline void setAndReg(bool newReg) { isAndReg_ = newReg; }
  inline void setPositive(bool newPos) { isPositive_ = newPos; }

  inline int complexity() const { return 1; }

  void mutateParams(int mutationProb, bool mutHillCoefSign, bool mutIsAndReg);

  // Text Serialization
  void loadFromString(QString &str);
  QString toString();

  friend QTextStream &operator<<(QTextStream &stream, const ModelLink &prod);
  friend QTextStream &operator>>(QTextStream &stream, ModelLink &prod);
  friend bool operator<(const ModelLink& prodA, const ModelLink& prodB);

 private:
  int regulator_; // These are product labels
  int regulated_;
  double disConst_;
  double hillCoef_;
  bool isAndReg_; // if false, is OR regulation
  bool isPositive_;
};

} // namespace LoboLab

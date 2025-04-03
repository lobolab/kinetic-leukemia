// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#pragma once

#include <QTextStream>

namespace LoboLab {

class ModelProd {
 public:
  explicit ModelProd(int label = 0, int type = 0); // Random product

  ModelProd(int label, double init, double lim, double posLim, double negLim, double deg, double intrGrow);

  virtual ~ModelProd();
  ModelProd(const ModelProd &source);

  inline int label() const { return label_; }
  inline double init() const { return init_; }
  inline double lim() const { return lim_; }
  inline double posLim() const { return posLim_; }
  inline double negLim() const { return negLim_; }
  inline double deg() const { return deg_; }
  inline double intrGrow() const { return intrGrow_; }
  inline double type() const { return type_; }
  
  inline void setLabel(int label) { label_ = label; }
  inline void setDeg(double deg) { deg_ = deg; }
  inline void setIntrGrow(double intrGrow) { intrGrow_ = intrGrow; }
  inline void setLim(double lim) { lim_ = lim; }
  inline void setPosLim(double newPosLim) { posLim_ = newPosLim; }
  inline void setNegLim(double newNegLim) { negLim_ = newNegLim; }
  inline void setType(int type) { type_ = type; }
  
  int complexity() const;

  void mutateParams(int mutationProb);

  // Text Serialization
  void loadFromString(QString &str);
  QString toString();

  friend QTextStream &operator<<(QTextStream &stream, const ModelProd &prod);
  friend QTextStream &operator>>(QTextStream &stream, ModelProd &prod);

 private:
  int label_;
  double init_;
  double lim_;
  double posLim_;
  double negLim_;
  double deg_;
  double intrGrow_;
  int type_;
};

bool prodLabelLessThan(const ModelProd *p1, const ModelProd *p2);

} // namespace LoboLab

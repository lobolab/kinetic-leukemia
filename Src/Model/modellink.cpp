// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#include "modellink.h"
#include "model.h"
#include "Common/mathalgo.h"

namespace LoboLab {

ModelLink::ModelLink(int regulator, int regulated, int isAndReg)
  : regulator_(regulator), regulated_(regulated) {
    
  disConst_ = 0.01 + 99.99 * MathAlgo::rand01();
  
  if (MathAlgo::randBool())
    hillCoef_ = MathAlgo::rand110();
  else
    hillCoef_ = -1.0 * MathAlgo::rand110();
      
  if (isAndReg == 1)
    isAndReg_ = true;
  else if (isAndReg == 0)
    isAndReg_ = false;
  else
    isAndReg_ = MathAlgo::randBool();

  isPositive_ = MathAlgo::randBool();
}

ModelLink::ModelLink(int rtor, int rted, double l, double d, double h, bool i, bool p)
  : regulator_(rtor), regulated_(rted), disConst_(d), hillCoef_(h), isAndReg_(i),
    isPositive_(p) {
}


ModelLink::~ModelLink() {}

ModelLink::ModelLink(const ModelLink &source)
  : regulator_(source.regulator_), regulated_(source.regulated_),
    disConst_(source.disConst_), hillCoef_(source.hillCoef_),
    isAndReg_(source.isAndReg_),
    isPositive_(source.isPositive_) {
}

void ModelLink::mutateParams(int mutationProb, bool mutHillCoefSign, 
                             bool mutIsAndReg) {

  if (MathAlgo::rand100() < mutationProb)
    disConst_ = 0.01 + 99.99 * MathAlgo::rand01();

  // mutates hill coefficient
  if (MathAlgo::rand100() < mutationProb) {
    if (mutHillCoefSign) {
      if (MathAlgo::randBool())
        hillCoef_ = MathAlgo::rand110();
      else
        hillCoef_ = -1.0 * MathAlgo::rand110();
    } else {
      if (hillCoef_ > 0)
        hillCoef_ = MathAlgo::rand110();
      else
        hillCoef_ = -1.0 * MathAlgo::rand110();
    }
  }

  if (mutIsAndReg && MathAlgo::rand100() < mutationProb)
    isAndReg_ = !isAndReg_;

  if (MathAlgo::rand100() < mutationProb)
    isPositive_ = !isPositive_;

}

// Serialization
QTextStream &operator<<(QTextStream &stream, const ModelLink &link) {
  stream << link.regulator_ << ' ' << link.regulated_
    << ' ' << link.disConst_ << ' ' << link.hillCoef_ << ' ' << link.isAndReg_
    << ' ' << link.isPositive_;

  return stream;
}

QTextStream &operator>>(QTextStream &stream, ModelLink &link) {
  stream >> link.regulator_;

  char c;
  stream >> c; // this one makes it crash
  Q_ASSERT(c == ' ');

  stream >> link.regulated_;
    
  stream >> c;
  Q_ASSERT(c == ' ');

  link.disConst_ = Model::parseDouble(stream);

  stream >> c;
  Q_ASSERT(c == ' ');

  link.hillCoef_ = Model::parseDouble(stream);
  
  stream >> c;
  Q_ASSERT(c == ' ');

  int b;
  stream >> b;
  link.isAndReg_ = b;

  stream >> b;
  link.isPositive_ = b;

  return stream;
}

bool operator<(const ModelLink& prodA, const ModelLink& prodB) {
  return prodA.regulatedProdLabel() < prodB.regulatedProdLabel(); 
}

}
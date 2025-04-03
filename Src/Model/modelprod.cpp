// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#include "modelprod.h"
#include "model.h"
#include "Common/mathalgo.h"

namespace LoboLab {

ModelProd::ModelProd(int label, int type)
    : label_(label) {
  init_ = 0;
  lim_ = 100 * MathAlgo::rand01();
  posLim_ = 0.1 * MathAlgo::rand01();
  negLim_ = 0.1 * MathAlgo::rand01();
  deg_ = 0.1 + 0.9 * MathAlgo::rand01();
  intrGrow_ = 0.01 * MathAlgo::rand01();
  type_ = type;
}


ModelProd::ModelProd(int label, double init, double lim, double posLim, double negLim, double deg, double intrGrow)
  : label_(label), init_(init), lim_(lim), posLim_(posLim), negLim_(negLim), deg_(deg), intrGrow_(intrGrow) {

  if (init_ < 0)
    init_ = 0;
  
  if (lim_ < 0)
    lim_ = 100 * MathAlgo::rand01();
  
  if (posLim_ < 0)
    posLim_ = 0.1 * MathAlgo::rand01();
  
  if (negLim_ < 0)
    negLim_ = 0.1 * MathAlgo::rand01();

  if (deg_ < 0) {
    deg_ = 0.1 + 0.9 * MathAlgo::rand01();
    intrGrow_ = 0.01 * MathAlgo::rand01();
  }
}


ModelProd::~ModelProd() {
}

ModelProd::ModelProd(const ModelProd &source)
  : label_(source.label_), init_(source.init_), lim_(source.lim_),
   posLim_(source.posLim_), negLim_(source.negLim_),
    deg_(source.deg_), intrGrow_(source.intrGrow_), type_(source.type_) {
}

int ModelProd::complexity() const {
  return 1;
}

void ModelProd::mutateParams(int mutationProb) {
  // Change lim
  if (MathAlgo::rand100() < mutationProb)
    lim_ = 100 * MathAlgo::rand01();

  if (MathAlgo::rand100() < mutationProb)
    posLim_ = 0.1 * MathAlgo::rand01();
  if (MathAlgo::rand100() < mutationProb)
    negLim_ = 0.1 * MathAlgo::rand01();

  // Change deg
  if (MathAlgo::rand100() < mutationProb) {
    deg_ = 0.1 + 0.9 * MathAlgo::rand01();
    intrGrow_ = 0.01 * MathAlgo::rand01();
}

}

// Serialization
QTextStream &operator<<(QTextStream &stream, const ModelProd &prod) {
  // Passing from to string
  stream << prod.label_ << ' ' << prod.init_ << ' ' << prod.lim_ << ' '
         <<  prod.posLim_ << ' ' << prod.negLim_ << ' '
         << prod.deg_ << ' ' << prod.intrGrow_ << ' ' << prod.type_;

  return stream;
}

QTextStream &operator>>(QTextStream &stream, ModelProd &prod) {
  // String to object
  stream >> prod.label_;

  char c;
  stream >> c;
  Q_ASSERT(c == ' ');
  
  prod.init_ = Model::parseDouble(stream);

  stream >> c;
  Q_ASSERT(c == ' ');

  prod.lim_ = Model::parseDouble(stream);

  stream >> c;
  Q_ASSERT(c == ' ');

  prod.posLim_ = Model::parseDouble(stream);

  stream >> c;
  Q_ASSERT(c == ' ');

  prod.negLim_ = Model::parseDouble(stream);

  stream >> c;
  Q_ASSERT(c == ' ');

  prod.deg_ = Model::parseDouble(stream);

  stream >> c;
  Q_ASSERT(c == ' ');

  prod.intrGrow_ = Model::parseDouble(stream);

  stream >> c;
  Q_ASSERT(c == ' ');

  prod.type_ = Model::parseDouble(stream);
  
  return stream;
}


bool prodLabelLessThan(const ModelProd *p1, const ModelProd *p2) {
  return p1->label() < p2->label();
}

}
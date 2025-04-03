// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#include "phenotype.h"
#include "experiment.h"
#include "product.h"

namespace LoboLab {

Phenotype::Phenotype(Experiment *e, Product *d)
  : experiment_(e), product_(d), ed_("Phenotype") {
}

Phenotype::Phenotype(Experiment *e, const DBElementData &ref)
    : experiment_(e), ed_("Phenotype", ref) {
  load();
}

Phenotype::~Phenotype() {
  delete product_;
}

Phenotype::Phenotype(const Phenotype &source, Experiment *exp,
                  bool maintainId)
  : experiment_(exp), concentration_(source.concentration_),
    time_(source.time_),
    ed_(source.ed_, maintainId) {
  product_ = new Product(*source.product_);
}

void Phenotype::setProduct(Product *newProduct) {
  delete product_;
  product_ = newProduct;
}

double Phenotype::maxConcInProduct() const {
  return experiment_->productMaxConc(product_->id());
}

void Phenotype::removeFromExperiment() {
  if (experiment_) {
    Experiment *e = experiment_;
    experiment_ = NULL;
    e->removePhenotype(this);
  }
}

bool Phenotype::operator==(const Phenotype& other) const {
  bool equal = product_->name() == other.product_->name() &&
    concentration_ == other.concentration_ &&
    time_ == other.time_;

  return equal;
}

// Persistence methods

void Phenotype::load() {
  concentration_ = ed_.loadValue(FConcentration).toDouble();
  time_ = ed_.loadValue(FTime).toDouble();
  product_ = new Product(ed_.loadValue(FProduct).toInt(), ed_.db());

  ed_.loadFinished();
}

int Phenotype::submit(DB *db) {

  QPair<QString, DBElement*> refMember("Experiment", experiment_);

  QHash<QString, DBElement*> members;
  members.insert("Product", product_);

  QHash<QString, QVariant> values;
  values.insert("Concentration", concentration_);
  values.insert("Time", time_);

  return ed_.submit(db, refMember, members, values);
}

bool Phenotype::erase() {

  return ed_.erase();
}

}
// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#include "experiment.h"
#include "phenotype.h"
#include "product.h"

#include <QSet>
#include <algorithm>

namespace LoboLab {

Experiment::Experiment()
  : ed_("Experiment") {
}

Experiment::Experiment(int id, DB *db)
  : ed_("Experiment", id, db) {
  load();
}

Experiment::~Experiment() {
  deleteAll();
}

Experiment::Experiment(const Experiment &source, bool maintainId)
  : ed_(source.ed_, maintainId) {
  copy(source, maintainId);
}

Experiment &Experiment::operator=(const Experiment &source) {
  deleteAll();
  ed_ = source.ed_;
  copy(source, true);

  return *this;
}

Experiment &Experiment::copy(const Experiment &source) {
  removeAll();
  copy(source, false);

  return *this;
}

void Experiment::copy(const Experiment &source, bool maintainId) {

  int n = source.nPhenotypes();
  for (int i = 0; i < n; ++i)
    phenotypes_.append(new Phenotype(*source.phenotype(i), this,
      maintainId));
}

void Experiment::deleteAll() {
  int n = phenotypes_.size();
  for (int i = 0; i < n; ++i)
    delete phenotypes_.at(i);

  phenotypes_.clear();
}

// This will remove the elements from the database if submitted
void Experiment::removeAll() {
  int n = phenotypes_.size();
  for (int i = n - 1; i >= 0; --i)
    removePhenotype(phenotypes_.at(i));
}

Phenotype *Experiment::addProduct(Product *product) {
  Phenotype *phenotype = new Phenotype(this, product);
  phenotypes_.append(phenotype);
  return phenotype;
}

bool Experiment::removePhenotype(Phenotype *phen) {
  bool taken = false;
  if (phenotypes_.removeOne(phen)) {
    phen->removeFromExperiment();
    ed_.removeReference(phen);
    taken = true;
  }

  return taken;
}

double Experiment::productMaxConc(int productId) const {
  double maxConc = 0;

  int n = phenotypes_.size();
  for (int i = 0; i < n; ++i)
    if (phenotypes_[i]->product()->id() == productId &&
      phenotypes_[i]->concentration() > maxConc)
      maxConc = phenotypes_[i]->concentration();

  return maxConc;
}

double Experiment::calcTimePeriod() const {
  double timePeriod = 0;

  int n = phenotypes_.size();
  for (int i = 0; i < n; ++i)
    if (phenotypes_[i]->time() > timePeriod)
      timePeriod = phenotypes_[i]->time();

  return timePeriod;
}

void Experiment::setProdConcsLinear() const {
  QHash<int, QList<Phenotype*>> productPhenotypes;

  int n = phenotypes_.size();
  for (int i = 0; i < n; ++i) {
    Phenotype *phenotype = phenotypes_[i];
    Product* product = phenotype->product();
    if (product->type() == 0 || product->type() == 2)
      productPhenotypes[product->id()].append(phenotype);
  }

  for (auto i = productPhenotypes.cbegin(), end = productPhenotypes.cend(); i != end; ++i) {
    QList<Phenotype*> phenotypes = i.value();
    int n = phenotypes.size();
    for (int i = 0; i < n-1; ++i) {
      double tSpan = phenotypes[i+1]->time() - phenotypes[i]->time();
      if (tSpan > 0)
        phenotypes[i]->constRate_ = (phenotypes[i+1]->concentration() - phenotypes[i]->concentration()) / tSpan;
      else
        phenotypes[i]->constRate_ = 0;
    }
    if (n > 1)
      phenotypes.last()->constRate_ = phenotypes[n - 2]->constRate_;
    else
      phenotypes.last()->constRate_ = 0;
  }
}

// Calculate the unique number of products per experiment
QSet<int> Experiment::calcnProducts() const {
  QSet<int> productIds;

  int n = phenotypes_.size();
  for (int i = 0; i < n; ++i) {
      productIds.insert(phenotypes_[i]->product()->id());
  }

  return productIds;
}

bool Experiment::operator==(const Experiment& other) const {
  
  bool equal = phenotypes_.size() == other.phenotypes_.size();

  int n = phenotypes_.size();
  for (int i = 0; i < n && equal; ++i)
    equal = *phenotypes_.at(i) == *other.phenotypes_.at(i);

  return equal;
}

// Persistence methods

void Experiment::load() {
  loadPhenotypes();
  setProdConcsLinear();
  ed_.loadFinished();
}

void Experiment::loadPhenotypes() {
  ed_.loadReferences("Phenotype", "Time");
  while (ed_.nextReference()) {
    Phenotype *phenotype = new Phenotype(this, ed_);
    phenotypes_.append(phenotype);
  }
}

int Experiment::submit(DB *db) {
  QHash<QString, QVariant> values;

  return ed_.submit(db, values, phenotypes_);
}

bool Experiment::erase() {
  QList<DBElement*> members;

  return ed_.erase(members, phenotypes_);
}

}
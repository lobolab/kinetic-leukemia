// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#pragma once

#include "DB/dbelementdata.h"

namespace LoboLab {

class Phenotype;
class Product;

class Experiment : public DBElement {
 public:
  Experiment();
  Experiment(int id, DB *db);
  virtual ~Experiment();

  Experiment(const Experiment &source, bool maintainId = true);
  Experiment &operator=(const Experiment &source);
  Experiment &copy(const Experiment &source);

  inline int nPhenotypes() const { return phenotypes_.size(); }
  inline Phenotype *phenotype(int i) const { return phenotypes_.at(i); }
  inline QList<Phenotype*> phenotypes() const { return phenotypes_; }

  Phenotype* addProduct(Product *product);
  bool removePhenotype(Phenotype *phen);

  void removeAll();

  double productMaxConc(int productId) const;
  double calcTimePeriod() const;
  QSet<int> calcnProducts() const;
  void setProdConcsLinear() const;

  bool operator==(const Experiment& other) const;
  inline bool operator!=(const Experiment& other) const {
    return !(*this == other);
  }

  inline virtual int id() const { return ed_.id(); }
  virtual int submit(DB *db);
  virtual bool erase();

 private:
  void copy(const Experiment &source, bool maintainId);
  void deleteAll();

  void load();
  void loadPhenotypes();

  QList<Phenotype*> phenotypes_;

  DBElementData ed_;

};

} // namespace LoboLab

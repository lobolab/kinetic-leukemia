// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#pragma once

#include "DB/dbelementdata.h"

namespace LoboLab {

class Experiment;
class Product;

class Phenotype : public DBElement {
  friend class Experiment;

 public:
   inline Experiment *experiment() const { return experiment_; };

   inline Product *product() const { return product_; }
   void setProduct(Product *newProduct);

   inline double concentration() const { return concentration_; }
   inline double constRate() const { return constRate_; }

   inline void setConcentration(double concentration)
   {
     concentration_ = concentration;
   }

   inline double time() const { return time_; }
   inline void setTime(double time)
   {
     time_ = time;
   }

   double maxConcInProduct() const;

   void removeFromExperiment();

   // void removeAll();

  protected:
    bool operator==(const Phenotype& other) const;
    inline bool operator!=(const Phenotype& other) const {
      return !(*this == other);
    }
    inline virtual int id() const { return ed_.id(); };
    virtual int submit(DB *db);
    virtual bool erase();

 private:
  Phenotype(Experiment *e, Product *p);
  Phenotype(Experiment *e, const DBElementData &ref);
  virtual ~Phenotype();

  Phenotype(const Phenotype &source, Experiment *e = NULL,
    bool maintainId = true);
  Phenotype &operator=(const Phenotype &source);
  Phenotype &copy(const Phenotype &source);
  void copy(const Phenotype &source, bool maintainId);
  void deleteAll();

  void load();

  Experiment *experiment_;
  Product *product_;
  double concentration_;
  double constRate_;
  double time_;
  DBElementData ed_;

// Persistence fields
 public:
  enum {
    FExperiment = 1,
    FProduct,
    FTime,
    FConcentration,
  };
};

} // namespace LoboLab

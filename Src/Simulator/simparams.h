// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#pragma once

#include "DB/dbelementdata.h"
#include <QSize>


namespace LoboLab {

class SimParams : public DBElement {
 public:
  SimParams(int id, DB *db);
  ~SimParams();

  SimParams(const SimParams &source, bool maintainId = true);
  SimParams &operator=(const SimParams &source);

  inline double localDistErrThreshold() { return localDistErrorThreshold; }
  inline double expDistErrThreshold() { return expDistErrorThreshold; }
  inline double globalDistErrThreshold() { return globalDistErrorThreshold; }
  inline double zValue() { return zVal; }

  inline virtual int id() const { return ed.id(); }
  virtual int submit(DB *db);
  virtual bool erase();


  QString name;  
  double timePeriod;
  double localDistErrorThreshold;
  double expDistErrorThreshold;
  double globalDistErrorThreshold;
  double zVal; 

 private:
  void copy(const SimParams &source);
  void load();

  DBElementData ed;

// Persistence fields
 public:
  enum {
    FName = 1,
    FTimePeriod,
    FLocalDistErrorThreshold,
    FExpDistErrorThreshold,
    FGlobalDistErrorThreshold,
    FzVal
  };
};

} // namespace LoboLab

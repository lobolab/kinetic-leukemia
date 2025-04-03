// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#pragma once

#include "DB/dbelementdata.h"


namespace LoboLab {

class SearchParams : public DBElement {
 public:
  SearchParams(int id, DB *db);
  ~SearchParams();

  SearchParams(const SearchParams &source, bool maintainId = true);
  SearchParams &operator=(const SearchParams &source);


  inline virtual int id() const { return ed.id(); };
  virtual int submit(DB *db);
  virtual bool erase();
  
  QString name;
  int nDemes;
  int demesSize;
  int nGenerations;
  int maxGenerationsNoImprov;
  int migrationPeriod;
  int saveIndividuals;

 private:
  void copy(const SearchParams &source);

  void load();

  DBElementData ed;

// Persistence fields
 public:
  enum {
    FName = 1,
    FNumDemes,
    FDemesSize,
    FNumGenerations,
    FmaxGenerationsNoImprov,
    FMigrationPeriod,
    FSaveIndividuals
  };
};

} // namespace LoboLab

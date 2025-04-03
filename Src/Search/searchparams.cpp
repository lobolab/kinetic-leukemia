// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#include "searchparams.h"
#include "DB/db.h"

namespace LoboLab {

SearchParams::SearchParams(int id, DB *db)
  : ed("SearchParams", id, db) {
  load();
}

SearchParams::~SearchParams() {
}

SearchParams::SearchParams(const SearchParams &source, bool maintainId)
  : ed(source.ed, maintainId) {
  copy(source);
}

SearchParams &SearchParams::operator=(const SearchParams &source) {
  ed = source.ed;
  copy(source);

  return *this;
}

void SearchParams::copy(const SearchParams &source) {
  name = source.name;
  nDemes = source.nDemes;
  demesSize = source.demesSize;
  nGenerations = source.nGenerations;
  maxGenerationsNoImprov = source.maxGenerationsNoImprov;
  migrationPeriod = source.migrationPeriod;
  saveIndividuals = source.saveIndividuals;
}

// Persistence methods

void SearchParams::load() {
  name = ed.loadValue(FName).toString();
  nDemes = ed.loadValue(FNumDemes).toInt();
  demesSize = ed.loadValue(FDemesSize).toInt();
  nGenerations = ed.loadValue(FNumGenerations).toInt();
  maxGenerationsNoImprov = ed.loadValue(FmaxGenerationsNoImprov).toInt();
  migrationPeriod = ed.loadValue(FMigrationPeriod).toInt();
  saveIndividuals = ed.loadValue(FSaveIndividuals).toInt();

  ed.loadFinished();
}

int SearchParams::submit(DB *db) {
  QHash<QString, QVariant> values;
  values.insert("Name", name);
  values.insert("NumDemes", nDemes);
  values.insert("DemesSize", demesSize);
  values.insert("MigrationPeriod", migrationPeriod);
  values.insert("SaveIndividuals", saveIndividuals);

  return ed.submit(db, values);
}

bool SearchParams::erase() {
  return ed.erase();
}
}
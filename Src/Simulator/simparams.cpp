// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#include "simparams.h"
#include "DB/db.h"

namespace LoboLab {

SimParams::SimParams(int id, DB *db)
  : ed("SimParams", id, db) {
  load();
}

SimParams::~SimParams() {
}

SimParams::SimParams(const SimParams &source, bool maintainId)
  : ed(source.ed, maintainId) {
  copy(source);
}

SimParams &SimParams::operator=(const SimParams &source) {
  ed = source.ed;
  copy(source);

  return *this;
}

void SimParams::copy(const SimParams &source) {
  name = source.name;
  timePeriod = source.timePeriod;
  localDistErrorThreshold = source.localDistErrorThreshold;
  expDistErrorThreshold = source.expDistErrorThreshold;
  globalDistErrorThreshold = source.globalDistErrorThreshold;
  zVal = source.zVal; 
}

// Persistence methods

void SimParams::load() {
  name = ed.loadValue(FName).toString();
  timePeriod = ed.loadValue(FTimePeriod).toInt();
  localDistErrorThreshold = ed.loadValue(FLocalDistErrorThreshold).toDouble();
  expDistErrorThreshold = ed.loadValue(FExpDistErrorThreshold).toDouble();
  globalDistErrorThreshold = ed.loadValue(FGlobalDistErrorThreshold).toDouble();
  zVal = ed.loadValue(FzVal).toDouble(); 

  ed.loadFinished();
}

int SimParams::submit(DB *db) {
  QHash<QString, QVariant> values;
  values.insert("Name", name);
  values.insert("TimePeriod", timePeriod);
  values.insert("LocalDistErrorThreshold", localDistErrorThreshold);
  values.insert("ExpDistErrorThreshold", expDistErrorThreshold);
  values.insert("GlobalDistErrorThreshold", globalDistErrorThreshold);
  return ed.submit(db, values);
}

bool SimParams::erase() {
  return ed.erase();
}
}
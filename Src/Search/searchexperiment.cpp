// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#include "searchexperiment.h"
#include "search.h"
#include "Experiment/experiment.h"

namespace LoboLab {

SearchExperiment::SearchExperiment(Search *s, Experiment *e, int isPred)
  : search_(s), experiment_(e), isPrediction_(isPred), ed_("SearchExperiment") {
}

SearchExperiment::SearchExperiment(Search *s, const DBElementData &ref)
  : search_(s), ed_("SearchExperiment", ref) {
  load();
}

SearchExperiment::~SearchExperiment() {
  delete experiment_;
}

SearchExperiment::SearchExperiment(const SearchExperiment &source, Search *s,
                                   bool maintainId)
  : search_(s), ed_(source.ed_, maintainId) {
  experiment_ = new Experiment(*source.experiment_);
  isPrediction_ = 0;
}

// Persistence methods

void SearchExperiment::load() {
  experiment_ = new Experiment(ed_.loadValue(FExperiment).toInt(), ed_.db());
  isPrediction_ = ed_.loadValue(FIsPrediction).toInt();
  ed_.loadFinished();
}

int SearchExperiment::submit(DB *db) {
  QPair<QString, DBElement*> refMember("Search", search_);

  QHash<QString, DBElement*> members;
  members.insert("Experiment", experiment_);

  QHash<QString, QVariant> values;
  values.insert("IsPrediction", isPrediction_);

  return ed_.submit(db, refMember, members, values);
}

bool SearchExperiment::erase() {
  return ed_.erase();
}

}
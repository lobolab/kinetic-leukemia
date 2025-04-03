// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#pragma once

#define SearchAlgo SearchAlgoDetCrowd

#include "DB/dbelementdata.h"
#include "searchexperiment.h"

#include <QList>
#include <QDateTime>

namespace LoboLab {

class SearchExperiment;
class Deme;
class Generation;
class Individual;
class Experiment;
class SearchParams;
class SimParams;
class ErrorCalculator;
class Product;
class SearchAlgo;

class Search : public DBElement {
 public:
  //Search();
  Search(int id, DB *db, bool loadEvolution = true);
  ~Search();

  inline const QString &name() const { return name_; }
  inline const QDateTime &startDatetime() const { return startDatetime_; }
  inline const QDateTime &endDatetime() const { return endDatetime_; }
  inline SearchParams *searchParams() const { return searchParams_; }
  inline SimParams *simParams() const { return simParams_; }
  inline unsigned int randSeed() const { return randSeed_; }

  inline void setName(const QString &newName) { name_ = newName; }
  inline void setRandSeed(unsigned int randSeed) { randSeed_ = randSeed; }

  inline void setStartDatetime(const QDateTime &startDatetime) 
  {startDatetime_ = startDatetime;}
  inline void setEndDatetime(const QDateTime &endDatetime)
  {endDatetime_ = endDatetime;}

  //inline QList<SearchExperiment*> searchExperiments() const { return searchExperiments_; }
  inline int nExperiments() const { return searchExperiments_.size(); }
  inline Experiment *experiment(int i) const { 
    return searchExperiments_.at(i)->experiment(); 
  }
  inline QList<SearchExperiment*> getExpList() { return searchExperiments_; }
  void addExperiment(Experiment *experiment);
  bool removeExperiment(Experiment *experiment);

  inline QList<SearchExperiment*> searchExpPreds() const { return searchExpPreds_; }
  inline int nExpPreds() const { return searchExpPreds_.size(); }
  inline Experiment *expPred(int i) const { return searchExpPreds_.at(i)->experiment(); }

  inline QList<SearchExperiment*> searchExpOthers() const { return searchExpOthers_; }
  inline int nExpOthers() const { return searchExpOthers_.size(); }
  inline Experiment *expOther(int i) const { return searchExpOthers_.at(i)->experiment(); }

  inline const QList<Deme*> &demes() const {return demes_;}
  
  // Functions used during evolution by the search algorithm
  void addNewIndividual(Individual *ind);
  void removeIndividual(Individual *ind);

  inline const QList<Product*> &allProducts() const { return allProducts_; }
  inline const QList<int> &inputLabels() const { return inputLabels_; }
  inline const QList<int> &outputLabels() const { return outputLabels_; }
  inline const int &maxProductLabel() const { return maxProductLabel_; }

  void runEvolution(ErrorCalculator *errorCalculator);

  inline virtual int id() const { return ed_.id(); };
  virtual int submit(DB *db);
  int submitDemes(DB *db);
  int submitParetoFront(DB *db);
  int submitBest(DB *db);
  int submitEvolution(DB *db);
  int submitShallow(DB *db);
  virtual bool erase();

 private:
  Search(const Search &source, bool maintainId = true);
  Search &operator=(const Search &source);
  //void copy(const Search &source, bool maintainId);
  void deleteAll();

  
  void calcParalEvolution(ErrorCalculator *errorCalculator);
  void processMigrationsAndReproduce(int iDeme, double meanGeneration, 
    const QList<SearchAlgo*> &searchAlgors, ErrorCalculator *errorCalculator,
    int *nMigrations, int *migrationStatus);
  void migrateIndividuals(Generation *gen1, Generation *gen2);
  void createMigrationPartners(int *migrationStatus);
  void releaseWaitingDemes(const QList<SearchAlgo*> &searchAlgors, 
    ErrorCalculator *errorCalculator, int *migrationStatus);

  //void updateParetoFront(Individual *newInd);
  void recalculateParetoFront();

  void load(bool loadEvolutionData);
  void loadSearchExperiments();
  QHash<int, Individual*> loadIndividuals();
  void loadDemes(const QHash<int, Individual*> &individualsIdMap);
  void loadAllProducts();

  QString name_;
  SearchParams *searchParams_;
  SimParams *simParams_;
  unsigned int randSeed_;
  QDateTime startDatetime_;
  QDateTime endDatetime_;

  QList<SearchExperiment*> searchExperiments_;
  QList<SearchExperiment*> searchExpPreds_;
  QList<SearchExperiment*> searchExpOthers_;
  QList<Deme*> demes_;
  QList<Individual*> individuals_;
  QList<Individual*> newIndividuals_; // Temporary storage of selected 
                                      // new individuals
  QList<Individual*> paretoFront_; // Pareto front individuals 
                                   // Sorted by complexity and then by error
  QList<Individual*> oldParetoFrontInds_; // Individuals that were in the 
                                          // pareto front.
  QList<Product*> allProducts_;
  QList<int> inputLabels_;
  QList<int> outputLabels_;
  int maxProductLabel_;

  DBElementData ed_;

// Persistence fields
 public:
  enum {
    FName = 1,
    FSearchParams,
    FSimParams,
    FRandSeed,
    FStartDatetime,
    FEndDatetime
  };
};

} // namespace LoboLab

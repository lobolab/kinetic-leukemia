// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#pragma once

#include <QList>

namespace LoboLab {

class SearchParams;
class Search;
class Generation;
class Deme;
class Individual;
class DB;
class Product;

class SearchAlgoDetCrowd {
 public:
  SearchAlgoDetCrowd(Deme *deme, Search *s);
  ~SearchAlgoDetCrowd(void);
  
  inline Deme *deme() const { return deme_; }
  inline Generation *currentGeneration() const { return generation_; }

  QList<Individual*> calcInitialPopulation();
  const QList<Individual*> &reproduce();
  void chooseNextGeneration();

 private:
  Individual *newRandIndividual() const;
  
  Search *search_;
  Deme *deme_;

  SearchParams *searchParams_;
  Generation *generation_;
  QList<Individual*> children_;
  int populationSize_;
  int *randPopulationInd_;

};

} // namespace LoboLab

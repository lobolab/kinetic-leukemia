// Copyright (c) Lobo Lab (lobolab.umbc.edu)
// All rights reserved.

#pragma once

#include "Search/errorcalculator.h"
#include <QList>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QQueue>
#include <QVector>
#include <QElapsedTimer>

namespace LoboLab {

class EvaluatorProducts;
class Model;
class DB;

class ErrorCalculatorMultiThread : public ErrorCalculator {

 public:
  ErrorCalculatorMultiThread(int nDemes, int nThreads, const Search &search);
  virtual ~ErrorCalculatorMultiThread(void);

  void process(int iDeme, const QList<Individual*> &individuals);
  int waitForAnyDeme();

 private:

  class CalculatorThread : public QThread {
   public:
    CalculatorThread(const Search &search, ErrorCalculatorMultiThread *parent);
    ~CalculatorThread(void);

    void stopThread();

   protected:
    void run();

   private:
    void processNextIndividual();
    void waitForIndividuals(); 
    void calcError(const Model &model, double maxError, double *error,
                   double *simTime);

    EvaluatorProducts *evaluator_;
    ErrorCalculatorMultiThread *parent_;
    Individual* individual_;

    bool endThread_;
    QElapsedTimer timer_;
  };

  int nDemes_;
  QQueue<int> pendDemeQueue_;
  QQueue<int> readyDemeQueue_;
  QQueue<Individual*> pendIndQueue_;
  QVector<int> nIndQueuedDeme_;
  QVector<int> nIndPendDeme_;
  
  QMutex mutex_;
  QWaitCondition threadsCondition_;
  QWaitCondition parentCondition_;

  QList<CalculatorThread*> calculatorThreads_;
};

} // namespace LoboLab

// Copyright (c) Lobo Lab (lobolab.umbc.edu)
// All rights reserved.

#include "errorcalculatormultithread.h"
#include "Search/evaluatorproducts.h"
#include "Search/individual.h"
#include "Search/search.h"
#include "Search/searchparams.h"
#include "Common/log.h"
#include <iostream>
#include <time.h>

namespace LoboLab {

ErrorCalculatorMultiThread::ErrorCalculatorMultiThread(int nDemes, 
  int nThreads, const Search &search)
    : ErrorCalculator(), 
      nDemes_(nDemes),
      nIndQueuedDeme_(nDemes_, 0),
      nIndPendDeme_(nDemes_, 0) {
    
  for (int i = 0; i < nThreads; ++i) {
    CalculatorThread *thread = new CalculatorThread(search, this);
    calculatorThreads_.append(thread);
    thread->start();
  }
}

ErrorCalculatorMultiThread::~ErrorCalculatorMultiThread(void) {
  for (int i = 0; i < calculatorThreads_.size(); ++i) {
    CalculatorThread *thread = calculatorThreads_.at(i);
    thread->stopThread();
    delete thread;
  }
}

void ErrorCalculatorMultiThread::process(int iDeme,
                                        const QList<Individual*> &individuals) {
  mutex_.lock();

  pendDemeQueue_.enqueue(iDeme);
  pendIndQueue_.append(individuals);
  nIndQueuedDeme_[iDeme] = individuals.size();
  nIndPendDeme_[iDeme] = individuals.size();

  threadsCondition_.wakeAll();
  
  mutex_.unlock();
}

int ErrorCalculatorMultiThread::waitForAnyDeme() {
  mutex_.lock();

  while (readyDemeQueue_.isEmpty())
    parentCondition_.wait(&mutex_);

  int iDemeReady = readyDemeQueue_.dequeue();

  mutex_.unlock();

  return iDemeReady;
}

// class CalculatorThread

ErrorCalculatorMultiThread::CalculatorThread::CalculatorThread(
    const Search &search,
    ErrorCalculatorMultiThread *p)
  : parent_(p) {
  evaluator_ = new EvaluatorProducts(search);
}

ErrorCalculatorMultiThread::CalculatorThread::~CalculatorThread() {
  stopThread();
  delete evaluator_;
}

void ErrorCalculatorMultiThread::CalculatorThread::run() {
  setPriority(LowestPriority);

  parent_->mutex_.lock();
  endThread_ = false;

  while (!endThread_) {
    if (parent_->pendIndQueue_.isEmpty())
      waitForIndividuals();
    else
      processNextIndividual();
  }

  parent_->mutex_.unlock();
}

void ErrorCalculatorMultiThread::CalculatorThread::processNextIndividual() {
  Individual *nextInd = parent_->pendIndQueue_.dequeue();
  int iDeme = parent_->pendDemeQueue_.head();

  if (--parent_->nIndQueuedDeme_[iDeme] == 0) // last individual in queue from deme
    parent_->pendDemeQueue_.dequeue();

  parent_->mutex_.unlock();

  double error, simTime;
  calcError(*nextInd->model(), nextInd->parentError(), &error, &simTime);
  nextInd->setError(error);
  nextInd->setSimTime(simTime);
  
  parent_->mutex_.lock();

  if (--parent_->nIndPendDeme_[iDeme] == 0) { // last individual processed in deme
    parent_->readyDemeQueue_.enqueue(iDeme);
    parent_->parentCondition_.wakeOne();
  }
}

void ErrorCalculatorMultiThread::CalculatorThread::calcError(const Model &model, 
                                                             double maxError,
                                                             double *error, 
                                                             double *simTime) {
  timer_.start();
  *error = evaluator_->evaluate(model, maxError);
  *simTime = timer_.elapsed() / 1000.0;
}

void ErrorCalculatorMultiThread::CalculatorThread::waitForIndividuals() {
  parent_->threadsCondition_.wait(&parent_->mutex_);
}

void ErrorCalculatorMultiThread::CalculatorThread::stopThread() {
  parent_->mutex_.lock();
  endThread_ = true;
  parent_->threadsCondition_.wakeAll();
  parent_->mutex_.unlock();
  wait();
}

}
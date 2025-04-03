// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#include "simulatorthread.h"

#include "Simulator/simulator.h"
#include "Simulator/simstate.h"

namespace LoboLab {

SimulatorThread::SimulatorThread(Simulator *sim, double timePeriodPerCycle, bool ratecheck, QObject *parent)
  : QThread(parent), 
    simulator_(sim), 
    timePeriodPerCycle_(timePeriodPerCycle) {
  ratecheck_ = ratecheck;
}

SimulatorThread::~SimulatorThread() {
  stopThread();
}

void SimulatorThread::run() {

  mutex_.lock();
  endThread_ = false;

  while (!endThread_) {
    double change = simulator_->simulate(timePeriodPerCycle_, ratecheck_);

    emit simCycleDone(change);

    if (!endThread_)
      thisThreadCond_.wait(&mutex_); //waiting for nextCycle
  }
  
  mutex_.unlock();
}

void SimulatorThread::stopThread() {
  mutex_.lock();
  endThread_ = true;
  thisThreadCond_.wakeOne();
  mutex_.unlock();
  wait();
}

void SimulatorThread::simNextCycle() {
  mutex_.lock();
  thisThreadCond_.wakeOne();
  mutex_.unlock();
}

void SimulatorThread::imageThreadDone() {
}

void SimulatorThread::updateImages() {
}

}
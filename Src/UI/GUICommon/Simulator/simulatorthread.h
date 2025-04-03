// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#pragma once

#include <QMutex>
#include <QSize>
#include <QThread>
#include <QWaitCondition>
#include <QImage>

namespace LoboLab {

class Simulator;

class SimulatorThread : public QThread {
  Q_OBJECT

 public:
  SimulatorThread(Simulator *simulator, double timePeriodPerCyle, bool ratecheck = true, QObject *parent = 0);
  ~SimulatorThread();
  
  inline int getTimePeriodPerCycle() const { return timePeriodPerCycle_; }
  inline void setTimePeriodPerCycle(double tspan) { timePeriodPerCycle_ = tspan; }

  void stopThread();
  void simNextCycle();
  void updateImages(); // blocking function: the calculation is done in the
  // caller's thread

 signals:
  void simCycleDone(double change);

 protected:
  void run();

 private:
  void imageThreadDone();

  QMutex mutex_;
  QWaitCondition thisThreadCond_;
  
  Simulator *simulator_;

  bool endThread_;
  double timePeriodPerCycle_;
  bool ratecheck_;
};

} // namespace LoboLab

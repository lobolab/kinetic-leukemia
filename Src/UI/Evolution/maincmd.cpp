// Copyright (c) Lobo Lab (lobolab.umbc.edu)
// All rights reserved.

#include "maincmd.h"
#include "version.h"
#include "DB/db.h"

#include "Search/search.h"
#include "Search/searchparams.h"
#include "Simulator/simparams.h"
#include "errorcalculatormultithread.h"
#include "Common/log.h"

#include <QTimer>
#include <QCoreApplication>
#include <QStringList>
#include <iostream>

namespace LoboLab {

MainCmd::MainCmd(QObject *parent)
  : QObject(parent) {
  Log::setLogName("modelfinder.log");
  Log::write() << "ModelFinder (version " V_PRODUCTVERSION ")" << endl;

  QTimer::singleShot(0, this, SLOT(run()));
}

MainCmd::~MainCmd(void) {
  closeDB();

  Log::write() << "Program closed. End of log." << endl;
  Log::closeLog();
}

void MainCmd::run() {
  QStringList args = QCoreApplication::arguments();

  QString dbFileName;
  int searchId = 0;

  if (args.size() > 1)
    dbFileName = args.at(1);
  if (args.size() > 2)
    searchId = args.at(2).toInt();
  if (args.size() > 3)
    nThreads_ = args.at(3).toInt();
  else
    nThreads_ = 3;


  if (searchId && connectDB(db_, dbFileName)) {
    search_ = new Search(searchId, &db_, false);
    runSearch();
    delete search_;
  } else {
    Log::write() << "Incorrect arguments." << endl;
    std::cout << "MainCmd Threads (version " V_PRODUCTVERSION ")" << std::endl;
    std::cout << "Copyright Lobo Lab (lobolab.umbc.edu)" <<
              std::endl;
    std::cout << "Usage: " <<
              QCoreApplication::applicationName().toStdString()
              << "search_database_name search_id n_threads" << std::endl;
    quit();
  }
}

void MainCmd::quit() {
  QCoreApplication::instance()->quit();
}

void MainCmd::runSearch() {
  QElapsedTimer timer;
  timer.start();
  
  int nDemes = search_->searchParams()->nDemes;
  ErrorCalculatorMultiThread errorCalculator(nDemes, nThreads_, *search_);

  search_->runEvolution(&errorCalculator);

  int s = timer.elapsed() / 1000;
  int d = s/(24*60*60);
  int h = (s % (24*60*60)) / (60*60);
  int m = (s % (60*60)) / 60;
  s %= 60;

  std::cout << "MainCmd: Search finished. Ending program. Elapsed time: " <<
            d << "d" << h << "h" << m << "m" << s << "s" << std::endl;
  Log::write() << "MainCmd: Search finished. Ending program. Elapsed time: " <<
    d << "d" << h << "h" << m << "m" << s << "s" << endl;
  quit();
}


void MainCmd::closeDB() {
  db_.disconnect();
}

bool MainCmd::connectDB(DB &db, const QString &dbFileName) {
#ifdef QT_DEBUG
  int error = db.connect(dbFileName, false); // Slow connection: checking foreign keys
#else
  int error = db.connect(dbFileName, true); // Fast connection: no foreign keys check, some operations are in memory, and disk write is delayed
#endif

  if (!error) {
    //ExperimentFactory::instance(&db);
  }
  else if (error == 1) {
    Log::write() << "Unable to open the database file (" << dbFileName << ")."
      << endl;
  }
  if (error == 2) {
    Log::write() << "Unable to find the database file (" << dbFileName << ")."
      << endl;
  }

  return error == 0;
}

} // namespace LoboLab

// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#include "mainwindow.h"
#include "version.h"
#include "errorplotwidget.h"
#include "simulatorwindow.h"
#include "UI/GUICommon/modelform.h"
#include "Experiment/phenotype.h"
#include "Experiment/product.h"
#include "DB/db.h"
#include "DB/dbsea.h"
#include "Common/fileutils.h"
#include "Search/search.h"
#include "Search/deme.h"
#include "Search/individual.h"
#include "Search/generation.h"
#include "Search/generationindividual.h"
#include "Search/evaluatorproducts.h"

#include "Model/model.h"
#include "Simulator/simparams.h"
#include "Simulator/simulator.h"
#include "UI/GUICommon/Simulator/simulatorthread.h"

#include <QAction>
#include <QMenuBar>
#include <QToolBar>
#include <QLabel>
#include <QStatusBar>
#include <QMessageBox>
#include <QApplication>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QTimer>
#include <QDebug>
#include <QFileDialog>
#include <QSettings>
#include <QSplitter>
#include <QProcess>
#include <QKeyEvent>
#include <QClipboard>

namespace LoboLab {

MainWindow::MainWindow()
    : search_(NULL),
      searchModel_(NULL), 
      demeModel_(NULL), 
      generationModel_(NULL),
      individualsModel_(NULL), 
      individualsSortProxyModel_(NULL),
      maxRangeValue_(0) {
  db_ = new DB();

  createWidgets();
  createActionsAndMenus();
  createToolBars();
  createStatusBar();

  setWindowTitle(tr("Search viewer"));
  readSettings();
  // This allows the window to show before trying to connect to the db
  QTimer::singleShot(0, this, SLOT(initDataBase()));
}

MainWindow::~MainWindow() {
  writeSettings();
  clearProducts();
  closeDB();
  delete db_;
}

void MainWindow::createWidgets() {
  setWindowIcon(QIcon(":/Images/quadrupleHead32.png"));

  searchComboBox_ = new QComboBox();
  connect(searchComboBox_, SIGNAL(currentIndexChanged(int)),
          this, SLOT(searchComboChanged(int)));

  QGroupBox *timeGeneGroup = new QGroupBox();
  timeButton_ = new QRadioButton("Time");
  geneButton_ = new QRadioButton("Generation");
  timeButton_->setChecked(true);
  
  connect(timeButton_, SIGNAL(toggled(bool)),
          this, SLOT(timeGenerationToggled(bool)));
  connect(geneButton_, SIGNAL(toggled(bool)),
          this, SLOT(timeGenerationToggled(bool)));
  
  QGroupBox *bestAllDemeGroup = new QGroupBox();
  bestButton_ = new QRadioButton("Best");
  allButton_ = new QRadioButton("All");
  demeButton_ = new QRadioButton("Deme");
  bestButton_->setChecked(true);
  
  connect(bestButton_, SIGNAL(toggled(bool)),
          this, SLOT(bestAllDemeToggled(bool)));
  connect(allButton_, SIGNAL(toggled(bool)),
          this, SLOT(bestAllDemeToggled(bool)));
  connect(demeButton_, SIGNAL(toggled(bool)),
          this, SLOT(bestAllDemeToggled(bool)));
  
  QGroupBox *logLinearGroup = new QGroupBox();
  logButton_ = new QRadioButton("Log");
  linearButton_ = new QRadioButton("Linear");
  logButton_->setChecked(true);
  
  connect(logButton_, SIGNAL(toggled(bool)),
          this, SLOT(logLinearToggled(bool)));
  connect(linearButton_, SIGNAL(toggled(bool)),
          this, SLOT(logLinearToggled(bool)));
    
  demeComboBox_ = new QComboBox();
  demeComboBox_->setDisabled(true);
  connect(demeComboBox_, SIGNAL(currentIndexChanged(int)),
          this, SLOT(demeComboChanged(int)));
  
  errorPlotWidget_ = new ErrorPlotWidget();
  errorPlotWidget_->setStyleSheet("background-color:white;");
  
  filterListCheckBox_ = new QCheckBox("Filter list:");
  connect(filterListCheckBox_, SIGNAL(stateChanged(int)),
          this, SLOT(filterListCheckChanged(int)));

  listSliderLabel_ = new QLabel();
  listSlider_ = new QSlider(Qt::Horizontal);
  listSlider_->setMinimum(0);
  listSlider_->setSingleStep(1);
  listSlider_->setPageStep(1);
  listSlider_->setEnabled(false);
  connect(listSlider_, SIGNAL(valueChanged(int)),
          this, SLOT(listSliderChanged()));
  connect(listSlider_, SIGNAL(sliderReleased()),
          this, SLOT(listSliderChanged()));

  individualsTable_ = new QTableView();
  connect(individualsTable_, SIGNAL(doubleClicked(const QModelIndex &)),
          this, SLOT(individualClicked()));
  

  QHBoxLayout *lay = new QHBoxLayout();
  lay->addWidget(timeButton_);
  lay->addWidget(geneButton_);
  timeGeneGroup->setLayout(lay);

  lay = new QHBoxLayout();
  lay->addWidget(bestButton_);
  lay->addWidget(allButton_);
  lay->addWidget(demeButton_);
  lay->addWidget(demeComboBox_, 1);
  bestAllDemeGroup->setLayout(lay);

  lay = new QHBoxLayout();
  lay->addWidget(logButton_);
  lay->addWidget(linearButton_);
  logLinearGroup->setLayout(lay);

  QHBoxLayout *topLay = new QHBoxLayout();
  topLay->addWidget(new QLabel("Search:"));
  topLay->addWidget(searchComboBox_, 3);
  topLay->addSpacing(15);
  topLay->addWidget(timeGeneGroup);
  topLay->addWidget(bestAllDemeGroup);
  topLay->addWidget(logLinearGroup);

  QHBoxLayout *genLay = new QHBoxLayout();
  genLay->addWidget(filterListCheckBox_);
  genLay->addWidget(listSliderLabel_);
  genLay->addWidget(listSlider_, 1);

  QVBoxLayout *midLay = new QVBoxLayout();
  midLay->addLayout(topLay);
  midLay->addWidget(errorPlotWidget_, 3);
  midLay->addLayout(genLay);

  QWidget *midWidget = new QWidget();
  midWidget->setLayout(midLay);

  QSplitter *splitter = new QSplitter();
  splitter->setOrientation(Qt::Vertical);
  splitter->addWidget(midWidget);
  splitter->addWidget(individualsTable_);
  splitter->setSizes(QList<int>() << 250 << 150);

  setCentralWidget(splitter);
}

void MainWindow::createActionsAndMenus() {
  QAction *newDBAction = new QAction(tr("&New database..."), this);
  newDBAction->setShortcut(tr("Ctrl+N"));
  newDBAction->setStatusTip(tr("Create a new database file"));
  newDBAction->setIcon(QIcon(
                         ":/Images/famfamfam_silk_icons/page_white_database.png"));
  connect(newDBAction, SIGNAL(triggered()), this, SLOT(newDB()));

  QAction *openDBAction = new QAction(tr("&Open database..."), this);
  openDBAction->setShortcut(tr("Ctrl+O"));
  openDBAction->setStatusTip(tr("Load an existing database file"));
  openDBAction->setIcon(QIcon(
                          ":/Images/famfamfam_silk_icons/folder_database.png"));
  connect(openDBAction, SIGNAL(triggered()), this, SLOT(openDB()));


  QAction *openDBAppDirAction = new QAction(tr("&Open database app dir..."), this);
  openDBAppDirAction->setShortcut(tr("Ctrl+A"));
  openDBAppDirAction->setStatusTip(tr("Load an existing database file from the application directory"));
  openDBAppDirAction->setIcon(QIcon(
                                ":/Images/famfamfam_silk_icons/folder_database.png"));
  connect(openDBAppDirAction, SIGNAL(triggered()), this, SLOT(openDBAppDir()));

  saveAsDBAction_ = new QAction(tr("&Save database as..."), this);
  saveAsDBAction_->setShortcut(tr("Ctrl+S"));
  saveAsDBAction_->setStatusTip(tr("Save the current database into a different"
                                  " file"));
  saveAsDBAction_->setIcon(QIcon(
                            ":/Images/famfamfam_silk_icons/database_save.png"));
  connect(saveAsDBAction_, SIGNAL(triggered()), this, SLOT(saveAsDB()));

  closeDBAction_ = new QAction(tr("C&lose database"), this);
  closeDBAction_->setShortcut(tr("Ctrl+L"));
  closeDBAction_->setStatusTip(tr("Close the current database"));
  closeDBAction_->setIcon(QIcon(
                           ":/Images/famfamfam_silk_icons/database_go.png"));
  connect(closeDBAction_, SIGNAL(triggered()), this, SLOT(closeDB()));

  importDBAction_ = new QAction(tr("&Import database..."), this);
  importDBAction_->setShortcut(tr("Ctrl+I"));
  importDBAction_->setStatusTip(tr("Import the experiments of another database "
                                  "into the current database"));
  importDBAction_->setIcon(QIcon(
                            ":/Images/famfamfam_silk_icons/database_add.png"));
  connect(importDBAction_, SIGNAL(triggered()), this, SLOT(importDB()));

  QAction *exitAction = new QAction(tr("Exit"), this);
  exitAction->setShortcut(tr("Alt+F4"));
  exitAction->setStatusTip(tr("Exit the application"));
  exitAction->setIcon(QIcon(
                        ":/Images/famfamfam_silk_icons/door_in.png"));
  connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));

  QAction *aboutAction = new QAction(tr("&About"), this);
  aboutAction->setShortcut(tr("Ctrl+A"));
  aboutAction->setStatusTip(tr("Show the applications's About box"));
  aboutAction->setIcon(QIcon(
                         ":/Images/famfamfam_silk_icons/brick.png"));
  connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));

  morDistAction_ = new QAction(tr("Morphology distance calculator"), this);
  morDistAction_->setStatusTip(tr("Tool to calculate the distance between "
                                 "morphologies"));
  connect(morDistAction_, SIGNAL(triggered()), this, SLOT(morDistCalc()));

  morDistCompAction_ = new QAction(tr("Morphology distance comparator"), this);
  morDistCompAction_->setStatusTip(tr("Tool to compare the distance of a "
                                     "morphology with the rest of morphologies in the database"));
  connect(morDistCompAction_, SIGNAL(triggered()), this, SLOT(morDistComp()));

  deleteExperimentsAction_ = new QAction(tr("Delete all experiments"), this);
  deleteExperimentsAction_->setStatusTip(tr("Delete all experiments in the database!"));
  deleteExperimentsAction_->setIcon(QIcon(
                                     ":/Images/famfamfam_silk_icons/table_delete.png"));
  connect(deleteExperimentsAction_, SIGNAL(triggered()),
          this, SLOT(deleteAllExperiments()));

  deleteExpAndManipulationsAction_ = new QAction(tr("Delete all experiments and manipulations"), this);
  deleteExpAndManipulationsAction_->setStatusTip(tr("Delete all experiments and manipulations in the database!"));
  deleteExpAndManipulationsAction_->setIcon(QIcon(
        ":/Images/famfamfam_silk_icons/table_delete.png"));
  connect(deleteExpAndManipulationsAction_, SIGNAL(triggered()),
          this, SLOT(deleteAllExpAndManipulations()));

  deleteAllAction_ = new QAction(tr("Delete all"), this);
  deleteAllAction_->setStatusTip(tr("Delete all experiments, manipulations, and morphologies in the database!"));
  deleteAllAction_->setIcon(QIcon(
                             ":/Images/famfamfam_silk_icons/database_delete.png"));
  connect(deleteAllAction_, SIGNAL(triggered()),
          this, SLOT(deleteAll()));

  // Create Menus
  QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(newDBAction);
  fileMenu->addAction(openDBAction);
  fileMenu->addAction(openDBAppDirAction);
  fileMenu->addAction(saveAsDBAction_);
  fileMenu->addAction(closeDBAction_);

  QAction *separator = new QAction(this);
  separator->setSeparator(true);
  fileMenu->addAction(separator);

  fileMenu->addAction(exitAction);

  QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
  editMenu->addAction(deleteExperimentsAction_);
  editMenu->addAction(deleteExpAndManipulationsAction_);
  editMenu->addAction(deleteAllAction_);

  QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
  helpMenu->addAction(aboutAction);
}

void MainWindow::createToolBars() {
}

void MainWindow::createStatusBar() {
  statusBarText_ = new QLabel();
  statusBar()->addWidget(statusBarText_, 1);
}

void MainWindow::readSettings() {
  QCoreApplication::setOrganizationName("Lobo Lab");
  QCoreApplication::setOrganizationDomain("lobolab.umbc.edu");
  QCoreApplication::setApplicationName("CancerSearchViewer");

  QSettings settings;
  settings.beginGroup("MainWindow");
  resize(settings.value("size", QSize(800, 600)).toSize());
  if (settings.contains("pos")) // This makes the first time a centered window.
    move(settings.value("pos", QPoint()).toPoint());
  if (settings.value("maximized", false).toBool())
    showMaximized();
  settings.endGroup();

  QStringList args = QCoreApplication::arguments();
  if (args.size() > 1) {
    dbFileName_ = args.at(1);
  } else {
    settings.beginGroup("DB");
    dbFileName_ = settings.value("lastFile", "").toString();
    settings.endGroup();
  }
}

void MainWindow::writeSettings() {
  QSettings settings;
  settings.beginGroup("MainWindow");
  if (isMaximized())
    settings.setValue("maximized", isMaximized());
  else {
    settings.setValue("maximized", false);
    settings.setValue("size", size());
    settings.setValue("pos", pos());
  }

  settings.endGroup();
  settings.beginGroup("DB");
  settings.setValue("lastFile", dbFileName_);
  settings.endGroup();
}

// private slot
void MainWindow::newDB() {
  QString fileName = QFileDialog::getSaveFileName(this, "New database", "",
                     "Search Databases (*.sdb)");

  if (!fileName.isEmpty()) {
    if (!fileName.toLower().endsWith(".sdb")) {
      fileName += ".sdb";
    }

    closeDB();

    if (db_->createEmptyDB(fileName)) {
      DBSea::buildDB(db_);

      db_->disconnect();
      dbFileName_ = fileName;
      connectDB();
    } else
      QMessageBox::critical(this, qApp->tr("Cannot create new database file"),
                            qApp->tr("Unable to create a new database file."), QMessageBox::Cancel);
  }

}

// Called only from macapp (double click open file)
void MainWindow::openDB(const QString &fileName) {
  dbFileName_ = fileName;
  connectDB();
}

// private slot
void MainWindow::openDB() {
  QString fileName = QFileDialog::getOpenFileName(this, "Open database",
                     dbFileName_, "Search databases (*.sdb)");

  if (!fileName.isEmpty()) {
    dbFileName_ = fileName;
    connectDB();
  }
}

// private slot
void MainWindow::openDBAppDir() {
  QString fileName = QFileDialog::getOpenFileName(this, "Open database:" +
                     qApp->applicationDirPath(),
                     qApp->applicationDirPath() + QString("\\"), "Search databases (*.sdb)");

  if (!fileName.isEmpty()) {
    dbFileName_ = fileName;
    connectDB();
  }
}

// private slot
void MainWindow::saveAsDB() {
  if (!dbFileName_.isEmpty()) {
    QString fileName = QFileDialog::getSaveFileName(this, "Save database", "",
                       "Search Databases (*.sdb)");

    if (!fileName.isEmpty()) {
      if (!fileName.toLower().endsWith(".sdb")) {
        fileName += ".sdb";
      }

      QString oldFileName = dbFileName_;
      closeDB();

      FileUtils::copyRaw(oldFileName, fileName);

      dbFileName_ = fileName;
      connectDB();
    }
  }
}

// private slot
void MainWindow::closeDB() {
  if (!dbFileName_.isEmpty()) {
    delete individualsSortProxyModel_;
    individualsSortProxyModel_ = NULL;
    delete individualsModel_;
    individualsModel_ = NULL;
    delete generationModel_;
    generationModel_ = NULL;
    delete demeModel_;
    demeModel_ = NULL;
    delete searchModel_;
    searchModel_ = NULL;
    delete search_;
    search_ = NULL;

    dbFileName_.clear();
    updateStatus();
    db_->disconnect();
  }
}

void MainWindow::connectDB(bool silent) {
  statusBarText_->setText("Opening database...");
  QString fileName = dbFileName_;
  dbFileName_.clear();
  int error = db_->connect(fileName);
  if (!error) {
    dbFileName_ = fileName;

    searchModel_ = db_->newTableModel("Search", "Name");
    searchComboBox_->setModelColumn(Search::FName);
    searchComboBox_->setModel(searchModel_);

    loadProducts();
    updatePlot();
    updateIndividuals();
    updateStatus();
  } else if (!silent) {
    if (error == 2) {

      QMessageBox::critical(this, qApp->tr("Cannot find database file"),
                            QString("Unable to find the database file (%1).").arg(fileName),
                            QMessageBox::Cancel);
    } else if (error == 1) {
      QMessageBox::critical(this, qApp->tr("Cannot open database"),
                            qApp->tr("Unable to establish a database connection."),
                            QMessageBox::Cancel);
    }
  }
}

void MainWindow::loadProducts() {
  clearProducts();

  QSqlQuery *query = db_->newTableQuery("Product");
  while (query->next()) {
    Product *product = new Product(query->value(0).toInt(), db_);
    if (product->id() < 77)
      products_.insert(product->id(), product);
  }
  delete query;
}

void MainWindow::clearProducts() {
  for (QHash<int, Product*>::const_iterator i = products_.constBegin();
         i!=products_.constEnd(); ++i)
    delete i.value();
}

// private slot
// Try to load the last used database.
void MainWindow::initDataBase() {
  if (!dbFileName_.isEmpty()) {
    if (QFile(dbFileName_).exists())
      connectDB(true);
    else
      dbFileName_.clear();
  }

  updateStatus();
}

void MainWindow::updateStatus() {
  if (dbFileName_.isEmpty()) {
    setWindowTitle(tr("Search Viewer"));
    statusBarText_->setText("No database opened");

    saveAsDBAction_->setEnabled(false);
    closeDBAction_->setEnabled(false);
    importDBAction_->setEnabled(false);
    morDistAction_->setEnabled(false);
    morDistCompAction_->setEnabled(false);
    deleteExperimentsAction_->setEnabled(false);
    deleteExpAndManipulationsAction_->setEnabled(false);
    deleteAllAction_->setEnabled(false);

  } else {
    QFileInfo dbFileInfo(dbFileName_);
    QString windowTitle = dbFileInfo.fileName();
    if (!dbFileInfo.isWritable())
      windowTitle += " (read only)";
    windowTitle += " - Search Viewer";
    setWindowTitle(windowTitle);
    statusBarText_->setText("Database opened");

    saveAsDBAction_->setEnabled(true);
    closeDBAction_->setEnabled(true);
    importDBAction_->setEnabled(true);
    morDistAction_->setEnabled(true);
    morDistCompAction_->setEnabled(true);
    deleteExperimentsAction_->setEnabled(true);
    deleteExpAndManipulationsAction_->setEnabled(true);
    deleteAllAction_->setEnabled(true);
  }
}

// private slot
void MainWindow::updateModel() {
  if (!dbFileName_.isEmpty()) {
  }
}

void MainWindow::databaseCorrupted() {
  QMessageBox::critical(this, qApp->tr("Database file corrupted"),
                        qApp->tr("The database file is corrupted."),
                        QMessageBox::Cancel);
  closeDB();
}

void MainWindow::updateStatusText(int n, const QString &eleName,
                                  const QString &eleNamePlural) {
  QString statusText;

  if (!eleName.isEmpty()) {
    if (n == 0)
      statusText = "No " + eleNamePlural + " in the database";
    else if (n == 1)
      statusText = "1 " + eleName + " in the database";
    else
      statusText = QString("%1 " + eleNamePlural + " in the database")
                   .arg(n);
  }

  statusBarText_->setText(statusText);
}

void MainWindow::about() {
  QMessageBox::about(this, tr("About AML Model Finder"),
                     "<table width=\"100%\"><tr>"
                     "<td align=\"left\">"
                     "<span style=\"font-size: 18pt;\">AML Model Finder</span>"
                     "</td></tr>"
                     "<tr><td align=\"left\">"
                     "<span style=\"font-size: 12pt;\">(AML Model Finder)</span>"
                     "</td>"
                     "<td align=\"right\" valign=\"bottom\">"
                     "Version " V_PRODUCTVERSION
                     "</td>"
                     "</tr></table>"
                     "<hr/>"
                     "<p>Copyright &copy; Lobo Lab (<a style='color:black;text-decoration:none' href=mailto:lobo@umbc.edu>lobo@umbc.edu</a>)"
                     "<p><small>This program makes use of "
                     "<a  style='text-decoration:none' href=http://qt-project.org>Qt</a> (<a style='text-decoration:none' href=http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html>"
                     "license</a>) and the <a style='text-decoration:none' href=http://www.famfamfam.com/lab/icons/silk>"
                     "Silk icon set</a> (<a style='text-decoration:none' href=http://creativecommons.org/licenses/by/2.5>license</a>), and "
                     "<a style='text-decoration:none' href=http://qwt.sourceforge.net>Qwt</a> (<a style='text-decoration:none' href=http://qwt.sourceforge.net/qwtlicense.html>license</a>)."
                     "</small>");
}

void MainWindow::deleteAllExperiments() {
  int ret = QMessageBox::warning(this, "Danger!", "This action will delete ALL"
                                 " the experiments stored in the database! Are you sure?",
                                 QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);

  if (ret == QMessageBox::Ok) {
  }
}

void MainWindow::deleteAllExpAndManipulations() {
  int ret = QMessageBox::warning(this, "Danger!", "This action will delete ALL"
                                 " the experiments and ALL the manipulations stored in the database! Are you sure?",
                                 QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);

  if (ret == QMessageBox::Yes) {
  }
}

void MainWindow::deleteAll() {
  int ret = QMessageBox::warning(this, "Danger!", "This action will delete ALL"
                                 " the experiments, manipulations, and morphologies stored in the database! Are you sure?",
                                 QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);

  if (ret == QMessageBox::Yes) {
  }
}

void MainWindow::simEnds() {
}

void MainWindow::searchVals() {
  Model *sModel;
  Simulator * exSim;

  QSettings settings;
  settings.beginGroup("search vals");

  QString fileName = db_->fileBaseName();
  fileName.chop(4);
  fileName.append("_SearchVals.csv");

  QFile file(fileName);
  file.open(QFile::WriteOnly);

  QTextStream out(&file);

  double gval;
  double error;
  int comp;
  int generation;
  QString mstring;
  exSim = new Simulator(*search_);
  QSqlQuery *queryM = db_->newQuery("SELECT Individual.Model, Individual.Complexity, Individual.Error, GenerationIndividual.Generation FROM Individual INNER JOIN GenerationIndividual ON "
    "GenerationIndividual.Individual = Individual.Id "
    "INNER JOIN Generation ON GenerationIndividual.Generation = Generation.Id");
  queryM->first();
  int i = 0;
  while (queryM->next()) {
    mstring = queryM->value(0).toString();
    error = queryM->value(2).toDouble();
    comp = queryM->value(1).toInt();
    generation = queryM->value(3).toInt();
    sModel = new Model;
    sModel->loadFromString(mstring);
    exSim->loadModel(sModel);
    exSim->loadExperiment(search_->experiment(1));
    exSim->initialize();
    exSim->simulate(search_->simParams()->timePeriod, false);
    gval = exSim->simulatedState().product(0);
    out << i << ",";
    out << comp << ",";
    out << error << ",";
    out << generation << ",";
    out << gval << endl;
    i++;
  }
  
  file.close();
}


//private slot 
void MainWindow::modelAnalysis() {
  QSettings settings;
  settings.beginGroup("ModelAnalysis");
  QDir dir; 
  QString curDir = dir.currentPath(); 
  
  QStringList dbFiles = QFileDialog::getOpenFileNames(this, 
    "Select One or More Databases to Open", curDir, "SDB File (*.sdb)");

  QStringList allFiles;
  QString dbPath;
  int pos = 0;
  if(!dbFiles.isEmpty()){
    for (int i = 0; i < dbFiles.size(); i++) {
      QString dirFixed = dbFiles.at(i);
      allFiles.append(dirFixed);
    }
    dbPath = dbFiles.at(0);
    int pos = dbPath.lastIndexOf(QChar('/'));
    QString dbDir = dbPath.left(pos); 

 
    for (int i = 0; i < dbFiles.size(); i++) {
      openDB(dbFiles.at(i));
      QSqlQuery* modQuery = db_->newQuery("SELECT Individual.Id FROM Individual "
        "ORDER BY Individual.Error, Individual.Complexity, Individual.SimTime "
        "LIMIT 1");
      modQuery->first();
      int bestModId = modQuery->value(0).toInt();
      delete modQuery;

      Individual *ind = new Individual(bestModId, db_);
      Model *model = new Model(*ind->model());
      ModelForm *modelForm = new ModelForm(search_->getExpList(), model, products_,
        db_, this, true, QString("Model of individual %1").arg(ind->id()));
      modelForm->runModelAnalysis();

      SimulatorWindow *simWind = new SimulatorWindow(ind, search_, products_,
        db_, true, this);
      simWind->show();
      simWind->raise();
      simWind->activateWindow();
      simWind->autoPredictions(); 
      delete simWind;
       
      //add file paths to allFiles List
      QString dbName = db_->fileBaseName();
      dbName.chop(4);
      QString saFile = curDir + "/" + dbName + QString("_SensitivityAnalysis.csv");
      QString predFile = curDir + "/" + dbName + QString("_predictions.csv");
      QString predFile2 = curDir + "/" + dbName + QString("_NovelParent_predictions.csv");

      allFiles.append(saFile); 
      allFiles.append(predFile); 
      allFiles.append(predFile2);
      closeDB();
    }
   
    QString pyScriptDir = QFileDialog::getExistingDirectory(this,
      "Select Python Script Folder", curDir, QFileDialog::ShowDirsOnly
      | QFileDialog::DontResolveSymlinks);
    
    QProcess* process = new QProcess(this);
    connect(process, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(processError(QProcess::ProcessError)));
    process->setStandardErrorFile("processError.txt");
    process->setStandardOutputFile("processStd.txt");
    process->setWorkingDirectory(pyScriptDir);

    QString pyScript = pyScriptDir + QString("\\callPyScripts.py");
    allFiles.insert(0, pyScript); 
    const QStringList files = allFiles; 
    
    QFile file("commands.txt");
    file.open(QFile::WriteOnly);
    QTextStream out(&file);
    out << QString("python ");
    for (int i = 0; i < files.size(); ++i) {
      out << files[i] << QString(" ");
    }
    file.close();
    
    process->start("python.exe", files);
    process->waitForFinished();
    QProcess::ProcessError check = process->error();  
    delete process; 
  }  
}

void MainWindow::processError(QProcess::ProcessError error) {
  int a = 0;
}

void MainWindow::emptyExperimentTables() {
  QList<QString> tables;
  tables.append("ExperimentDrug");
  tables.append("ExperimentRNAi");
  tables.append("ResultantMorphology");
  tables.append("ResultSet");
  tables.append("Experiment");
  db_->emptyTables(tables);
}

void MainWindow::emptyManipulationTables() {
  QList<QString> tables;
  tables.append("Manipulation");
  tables.append("RemoveActionAreaPoint");
  tables.append("CropActionAreaPoint");
  tables.append("IrradiationActionAreaPoint");
  db_->emptyTables(tables);

  tables.clear();
  // the rest is not foreign key compliant
  tables.append("ManipulationAction");
  tables.append("MorphologyAction");
  tables.append("RemoveAction");
  tables.append("CropAction");
  tables.append("JoinAction");
  tables.append("IrradiationAction");
  db_->emptyTablesNoKeys(tables);
}

void MainWindow::emptyMorphologyTables() {
  QList<QString> tables;
  tables.append("Organ");
  tables.append("LineOrgan");
  tables.append("SpotOrgan");
  tables.append("RegionsLink");
  tables.append("RegionParam");
  tables.append("RegionParam");
  tables.append("Region");
  tables.append("Morphology");

  db_->emptyTables(tables);
}

void MainWindow::closeEvent(QCloseEvent *event) {
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
  if (event->matches(QKeySequence::Copy))
    QApplication::clipboard()->setText(individualsTable_->currentIndex().data().toString());
  else
    QMainWindow::keyPressEvent(event);
}

void MainWindow::searchComboChanged(int index) {
  individualsTable_->reset();
  delete individualsSortProxyModel_;
  individualsSortProxyModel_ = NULL;
  delete individualsModel_;
  individualsModel_ = NULL;
  errorPlotWidget_->clear();
  delete generationModel_;
  generationModel_ = NULL;
  listSlider_->setEnabled(false);
  listSliderLabel_->setText("");
  demeComboBox_->clear();
  delete demeModel_;
  demeModel_ = NULL;
  delete search_;
  search_ = NULL;

  if (index > -1) {
    int searchId = db_->getModelId(searchModel_, index);
    search_ = new Search(searchId, db_, false);
    demeModel_ = db_->newTableModel("Deme", "Search", searchId);
    demeComboBox_->setModel(demeModel_);
  }
}

void MainWindow::demeComboChanged(int index) {
  if (demeComboBox_->isEnabled() && index > -1) {
    updatePlot();
    updateIndividuals();
  }
}

void MainWindow::timeGenerationToggled(bool checked) {
  if (checked)
    updatePlot();
}

void MainWindow::bestAllDemeToggled(bool checked) {
  if (checked) {
    updatePlot();

    if (demeComboBox_->isEnabled() && !demeButton_->isChecked()) {
      demeComboBox_->setEnabled(false);
      updateIndividuals();
    } else if (!demeComboBox_->isEnabled() && demeButton_->isChecked()) {
      demeComboBox_->setEnabled(true);
      updateIndividuals();
    }
  }
}

void MainWindow::logLinearToggled(bool checked) {
  if (checked) {
    if (logButton_->isChecked())
      errorPlotWidget_->setLogScale();
    else
      errorPlotWidget_->setLinScale();
  }
}

void MainWindow::updateIndividuals() {
  if (maxRangeValue_ > 0)  {
    filterListCheckBox_->setEnabled(true);
    listSlider_->setMinimum(0);
    listSlider_->setMaximum(maxRangeValue_-1);

    if (filterListCheckBox_->isChecked()) {
      listSlider_->setEnabled(true);
      listSliderLabel_->setText(QString("%1").arg(listSlider_->value()));
    } else {
      listSlider_->setEnabled(false);
      listSliderLabel_->setText("");
    }
  } else {
    filterListCheckBox_->setEnabled(false);
    listSlider_->setEnabled(false);
    listSliderLabel_->setText("");
  } 

  updateIndividualsList();
}

void MainWindow::filterListCheckChanged(int state) {
  updateIndividuals();
}

void MainWindow::updatePlot() {
  setCursor(Qt::WaitCursor);
  QApplication::processEvents();

  if (timeButton_->isChecked()) {
    if (bestButton_->isChecked()) {
      maxRangeValue_ = errorPlotWidget_->plotTimeBest(search_->id(), db_);
    } else if (allButton_->isChecked()) {
      maxRangeValue_ = errorPlotWidget_->plotTimeAll(search_->id(), db_);
    } else { // demeButton
      int selectedDemeId = db_->getModelId(demeModel_, 
                                           demeComboBox_->currentIndex());
      maxRangeValue_ = errorPlotWidget_->plotTimeDeme(selectedDemeId, db_);
    }
  } else { // Generation button
    if (bestButton_->isChecked()) {
      maxRangeValue_ = errorPlotWidget_->plotGeneBest(search_->id(), db_);
    } else if (allButton_->isChecked()) {
      maxRangeValue_ = errorPlotWidget_->plotGeneAll(search_->id(), db_);
    } else { // demeButton
      int selectedDemeId = db_->getModelId(demeModel_, 
                                           demeComboBox_->currentIndex());
      maxRangeValue_ = errorPlotWidget_->plotGeneDeme(selectedDemeId, db_);
    }
  }

  setCursor(Qt::ArrowCursor);
}

void MainWindow::listSliderChanged() {
  listSliderLabel_->setText(QString("%1").arg(listSlider_->value()));

  if (!listSlider_->isSliderDown())
    updateIndividualsList();
}

void MainWindow::updateIndividualsList() {
  setCursor(Qt::WaitCursor);
  QApplication::processEvents();
  
  delete individualsSortProxyModel_;
  individualsSortProxyModel_ = NULL;
  delete individualsModel_;
  individualsModel_ = NULL;
  
  QString sql= "SELECT MIN(Generation.Time) AS 'Time', "
    "MIN(Generation.Ind) AS 'First Gen', "
    "MAX(Generation.Ind) AS 'Last Gen', MIN(Generation.Deme) AS 'Min Deme', "
    "MAX(Generation.Deme) AS 'Max Deme', Individual.Id, Individual.Error, "
    "Individual.Complexity, Individual.SimTime, "
    "Individual.Parent1, Individual.Parent2 "
    "FROM Individual INNER JOIN GenerationIndividual ON "
    "GenerationIndividual.Individual = Individual.Id "
    "INNER JOIN Generation ON GenerationIndividual.Generation = Generation.Id";

  bool firstCond = true;

  if (demeButton_->isChecked()) {
    firstCond = false;
    int selectedDemeId = db_->getModelId(demeModel_, 
                                         demeComboBox_->currentIndex());
    sql += QString(" WHERE Generation.Deme = %1").arg(selectedDemeId);
  }
    
  if (filterListCheckBox_->isChecked()) {
    if (firstCond) 
      sql += " WHERE ";
    else
      sql += " AND ";

    if (timeButton_->isChecked())
      sql += QString("Generation.Time = %1").arg(listSlider_->value());
    else
      sql += QString("Generation.Ind = %1").arg(listSlider_->value());
  }

  sql += " GROUP BY Individual.Id";
  
  individualsModel_ = db_->newModel(sql);
  individualsSortProxyModel_ = new QSortFilterProxyModel();
  individualsSortProxyModel_->setSourceModel(individualsModel_);
  individualsTable_->setModel(individualsSortProxyModel_);
  individualsTable_->sortByColumn(0, Qt::AscendingOrder);
  individualsTable_->sortByColumn(7, Qt::AscendingOrder);
  individualsTable_->sortByColumn(6, Qt::AscendingOrder);
  individualsTable_->setSortingEnabled(true);

  setCursor(Qt::ArrowCursor);
}

void MainWindow::individualClicked() {
  QModelIndex index = individualsSortProxyModel_->mapToSource(
    individualsTable_->currentIndex());
  if (index.isValid()) {
    int indId = individualsModel_->index(index.row(), 5).data().toInt();

    if (index.column() == 7) { // Complexity => show model
      Individual *ind = new Individual(indId, db_);
      Model *model = new Model(*ind->model());
      ModelForm *modelForm = new ModelForm(search_->getExpList(), model, products_,
        db_, this, true, QString("Model of individual %1").arg(ind->id()));
      modelForm->show();
      modelForm->raise();
      modelForm->activateWindow();
      delete ind;
    } else { // show individual
      Individual *ind = new Individual(indId, db_);
      SimulatorWindow *simWind = new SimulatorWindow(ind, search_, products_, 
                                                     db_, true, this);
      simWind->show();
      simWind->raise();
      simWind->activateWindow();
    }
  }
}
}
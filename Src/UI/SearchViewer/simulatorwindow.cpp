// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#include "simulatorwindow.h"

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
#include <QComboBox>

#include <QFileDialog>
#include <QSettings>
#include <QImage>
#include <QWheelEvent>
#include <QElapsedTimer>
#include <iostream>
#include <QBuffer>

#include <QApplication>

#include "version.h"
#include "DB/db.h"

#include "Common/mathalgo.h"
#include "Common/fileutils.h"

#include "UI/GUICommon/Simulator/simulatorthread.h"
#include "UI/GUICommon/Simulator/concentplotwidget.h"

#include "Search/generationindividual.h"
#include "Search/individual.h"
#include "Search/generation.h"
#include "Search/deme.h"
#include "Search/search.h"
#include "Search/searchexperiment.h"

#include "Model/model.h"

#include "Simulator/simulator.h"
#include "Simulator/simstate.h"
#include "Simulator/simparams.h"

#include "UI/GUICommon/experimentform.h"
#include "UI/GUICommon/modelform.h"
#include "UI/GUICommon/imagesaver.h"
#include "UI/GUICommon/modelgraphview.h"

#include "Experiment/phenotype.h"
#include "Experiment/experiment.h"
#include <qwt_plot_renderer.h>

namespace LoboLab {

SimulatorWindow::SimulatorWindow(Individual *ind, Search *sea, 
                                 const QHash<int, Product*> &products, DB *d, 
                                 bool autod, QWidget *parent)
  : QDialog(parent), 
    isAutoDelete_(autod), 
    db_(d), 
    individual_(ind),
    experiment_(sea->experiment(0)),
    model_(ind->model()),
    search_(sea),
    simParams_(sea->simParams()),
    modelForm_(NULL), 
    simulator_(NULL),
    simThread_(NULL), 
    comparisonPhenotype_(NULL),
    evaluatorProducts_(*search_),
    evaluatorCCC_(*search_),
    products_(products),
    simulating_(false),
    exporting_(false),
    testing_(false),
    nExp_(0),
    nExpPred_(0),
    nextExp_(0),
    includeAllFeatures_(false),
    movieSaver_(this),
    plotXSize_(1024),
    simTimePerPlotPixel_(simParams_->timePeriod / plotXSize_) {
  setWindowTitle(tr("Simulator"));

  prodLabels_ = model_->calcProductLabelsInUse().toList();
  qSort(prodLabels_);
  nProducts_ = prodLabels_.size();

  createWidgets();
  readSettings();

  if (isAutoDelete_)
    setAttribute(Qt::WA_DeleteOnClose);

  QTimer::singleShot(0, this, SLOT(loadModel()));
  QTimer::singleShot(0, this, SLOT(testModel()));
}

SimulatorWindow::~SimulatorWindow() {
  writeSettings();
  delete simThread_;
  delete modelForm_;
  delete simulator_;

  trErrorTable_.clear();
  tsErrorTable_.clear();

  if (isAutoDelete_)
    delete individual_;
}

void SimulatorWindow::autoPredictions() {
  loadModel();  
}

void SimulatorWindow::createWidgets() {  
  QPushButton *editExperimentButton = new QPushButton("Exp:");
  editExperimentButton->setMinimumWidth(30);
  editExperimentButton->setMaximumWidth(40);
  connect(editExperimentButton, SIGNAL(clicked()),
          this, SLOT(editExperiment()));

  experimentComboBox_ = new QComboBox();
  int nExp = search_->nExperiments();
  for (int i = 0; i < nExp; ++i) 
    experimentComboBox_->addItem(QString::number(search_->experiment(i)->id()));
  int nExpPred = search_->nExpPreds();
  for (int i = 0; i < nExpPred; ++i) {
    experimentComboBox_->addItem(QString("Pred: ") + QString::number(search_->expPred(i)->id()));
  }
  int nExpOther = search_->nExpOthers();
  for (int i = 0; i < nExpOther; ++i) {
    experimentComboBox_->addItem(QString("Other: ") + QString::number(search_->expOther(i)->id()));
  }
  experimentComboBox_->setMinimumWidth(10);
  connect(experimentComboBox_, SIGNAL(currentIndexChanged(int)),
    this, SLOT(experimentComboChanged(int)));

 QPushButton *editModelButton = new QPushButton(QString("Ind %1")
    .arg(individual_->id()));
  editModelButton->setMinimumWidth(40);
  connect(editModelButton, SIGNAL(clicked()),
          this, SLOT(editModel()));


  QPushButton *fitButton = new QPushButton("Error");
  fitButton->setMinimumWidth(40);
  connect(fitButton, SIGNAL(clicked()), this, SLOT(calcError()));

  QPushButton *testButton = new QPushButton("Test");
  testButton->setMinimumWidth(40);
  connect(testButton, SIGNAL(clicked()), this, SLOT(testModel()));

  QPushButton *loadButton = new QPushButton("Load");
  loadButton->setMinimumWidth(40);
  connect(loadButton, SIGNAL(clicked()), this, SLOT(loadModel()));

  startStopButton_ = new QPushButton("Start");
  startStopButton_->setMinimumWidth(40);
  connect(startStopButton_, SIGNAL(clicked()), this, SLOT(startStopSim()));

  QPushButton *predButton = new QPushButton("Pred");
  predButton->setMinimumWidth(40);
  connect(predButton, SIGNAL(clicked()), this, SLOT(showAllPredictions()));
  
  QPushButton *exportButton = new QPushButton("Export");
  predButton->setMinimumWidth(40);
  connect(exportButton, SIGNAL(clicked()), this, SLOT(exportExperimentData()));

  movieButton_ = new QPushButton("Mov");
  movieButton_->setCheckable(true);
  movieButton_->setMinimumWidth(40);
  connect(movieButton_, SIGNAL(toggled(bool)), this, SLOT(startStopMovie(bool)));

  QGroupBox *usedAllGroup = new QGroupBox();
  usedButton_ = new QRadioButton("Used");
  allButton_ = new QRadioButton("All");
  usedButton_->setChecked(true);

  connect(usedButton_, SIGNAL(toggled(bool)),
    this, SLOT(usedAllToggled(bool)));
  connect(allButton_, SIGNAL(toggled(bool)),
    this, SLOT(usedAllToggled(bool)));

  modelGraphView_ = new ModelGraphView(model_, products_, true);
  modelGraphView_->setStyleSheet("border: 1px solid #C0C0C0");
  
  plotsWidget_ = new ConcentPlotWidget(plotXSize_, simTimePerPlotPixel_);

  statusText_ = new QLabel("Individual view");
  statusText_->setMinimumWidth(100);
  speedText_ = new QLabel("Sel. speed: ");
  speedSlider_ = new QSlider(Qt::Horizontal);
  speedSlider_->setMinimum(1);
  speedSlider_->setMaximum(maxSpeedSlider());
  speedSlider_->setSingleStep(1);
  speedSlider_->setPageStep(10);
  connect(speedSlider_, SIGNAL(valueChanged(int)),
          this, SLOT(speedSliderChanged(int)));
  
  QLabel *modelLabel = new QLabel("Model");
  modelLabel->setAlignment(Qt::AlignCenter);
  QLabel *concentLabel = new QLabel("Product concentrations");
  concentLabel->setAlignment(Qt::AlignCenter);

  QHBoxLayout *imgLabelsLay = new QHBoxLayout();
  imgLabelsLay->addWidget(modelLabel, 1);
  imgLabelsLay->addWidget(concentLabel, 2);

  QHBoxLayout *simLay = new QHBoxLayout();
  simLay->addWidget(modelGraphView_, 1);
  simLay->addWidget(plotsWidget_, 2);

  QHBoxLayout *lay = new QHBoxLayout();
  lay->addWidget(new QLabel("Features:"));
  lay->addWidget(usedButton_);
  lay->addWidget(allButton_);
  usedAllGroup->setLayout(lay);
  
  QHBoxLayout *buttonsLay = new QHBoxLayout();
  buttonsLay->addWidget(editExperimentButton);
  buttonsLay->addWidget(experimentComboBox_);
  buttonsLay->addWidget(editModelButton);
  buttonsLay->addWidget(fitButton);
  buttonsLay->addWidget(testButton);
  buttonsLay->addWidget(loadButton);
  buttonsLay->addWidget(startStopButton_);
  buttonsLay->addWidget(predButton);
  buttonsLay->addWidget(exportButton);
  buttonsLay->addWidget(movieButton_);
  buttonsLay->addWidget(usedAllGroup);

  QHBoxLayout *bottomLay = new QHBoxLayout();
  bottomLay->addWidget(statusText_);
  bottomLay->addSpacing(15);
  bottomLay->addWidget(speedText_);
  bottomLay->addWidget(speedSlider_, 1);

  QVBoxLayout *graphsLay = new QVBoxLayout();
  graphsLay->addLayout(imgLabelsLay);
  graphsLay->addLayout(simLay);
  
  graphsWidget_ = new QWidget();
  graphsWidget_->setStyleSheet("background-color:white;");
  graphsWidget_->setLayout(graphsLay);

  QVBoxLayout *mainLay = new QVBoxLayout();
  mainLay->addLayout(buttonsLay);
  mainLay->addWidget(graphsWidget_);
  mainLay->addLayout(bottomLay);

  setLayout(mainLay);
}

void SimulatorWindow::readSettings() {
  QSettings settings;
  settings.beginGroup("SimulatorWindow");
  resize(settings.value("size", size()).toSize());
  move(settings.value("pos", pos()).toPoint());
  if (settings.value("maximized", false).toBool())
    showMaximized();

  speedSlider_->setValue(settings.value("speed", maxSpeedSlider()).toInt());
  speedSliderChanged(speedSlider_->value());
}

void SimulatorWindow::writeSettings() {
  QSettings settings;
  settings.beginGroup("SimulatorWindow");
  if (isMaximized())
    settings.setValue("maximized", isMaximized());
  else {
    settings.setValue("maximized", false);
    settings.setValue("size", size());
    settings.setValue("pos", pos());
  }

  settings.setValue("speed", speedSlider_->value());
}

void SimulatorWindow::loadModel() {
  if (simulator_) {
    if (simulating_)
      startStopSim();

    delete simThread_;
    delete simulator_;
  }

  simulator_ = new Simulator(*search_);
  simulator_->loadModel(model_, includeAllFeatures_);
  simulator_->loadExperiment(experiment_);
  simulator_->initialize();

  plotsWidget_->setSimulator(simulator_, *model_);
  plotsWidget_->updatePlots();
  simThread_ = new SimulatorThread(simulator_, simTimePerPlotPixel_, false, this);
  connect(simThread_, SIGNAL(simCycleDone(double)),
          this, SLOT(simCycleDone(double)));

  testing_ = false;
  simulationStatus_ = "Loaded";
  updateStatusText();
}

void SimulatorWindow::calcError() {
  loadModel();

  QElapsedTimer timer;
  timer.start();
  double fMorph = evaluatorProducts_.evaluate(*model_, 1000);
  QMessageBox::information(this, "Evaluator", QString("Evaluator error (time=%1s): "
                           "products = %2").arg(timer.elapsed()/1000.0)
                           .arg(fMorph));
}

void SimulatorWindow::testModel() {
  loadModel();
  startStopButton_->setText("Stop");
  simulating_ = true;
  testing_ = true;
  simulationTimer_.restart();
  simThread_->start();
}

void SimulatorWindow::startStopSim() {
  if (simulator_) {
    if (simulating_) {
      startStopButton_->setText("Start");
      if (testing_)
        simulationStatus_.prepend("Paused - ");
      else
        simulationStatus_ = "Paused";
      simThread_->stopThread();
      QApplication::processEvents(); // process last simCycleDone
      updateViews();
      simulating_ = false;
    } else {
      if (testing_)
        simulationStatus_.remove(0, 9);
      else
        simulationStatus_ = "Simulating";
      startStopButton_->setText("Stop");
      simulationTimer_.restart();
      simThread_->start();
      simulating_ = true;
    }
  }
}

// Signaled by the simulator thread
void SimulatorWindow::simCycleDone(double change) {
  double timePeriodPerCycle = simThread_->getTimePeriodPerCycle();

  int elapsed = simulationTimer_.restart();
  double nSimTimePerSecond = timePeriodPerCycle / elapsed;

  double timeToEnd = simParams_->timePeriod - simulator_->time();

  if (testing_ && timeToEnd <= 0) {
    simThread_->stopThread();
    startStopButton_->setText("Start");
    testing_ = false;
    simulating_ = false;
    updateViews();
    updateStatusText(nSimTimePerSecond, change);

    if (exporting_)
      exportReady();

    return;
  }

  int selectedSpeed = getSelectedSpeed();
  if (selectedSpeed == maxSpeedSlider()) {
    plotsWidget_->updatePlots(false);
    simNextCycle();
  } else {
    updateViews();
    updateStatusText(nSimTimePerSecond, change);

    double selectedTimePerCycle = selectedSpeed;
    static int sleepTime = 0;
    simThread_->setTimePeriodPerCycle(simTimePerPlotPixel_);
    sleepTime += (33 / selectedTimePerCycle) - elapsed;
    if (sleepTime > 0)
      QTimer::singleShot(sleepTime, this, SLOT(simNextCycle()));
    else {
      sleepTime = 0;
      simNextCycle();
    }
  }
}


void SimulatorWindow::simNextCycle() {
  simThread_->simNextCycle();
}

void SimulatorWindow::updateStatusText(double sps, double change) {
  Q_ASSERT(simulator_);
  double dist = evaluatorProducts_.calcDistance(simulator_->simulatedState(),
                                                simulator_->labels2Ind(),
                                                *experiment_);

  statusText_->setText(QString("%1 - Dist=%2 t=%3 TPS=%4 cc=%5")
                      .arg(simulationStatus_)
                      .arg(dist, 0, 'f', 2)
                      .arg(simulator_->time(), 0, 'f', 3)
                      .arg(sps, 7, 'f', 2, QChar('0'))
                      .arg(change, 0, 'e', 1)
                     + QString(" (%1x%2)").arg(graphsWidget_->size().width())
                                          .arg(graphsWidget_->size().height()));
}

void SimulatorWindow::updateViews() {
  plotsWidget_->updatePlots();

  if (movieSaver_.isRecording())
    movieSaver_.saveFrame();
}

void SimulatorWindow::translatorTest() {
}

void SimulatorWindow::manipulationTest() {
}

void SimulatorWindow::experimentComboChanged(int index) {
  if (simulating_)
    startStopSim();

  if (index < search_->nExperiments())
    experiment_ = search_->experiment(index);
  else if (index < (search_->nExperiments() + search_->nExpPreds()))
    experiment_ = search_->expPred(index - search_->nExperiments());
  else
    experiment_ = search_->expOther(index - (search_->nExperiments() + search_->nExpPreds()));

  loadModel();
  testModel();
}

void SimulatorWindow::editExperiment() {
  ExperimentForm *form = new ExperimentForm(experiment_, db_, this);
  bool ok = form->exec();
  delete form;
}

void SimulatorWindow::editModel() {

  if (!modelForm_)
    modelForm_ = new ModelForm(search_->getExpList(), model_, products_, db_, 
                               this);

  modelForm_->show();
  modelForm_->raise();
  modelForm_->activateWindow();
}

void SimulatorWindow::speedSliderChanged(int value) {
  if (value == maxSpeedSlider())
    speedText_->setText("Sel. Speed: MAX");
  else
    speedText_->setText(QString("Sel. Speed: %1").arg(getSelectedSpeed()));
}

// The selected speed is a quadratic function in order to make easier to select
// lower speeds
int SimulatorWindow::getSelectedSpeed() {
  int sliderValue = speedSlider_->value();
  return (1.0 / (1.0 + maxSpeedSlider())) * sliderValue * sliderValue +
         maxSpeedSlider() / (1.0 + maxSpeedSlider());
}

void SimulatorWindow::showAllPredictions() {
  loadModel();
  
  QElapsedTimer timer;
  timer.start();
  double fTrain = evaluatorProducts_.evaluate(*model_, 1000);

  double fTest= evaluatorCCC_.errorEvaluate(*model_, 1000);
  double fTrainRsquared = evaluatorCCC_.rSquaredTrainEvaluate(*model_, 1000, true);
  double fTestRsquared = evaluatorCCC_.rSquaredTestEvaluate(*model_, 1000, true);
  QMessageBox::information(this, "Evaluator", QString("Evaluator error (time=%1s): "
  	"trainingSet = %2 testSet = %3 RMSETrain = %4 RMSETest = %5").arg(timer.elapsed()/1000.0)
  	.arg(fTrain).arg(fTest).arg(fTrainRsquared).arg(fTestRsquared));
}

void SimulatorWindow::usedAllToggled(bool checked) {
  if (checked) {
    if (allButton_->isChecked())
      includeAllFeatures_ = true;
    else 
      includeAllFeatures_ = false;
    loadModel();
    testModel();
    modelGraphView_->updateModel(true, includeAllFeatures_);
  }
}

void SimulatorWindow::exportExperimentData() {
  loadModel();
  trErrorTable_ = evaluatorProducts_.createErrorTable(*model_, 1000);
  tsErrorTable_ = evaluatorCCC_.createErrorTable(*model_, 1000);
  nextExp_ = 0;
  nExp_ = search_->nExperiments();
  nExpPred_ = nExp_ + search_->nExpPreds();
  exporting_ = true;
  experimentComboChanged(0);

  // Get the current directory and create new directories for exporting data
  QString curDir = db_->fileName().section('/', 0, -3);
  QString newDir = curDir + "/plot";

  // Create "plot1" directory if it does not exist
  QDir dir(newDir);
  if (!dir.exists() && !dir.mkpath(newDir)) {
    qDebug() << "Failed to create directory:" << newDir;
    return;
  }

  // Get searchID from the database file name
  QString searchID = db_->fileBaseName().split("_")[0];

  // Create a subdirectory for the searchID
  serachDir_ = newDir + "/" + searchID;
  QDir searchDir(serachDir_);
  if (!searchDir.exists() && !searchDir.mkpath(serachDir_)) {
    qDebug() << "Failed to create directory:" << serachDir_;
    return;
  }

  // Create "features" directory
  QString featureDir = newDir + "/features";
  QDir featDir(featureDir);
  if (!featDir.exists() && !featDir.mkpath(featureDir)) {
    qDebug() << "Failed to create directory:" << featureDir;
    return;
  }

  // Define the file path for exporting the product labels
  QString fileName = featureDir + "/" + searchID + "_features.txt";
  QFile file(fileName);

  // Try to open the file for writing
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    std::cerr << "Failed to open file for writing: " << fileName.toStdString() << std::endl;
    return;
  }

  QTextStream out(&file);

  // Write each element of the QList<int> to the file
  QList<int> prodList = simulator_->productLabels();
  int listSize = prodList.size();
  for (int i = 0; i < listSize; ++i) {
    out << prodList[i];
    // Add a comma after each element except the last one
    if (i < listSize - 1) {
      out << ", ";
    }
  }

  file.close();
  qDebug() << "File successfully written to:" << fileName;
}

void SimulatorWindow::exportReady() {
  qApp->processEvents();
  if (nextExp_ < nExp_) {
    double fexp = trErrorTable_[search_->experiment(nextExp_)->id()];
    saveExperimentData(*search_->experiment(nextExp_), "train", fexp);
  } else {
    double fexp = tsErrorTable_[search_->expPred(nextExp_-nExp_)->id()];
    saveExperimentData(*search_->expPred(nextExp_ - nExp_), "predi", fexp);
  }
  nextExp_++;

  if (nextExp_ < nExpPred_)
    experimentComboChanged(nextExp_);
  else
    exporting_ = false;
}

void SimulatorWindow::saveExperimentData(const Experiment &exp, QString type, double error) {

  QSettings settings;
  settings.beginGroup("SimulatorWindow");

  QString fileName = serachDir_ + "/" + QString::number(exp.id()) + type + "_" + QString::number(error) + ".dat";

  if (!fileName.isEmpty()) {

    QFile file(fileName);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);

    QList<int> prodList = simulator_->productLabels();
    QList<double *> concent;
    int nProd = simulator_->nProducts();
    int nSteps = exp.phenotypes().last()->time();
    double timePerStep = 1;
    for (int i = 0; i < nProd; ++i)
      concent.append(new double[nSteps]);

    int nSimulations = 1;

    simulator_->initialize();
    double t = simulator_->time();
    out << nSimulations << "," << nProd << endl;

    for (int iSim = 0; iSim < nSimulations; ++iSim) {
      for (int iStep = 0; iStep < nSteps; ++iStep) {
        if (iStep == 0)
          simulator_->simulate(0);
        else
          simulator_->simulate(timePerStep);
        for (int i = 0; i < nProd; ++i) {
          concent[i][iStep] = simulator_->simulatedState().product(i);
        }
      }

      out << nSteps << endl;
      for (int i = 0; i < nProd; ++i) {
        out << prodList[i] << ';';
        for (int j = 0; j < nSteps - 1; ++j)
          // Output all the way to the second to last
          out << concent[i][j] << ',';
        // And output the last step we did
        out << concent[i][nSteps - 1] << endl;
      }
    }

    file.close();

    for (int i = 0; i < nProd; ++i)
      delete[] concent[i];
  }

}

void SimulatorWindow::startStopMovie(bool start) {
  if (start) {
    movieButton_->setStyleSheet("QPushButton {color: red; font: bold}");
    movieSaver_.startMovie(graphsWidget_);
  }
  else {
    if (simulating_)
      startStopSim();

    movieSaver_.endMovie();
    movieButton_->setStyleSheet("QPushButton {color: black}");
  }

}

}
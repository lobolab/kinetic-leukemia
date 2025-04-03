// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#pragma once

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QComboBox>
#include <QTime>
#include <QStandardItemModel>
#include <QRadioButton>
#include "UI/GUICommon/moviesaver.h"
#include "Search/evaluatorproducts.h"
#include "Search/evaluatorccc.h"

namespace LoboLab {

class Individual;
class Search;
class Simulator;
class SimState;
class Model;
class SimParams;
class SimulatorThread;
class CellsView;
class ImageView;
class ConcentPlotWidget;
class Experiment;
class DataTableView;
class ModelForm;
class ModelGraphView;
class Product;
class Phenotype;
class DB;

class SimulatorWindow : public QDialog {
  Q_OBJECT

 public:
  SimulatorWindow(Individual *ind, Search *search, 
                  const QHash<int, Product*> &products, DB *db, bool autoDelete,
                 QWidget *parent = NULL);
  virtual ~SimulatorWindow();
  void autoPredictions(); 

 private slots:
  void calcError();
  void testModel();
  void loadModel();
  void startStopSim();
  void simCycleDone(double change);

  void translatorTest();
  void manipulationTest();

  void experimentComboChanged(int index);
  void editExperiment();
  void editModel();

  void speedSliderChanged(int value);
  void simNextCycle();
  
  void showAllPredictions();
  void exportExperimentData();
  void saveExperimentData(const Experiment &exp, QString type, double error);
  void exportReady();
  void startStopMovie(bool start);
  void usedAllToggled(bool checked);

 private:
  void createWidgets();
  void readSettings();
  void writeSettings();
  void updateViews();
  void updateStatusText(double sps = 0.0, double change = 0.0);
  int getSelectedSpeed();
  static inline int maxSpeedSlider() { return 4000; }
  
  bool isAutoDelete_; // auto delete the window and individual when closed

  DB *db_;
  Individual *individual_;
  Experiment *experiment_;
  Model *model_;
  Search *search_;
  SimParams *simParams_;

  ModelGraphView *modelGraphView_;
  ModelForm *modelForm_;
  
  QComboBox *experimentComboBox_;
  QPushButton *movieButton_;
  QRadioButton *usedButton_;
  QRadioButton *allButton_;

  ConcentPlotWidget *plotsWidget_;
  
  QWidget *graphsWidget_;
  QLabel *statusText_;
  QLabel *speedText_;
  QSlider *speedSlider_;
  
  QPushButton *startStopButton_;
  QString serachDir_;

  Simulator *simulator_;
  SimulatorThread *simThread_;
  Phenotype *comparisonPhenotype_;
  EvaluatorProducts evaluatorProducts_; 
  EvaluatorCCC evaluatorCCC_;
  
  int nProducts_;
  QList<int> prodLabels_;
  QStringList prodNames_;

  const QHash<int, Product*> &products_;

  bool simulating_;
  bool exporting_;
  bool testing_;
  QTime simulationTimer_;
  QString simulationStatus_;
  
  int nExp_;
  int nExpPred_;
  int nextExp_;
  QHash<int, double> trErrorTable_;
  QHash<int, double> tsErrorTable_;

  int plotXSize_;
  double simTimePerPlotPixel_;

  MovieSaver movieSaver_;
  bool includeAllFeatures_;
};

}
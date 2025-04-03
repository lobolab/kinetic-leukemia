// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QAbstractItemModel>
#include <QTableWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QSqlQueryModel>
#include <QSlider>
#include <QSortFilterProxyModel>
#include <QProcess>

namespace LoboLab {

//class DataTableView;
class ErrorPlotWidget;
class Search;
class Product;
class DB;

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow();
  virtual ~MainWindow();

  void openDB(const QString &file);

 protected:
  void closeEvent(QCloseEvent *event);
  void keyPressEvent(QKeyEvent *event);

 private slots:
  void newDB();
  void openDB();
  void openDBAppDir();
  void saveAsDB();
  void closeDB();
  void initDataBase();
  void processError(QProcess::ProcessError error);

  void about();
  void deleteAllExperiments();
  void deleteAllExpAndManipulations();
  void deleteAll();
  void simEnds();
  void searchVals();
  void modelAnalysis(); 

  void updateModel();

  void databaseCorrupted();
  void updateStatusText(int nEle, const QString &eleName,
                        const QString &eleNamePlural);

  void searchComboChanged(int index);
  void demeComboChanged(int index);
  void timeGenerationToggled(bool checked);
  void bestAllDemeToggled(bool checked);
  void logLinearToggled(bool checked);

  void filterListCheckChanged(int state);
  void listSliderChanged();

  void individualClicked();

 private:
  void createWidgets();
  void createActionsAndMenus();
  void createToolBars();
  void createStatusBar();
  void connectDB(bool silent = false);
  void readSettings();
  void writeSettings();
  void loadProducts();
  void clearProducts();
  
  void updatePlot();
  void updateIndividuals();
  void updateIndividualsList();
  void updateStatus();

  void emptyExperimentTables();
  void emptyManipulationTables();
  void emptyMorphologyTables();


  QString dbFileName_;
  DB *db_;

  Search *search_;

  QSqlQueryModel *searchModel_;
  QSqlQueryModel *demeModel_;
  QSqlQueryModel *generationModel_;
  QSqlQueryModel *individualsModel_;
  QSortFilterProxyModel *individualsSortProxyModel_;
  
  QComboBox *searchComboBox_;

  QRadioButton *timeButton_;
  QRadioButton *geneButton_;
  QRadioButton *bestButton_;
  QRadioButton *allButton_;
  QRadioButton *demeButton_;
  QRadioButton *logButton_;
  QRadioButton *linearButton_;
  QComboBox *demeComboBox_;

  ErrorPlotWidget *errorPlotWidget_;

  int maxRangeValue_;
  
  QCheckBox *filterListCheckBox_;
  QLabel *listSliderLabel_;
  QSlider *listSlider_;

  QTableView *individualsTable_;
  
  QAction *saveAsDBAction_;
  QAction *closeDBAction_;
  QAction *importDBAction_;

  QAction *morDistAction_;
  QAction *morDistCompAction_;

  QAction *deleteExperimentsAction_;
  QAction *deleteExpAndManipulationsAction_;
  QAction *deleteAllAction_;

  QLabel *statusBarText_;

  QHash<int, Product*> products_;
  
};

}

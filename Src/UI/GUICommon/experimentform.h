// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QGroupBox>
#include <QSqlQueryModel>

namespace LoboLab {

class Experiment;
class ResultSetsWidget;
class DB;

class ExperimentForm : public QDialog {
  Q_OBJECT

 public:
  ExperimentForm(Experiment *experiment, DB *db, QWidget *parent = NULL);
  virtual ~ExperimentForm();
  
 private slots:
  void nameChanged();
  void keyPressEvent(QKeyEvent *e);
  void formAccepted();

 private:
  void createWidgets();
  void readSettings();
  void writeSettings();

  QLineEdit *idEdit_;
  QString originalName_;

  DB *db_;

  Experiment *experiment_;
};

} // namespace LoboLab

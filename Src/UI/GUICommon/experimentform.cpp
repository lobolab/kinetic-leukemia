// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#include "experimentform.h"
#include "Experiment/experiment.h"
#include "DB/db.h"

#include <QDataWidgetMapper>
#include <QGridLayout>
#include <QPushButton>
#include <QInputDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QSettings>
#include <QFormLayout>

namespace LoboLab {

ExperimentForm::ExperimentForm(Experiment *e, DB *d, QWidget *parent)
  : QDialog(parent), db_(d), experiment_(e) {
  if (experiment_->id()) {
    setWindowTitle(tr("Edit experiment"));
    originalName_ = experiment_->id();
  } else
    setWindowTitle(tr("New experiment"));

  createWidgets();
  readSettings();
}

ExperimentForm::~ExperimentForm() {
  writeSettings();
}

void ExperimentForm::readSettings() {
  QSettings settings;
  settings.beginGroup("ExperimentForm");
  resize(settings.value("size", QSize(1000, 800)).toSize());
  if (settings.value("maximized", false).toBool())
    showMaximized();
}

void ExperimentForm::writeSettings() {
  QSettings settings;
  settings.beginGroup("ExperimentForm");
  if (isMaximized())
    settings.setValue("maximized", isMaximized());
  else {
    settings.setValue("maximized", false);
    settings.setValue("size", size());
  }

  settings.endGroup();
}

void ExperimentForm::formAccepted() {
    accept();
}

void ExperimentForm::createWidgets() {
  setWindowFlags(Qt::Window);

  QString str;
  idEdit_ = new QLineEdit();
  idEdit_->setText(str.setNum(experiment_->id()));
  connect(idEdit_, SIGNAL(editingFinished()), this, SLOT(nameChanged()));

  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | 
                                                     QDialogButtonBox::Cancel);
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(formAccepted()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));


  // Layouts
  QFormLayout *formLay = new QFormLayout();
  formLay->addRow("Name:", idEdit_);
  formLay->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

  QVBoxLayout *topLay = new QVBoxLayout();
  topLay->addLayout(formLay);
  
  QVBoxLayout *mainLay = new QVBoxLayout();
  mainLay->addLayout(topLay, 1);
  mainLay->addWidget(buttonBox);

  setLayout(mainLay);
}

// private slot
void ExperimentForm::nameChanged() {
}

// private slot
void ExperimentForm::keyPressEvent(QKeyEvent *e) {
  if (e->key()!=Qt::Key_Escape)
    QDialog::keyPressEvent(e);
}

}

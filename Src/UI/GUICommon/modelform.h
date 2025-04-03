// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#pragma once

#include "Experiment/product.h"
#include "Search/search.h"
#include "Experiment/experiment.h"
#include "Experiment/phenotype.h"
#include "Model/model.h"
#include "Model/modellink.h"
#include "DB/db.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QTextEdit>
#include <QLabel>
#include <QCheckBox>
#include <qtextstream.h>

#include <qwt_text_label.h>

namespace LoboLab {

class Model;
class ModelProdListWidget;
class ModelFormulaWidget;
class ModelGraphView;
class Product;
class Search;
class DB;
class SensitivityAnalysis; 

class ModelForm : public QDialog {
  Q_OBJECT

 public:
  ModelForm(QList<SearchExperiment*> searchExps, Model *model, const QHash<int, Product*> &products, 
            DB *db, QWidget *parent = NULL, bool autoDelete = false,
            const QString &windowTitle = "Edit model");
  virtual ~ModelForm();

  Model *getModel() const;
  QHash<int, Product*> prods_;
  void runModelAnalysis(); 

  typedef struct {
    QList<int> numSigs;
    QList<QString> sigmaNames;
    QList<QString> alphNames;
    QList<QString> rhNames;
    QMap<QString, double> alphaValues;
  } ParemLists;

 private slots:
  void formAccepted();
  void modelProdListWidgetChanged();
  void textChanged();
  void addProduct();
  void removeProduct();
  void clearModel();
  void randomModel();
  void hideNotUsedCheckBoxChanged(int state);
  void copyMathML();
  void saveSBML();
  void sensAnaly(); 

 private:
  void readSettings();
  void writeSettings();
  void createWidgets(const QHash<int, Product*> &products);
  void updateLabels();

  void printIntroUnitsCo(QTextStream& out);
  QMap<int,QString> printSpecies(QTextStream& out); 
  ParemLists listOfParameters_;
  void printParameters(QTextStream& out, QList<int> labels);
  void printRules(QTextStream& out, QMap<int,QString> label2name);
  bool linkRegulatorLessThan(ModelLink *l1, ModelLink *l2);
  void printEvents(QTextStream& out, QMap<int, QString> label2name);

  SensitivityAnalysis *sensAnaly_; 
  DB* db_; 

  Model *model_;
  QList<SearchExperiment*> searchExps_;

  QLabel *nProductsLabel_;
  QLabel *nLinksLabel_;
  QLabel *complexityLabel_;
  ModelProdListWidget *modelProdListWidget_;
  ModelFormulaWidget *modelFormulaWidget_;
  ModelGraphView *modelGraphView_;
  QTextEdit *modelTextEdit_;
  QCheckBox *hideNotUsedCheckBox_;

  bool autoDelete_;
  bool isUpdating_;
};

} // namespace LoboLab

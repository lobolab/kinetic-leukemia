// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#pragma once

#include "UI\GUICommon\modelform.h"
#include "Model\modellink.h"
#include "Experiment\phenotype.h"

#include <QSettings>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QFileDialog>
#include <QList>
#include <QMap>
#include <QPair>
#include <qsqlquery.h>

namespace LoboLab {

  class Model;
  class ModelForm; 
  class ModelProd; 
  class ModelLink; 
  class Experiment;
  class Search;
  class Phenotype;
  class EvaluatorProducts;
  class DB;

  class SensitivityAnalysis {
    public:
      SensitivityAnalysis();

      typedef struct {
        QString name;
        int regulatorLabel;
        int regulatedLabel;
        double value;
        bool isProd;
        double senseCoef; 
        double auc;
      }Parameter;

      QList<Parameter> getParameters(ModelForm* modelFormPtr, Model* modelPtr);
      void runSensitAnalysis(ModelForm* modelFormPtr, Model* modelPtr, DB* dbPtr, Search* search); 
      Experiment* createOrigExp(DB* dbPtr); 
      double simExp(Experiment* expPtr, Model* modelPtr, Search* searchPtr); 
      Model* changeModel(Parameter param, Model* modPtr);
      double calcSensitCoeff(double origArea, double expArea, double origPara, double para150); 
      void exportData(ModelForm* modelFormPtr, QList<Parameter> data, double origArea, DB* dbPtr);
  };
}

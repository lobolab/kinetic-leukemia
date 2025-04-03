// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#include "UI/GUICommon/sensitivityanalysis.h"

#include "DB/db.h"
#include "Model/model.h"
#include "Model/modelprod.h"
#include "Experiment/phenotype.h"
#include "Experiment/product.h"
#include "Experiment/experiment.h"
#include "Search/search.h"
#include "Search/evaluatorproducts.h"


namespace LoboLab {

  SensitivityAnalysis::SensitivityAnalysis() {}


  void SensitivityAnalysis::runSensitAnalysis(ModelForm* modelFormPtr, Model* modelPtr,
    DB* dbPtr, Search* search) {
    QList<Parameter> parameters = getParameters(modelFormPtr, modelPtr);
    Experiment* origExp = createOrigExp(dbPtr);    
    double origArea = simExp(origExp, modelPtr, search);
     
     for (int i = 0; i < parameters.size(); i++) {
       Model* newMod = changeModel(parameters[i], modelPtr); 
       double auc = simExp(origExp, newMod, search);
       parameters[i].auc = auc; 
       double param150 = parameters[i].value * 1.5;
       double sensitCoef = calcSensitCoeff(origArea, auc, parameters[i].value, param150);
       parameters[i].senseCoef = sensitCoef; 
      }
     exportData(modelFormPtr, parameters, origArea, dbPtr);    
  }


  QList<SensitivityAnalysis::Parameter> SensitivityAnalysis::getParameters(ModelForm* modelFormPtr,
    Model* model) {

    QList<Parameter> paras; 
    int numProds = model->nProducts();

    //stores link objects instead of link pointers
    QList<ModelLink> linkList;
    int i = 0;
    int listSize = model->links().size();
    for (; i < listSize; i++) {
      ModelLink temp = *((model->links())[i]);
      if (!(temp.regulatedProdLabel() > 20))
        if (model->calcLinksInUse().contains((model->links())[i]))
          linkList.append(temp);
    }
    //sorts links in order by subclone that is regulated
    std::sort(linkList.begin(), linkList.end());

    for (int i = 0; i < numProds + 1; ++i) {
      int label = (model->product(i)->label());
      
      if (label != 1 && model->calcProductLabelsInUse().contains(label)) {
        double deg = model->product(i)->deg();
        QString lamName = "lambda_" + QString::number(label);
        Parameter lam;
        lam.name = lamName;
        lam.regulatorLabel = 0;
        lam.regulatedLabel = label;
        lam.value = deg;
        lam.isProd = true;
        paras.append(lam);

        double posLim = model->product(i)->posLim();
        QString posName = "posLim_" + QString::number(label);
        Parameter pos;
        pos.name = posName;
        pos.regulatorLabel = 0;
        pos.regulatedLabel = label;
        pos.value = posLim;
        pos.isProd = true;
        paras.append(pos);

        double negLim = model->product(i)->negLim();
        QString negName = "negLim_" + QString::number(label);
        Parameter neg;
        neg.name = negName;
        neg.regulatorLabel = 0;
        neg.regulatedLabel = label;
        neg.value = negLim;
        neg.isProd = true;
        paras.append(neg);
      }
    }

    for (int i = 0; i < linkList.size(); i++) {
      int regulator = linkList[i].regulatorProdLabel();
      int regulated = linkList[i].regulatedProdLabel();

      QString hillName = "hillCoef_" + QString::number(regulator) + "_" +
        QString::number(regulated);
      QString disName = "disConst_" + QString::number(regulator) + "_" +
        QString::number(regulated);
      QString limName = "lim_" + QString::number(regulator) + "_" +
        QString::number(regulated);
      double hillVal = linkList[i].hillCoef();
      double disVal = linkList[i].disConst();

      Parameter hill;
      hill.name = hillName;
      hill.regulatorLabel = regulator;
      hill.regulatedLabel = regulated;
      hill.value = hillVal;
      hill.isProd = false;

      Parameter dis;
      dis.name = disName;
      dis.regulatorLabel = regulator;
      dis.regulatedLabel = regulated;
      dis.value = disVal;
      dis.isProd = false;
      
      paras.append(hill);
      paras.append(dis);
    }
    return paras;
  }


  Experiment* SensitivityAnalysis::createOrigExp(DB* dbPtr) {
    QSqlQuery* query = dbPtr->newTableQuery("Experiment",2);  
    query->first(); 
    Experiment *exp = new Experiment(query->value(0).toInt(), dbPtr);

    delete query; 
    return exp;
  }


  double SensitivityAnalysis::simExp(Experiment* expPtr, Model* modelPtr, Search* searchPtr) {
    Simulator* simPtr = new Simulator(*(searchPtr)); 
    simPtr->loadModel(modelPtr);
    simPtr->loadExperiment(expPtr); 

    double area = 0; 
    delete simPtr; 
    return area; 
  }


  Model* SensitivityAnalysis::changeModel(Parameter param, Model* modPtr) {
    Model* modCopy = new Model(*(modPtr)); 
    double param150 = (param.value)*1.5; 

    if (param.isProd == false) {
      QList<ModelLink*> links = modCopy->links();
      for (int i = 0; i < links.size(); i++) {
        if (links[i]->regulatorProdLabel() == param.regulatorLabel &&
          links[i]->regulatedProdLabel() == param.regulatedLabel) {
          if (param.value == links[i]->disConst()) {
            modCopy->link(i)->setDisConst(param150);
            return modCopy; 
          }
          else if (param.value == links[i]->hillCoef()) {
            modCopy->link(i)->setHillCoef(param150);
            return modCopy; 
          }
        }
      }
    }
    else {
      ModelProd* prod = modCopy->prodWithLabel(param.regulatedLabel);
      if (param.value == prod->deg()) {
        prod->setDeg(param150);
        return modCopy;
      }
      else if (param.value == prod->posLim()) {
        prod->setPosLim(param150);
        return modCopy;
      }
      else if (prod->label() < 21 && param.value == prod->negLim()) {
        prod->setNegLim(param150);
        return modCopy;
      }
    }
    return modCopy; 
  }


  double SensitivityAnalysis::calcSensitCoeff(double origArea, double expArea, 
    double origPara, double para150) {

    double areaChange = expArea - origArea; 
    double paramChange = para150 - origPara; 

    double areaCalc = areaChange / origArea; 
    double paramCalc = origPara / paramChange; 

    return(areaCalc*paramCalc); 
  }


  void SensitivityAnalysis::exportData(ModelForm* modelFormPtr,
    QList<Parameter> data, double origArea, DB* dbPtr) {
    QSettings settings;
    settings.beginGroup("SAfile");

    QString fileName = dbPtr->fileBaseName();
    fileName.chop(4);
    fileName.append("_SensitivityAnalysis.csv");

    QFile file(fileName);
    file.open(QFile::WriteOnly);

    QTextStream out(&file);

    out << "Parameter,Regulated,Regulator,Sensitivity Coefficient,Parameter Value,";
    out << "Varied Parameter Value,Original Area,New Area" << endl;
    for (int i = 0; i < data.size(); i++) {
      out << data[i].name << "," << data[i].regulatedLabel << ",";
      out << data[i].regulatorLabel << "," << data[i].senseCoef << ",";
      out << data[i].value << "," << (data[i].value)*1.5 << ",";
      out << origArea << "," << data[i].auc << endl; 
    }
    file.close();
  }
}
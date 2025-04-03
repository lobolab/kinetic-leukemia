// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#include "modelform.h"
#include "sensitivityanalysis.h"

#include "Model/model.h"
#include "Model/modelprod.h"
#include "Model/modellink.h"
#include "Private/modelprodlistwidget.h"
#include "Private/modelformulawidget.h"
#include "modelgraphview.h"

#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QSettings>
#include <QPushButton>
#include <QLabel>
#include <QInputDialog>
#include <QApplication>
#include <QClipboard>
#include <QSplitter>
#include <QFile>
#include<iostream>
#include<QFileDialog>


using namespace std; 

namespace LoboLab {

  ModelForm::ModelForm(QList<SearchExperiment*> searchExps, Model *m, const QHash<int, Product*> &products,
    DB * db, QWidget *parent, bool autoDelete, const QString &windowTitle):
    QDialog(parent),
    searchExps_(searchExps),
    model_(m),
    autoDelete_(autoDelete),
    prods_(products),
    db_(db), 
    isUpdating_(false) {
    setWindowTitle(windowTitle);
    createWidgets(products);
    readSettings();
    updateLabels();
  }

  ModelForm::~ModelForm() {
    writeSettings();
    if (autoDelete_)
      delete model_;
  }

  void ModelForm::readSettings() {
    QSettings settings;
    settings.beginGroup("ModelForm");
    resize(settings.value("size", QSize(450, 600)).toSize());
    move(settings.value("pos", pos()).toPoint());
    if (settings.value("maximized", false).toBool())
      showMaximized();
  }

  void ModelForm::writeSettings() {
    QSettings settings;
    settings.beginGroup("ModelForm");
    if (isMaximized())
      settings.setValue("maximized", isMaximized());
    else {
      settings.setValue("maximized", false);
      settings.setValue("size", size());
      settings.setValue("pos", pos());
    }

    settings.endGroup();
  }

  void ModelForm::formAccepted() {
    accept();
  }

  Model *ModelForm::getModel() const {
    return model_;
  }

  void ModelForm::runModelAnalysis() {
    //saveSBML(); 
    sensAnaly(); 
    this->close(); 
  }

  void ModelForm::createWidgets(const QHash<int, Product*> &products) {
    setWindowFlags(Qt::Window);

    nProductsLabel_ = new QLabel();
    nLinksLabel_ = new QLabel();
    complexityLabel_ = new QLabel();

    QPushButton *addProductButton = new QPushButton("Add prod.");
    connect(addProductButton, SIGNAL(clicked()), this, SLOT(addProduct()));

    QPushButton *removeProductButton = new QPushButton("Remove prod.");
    connect(removeProductButton, SIGNAL(clicked()), this, SLOT(removeProduct()));


    modelProdListWidget_ = new ModelProdListWidget(model_, this);
    connect(modelProdListWidget_, SIGNAL(modified()),
      this, SLOT(modelProdListWidgetChanged()));

    modelGraphView_ = new ModelGraphView(model_, products); //R targetModel_);

    modelTextEdit_ = new QTextEdit();
    modelTextEdit_->setAcceptRichText(false);
    QString modelStr = model_->toString();
    modelTextEdit_->setText(modelStr);
    connect(modelTextEdit_, SIGNAL(textChanged()), this, SLOT(textChanged()));

    modelFormulaWidget_ = new ModelFormulaWidget(model_, products);

    hideNotUsedCheckBox_ = new QCheckBox("Hide not used products");
    connect(hideNotUsedCheckBox_, SIGNAL(stateChanged(int)),
      this, SLOT(hideNotUsedCheckBoxChanged(int)));

    QPushButton *resetButton = new QPushButton("Clear");
    connect(resetButton, SIGNAL(clicked()), this, SLOT(clearModel()));

    QPushButton *randomButton = new QPushButton("Random Params");
    connect(randomButton, SIGNAL(clicked()), this, SLOT(randomModel()));

    QPushButton *copyMathMLButton = new QPushButton("Copy MathML");
    connect(copyMathMLButton, SIGNAL(clicked()), this, SLOT(copyMathML()));

    QPushButton *saveSBML = new QPushButton("Save in SBML");
    connect(saveSBML, SIGNAL(clicked()), this, SLOT(saveSBML()));

    QPushButton *sensAnalyButton = new QPushButton("Export Sensitivity Analysis");
    connect(sensAnalyButton, SIGNAL(clicked()), this, SLOT(sensAnaly()));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok |
      QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(formAccepted()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QHBoxLayout *butLay = new QHBoxLayout();
    butLay->addWidget(resetButton);
    butLay->addWidget(randomButton);
    butLay->addWidget(copyMathMLButton);
    butLay->addWidget(saveSBML);
    butLay->addWidget(sensAnalyButton);
    butLay->addStretch();
    butLay->addWidget(hideNotUsedCheckBox_);
    butLay->addWidget(buttonBox);


    QHBoxLayout *topLay = new QHBoxLayout();
    topLay->addWidget(new QLabel("Prod:"));
    topLay->addWidget(nProductsLabel_);
    topLay->addWidget(new QLabel("Links:"));
    topLay->addWidget(nLinksLabel_);
    topLay->addWidget(new QLabel("Complex:"));
    topLay->addWidget(complexityLabel_);
    topLay->addWidget(addProductButton);
    topLay->addWidget(removeProductButton);

    QSplitter *leftSplitter = new QSplitter(Qt::Vertical);
    leftSplitter->addWidget(modelFormulaWidget_);
    leftSplitter->addWidget(modelProdListWidget_);
    leftSplitter->addWidget(modelTextEdit_);
    leftSplitter->setSizes(QList<int>() << 10 << 500 << 10);

    QVBoxLayout *leftLay = new QVBoxLayout();
    leftLay->addLayout(topLay);
    leftLay->addWidget(leftSplitter, 1);

    QVBoxLayout *rightLay = new QVBoxLayout();
    rightLay->addWidget(modelGraphView_);
    rightLay->setMargin(0);

    QWidget *leftWidget = new QWidget();
    leftWidget->setLayout(leftLay);
    leftLay->setMargin(0);

    QWidget *rightWidget = new QWidget();
    rightWidget->setLayout(rightLay);

    QSplitter *centerSplitter = new QSplitter(Qt::Horizontal);
    centerSplitter->addWidget(leftWidget);
    centerSplitter->addWidget(rightWidget);
    centerSplitter->setSizes(QList<int>() << 100 << 400);

    QVBoxLayout *mainLay = new QVBoxLayout();
    mainLay->addWidget(centerSplitter, 1);
    mainLay->addLayout(butLay);

    setLayout(mainLay);
  }

  void ModelForm::updateLabels() {
    nProductsLabel_->setText(QString("%1 (%2)")
      .arg(model_->calcProductLabelsInUse().size())
      .arg(model_->nProducts()));
    nLinksLabel_->setText(QString("%1 (%2)")
      .arg(model_->calcLinksInUse().size())
      .arg(model_->nLinks()));
    complexityLabel_->setText(QString("%1 (%2)")
      .arg(model_->calcComplexityInUse())
      .arg(model_->calcComplexity()));
  }

  void ModelForm::modelProdListWidgetChanged() {
    isUpdating_ = true;
    modelTextEdit_->setText(model_->toString());
    modelGraphView_->updateModel(hideNotUsedCheckBox_->isChecked());
    updateLabels();
    isUpdating_ = false;
  }

  void ModelForm::textChanged() {
    if (!isUpdating_) {
      model_->loadFromString(modelTextEdit_->toPlainText());
      updateLabels();
      modelProdListWidget_->updateList();
      modelFormulaWidget_->updateFormula();
      modelGraphView_->updateModel(hideNotUsedCheckBox_->isChecked());
    }
  }

  void ModelForm::addProduct() {
  }

  void ModelForm::removeProduct() {
    bool ok;
    int delProd = QInputDialog::getInt(this, tr("Select Product to Remove"),
      tr("Product ind:"), 0, 0,
      model_->nProducts(), 1, &ok);

    if (ok) {
      model_->removeProduct(delProd);
      isUpdating_ = true;
      updateLabels();
      modelProdListWidget_->updateList();
      modelFormulaWidget_->updateFormula();
      modelGraphView_->updateModel(hideNotUsedCheckBox_->isChecked());
      modelTextEdit_->setText(model_->toString());
      isUpdating_ = false;
    }
  }

  void ModelForm::clearModel() {
    model_->clear();
    modelProdListWidget_->update();
    modelFormulaWidget_->updateFormula();
    modelTextEdit_->setText(model_->toString());
  }

  void ModelForm::randomModel() {
    int nProducts = model_->nProducts();
    for (int i = 0; i < nProducts; ++i)
      model_->product(i)->mutateParams(100);

    int nLinks = model_->nLinks();
    for (int i = 0; i < nLinks; ++i)
      model_->link(i)->mutateParams(100, true, true);

    modelProdListWidget_->update();
    modelFormulaWidget_->updateFormula();
    modelTextEdit_->setText(model_->toString());
  }

  void ModelForm::hideNotUsedCheckBoxChanged(int state) {
    modelGraphView_->updateModel(state > 0);
  }

  void ModelForm::copyMathML() {
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(modelFormulaWidget_->mathMLStr());
  }

  void ModelForm::sensAnaly() {
    if (!sensAnaly_)
      sensAnaly_ = new SensitivityAnalysis();
    
    Search* searchPtr = searchExps_[0]->search(); 
    sensAnaly_->runSensitAnalysis(this, model_, db_, searchPtr); 
  }

  void ModelForm::saveSBML() {
  }

  void ModelForm::printIntroUnitsCo(QTextStream &out) {
    return;
  }

  QMap<int,QString> ModelForm::printSpecies(QTextStream &out) {
    QMap<int, QString> label2name; 
    QList<QString> names;
    out << "\t\t<listOfSpecies>" << endl;
    QList<int> usedLabels = model_->calcProductLabelsInUse().toList();
    qSort(usedLabels);
    int nProducts = usedLabels.size();
    int auxProdChar = 0;
    for (int i = 0; i < nProducts; ++i) {
      int label = usedLabels[i];
      Product *prod = prods_.value(label);
      if (prod) {
        QString nameTemp = prod->name();
        nameTemp.remove(QChar('\t'));
        QString nameFinal = nameTemp.simplified();
        label2name[label] = nameFinal;
        names.append(nameFinal);

      }
      else {
        int letterProd = auxProdChar;
        QString name = QChar((letterProd % 26) + 97);
        while (letterProd >= 26) {
          letterProd /= 26;
          name = QChar(((letterProd - 1) % 26) + 97) + name;
        }
        auxProdChar++;
        label2name[label]=name;
        names.append(name);
      }
    }
   
    out << "\t\t</listOfSpecies>" << endl;
    out << endl;

    return label2name;
  }

  void ModelForm::printParameters(QTextStream &out, QList<int> labels) {
    sort(labels.begin(), labels.end());
    out << "\t\t<listOfParameters>" << endl;
    
    for (int i = 0; i < labels.size(); ++i) {
      ModelProd* prod = model_->prodWithLabel(labels.at(i));
      out << "\t\t\t<parameter id=\"lambda" << labels.at(i) << "\" constant=\"true\" value=\""
        << prod->deg() << "\"/>" << endl;
    }

    //stores link objects instead of link pointers
    QList<ModelLink> linkList;
    int i = 0;
    int listSize = model_->links().size();
    for (; i < listSize; i++) {
      ModelLink temp = *((model_->links())[i]);
      linkList.append(temp);
    }
    //sorts links in order by subclone that is regulated
    sort(linkList.begin(), linkList.end());
    for (int i = 0; i < linkList.size(); ++i) {
      ModelLink link = linkList.at(i);
      int regulator = link.regulatorProdLabel();
      int regulated = link.regulatedProdLabel(); 
      if (labels.contains(regulator) && labels.contains(regulated)) {
        out << "\t\t\t<parameter id=\"eta" << regulator << "_" << regulated << "\" constant=\"true\" value=\""
          << link.hillCoef() << "\"/>" << endl;
        out << "\t\t\t<parameter id=\"k" << regulator << "_" << regulated << "\" constant=\"true\" value=\""
          << link.disConst() << "\"/>" << endl;
      }
    }
    out << "\t\t</listOfParameters>" << endl << endl;
  }

  void ModelForm::printRules(QTextStream& out, QMap<int,QString> label2name) {
    out << "\t\t<listOfRules>" << endl;
    QList<int> labels = label2name.keys();
    
    //Total Tumor Volume 
    out << "\t\t\t<assignmentRule variable=\"Tumor_\">" << endl;
    out << "\t\t\t\t<math xmlns=\"http://www.w3.org/1998/Math/MathML\"> " << endl;
    out << "\t\t\t\t\t<apply>" << endl << "\t\t\t\t\t\t<plus/>" << endl;
    for (int i = 0; i < labels.size(); i++) {
      if(labels[i] > 1 && labels[i] < 21)
        out << "\t\t\t\t\t\t<ci>" << label2name[labels[i]] << "</ci>" << endl;
    }
    out << "\t\t\t\t\t</apply>" << endl << "\t\t\t\t</math>" << endl;
    out << "\t\t\t</assignmentRule>" << endl;
    
    // create dictionary of links for each subclone label
    QMap<int, QList<ModelLink*>> clonelinks;
    for (int i = 0; i < labels.size(); ++i) {
      QList<ModelLink*> links; 
      if (labels[i] > 1) {
        links = model_->linksToLabel(labels[i]);
        clonelinks[labels[i]] = links; 
      }
    }

    //Subclone rate rules 
    for (int i = 1; i < labels.size(); i++) {
      out << "\t\t\t<rateRule variable=\"" << label2name[labels[i]] << "\">" << endl;
      out << "\t\t\t\t<math xmlns=\"http://www.w3.org/1998/Math/MathML\"> " << endl;
      
      out << "\t\t\t\t</math>" << endl;
      out << "\t\t\t</rateRule>" << endl;
    }
    out << "\t\t</listOfRules>" << endl;
    return; 
  }
  
  // For sorting
  bool ModelForm::linkRegulatorLessThan(ModelLink *l1, ModelLink *l2) {
    return (l1->regulatorProdLabel() < l2->regulatorProdLabel());
  }

  void ModelForm::printEvents(QTextStream& out, QMap<int, QString> label2name) {
  }
  
}
// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#pragma once

#include "formulawidget.h"

#include <QMenu>

namespace LoboLab {

class Model;
class ModelProd;
class ModelLink;
class Product;

class ModelFormulaWidget : public FormulaWidget {
  Q_OBJECT

 public:
  ModelFormulaWidget(Model *m, const QHash<int, Product*> &products, QWidget * parent = NULL);
  virtual ~ModelFormulaWidget();

  void updateFormula();

 signals:
  void modified();
    
 private:
  QString createLinkLinearFormula(const ModelLink *link, const QString &regulator) const;
  QString createLinkHillFormula(const ModelLink *link, const QString &regulator) const;
  static bool linkRegulatorLessThan(ModelLink *l1, ModelLink *l2);

  Model *model_;
  const QHash<int, Product*> &products_;
};

} // namespace LoboLab

// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#pragma once

#include <QListWidget>

namespace LoboLab {

class Model;
class ModelProd;

class ModelProdListWidget : public QListWidget {
  Q_OBJECT

 public:
  ModelProdListWidget(Model *m, QWidget * parent = NULL);
  virtual ~ModelProdListWidget();

  void updateList();

 signals:
  void modified();

 private slots:
  void contextMenuCalled(const QPoint &pos);

  void addModelProd();
  void replaceModelProd();
  void removeSelectedModelProd();

 private:
  void createActions();
  void clearActions();
  void createList();

  ModelProd *createModelProd();

  int selectProduct(bool *ok, const QString &label);
  double selectRealValue(bool *ok, const QString &label);
  bool selectBoolValue(bool *ok, const QString &label);
  int selectDirection(bool *ok);

  Model *model_;

  QMenu *prodContextMenu_;
  QMenu *blankContextMenu_;
};

} // namespace LoboLab

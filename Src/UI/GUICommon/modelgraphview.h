// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#pragma once

#include "Private/zoomgraphicsview.h"

#include <QGraphicsSvgItem>
#include <QProcess>
#include <QTextStream>

namespace LoboLab {

class Model;
class ModelComp;
class Product;
class ConcentPlotWidget;

class ModelGraphView : public ZoomGraphicsView {
  Q_OBJECT

 public:
  ModelGraphView(Model *m, const QHash<int, Product*> &products,
                 bool hideNotUsed = false, bool includeAllFeatures = false, QWidget * parent = NULL);
  virtual ~ModelGraphView();

 public slots:
  void updateModel(bool hideNotUsed, bool includeAllFeatures = false);

 signals:
  //void modified();
  
 private:
  void createGraph(bool hideNotUsed, bool includeAllFeatures);
  void writeModel2Dot(QTextStream &stream, bool hideNotUsed, bool includeAllFeatures);
  void writeProd(QTextStream &stream, int label, int fillColorInd, 
                 const QString name, bool isPlain) const;
  void writeProdAttribs(QTextStream &stream, const QStringList &props,
                        const QStringList &style) const;

  Model *model_;
  ConcentPlotWidget *concPlotWidget_;
  QGraphicsSvgItem *svgItem_;
  QProcess *process_;
  QStringList args_;

  const QHash<int, Product*> &products_;
  QList<int> outputLabels_;
  QList<int> inputLabels_;
  QList<int> intermediateLabels_;
  int maxProductLabel_;
};

} // namespace LoboLab

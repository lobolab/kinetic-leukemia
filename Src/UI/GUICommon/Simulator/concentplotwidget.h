// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#pragma once

#include "Model/model.h"

#include <QWidget>
#include <QAction>
#include <QHash>
#include <qwt_plot_magnifier.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_shapeitem.h>

namespace LoboLab {

class Simulator;
class Search;
class SimParams;
class Phenotype;

class ConcentPlotWidget : public QWidget {
  Q_OBJECT

 public:
  ConcentPlotWidget(int xAxisSize, double simTimePerPlotPixel, QWidget *parent = NULL);
  ~ConcentPlotWidget();

  void setSimulator(const Simulator *sim, const Model &model);

  void clear();

  virtual QSize	sizeHint () const {return QSize(10,10); }
  virtual QSize	minimumSizeHint () const {return QSize(10,10); }

 public slots:
  void updatePlots(bool doReplot = true);
  void saveImage(int expId, QString type, double error);

 private slots:
  void saveImage();
  void hideExpData();

 private:

  class PlotMagnifierZeroLimited : public QwtPlotMagnifier {
   public:
    explicit PlotMagnifierZeroLimited(QwtPlotCanvas * canvas)
      : QwtPlotMagnifier(canvas) {}

   protected:
    virtual void rescale(double factor);
  };

  QList<QwtPlotCurve*> getExpProductCurves(const QHash<int, Phenotype*> &pProductTable);
  QList<QwtPlotShapeItem*> getCones(const QHash<int, Phenotype*> &expProductTable);
  void initPlots();
  double *createCurveData(int size);
  void updateCurveXdata();

  const Simulator *simulator_;
  QList<int> productsShown_;
  int maxProductLabel_;

  QwtPlot *plotX_;
  QList<QwtPlotCurve*> curvesX_;
  QList<double*> curvesXdata_;
  QList<double*> curvesExpdata_;
  double *axisXvalues_;
  QList<QwtPlotCurve*> curvesExp_;
  int xSize_;  
  double simTimePerPlotPixel_;
    
  QAction *saveImageAction_;
  QAction *hideExpDataAction_;
  bool expDataIsShown_;

  QList<QwtPlotShapeItem*> stdCones_;
  QHash<int, QColor> productsColor_;

};

} // namespace LoboLab

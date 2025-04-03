// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#include "concentplotwidget.h"

#include "Simulator/simulator.h"
#include "Search/search.h"
#include "Simulator/simparams.h"
#include "Simulator/simstate.h"
#include "Model/modelProd.h"
#include "../Private/colorsconfig.h"
#include "Experiment/product.h"
#include "Experiment/phenotype.h"
#include "Experiment/experiment.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QSettings>
#include <QFileDialog>
#include <QHash>
#include <qwt_curve_fitter.h>
#include <qwt_plot_renderer.h>
#include <qwt_scale_engine.h>
#include <qwt_symbol.h>
#include <qwt_plot_shapeitem.h>
#include <qmath.h>

namespace LoboLab {

ConcentPlotWidget::ConcentPlotWidget(int xAxisSize, double simTimePerPlotPixel, QWidget *parent)
  : QWidget(parent), simulator_(NULL), axisXvalues_(NULL), xSize_(xAxisSize), 
    simTimePerPlotPixel_(simTimePerPlotPixel), expDataIsShown_(true) {
  initPlots();
  
  saveImageAction_ = new QAction(tr("Save image..."), this);
  saveImageAction_->setIcon(QIcon(":/Images/famfamfam_silk_icons/picture_save.png"));
  connect(saveImageAction_, SIGNAL(triggered()), this, SLOT(saveImage()));

  setContextMenuPolicy(Qt::ActionsContextMenu);
  addAction(saveImageAction_); // add the action to the context menu

  hideExpDataAction_ = new QAction(tr("Toggle Experimental Data Curves"), this);
  hideExpDataAction_->setIcon(QIcon(":/Images/famfamfam_silk_icons/picture_save.png"));
  connect(hideExpDataAction_, SIGNAL(triggered()), this, SLOT(hideExpData()));
  addAction(hideExpDataAction_); // add the action to the context menu
}

ConcentPlotWidget::~ConcentPlotWidget() {
  clear();
  delete plotX_;
}

void ConcentPlotWidget::initPlots() {
  plotX_ = new QwtPlot();

  plotX_->setCanvasBackground(Qt::white);

  plotX_->setAxisScale(QwtPlot::yLeft, 0, 100);
  plotX_->setAxisScale(QwtPlot::xBottom, 0, 100);
  
  plotX_->setAxisFont(QwtPlot::yLeft, QFont("Arial", 8));
  plotX_->setAxisFont(QwtPlot::xBottom, QFont("Arial", 8));
    
  PlotMagnifierZeroLimited* magnifier = new PlotMagnifierZeroLimited((QwtPlotCanvas *) plotX_->canvas() );
  magnifier->setAxisEnabled(QwtPlot::xBottom, false);
    
  QVBoxLayout *lay = new QVBoxLayout();
  lay->addWidget(plotX_);

  lay->setMargin(0);

  setLayout(lay);
}

void ConcentPlotWidget::clear() {
  plotX_->detachItems();
  delete [] axisXvalues_;
  axisXvalues_ = NULL;

  for (int i = 0; i<curvesXdata_.size(); ++i)
    delete [] curvesXdata_.at(i);

  for (int i = 0; i<curvesExpdata_.size(); ++i)
    delete[] curvesExpdata_.at(i);

  curvesX_.clear();
  curvesXdata_.clear();
  curvesExpdata_.clear();
  curvesExp_.clear();
  productsShown_.clear();
  productsColor_.clear();

}

void ConcentPlotWidget::setSimulator(const Simulator *sim, 
                                     const Model &model) {
  clear();

  simulator_ = sim;

  plotX_->setAxisScale(QwtPlot::xBottom, 0, xSize_*simTimePerPlotPixel_);
  axisXvalues_ = new double[xSize_ + 1];
  for (int i=0; i<=xSize_; ++i)
    axisXvalues_[i] = i*simTimePerPlotPixel_;

  int nProducts = simulator_->nProducts();
  QColor color;
  QList<int> productLabels = simulator_->productLabels();
  QList<int> outputLabels = simulator_->search()->outputLabels();
  maxProductLabel_ = simulator_->search()->maxProductLabel();
  int intermediateLabels = maxProductLabel_;

  for (int i = 0; i < nProducts; ++i) {
    productsShown_.append(i);
  }

  for (int i = 0; i < nProducts; ++i) {
    int productL = simulator_->productLabel(i);
    if (productL <= maxProductLabel_) {
      color = ColorsConfig::colorsCells[productL];
      productsColor_[productL] = color;
    }
    else
    {
      color = ColorsConfig::colorsCells[intermediateLabels];
      productsColor_[productL] = color;
      intermediateLabels++;
    }

    QPen pen(color);
    if(outputLabels.contains(productL))
    	pen.setWidthF(10);
    else
      pen.setWidthF(3);

    QwtPlotCurve *curveX = new QwtPlotCurve();
    curveX->setStyle(QwtPlotCurve::Lines);
    curveX->setPen(pen);
    curveX->setRenderHint(QwtPlotItem::RenderAntialiased);
    double *curveXdata = new double[xSize_ + 1];
    for (int j = 0; j <= xSize_; ++j)
      curveXdata[j] = 0;
    curveX->setRawSamples(axisXvalues_, curveXdata, xSize_);
    curveX->attach(plotX_);
    curvesXdata_.append(curveXdata);
    curvesX_.append(curveX);
  }
  
  //add values from experiment to plotX
  //hash table variables
  QHash<int, Phenotype*> expProductTable;
  QList<Phenotype*> expProducts = sim->experiment()->phenotypes();
  QList<int> simulatorLabels = simulator_->productLabels();
  for (int i = 0; i < expProducts.size(); i++) {
    int prodLabel = expProducts[i]->product()->id();
    if (simulatorLabels.contains(prodLabel))
      expProductTable.insertMulti(prodLabel, expProducts[i]);//allows multiple products per key
  }

  curvesExp_ = getExpProductCurves(expProductTable);
  stdCones_ = getCones(expProductTable);

  for (int c = 0; c < curvesExp_.size(); c++)
    curvesExp_[c]->attach(plotX_);
  for (int c = 0; c < stdCones_.size(); c++)
    stdCones_[c]->attach(plotX_);
}

void ConcentPlotWidget::updatePlots(bool doReplot) {
  updateCurveXdata();
  if (doReplot)
    plotX_->replot();
}

void ConcentPlotWidget::updateCurveXdata() {
  const SimState &state = simulator_->simulatedState();
  int n = curvesXdata_.size();
  for (int i=0; i<n; ++i) {
    double *curveXdata = curvesXdata_.at(i);
    double conc = state.product(i);
    memmove(curveXdata, curveXdata + 1, xSize_*sizeof(double));

    curveXdata[xSize_] = conc;
  }
}

void ConcentPlotWidget::PlotMagnifierZeroLimited::rescale(double factor) {
  factor = qAbs( factor );
  if ( factor == 1.0 || factor == 0.0 )
    return;

  bool doReplot = false;
  QwtPlot* plt = plot();

  const bool autoReplot = plt->autoReplot();
  plt->setAutoReplot( false );

  for ( int axisId = 0; axisId < QwtPlot::axisCnt; axisId++ ) {
    const QwtScaleDiv &scaleDiv = plt->axisScaleDiv( axisId );
    if ( isAxisEnabled( axisId ) && !scaleDiv.isEmpty() ) {
      //const double center =
      //    scaleDiv->lowerBound() + scaleDiv->range() / 2;
      //const double width_2 = scaleDiv->range() / 2 * factor;

      //plt->setAxisScale( axisId, center - width_2, center + width_2 );
      plt->setAxisScale(axisId, scaleDiv.lowerBound() * factor,
                                scaleDiv.upperBound() * factor);
      doReplot = true;
    }
  }

  plt->setAutoReplot( autoReplot );

  if ( doReplot )
    plt->replot();
}

// public slot
void ConcentPlotWidget::saveImage(int expId, QString type, double error) {
  QSettings settings;
  settings.beginGroup("ConcentPlotWidget");
  // QString lastDir = settings.value("lastDir").toString();
  QString lastDir = "D:/Reza/CancerPatterning/plot/20230730a/";

  QString fileName = lastDir + QString::number(error) + type + QString::number(expId) + ".png";

  if (!fileName.isEmpty()) {
    QString newDir = QFileInfo(fileName).absolutePath();
    if (newDir != lastDir)
      settings.setValue("lastDir", newDir);

    QwtPlotRenderer renderer;
    // plotX_->replot();

    QSizeF renderSize = size();
    renderSize.scale(100, 100, Qt::KeepAspectRatio);
    renderer.renderDocument(plotX_, fileName, renderSize, 100);
  }
}

// private slot
void ConcentPlotWidget::saveImage() {
  QwtPlotCanvas *plotCanvas = dynamic_cast<QwtPlotCanvas*>(
                                        childAt(mapFromGlobal(QCursor::pos())));
  QwtPlot *plot;
  if (plotCanvas) {
    plot = plotCanvas->plot(); 

    QSettings settings;
    settings.beginGroup("ConcentPlotWidget");
    QString lastDir = settings.value("lastDir").toString();

    QString fileName = QFileDialog::getSaveFileName(this, "Save image",
                       lastDir, "Images (*.svg *.pdf *.ps *.png *.jpg *.bmp)");

    if (!fileName.isEmpty()) {
      QString newDir = QFileInfo(fileName).absolutePath();
      if (newDir != lastDir)
        settings.setValue("lastDir", newDir);

      QSizeF renderSize = plotCanvas->size();
      renderSize /= 5;
      QwtPlotRenderer renderer;
      renderer.renderDocument(plot, fileName, renderSize, 100);
    }
  }
}

//private slot
void ConcentPlotWidget::hideExpData() {

  QList<int> outputLabels = simulator_->search()->outputLabels();

  for (int i = 0; i < curvesExp_.size(); i++) {
    const QwtSymbol *theSymbol = curvesExp_[i]->symbol();
    const QPen &thePen = theSymbol->pen();
    const QBrush &theBrush = theSymbol->brush();
    int productL = simulator_->productLabel(i);
      
    if (expDataIsShown_)
      if (outputLabels.contains(productL))
        curvesExp_[i]->setSymbol(new QwtSymbol(QwtSymbol::NoSymbol, theBrush, thePen, QSize(10, 10)));
      else
        curvesExp_[i]->setSymbol(new QwtSymbol(QwtSymbol::NoSymbol, theBrush, thePen, QSize(5, 5)));
    else 
      curvesExp_[i]->setSymbol(new QwtSymbol(QwtSymbol::Ellipse, theBrush, thePen, QSize(10, 10)));
            
    curvesExp_[i]->attach(plotX_);
  }
    
  expDataIsShown_ = !expDataIsShown_; 
  plotX_->replot();
}

QList<QwtPlotCurve*> ConcentPlotWidget::getExpProductCurves(
  const QHash<int, Phenotype*> &pProductTable) {
  QList<QwtPlotCurve*> expCurveList;

  QList<int> keys = pProductTable.uniqueKeys();//get a list of all the products
  qSort(keys);

  QList<int> outputLabels = simulator_->search()->outputLabels();
  QList<int> inputLabels = simulator_->search()->inputLabels();
  int intermediateLabels = maxProductLabel_;

  for (int i = 0; i < keys.size(); i++) {

    QList<Phenotype*> pProductsI = pProductTable.values(keys[i]);
    //obtain X and Y data for this product's curve
    double *pProductsIX = new double[pProductsI.size()];
    double *pProductsIY = new double[pProductsI.size()];
    curvesExpdata_.append(pProductsIX);
    curvesExpdata_.append(pProductsIY);

    for (int j = 0; j < pProductsI.size(); j++) {
      pProductsIX[j] = pProductsI[j]->time();
      pProductsIY[j] = pProductsI[j]->concentration();
    }
    //Determine curve color, set pen and raw data for the curve
    QColor color = productsColor_[keys[i]];
    QPen pen(color);
    if (outputLabels.contains(keys[i]))
      pen.setWidth(10);
    else
      pen.setWidth(2);
    QwtPlotCurve *expCurveI = new QwtPlotCurve;
    expCurveI->setPen(pen);
    expCurveI->setStyle(QwtPlotCurve::NoCurve);
    expCurveI->setSymbol(new QwtSymbol(QwtSymbol::Ellipse, QBrush(QColor(0,0,0)), pen, QSize(10,10)));
    expCurveI->setRawSamples(pProductsIX, pProductsIY, pProductsI.size());
    expCurveList.append(expCurveI);
  }
  return expCurveList;
}

QList<QwtPlotShapeItem*> ConcentPlotWidget::getCones(const QHash<int, Phenotype*> &expProductTable) {
  QList<QwtPlotShapeItem*> expPolyList;

  QList<int> keys = expProductTable.uniqueKeys();//get a list of all the products
  qSort(keys);

  int intermediateLabels = maxProductLabel_;

  for (int i = 0; i < keys.size(); i++) {
    QList<Phenotype*> expProductsI = expProductTable.values(keys[i]);
    QVector<QPointF> pointstop;
    QVector<QPointF> pointsbot;
    //obtain X and Y data for this product's curve

    for (int j = 0; j < expProductsI.size(); j++) {
      double x1 = expProductsI[j]->time();
      double ymargin = 0.25 * expProductsI[j]->maxConcInProduct();
      
      double y1 = expProductsI[j]->concentration() + ymargin;
      double x2 = expProductsI[j]->time();
      double y2 = expProductsI[j]->concentration() - ymargin;
      if (y2 < 0 )
        y2 = 0;

      pointstop.append(QPointF(x1, y1));
      pointsbot.push_front(QPointF(x2, y2));
    }
   
    if (i >= 0) {
      for (int k = 0; k < pointstop.size(); k++) {
        QColor color = productsColor_[keys[i]];
        color = QColor(color.red(), color.green(), color.blue(), 255);
        QPen pen(color);
        pen.setWidth(100);
        QwtPlotShapeItem *expCurveI = new QwtPlotShapeItem;
        expCurveI->setPen(pen);
        QVector<QPointF> pts;
        pts.append(pointstop[k]);
        expCurveI->setPolygon(pts);
        expPolyList.append(expCurveI);
      }
    }
  }
  return expPolyList;
}


}


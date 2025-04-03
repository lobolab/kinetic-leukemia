// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#include "modelgraphview.h"

#include "Private/colorsconfig.h"
#include "Model/model.h"
#include "Model/modelProd.h"
#include "Model/modelLink.h"
#include "Experiment/product.h"
#include "UI/GUICommon/Simulator/concentplotwidget.h"

#include <QFile>
#include <QFileInfo>
#include <QStringList>

namespace LoboLab {

ModelGraphView::ModelGraphView(Model *m, const QHash<int, Product*> &products, 
                               bool hideNotUsed, bool includeAllFeatures,
                               QWidget * parent)
  : ZoomGraphicsView(parent, true, false), 
    model_(m),
    products_(products) {
  process_ = new QProcess(this);

  QList<int> keys = products_.uniqueKeys();
  qSort(keys);

  for (int i = 0; i < keys.size(); i++)
  {
    if (products_.values(keys[i]).at(0)->type() < 2)
      inputLabels_.append(products_.values(keys[i]).at(0)->label());
    else if (products_.values(keys[i]).at(0)->type() == 2)
      outputLabels_.append(products_.values(keys[i]).at(0)->label());
    else
      intermediateLabels_.append(products_.values(keys[i]).at(0)->label());
  }
  qSort(outputLabels_);
  maxProductLabel_ = outputLabels_.last();
  args_.append("-Tsvg");
  args_.append("graph.gv");
  args_.append("-ograph.svg");

  createGraph(hideNotUsed, includeAllFeatures);
}

ModelGraphView::~ModelGraphView() {
  delete process_;
}

void ModelGraphView::createGraph(bool hideNotUsed, bool includeAllFeatures) {
  QFile gvFile("graph.gv");

  gvFile.open(QIODevice::Truncate | QIODevice::WriteOnly | QIODevice::Text);
  Q_ASSERT(gvFile.isOpen());

  QTextStream gvStream(&gvFile);
  writeModel2Dot(gvStream, hideNotUsed, includeAllFeatures);
  gvStream.flush();
  gvFile.close();

  process_->start("dot", args_);
  process_->waitForFinished();

  QFile svgFile("graph.svg");
  Q_ASSERT(svgFile.exists());

  svgItem_ = new QGraphicsSvgItem("graph.svg");

  scene_->addItem(svgItem_);

  bool ok = gvFile.remove();
  Q_ASSERT(ok);

  ok = QFile::remove("graph.svg");
  Q_ASSERT(ok);
}

void ModelGraphView::writeModel2Dot(QTextStream &stream, bool hideNotUsed, bool includeAllFeatures) {
  stream << "digraph G {bgcolor=transparent;";
  stream << "node [fontname=\"Myriad Pro\"];";

  QList<int> labels = model_->calcProductLabels().toList();
  qSort(labels);
  QSet<int> usedLabels = model_->calcProductLabelsInUse(includeAllFeatures);
  int nProducts = labels.size();
  int colorInd = maxProductLabel_;
  int auxProdChar = 0;
  for (int i = 0; i < nProducts; ++i) {
    int label = labels.at(i);
    Product *prod = products_.value(label, NULL);
    QString name;
    if (prod)
      name = prod->name();
    else if (usedLabels.contains(label)) {
      int letterProd = auxProdChar;
      name = QChar((letterProd % 26) + 97);
      while (letterProd >= 26) {
        letterProd /= 26;
        name = QChar(((letterProd-1) % 26) + 97) + name;
      }
      auxProdChar++;
    }
    else
      name = QString("%1").arg(label);

    // Drugs are showed with plain nodes
    bool isPlain;
    ModelProd *modelProd = model_->prodWithLabel(label);
      isPlain = false;

    if (usedLabels.contains(labels.at(i))) {
      if (isPlain)
        writeProd(stream, label, 0, name, isPlain);
      else
        if (label <= maxProductLabel_)
          writeProd(stream, label, label, name, isPlain);
        else
          writeProd(stream, label, colorInd++, name, isPlain);
    }
    else if (!hideNotUsed) {
        writeProd(stream, label, -1, name, isPlain);
    }
  }

  // Links
  int n = labels.size();
  for (int i = 0; i < n; ++i) {
    int regulatedLabel = labels.at(i);
    QList<ModelLink*> links = model_->linksToLabel(regulatedLabel);

    int nLinks = links.size();
    for (int j = 0; j < nLinks; ++j) {
      ModelLink *link = links.at(j);

      bool linkIsUsed = usedLabels.contains(link->regulatorProdLabel()) &&
                        usedLabels.contains(link->regulatedProdLabel());

      if (linkIsUsed || !hideNotUsed) {
        stream << link->regulatorProdLabel() << " -> " << regulatedLabel;

        QString color;
        QString arrowhead;

        if (link->hillCoef() < 0) {
          color = "ff0000";
          arrowhead = "tee";
        }
        else {
          color = "0000ff";
          arrowhead = "normal";
        }

        if (!linkIsUsed)
          color += "80"; // alpha channel

        stream << " [color=\"#" << color << "\",arrowhead=" << arrowhead;

        //Larger arrowsizes
        stream << ",arrowsize=1.25";

        //Link line width
        stream << ",penwidth=" << 2.0 + 2;
        
        if (link->isAndReg() || link->hillCoef() < 0)
          stream << ",style=bold];";
        else
          stream << ",style=\"bold,dashed\"];";

        }
      //R }
    }
  }

  stream << "}";
}

void ModelGraphView::writeProd(QTextStream &stream, int label, int fillColorInd, 
                               const QString name, bool isPlain) const {
  stream << label;

  QStringList props;
  QStringList style;

  props += QString("label=\"%1\"").arg(name);
  
  if (isPlain) {
    props += "shape=plaintext";
    props += "width=0";
    props += "height=0";
    props += "margin=0";
  }
  else if (outputLabels_.contains(label)) {
    props += "shape=diamond";
    props += "margin=\"0.05,0.076\"";
  }
  else if (inputLabels_.contains(label)) {
    props += "shape=rectangle";
    props += "margin=\"0.05,0.076\"";
  }
  else
  {
    props += "shape=circle";
    props += "margin=\"0.05,0.076\"";
  }
  
  if(isPlain)  {
    props += "fontcolor=black";
  } else if(fillColorInd > -1) {
    // With alpha channel
    QRgb color = 0x80 + 0x100 *
      (ColorsConfig::colorsCells[fillColorInd]);

    QString colStr = QString("\"#%1\"").arg(color, 8, 16, QChar('0')).toUpper();
    
    style += "filled";
    props += "fillcolor=" + colStr;
  } else {
    props += "fontcolor=gray";
    props += "color=gray";
  }

  writeProdAttribs(stream, props, style);

  stream << ';';
}

void ModelGraphView::writeProdAttribs(QTextStream &stream,
                                      const QStringList &props, const QStringList &style) const {
  if (!props.isEmpty() || !style.isEmpty()) {
    stream << " [";

    if (!props.isEmpty()) {
      stream << props.first();
      for (int i = 1; i < props.size(); ++i)
        stream << ',' << props.at(i);
    }

    if (!style.isEmpty()) {
      if (!props.isEmpty())
        stream << ',';

      stream << "style=\"";

      stream << style.first();
      for (int i = 1; i < style.size(); ++i)
        stream << ',' << style.at(i);

      stream << '\"';
    }

    stream << ']';
  }
}

void ModelGraphView::updateModel(bool hideNotUsed, bool includeAllFeatures) {
  delete svgItem_;
  createGraph(hideNotUsed, includeAllFeatures);
  fitView();
}


}
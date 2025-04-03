// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#pragma once

#include "zoomgraphicsview.h"

#include <QGraphicsSvgItem>
#include <QProcess>
#include <QTextStream>

namespace LoboLab {

class StateDiagramView : public ZoomGraphicsView {
  Q_OBJECT

 public:
  StateDiagramView(QWidget * parent = NULL);
  virtual ~StateDiagramView();
  
  void createDiagram(const QMultiHash<int, int> &stateTransitions);
  void clear();
  
 signals:

 private slots:

 private:
  void writeDiagram2Dot(QTextStream &stream, 
                        const QMultiHash<int, int> &stateTransitions) const;
  void writeProd(QTextStream &stream, int label, int fillColorInd = -1, 
                 char *name = NULL) const;
  void writeProdAttribs(QTextStream &stream, const QStringList &props,
                        const QStringList &style) const;
  QString calcStateLabel(int state) const;

  QGraphicsSvgItem *svgItem_;
  QProcess *process_;
  QStringList args_;
};

} // namespace LoboLab

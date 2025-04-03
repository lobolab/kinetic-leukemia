// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#include "statediagramview.h"

#include "colorsconfig.h"
#include "Model/model.h"
#include "Model/modelProd.h"
#include "Model/modelLink.h"

#include <QList>
#include <QFile>
#include <QFileInfo>
#include <QStringList>

namespace LoboLab {

StateDiagramView::StateDiagramView(QWidget * parent)
    : ZoomGraphicsView(parent, true, false),
      svgItem_(NULL) {
  process_ = new QProcess(this);

  args_.append("-Tsvg");
  args_.append("graph.gv");
  args_.append("-ograph.svg");
}

StateDiagramView::~StateDiagramView() {
  delete process_;
}

void StateDiagramView::clear() {
  delete svgItem_;
}

void StateDiagramView::createDiagram(
                                 const QMultiHash<int, int> &stateTransitions) {
  clear();
  QFile gvFile("graph.gv");

  gvFile.open(QIODevice::Truncate | QIODevice::WriteOnly | QIODevice::Text);
  Q_ASSERT(gvFile.isOpen());

  QTextStream gvStream(&gvFile);
  writeDiagram2Dot(gvStream, stateTransitions);
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
  
  fitView();
}

void StateDiagramView::writeDiagram2Dot(QTextStream &stream, 
                           const QMultiHash<int, int> &stateTransitions) const {
  stream << "digraph G {bgcolor=transparent;";
  stream << "node [fontname=Arial];";


  QList<int> states = stateTransitions.uniqueKeys();

  int nStates = states.size();
  for (int i = 0; i < nStates; ++i) {
    stream << states.at(i) << " [label=\"" << calcStateLabel(states.at(i)) << "\"];";
  }


  QHashIterator<int, int> i(stateTransitions);
  while (i.hasNext()) {
    i.next();
    stream << i.key() << " -> " << i.value() << ";";
  }
  
  stream << "}";
}

QString StateDiagramView::calcStateLabel(int state) const {
  QString label;
  if (state & 1)
    label += 'H';
  if (state & (1 << 1))
    label += 'K';
  if (state & (1 << 2))
    label += 'T';
  if (state & (1 << 3))
    label += 'W';

  int i = 4;
  state >>= i;
  while (state) {
    if (state & 1)
      label += QString("%1").arg(i);
    ++i;
    state >>= 1;
  }
  
  return label;
}

void StateDiagramView::writeProd(QTextStream &stream, int label, int fillColorInd, 
                               char *name) const {
  
}

void StateDiagramView::writeProdAttribs(QTextStream &stream,
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


}
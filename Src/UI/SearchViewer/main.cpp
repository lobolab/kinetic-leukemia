// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#include "mainwindow.h"

#include <QApplication>

#include <qwt_text.h>
#include <qwt_mathml_text_engine.h>

#ifdef QT_DEBUG
#ifndef Q_WS_X11
  #include <vld.h>
#endif
#endif

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);

#ifdef QT_DEBUG
#ifndef Q_WS_X11
  VLD_UINT options = VLDGetOptions();
  QString filename("memory_leaks_viewer.txt");
  VLDSetReportOptions(VLD_OPT_REPORT_TO_FILE, (const wchar_t *)filename.utf16());
#endif
#endif
  
  // This needs to be done only once before using the MathML engine
  QwtText::setTextEngine(QwtText::MathMLText, new QwtMathMLTextEngine());

  LoboLab::MainWindow mw;
  mw.show();

  return a.exec();
}

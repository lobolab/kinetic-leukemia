// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#pragma once

#include <QColor>

namespace LoboLab {

struct ColorsConfig {
 public:
  static const int nColors;
  static const QRgb colorsMorp[];
  static const QRgb colorsCells[];
  static const QRgb background;
};

} // namespace LoboLab

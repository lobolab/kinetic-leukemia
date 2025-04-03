// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#include "product.h"
#include "DB/db.h"

namespace LoboLab {

Product::Product()
  : ed_("Product"), label_(-1), type_(0) {
}


Product::Product(int id, DB *db)
  : ed_("Product", id, db){
  load();
}

Product::~Product() {
}

Product::Product(const Product &source, bool maintainId)
  : name_(source.name_), type_(source.type_),
  ed_(source.ed_, maintainId), label_(source.label_) {
}

Product &Product::operator=(const Product &source) {
  name_ = source.name_;
  type_ = source.type_;
  ed_ = source.ed_;
  label_ = source.label_; 

  return *this;
}

Product &Product::copy(const Product &source) {
  name_ = source.name_;
  type_ = source.type_;
  return *this;
}

bool Product::operator ==(const Product& other) const {
  return name_ == other.name_ && type_ == other.type_;
}

// Persistence methods

void Product::load() {
  name_ = ed_.loadValue(FName).toString();
  type_ = ed_.loadValue(FType).toInt();
  label_ = ed_.id();
  ed_.loadFinished();
}

int Product::submit(DB *db) {
  QHash<QString, QVariant> values;
  values.insert("Name", name_);
  values.insert("Type", type_);
  return ed_.submit(db, values);
}

bool Product::erase() {
  return ed_.erase();
}

}
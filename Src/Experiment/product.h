// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#pragma once

#include "DB/dbelementdata.h"

namespace LoboLab {

class Product : public DBElement {
 public:
  Product();
  Product(int id, DB *db);
  virtual ~Product();

  Product(const Product &source, bool maintainId = true);
  Product &operator=(const Product &source);
  Product &copy(const Product &source);

  inline const QString &name() const { return name_; }
  inline void setName(const QString &name) { name_ = name; }

  inline const int &type() const { return type_; }
  inline void setType(const int &type) { type_ = type; }

  bool operator==(const Product& other) const;
  inline bool operator!=(const Product& other) const {
    return !(*this == other);
  }

  inline virtual int id() const { return ed_.id(); };
  inline virtual int label() const { return label_; }; 
  inline void setLabel(int num) { label_ = num; };
  virtual int submit(DB *db);
  virtual bool erase();

 private:
  void load();

  QString name_;
  int type_;

  DBElementData ed_;

  int label_;

// Persistence fields
 public:
  enum {
    FName = 1,
	  FType
  };
};

} // namespace LoboLab

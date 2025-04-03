// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#pragma once

#include <QSet>
#include <QTextStream>

namespace LoboLab {

class ModelProd;
class ModelLink;
class Product;

class Model {

 public:
  Model();
  ~Model();

  Model(const Model &source);
  Model &operator=(const Model &source);

  // static Model *createRandom(const TargetModel &targetModel);
  static Model *createRandom(const QList<Product*> products, const QList<int> outputLabels);
  static void cross(const Model *parent1, const Model *parent2,
    Model *&child1, Model *&child2);

  QSet<int> calcProductLabels() const;
  QSet<int> calcProductLabelsInUse(bool includeAllFeatures = false) const;

  inline int nProducts() const { return products_.size(); }
  inline ModelProd *product(int i) const { return products_.at(i); }
  ModelProd *prodWithLabel(int label, int *i = NULL) const;
  
  void addRandomProduct(int label, int type);
  ModelProd * duplicateProduct(int i, const QList<int> inputLabels, const QList<int> outputLabels);
  void removeProduct(int i);
  void removeProductWithLabel(int label);
  //void replaceProductWithLabelByRandomLinks(int label);

  
  inline int nLinks() const { return links_.size(); }
  inline ModelLink *link(int i) const { return links_.at(i); }
  inline QList<ModelLink*> links() const { return links_; }
  QList<ModelLink*> linksToLabel(int label) const;
  QList<ModelLink*> calcLinksInUse() const;
  int calcNLinksFromProd(int label) const;
  ModelLink *findLink(int regulator, int regulated) const;

  void addOrReplaceRandomLink();
  void addOrReplaceRandomLink(int regulatorLabel, int regulatedLabel);
  ModelLink *duplicateLink(int i, int toProductId, 
    const QList<int> inputLabels, const QList<int> outputLabels);
  void removeLink(int i);
  void removeLink(int regulatorLabel, int regulatedLabel);

  int calcComplexity() const;
  int calcComplexityInUse() const;

  void mutate(const QList<int> inputLabels, const QList<int> outputLabels, const int maxProductLabel);
  void clear();

  //bool hasSameTopology(const Model &other) const;

  // Text Serialization
  void loadFromString(QString &str);
  QString toString();

  friend QTextStream &operator<<(QTextStream &stream, const Model &model);
  friend QTextStream &operator>>(QTextStream &stream, Model &model);

  static double parseDouble(QTextStream &stream);
  
 private:
  static void distributeProducts(const QSet<int> &fromProds, 
                                 QSet<int> *toProds1, QSet<int> *toProds2);
  static void copyProductsAndLinks(Model *to,
    const Model *from1, const QSet<int> &products1,
    const Model *from2, const QSet<int> &products2);
  static void copyProducts(Model *to,const Model *from, 
                           const QSet<int> &products);
  static void copyLinks(Model *to,
    const Model *from1, const QSet<int> &products1,
    const Model *from2, const QSet<int> &products2);
  QMap<int, QList<int> > calcProductRegulators() const;
  int createNewLabel();
  ModelProd *findRandomLinkableFromProduct() const;
  int findRandomLinkableToLabel(const QList<int> inputLabels) const;

  QList<ModelProd*> products_;
  QList<ModelLink*> links_;

};

} // namespace LoboLab

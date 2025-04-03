// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#include "model.h"

#include "modelprod.h"
#include "modellink.h"
#include "Common/mathalgo.h"
#include "Common/log.h"
#include "Experiment/product.h"

namespace LoboLab {

Model::Model() {}

Model::~Model() {
  clear();
}

// Product-based uniform cross, including exclusive products.
// Child1 is more similar to parent1 and child2 is more similar to parent2.
void Model::cross(const Model *parent1, const Model *parent2,
                  Model *&child1, Model *&child2){
    QSet<int> prodsParent1 = parent1->calcProductLabels();
    QSet<int> prodsParent2 = parent2->calcProductLabels();
    // Common products
    QSet<int> commProdsCopied;
    QSet<int> commProdsSwapped;
    QSet<int> commonProds = prodsParent1;
    commonProds.intersect(prodsParent2);
    distributeProducts(commonProds, &commProdsCopied, &commProdsSwapped);

    // Exclusive products 1
    QSet<int> exclusive1to1;
    QSet<int> exclusive1to2;
    distributeProducts(prodsParent1.subtract(commonProds), &exclusive1to1, 
      &exclusive1to2);

    // Exclusive products 2
    QSet<int> exclusive2to1;
    QSet<int> exclusive2to2;
    distributeProducts(prodsParent2.subtract(commonProds), &exclusive2to2, 
      &exclusive2to1);
    
    child1 = new Model();
    child2 = new Model();

    // Copy the components using the selected products
    copyProductsAndLinks(child1, parent1, commProdsCopied + exclusive1to1,
      parent2, commProdsSwapped + exclusive2to1);
    copyProductsAndLinks(child2, parent2, commProdsCopied + exclusive2to2,
      parent1, commProdsSwapped + exclusive1to2);

    // Check if child1 is closer to parent2
    if (commProdsCopied.size() + exclusive1to1.size() < 
        commProdsSwapped.size() + exclusive2to1.size()) {
      Model *temp = child2;
      child2 = child1;
      child1 = temp;
    }

#ifdef QT_DEBUG
    // Check that the models are coherent
    QSet<int> labels = child1->calcProductLabels();
    int n = child1->nLinks();
    for (int i = 0; i < n; ++i) {
      ModelLink * link = child1->link(i);
      Q_ASSERT(labels.contains(link->regulatedProdLabel()) &&
        labels.contains(link->regulatedProdLabel()));
    }

    labels = child2->calcProductLabels();
    n = child2->nLinks();
    for (int i = 0; i < n; ++i) {
      ModelLink * link = child2->link(i);
      Q_ASSERT(labels.contains(link->regulatedProdLabel()) &&
        labels.contains(link->regulatedProdLabel()));
    }
#endif
}

void Model::distributeProducts(const QSet<int> &fromProds, QSet<int> *toProds1, 
                               QSet<int> *toProds2) {
  static const int productCrossRate = 50;

  QSetIterator<int> ite(fromProds);
  while (ite.hasNext()) {
    int label = ite.next();
    if (MathAlgo::rand100() < productCrossRate)
      toProds2->insert(label);
    else // products copied
      toProds1->insert(label);
  }
}

void Model::copyProductsAndLinks(Model *to,
                                 const Model *from1, const QSet<int> &products1,
                                 const Model *from2, const QSet<int> &products2){

  copyProducts(to, from1, products1);
  copyProducts(to, from2, products2);
  copyLinks(to, from1, products1, from2, products2);
}

void Model::copyProducts(Model *to,
                         const Model *from, const QSet<int> &products) {
  int n = from->nProducts();
  for (int i = 0; i<n; ++i) {
    ModelProd *prod = from->product(i);
    if (products.contains(prod->label()))
      to->products_.append(new ModelProd(*prod));
  }
}

void Model::copyLinks(Model *to, const Model *from1, const QSet<int> &products1,
                      const Model *from2, const QSet<int> &products2){
  // regulators list are possible regulators for substitution
  QList<int> regulators1 = products1.toList();
  qSort(regulators1);
  int i = 0;
  while (i < regulators1.size()) {
    regulators1.takeAt(i);
  }

  QList<int> regulators2 = products2.toList();
  qSort(regulators2);
  i = 0;
  while (i < regulators2.size()){
    regulators2.takeAt(i);
  }

  int n = from1->nLinks();
  for (int i = 0; i < n; ++i) {
    ModelLink *link = from1->link(i);
    int regulated = link->regulatedProdLabel();

    if (products1.contains(regulated)) {
      int regulator = link->regulatorProdLabel();
      if (products1.contains(regulator) || // Use the original regulator
          products2.contains(regulator)) {
        to->links_.append(new ModelLink(*link));
      } else if (!regulators2.isEmpty()) { // Substitute the regulator
        int newRegulator = regulators2[MathAlgo::randInt(regulators2.size())];
        if (!from1->findLink(newRegulator, regulated)) {
          ModelLink *newLink = new ModelLink(*link);
          newLink->setRegulator(newRegulator);
          to->links_.append(newLink);
        }
      }
    }
  }
  
  n = from2->nLinks();
  for (int i = 0; i < n; ++i) {
    ModelLink *link = from2->link(i);
    int regulated = link->regulatedProdLabel();

    if (products2.contains(regulated)) {
      int regulator = link->regulatorProdLabel();
      if (products2.contains(regulator) || // Use the original regulator
          products1.contains(regulator)) {
        to->links_.append(new ModelLink(*link));
      } else if (!regulators1.isEmpty()) { // Substitute the regulator
        int newRegulator = regulators1[MathAlgo::randInt(regulators1.size())];
        if (!from2->findLink(newRegulator, regulated)) {
          ModelLink *newLink = new ModelLink(*link);
          newLink->setRegulator(newRegulator);
          to->links_.append(newLink);
        }
      }
    }
  }
}

Model::Model(const Model &source) {
  *this = source;
}

Model &Model::operator=(const Model &source) {
  clear();
  int n = source.nProducts();
  for (int i = 0; i<n; ++i)
    products_.append(new ModelProd(*source.product(i)));

  n = source.nLinks();
  for (int i = 0; i<n; ++i)
    links_.append(new ModelLink(*source.link(i)));

  return *this;
}

void Model::clear() {
  int n = products_.size();
  for (int i=0; i<n; ++i)
    delete products_.at(i);

  products_.clear();

  n = links_.size();
  for (int i=0; i<n; ++i)
    delete links_.at(i);

  links_.clear();
}

QSet<int> Model::calcProductLabels() const {
  QSet<int> labels;
  int n = products_.size();
  for (int i = 0; i < n; ++i)
    labels += products_.at(i)->label();

  return labels;
}

ModelProd *Model::prodWithLabel(int label, int *ind) const {
  int nProducts = products_.size();
  int i = 0;
  while (i < nProducts && products_.at(i)->label() != label)
    ++i;

  if (i < nProducts) {
    if (ind)
      *ind = i;
    return products_.at(i);
  } else {
    return NULL;
  }
}  

// The products in use are those with a path ending in a structural product.
QSet<int> Model::calcProductLabelsInUse(bool includeAllFeatures) const {
  // int numStrucProd = products_.length();
  QSet<int> productsInUse;

  for (int i = 0; i < products_.length(); i++)
  {
    if (products_[i]->type() == 2)
      productsInUse.insert(products_[i]->label());
    else if (includeAllFeatures && products_[i]->type() < 3)
      productsInUse.insert(products_[i]->label());
  }
  
  // Pre-calculate the links in the model
  QMap<int, QList<int> > productRegulators = calcProductRegulators();

  QList<int> productsToVisit = productsInUse.toList();

  // Products regulating a product in use are in use
  while (!productsToVisit.isEmpty()) {
    const QList<int> &regulators = productRegulators[productsToVisit.takeFirst()];
    int nRegulators = regulators.size();
    for (int j = 0; j < nRegulators; ++j) {
      int r = regulators[j];
      if (!productsInUse.contains(r)) {
        productsInUse.insert(r);
        productsToVisit.append(r);
      }
    }
  }

  return productsInUse;
}

QMap<int, QList<int> > Model::calcProductRegulators() const {
  QMap<int, QList<int> > productRegulators;

  int n = links_.size();
  for (int i = 0; i < n; ++i) {
    ModelLink *link = links_.at(i);
    productRegulators[link->regulatedProdLabel()].append(
      link->regulatorProdLabel());
  }

  return productRegulators;
}

Model *Model::createRandom(const QList<Product*> products, const QList<int> outputLabels) {
  Model *model = new Model();
  QList<int> prodLabels;

  int nProducts = products.size();
  for (int i = 0; i < nProducts; ++i) {
    model->addRandomProduct(products[i]->label(), products[i]->type());
    prodLabels.append(products[i]->label());
  }
  qSort(prodLabels);

  int nLinks = 0.5 * nProducts; 
  int nOutputs = outputLabels.size();

  // for (int i = 0; i < nProducts - outputLabels.size(); ++i) { // output node is not regulated by itself
  for (int i = 0; i < nProducts; ++i) { // output node is regulated by itself
    int nReg = MathAlgo::randInt(1 + nLinks);
    for (int j = 0; j < nReg; ++j) 
      model->addOrReplaceRandomLink(prodLabels[i],
        prodLabels[nProducts - 1]);
  }

#ifdef QT_DEBUG
  // Check that the model is coherent
  QSet<int> labels = model->calcProductLabels();
  int n = model->nLinks();
  for (int i = 0; i < n; ++i) {
    ModelLink * link = model->link(i);
    Q_ASSERT(labels.contains(link->regulatedProdLabel()) &&
             labels.contains(link->regulatedProdLabel()));
  }
#endif

  return model;
}


ModelProd* Model::duplicateProduct(int i, const QList<int> inputLabels, const QList<int> outputLabels) {
  ModelProd *newProd = new ModelProd(*products_[i]);
  int newLabel = createNewLabel();
  newProd->setLabel(newLabel);
  newProd->setType(3);
  products_.append(newProd);

  // Create two random links using linkable products
  //int fromLabel = newLabel;
  // while(fromLabel == newLabel || outputLabels.contains(fromLabel)) 
  // while (fromLabel == newLabel)
  int fromLabel = findRandomLinkableFromProduct()->label();
  addOrReplaceRandomLink(fromLabel, newLabel);
  
  int toLabel = findRandomLinkableToLabel(inputLabels);
  addOrReplaceRandomLink(newLabel, toLabel);

  //addOrReplaceRandomLink(findRandomLinkableFromProduct(targetModel)->label(), 
  //                       newLabel);

  return newProd;
}

ModelProd *Model::findRandomLinkableFromProduct() const {
  ModelProd *prod;
  prod = products_[MathAlgo::randInt(nProducts())];

  return prod;
}

int Model::findRandomLinkableToLabel(const QList<int> inputLabels) const {
  QList<int> linkableLabels;
  linkableLabels.reserve(nProducts());
  for (int i = 0; i < nProducts(); ++i) {
    if (products_[i]->label() > inputLabels.size())
      linkableLabels.append(products_[i]->label());
  }
  return linkableLabels[MathAlgo::randInt(linkableLabels.size())];
}

void Model::removeProduct(int i) {
  ModelProd *prod = products_.takeAt(i);
  int label = prod->label();
  delete prod;

  int n = links_.size();
  for (int i = n-1; i >= 0; --i) {
    ModelLink *link = links_.at(i);
    if (link->regulatorProdLabel() == label ||
        link->regulatedProdLabel() == label) {
      links_.removeAt(i);
      delete link;
    }
  }
}

void Model::removeProductWithLabel(int label) {
  int i;
  ModelProd *prod = prodWithLabel(label, &i);

  if (prod) {
    products_.takeAt(i);
    int label = prod->label();
    delete prod;

    int n = links_.size();
    for (int i = n-1; i >= 0; --i) {
      ModelLink *link = links_.at(i);
      if (link->regulatorProdLabel() == label ||
          link->regulatedProdLabel() == label) {
        links_.removeAt(i);
        delete link;
      }
    }
  }
}

int Model::createNewLabel() {
    // Select always a new label
  static int newLabel = 200;
  newLabel++;

  return newLabel;
}

void Model::addRandomProduct(int label, int type) {
  if (ModelProd *oldProd = prodWithLabel(label)) {
    products_.removeOne(oldProd);
    delete oldProd;
  }

  ModelProd *newProd = new ModelProd(label, type);
  products_.append(newProd);
}

QList<ModelLink*> Model::linksToLabel(int label) const {
  QList<ModelLink*> linksToLabel;
  int nLinks = links_.size();
  for (int i = 0; i < nLinks; ++i)
    if (links_.at(i)->regulatedProdLabel() == label)
      linksToLabel.append(links_.at(i));

  return linksToLabel;
}

QList<ModelLink*> Model::calcLinksInUse() const {
  QList<ModelLink*> linksInUse;
  QSet<int> labelsInUse = calcProductLabelsInUse();
  int nLinks = links_.size();
  for (int i = 0; i < nLinks; ++i) {
    ModelLink *link = links_.at(i);
    if (labelsInUse.contains(link->regulatorProdLabel()) &&
        labelsInUse.contains(link->regulatedProdLabel()))
      linksInUse.append(link);
  }

  return linksInUse;
}

int Model::calcNLinksFromProd(int prodLabel) const {
  int n = 0;
  int nLinks = links_.size();
  for (int i = 0; i < nLinks; ++i)
    if (prodLabel == links_[i]->regulatorProdLabel())
      ++n;

  return n;
}

ModelLink *Model::findLink(int regulator, int regulated) const {
  int nLinks = links_.size();
  int i = 0;
  while (i < nLinks) {
    ModelLink *link = links_[i];
    if (link->regulatorProdLabel() == regulator && 
        link->regulatedProdLabel() == regulated)
      return link;
    else
      ++i;
  }

  return NULL;
}


void Model::addOrReplaceRandomLink() {
  int nProducts = products_.size();
  int regulatorLabel = 0;
  int regulatedLabel = 0;
  while (regulatorLabel == regulatedLabel) {
    regulatorLabel = products_.at(MathAlgo::randInt(nProducts))->label();
    regulatedLabel = products_.at(MathAlgo::randInt(nProducts))->label();
  }
  addOrReplaceRandomLink(regulatorLabel, regulatedLabel);
}

void Model::addOrReplaceRandomLink(int regulatorLabel, int regulatedLabel) {
  removeLink(regulatorLabel, regulatedLabel);

  // new ModelLink will check if regulatedLabel is a metabolite
  ModelLink *newLink = new ModelLink(regulatorLabel, regulatedLabel);
  links_.append(newLink);
}

//If the link exists, delete it
void Model::removeLink(int regulatorLabel, int regulatedLabel) {
  int nLinks = links_.size();
  bool found = false;
  int i = 0;
  while (!found && i < nLinks) {
    ModelLink *link = links_.at(i);
    if (regulatorLabel == link->regulatorProdLabel() &&
        regulatedLabel == link->regulatedProdLabel()) {
      found = true;
      links_.removeAt(i);
      delete link;
    } else
      ++i;
  }
}

ModelLink *Model::duplicateLink(int i, int toProductId, 
  const QList<int> inputLabels, const QList<int> outputLabels) {
  ModelLink *newLink = new ModelLink(*links_.at(i));

  int regulatorLabel = 0;
  int regulatedLabel = 0;

  regulatorLabel = findRandomLinkableFromProduct()->label();
  regulatedLabel = findRandomLinkableToLabel(inputLabels);

  removeLink(regulatorLabel, regulatedLabel);
  newLink->setRegulator(regulatorLabel);
  newLink->setRegulated(regulatedLabel);
  links_.append(newLink);

  return newLink;
}

void Model::removeLink(int i) {
  delete links_.takeAt(i);
}

void Model::mutate(const QList<int> inputLabels, const QList<int> outputLabels, const int maxProductLabel) {
  // Copy based mutations & Individual param mutations
  static const int paramMutationProb = 1;
  // qSort(outputLabels);
  // int maxProductLabel = maxProductLabel;

  // Product mutations without external factors
  for (int i = nProducts() - 1; i >= 0; --i) {
    ModelProd *prod = products_[i];
    // Param mutations for target product

    if (MathAlgo::rand100() < 1) // Copy product
      duplicateProduct(i, inputLabels, outputLabels);

    if (prod->label() > maxProductLabel && MathAlgo::rand1000() < 15) // Remove product
      removeProduct(i);

    prod->mutateParams(paramMutationProb);

  }
  
  // Link mutations
  for (int i = nLinks()-1; i >= 0;  --i) {
    ModelLink *link = links_[i];
    int toProductId = link->regulatedProdLabel();
 
    // Copy link
    if (MathAlgo::rand100() < 1) 
      duplicateLink(i, toProductId, inputLabels, outputLabels)->mutateParams(
        paramMutationProb, true, true);

    // Remove link
    if (MathAlgo::rand1000() < 15)
      removeLink(i);
    else // Param mutations for not target link
      link->mutateParams(paramMutationProb, true, true);
    // } 
  }

#ifdef QT_DEBUG
  {
    // Check that the model is coherent
    QSet<int> labels = calcProductLabels();
    for (int i = 0; i < nLinks(); ++i) {
      ModelLink * link = links_.at(i);
      Q_ASSERT(labels.contains(link->regulatedProdLabel()) &&
               labels.contains(link->regulatedProdLabel()));
    }
  }
#endif
}

int Model::calcComplexity() const {
  int complexity = 0;

  int n = products_.size();
  for (int i=0; i < n; ++i)
    complexity += products_.at(i)->complexity();

  n = links_.size();
  for (int i=0; i < n; ++i)
    complexity += links_.at(i)->complexity();

  return complexity;
}

int Model::calcComplexityInUse() const {
  int complexity = 0;

  const QSet<int> labelsInUse = calcProductLabelsInUse();

  int n = products_.size();
  for (int i=0; i < n; ++i) {
    ModelProd *prod = products_.at(i);
    if (labelsInUse.contains(prod->label()))
      complexity += prod->complexity();
  }

  n = links_.size();
  for (int i=0; i < n; ++i) {
    ModelLink *link = links_.at(i);
    if (labelsInUse.contains(link->regulatorProdLabel()) &&
        labelsInUse.contains(link->regulatedProdLabel()))
      complexity += link->complexity();
  }

  return complexity;
}

// Text Serialization
void Model::loadFromString(QString &str) {
  QTextStream stream(&str, QIODevice::ReadOnly);
  stream >> *this;
}

QString Model::toString() {
  QString str;
  QTextStream stream(&str, QIODevice::WriteOnly);
  stream << *this;

  return str;
}

QTextStream &operator<<(QTextStream &stream, const Model &model) {
  stream << '(';

  int n = model.products_.size();
  if (n > 0) {
    for (int i=0; i<n-1; ++i) {
      stream << *model.products_.at(i);
      stream << '|';
    }

    stream << *model.products_.last();
  }

  stream << '*';

  n = model.links_.size();
  if (n > 0) {
    for (int i=0; i<n-1; ++i) {
      stream << *model.links_.at(i);
      stream << '|';
    }

    stream << *model.links_.last();
  }

  stream << ')';

  Q_ASSERT(model.nProducts() > 0);

  return stream;
}

QTextStream &operator>>(QTextStream &stream, Model &model) {
  model.clear();

  char c;
  stream >> c;
  Q_ASSERT(c == '(');

  while (c != '*') {
    ModelProd *modelProd = new ModelProd();
    stream >> *modelProd;
    model.products_.append(modelProd);
    stream >> c;
  }

  stream >> c;
  if (c != ')') {
    stream.seek(stream.pos()-1);

    while (c != ')') {
      ModelLink *modelLink = new ModelLink();
      stream >> *modelLink;
      model.links_.append(modelLink);
      stream >> c;
    }
  }

  Q_ASSERT(model.nProducts() > 0);

  return stream;
}

double Model::parseDouble(QTextStream &stream) {
  QChar c;
  QString str;
  bool end = false;
  while (!end) {
    stream >> c;
    if (c == ' ' || c == '|' || c == '*' || c == ')') {
      end = true;
      stream.seek(stream.pos() - 1);
    } else if (c == 0) // end of stream
      end = true;
    else
      str.append(c);
  }

  return str.toDouble();
}

}
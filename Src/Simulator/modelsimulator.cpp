// Copyright (c) Lobo Lab (lobo@umbc.edu)
// All rights reserved.

#include "modelsimulator.h"

#include "simstate.h"
#include "simop.h"
#include "simopone.h"
#include "simopzero.h"
#include "simophillact.h"
#include "simophillrep.h"
#include "simopcopy.h"
#include "simopand.h"
#include "simopor.h"
#include "simopdiv.h"
#include "simophalf.h"
#include "Search/searchalgodetcrowd.h"

#include "Model/model.h"
#include "Model/modelprod.h"
#include "Model/modellink.h"

#include "Common/log.h"

namespace LoboLab {

ModelSimulator::ModelSimulator()
  : h_(0), nProducts_(0), nAllocatedProducts_(0), oldConcs_(NULL),
    regul_(NULL), limits_(NULL), productions_(NULL),
    degradations_(NULL), degradationFactors_(NULL),
    rates1_(NULL), rates2_(NULL), rates3_(NULL), rates4_(NULL), rates5_(NULL),
    rates6_(NULL), rates7_(NULL), rates8_(NULL), rates9_(NULL), rates10_(NULL),
    nOps_(0), nAllocatedOps_(0), ops_(NULL) {
}

ModelSimulator::~ModelSimulator() {
  clearAll();
}

void ModelSimulator::clearAll() {
  clearLabels();
  clearProducts();
  clearOps();
}

void ModelSimulator::clearLabels() {
  labels_.clear();
  labels2Ind_.clear();
  outputLabels_.clear();
}

void ModelSimulator::clearProducts() {
  delete[] oldConcs_;
  delete[] regul_;
  delete[] limits_;
  delete[] productions_;
  delete[] degradations_;
  delete[] degradationFactors_;
  delete[] rates1_;
  delete[] rates2_;
  delete[] rates3_;
  delete[] rates4_;
  delete[] rates5_;
  delete[] rates6_;
  delete[] rates7_;
  delete[] rates8_;
  delete[] rates9_;
  delete[] rates10_;

  nAllocatedProducts_ = 0;
  nConstRateProducts_ = 0;
  nIntermediateProducts_ = 0;
  nOutputProducts_ = 0;
}

void ModelSimulator::clearOps() {
  deleteOps();

  delete[] ops_;

  nAllocatedOps_ = 0;
}

void ModelSimulator::deleteOps() {
  for (int i = 0; i < nOps_; ++i)
    delete ops_[i];
}

void ModelSimulator::loadModel(const Model &model, bool includeAllFeatures) {
  clearLabels();
  QSet<int> labelSet = model.calcProductLabelsInUse(includeAllFeatures);

  labels_ = labelSet.toList();

  std::sort(labels_.begin(), labels_.end(), [model](int a, int b) {
    return model.prodWithLabel(a)->type() < model.prodWithLabel(b)->type()
             || (a < b) ;
  });
  nProducts_ = labels_.size();
  nConstRateProducts_ = 0;
  nIntermediateProducts_ = 0;
  nOutputProducts_ = 0;

  labels2Ind_ = QHash<int, int>();
  for (int i = 0; i < nProducts_; ++i)
    labels2Ind_[labels_.at(i)] = i;


  if (nProducts_ > nAllocatedProducts_) {
    clearProducts();

    oldConcs_ = new double[nProducts_];
    regul_ = new double[nProducts_];
    productions_ = new double[nProducts_];
    limits_ = new double[nProducts_];
    constRates_ = new double[nProducts_];
    degradations_ = new double[nProducts_];
    degradationFactors_ = new double[nProducts_];

    rates1_ = new double[nProducts_];
    rates2_ = new double[nProducts_];
    rates3_ = new double[nProducts_];
    rates4_ = new double[nProducts_];
    rates5_ = new double[nProducts_];
    rates6_ = new double[nProducts_];
    rates7_ = new double[nProducts_];
    rates8_ = new double[nProducts_];
    rates9_ = new double[nProducts_];
    rates10_ = new double[nProducts_];

    nAllocatedProducts_ = nProducts_;
  }

  QList<SimOp*> opsList; // Temporary storage for the operations
  for (int i = 0; i < nProducts_; ++i) {
    // Process product constants
    ModelProd *prod = model.prodWithLabel(labels_.at(i));
    productions_[i] = 1;
    limits_[i] = prod->lim();
    constRates_[i] = 0;
    degradations_[i] = prod->deg();
    degradationFactors_[i] = 0; // Use degradationsFactors to handle drugs
    if (prod->type() <= 1)
      nConstRateProducts_++;

    if (prod->type() == 2)
      outputLabels_.append(prod->label());

    // Process product links
    QList<ModelLink*> links = model.linksToLabel(labels_.at(i));

    // Categorize links
    int n = links.size();

    QList<ModelLink*> orLinks;
    QList<ModelLink*> andLinks;
    for (int j = 0; j < n; ++j) {
      ModelLink *link = links[j];
      if (labelSet.contains(link->regulatorProdLabel())) {
        if (link->isAndReg())
          andLinks.append(link);
        else
          orLinks.append(link);
      }
    }

    if (orLinks.isEmpty() && andLinks.isEmpty()) // No links
      opsList.append(new SimOpZero(&regul_[i]));
    else
      opsList.append(createProductOps(i, orLinks, andLinks));

  }
  nOutputProducts_ = nProducts_ - (nConstRateProducts_ + nIntermediateProducts_);

  int nNewOps = opsList.size();

  if (nNewOps > nAllocatedOps_) {
    clearOps();
    ops_ = new SimOp*[nNewOps];
    nAllocatedOps_ = nNewOps;
  }
  else
    deleteOps();

  nOps_ = nNewOps;

  for (int i = 0; i < nOps_; ++i)
    ops_[i] = opsList.at(i);

  h_ = hini;
  errold_ = erroldini;
  success_ = true;
}

QList<SimOp*> ModelSimulator::createProductOps(int p, const QList<ModelLink*> &orLinks,
  const QList<ModelLink*>& andLinks) {

  QList<SimOp*> opsList;
  bool regulTempUsed = false;

  if (orLinks.isEmpty() && andLinks.isEmpty())
    opsList.append(new SimOpZero(&regul_[p]));
  else {
  // Process OR links
    int n = orLinks.size();
    for (int i = 0; i < n; ++i) {
      ModelLink *link = orLinks[i];
      if (link->hillCoef() >= 0) {
        if (!regulTempUsed) {
          opsList.append(new SimOpZero(&regul_[p]));
          regulTempUsed = true;
        }
        opsList.append(new SimOpOr(&oldConcs_[labels2Ind_[link->regulatorProdLabel()]],
          link->disConst(), link->hillCoef(), &regul_[p]));
      }
    }

    // Process AND links
    n = andLinks.size();
    for (int i = 0; i < n; ++i) {
      ModelLink *link = andLinks[i];
      if (link->hillCoef() >= 0) {
        if (!regulTempUsed) {
          opsList.append(new SimOpOne(&regul_[p]));
          regulTempUsed = true;
        }
        opsList.append(new SimOpAnd(&oldConcs_[labels2Ind_[link->regulatorProdLabel()]],
          link->disConst(), link->hillCoef(), &regul_[p]));
      }
    }

    if (!regulTempUsed) // No activator
      opsList.append(new SimOpOne(&regul_[p]));

    // Process division for And links 
    n = andLinks.size();
    for (int i = 0; i < n; ++i) {
      ModelLink *link = andLinks[i];
      opsList.append(new SimOpDiv(&oldConcs_[labels2Ind_[link->regulatorProdLabel()]],
        link->disConst(), fabs(link->hillCoef()), &regul_[p]));
    }

    // Process division for Or links
    n = orLinks.size();
    for (int i = 0; i < n; ++i) {
      ModelLink *link = orLinks[i];
      opsList.append(new SimOpDiv(&oldConcs_[labels2Ind_[link->regulatorProdLabel()]],
        link->disConst(), fabs(link->hillCoef()), &regul_[p]));
    }
  }
  return opsList;
}

SimOp *ModelSimulator::createHillOpForLink(ModelLink *link, double *to) const {
  if (link->hillCoef() >= 0)
    return new SimOpHillAct(
             &oldConcs_[labels2Ind_[link->regulatorProdLabel()]],
             to, link->disConst(), link->hillCoef());
  else
    return new SimOpHillRep(
             &oldConcs_[labels2Ind_[link->regulatorProdLabel()]],
             to, link->disConst(), -1.0 * link->hillCoef());
}

const double ModelSimulator::b1 = 5.42937341165687622380535766363e-2;
const double ModelSimulator::b6 = 4.45031289275240888144113950566e0;
const double ModelSimulator::b7 = 1.89151789931450038304281599044e0;
const double ModelSimulator::b8 = -5.8012039600105847814672114227e0;
const double ModelSimulator::b9 = 3.1116436695781989440891606237e-1;
const double ModelSimulator::b10 = -1.52160949662516078556178806805e-1;
const double ModelSimulator::b11 = 2.01365400804030348374776537501e-1;
const double ModelSimulator::b12 = 4.47106157277725905176885569043e-2;

const double ModelSimulator::bhh1 = 0.244094488188976377952755905512e+00;
const double ModelSimulator::bhh2 = 0.733846688281611857341361741547e+00;
const double ModelSimulator::bhh3 = 0.220588235294117647058823529412e-01;

const double ModelSimulator::a21 = 5.26001519587677318785587544488e-2;
const double ModelSimulator::a31 = 1.97250569845378994544595329183e-2;
const double ModelSimulator::a32 = 5.91751709536136983633785987549e-2;
const double ModelSimulator::a41 = 2.95875854768068491816892993775e-2;
const double ModelSimulator::a43 = 8.87627564304205475450678981324e-2;
const double ModelSimulator::a51 = 2.41365134159266685502369798665e-1;
const double ModelSimulator::a53 = -8.84549479328286085344864962717e-1;
const double ModelSimulator::a54 = 9.24834003261792003115737966543e-1;
const double ModelSimulator::a61 = 3.7037037037037037037037037037e-2;
const double ModelSimulator::a64 = 1.70828608729473871279604482173e-1;
const double ModelSimulator::a65 = 1.25467687566822425016691814123e-1;
const double ModelSimulator::a71 = 3.7109375e-2;
const double ModelSimulator::a74 = 1.70252211019544039314978060272e-1;
const double ModelSimulator::a75 = 6.02165389804559606850219397283e-2;
const double ModelSimulator::a76 = -1.7578125e-2;
const double ModelSimulator::a81 = 3.70920001185047927108779319836e-2;
const double ModelSimulator::a84 = 1.70383925712239993810214054705e-1;
const double ModelSimulator::a85 = 1.07262030446373284651809199168e-1;
const double ModelSimulator::a86 = -1.53194377486244017527936158236e-2;
const double ModelSimulator::a87 = 8.27378916381402288758473766002e-3;
const double ModelSimulator::a91 = 6.24110958716075717114429577812e-1;
const double ModelSimulator::a94 = -3.36089262944694129406857109825e0;
const double ModelSimulator::a95 = -8.68219346841726006818189891453e-1;
const double ModelSimulator::a96 = 2.75920996994467083049415600797e1;
const double ModelSimulator::a97 = 2.01540675504778934086186788979e1;
const double ModelSimulator::a98 = -4.34898841810699588477366255144e1;
const double ModelSimulator::a101 = 4.77662536438264365890433908527e-1;
const double ModelSimulator::a104 = -2.48811461997166764192642586468e0;
const double ModelSimulator::a105 = -5.90290826836842996371446475743e-1;
const double ModelSimulator::a106 = 2.12300514481811942347288949897e1;
const double ModelSimulator::a107 = 1.52792336328824235832596922938e1;
const double ModelSimulator::a108 = -3.32882109689848629194453265587e1;
const double ModelSimulator::a109 = -2.03312017085086261358222928593e-2;
const double ModelSimulator::a111 = -9.3714243008598732571704021658e-1;
const double ModelSimulator::a114 = 5.18637242884406370830023853209e0;
const double ModelSimulator::a115 = 1.09143734899672957818500254654e0;
const double ModelSimulator::a116 = -8.14978701074692612513997267357e0;
const double ModelSimulator::a117 = -1.85200656599969598641566180701e1;
const double ModelSimulator::a118 = 2.27394870993505042818970056734e1;
const double ModelSimulator::a119 = 2.49360555267965238987089396762e0;
const double ModelSimulator::a1110 = -3.0467644718982195003823669022e0;
const double ModelSimulator::a121 = 2.27331014751653820792359768449e0;
const double ModelSimulator::a124 = -1.05344954667372501984066689879e1;
const double ModelSimulator::a125 = -2.00087205822486249909675718444e0;
const double ModelSimulator::a126 = -1.79589318631187989172765950534e1;
const double ModelSimulator::a127 = 2.79488845294199600508499808837e1;
const double ModelSimulator::a128 = -2.85899827713502369474065508674e0;
const double ModelSimulator::a129 = -8.87285693353062954433549289258e0;
const double ModelSimulator::a1210 = 1.23605671757943030647266201528e1;
const double ModelSimulator::a1211 = 6.43392746015763530355970484046e-1;
const double ModelSimulator::a141 = 5.61675022830479523392909219681e-2;
const double ModelSimulator::a147 = 2.53500210216624811088794765333e-1;
const double ModelSimulator::a148 = -2.46239037470802489917441475441e-1;
const double ModelSimulator::a149 = -1.24191423263816360469010140626e-1;
const double ModelSimulator::a1410 = 1.5329179827876569731206322685e-1;
const double ModelSimulator::a1411 = 8.20105229563468988491666602057e-3;
const double ModelSimulator::a1412 = 7.56789766054569976138603589584e-3;
const double ModelSimulator::a1413 = -8.298e-3;
const double ModelSimulator::a151 = 3.18346481635021405060768473261e-2;
const double ModelSimulator::a156 = 2.83009096723667755288322961402e-2;
const double ModelSimulator::a157 = 5.35419883074385676223797384372e-2;
const double ModelSimulator::a158 = -5.49237485713909884646569340306e-2;
const double ModelSimulator::a1511 = -1.08347328697249322858509316994e-4;
const double ModelSimulator::a1512 = 3.82571090835658412954920192323e-4;
const double ModelSimulator::a1513 = -3.40465008687404560802977114492e-4;
const double ModelSimulator::a1514 = 1.41312443674632500278074618366e-1;
const double ModelSimulator::a161 = -4.28896301583791923408573538692e-1;
const double ModelSimulator::a166 = -4.69762141536116384314449447206e0;
const double ModelSimulator::a167 = 7.68342119606259904184240953878e0;
const double ModelSimulator::a168 = 4.06898981839711007970213554331e0;
const double ModelSimulator::a169 = 3.56727187455281109270669543021e-1;
const double ModelSimulator::a1613 = -1.39902416515901462129418009734e-3;
const double ModelSimulator::a1614 = 2.9475147891527723389556272149e0;
const double ModelSimulator::a1615 = -9.15095847217987001081870187138e0;

const double ModelSimulator::er1 = 0.1312004499419488073250102996e-01;
const double ModelSimulator::er6 = -0.1225156446376204440720569753e+01;
const double ModelSimulator::er7 = -0.4957589496572501915214079952e+00;
const double ModelSimulator::er8 = 0.1664377182454986536961530415e+01;
const double ModelSimulator::er9 = -0.3503288487499736816886487290e+00;
const double ModelSimulator::er10 = 0.3341791187130174790297318841e+00;
const double ModelSimulator::er11 = 0.8192320648511571246570742613e-01;
const double ModelSimulator::er12 = -0.2235530786388629525884427845e-01;

const double ModelSimulator::aTol = 1.0e-6; // Absolute tolerance
const double ModelSimulator::rTol = 1.0e-6; // Relative tolerance
const double ModelSimulator::hini = 1.0e-3; // Initial step size
const double ModelSimulator::hmin = 1.0e-6; // Minimum step size
const double ModelSimulator::cmin = 1.0e-6; // Minimum concentration
const double ModelSimulator::cmax = 1.0e+9; // Maximum concentration
const double ModelSimulator::erroldini = 1.0e-4;
const double ModelSimulator::erroldmin = 1.0e-4;
const double ModelSimulator::beta = 0.0;
const double ModelSimulator::alpha = 1.0 / 8.0 - beta*0.2;
const double ModelSimulator::safe = 0.9;
const double ModelSimulator::minscale = 0.333;
const double ModelSimulator::maxscale = 6.0;

// 8th order Runge-Kutta with adaptive stepsize
// See Numerical recipes, 3rd ed, Chapter 17 for an introduction
double ModelSimulator::simulate(double tSpan, SimState &state, bool rateCheck) {
  double maxChange = 0.0;
  double growthLim = 1.0;
  double growthpen = 0;
  double growPenConst = 1000;
  double *y = state.products();
  double t = 0.0;
  double hovershot = 0.0;
  while (t < tSpan) { // Loop until full time span is integrated
    if ((t + h_*1.0001) > tSpan) {
      hovershot = h_;
      h_ = tSpan - t;
    }
    
    for (int i = 0; i < nProducts_; ++i) {
      oldConcs_[i] = y[i];
    }
    calcRates(rates1_);

    double hnext;
    do { // Loop until found a small enough timestep with a successful integration
      double errRat = integrate(y);
      hnext = checkSuccess(errRat);
      if (!success_) {
        hovershot = 0;
        if (hnext < hmin) {
          Log::write() << "ModelSimulator::simulate: ERROR: Minimum h overflow at t = " << t << ", tspan = " << tSpan << " used h = " << h_ << ", new h = " << hnext << endl;
          return -1.0;
        }
        h_ = hnext;
      }
    } while (!success_);

    double stepMaxChange = 0.0;
    for (int i = 0; i < nProducts_; ++i) {
      if (stepMaxChange < qAbs(rates4_[i])) {
        stepMaxChange = qAbs(rates4_[i]);
      }

      double c = y[i] + h_ * rates4_[i];
      if (c > cmax) {
        Log::write() << "ModelSimulator::simulate: ERROR: Maximum c overflow at t = " << t << ", tspan = " << tSpan << " used h = " << h_ << ", new h = " << hnext << endl;
        return -2.0;
      }
      else if (c < cmin)
        y[i] = 0.0;
      else
        y[i] = c;
    }

    maxChange += stepMaxChange;
    t += h_;
    h_ = MathAlgo::min(1.0, hnext); // Do not allow time steps higher than 1 day,
                                    // in order to compute the growh limit at a maximum of 1 day span.
  }

  if (h_ < hovershot)
    h_ = hovershot;
    
  return maxChange;
}

double ModelSimulator::integrate(const double* y) {
  for (int i = 0; i < nProducts_; ++i)
    oldConcs_[i] = MathAlgo::max(0.0, y[i] + h_*a21*rates1_[i]);
  calcRates(rates2_);
  for (int i = 0; i < nProducts_; ++i)
    oldConcs_[i] = MathAlgo::max(0.0, y[i] + h_*(a31*rates1_[i] + a32*rates2_[i]));
  calcRates(rates3_);
  for (int i = 0; i < nProducts_; ++i)
    oldConcs_[i] = MathAlgo::max(0.0, y[i] + h_*(a41*rates1_[i] + a43*rates3_[i]));
  calcRates(rates4_);
  for (int i = 0; i < nProducts_; ++i)
    oldConcs_[i] = MathAlgo::max(0.0, y[i] + h_*(a51*rates1_[i] + a53*rates3_[i] + a54*rates4_[i]));
  calcRates(rates5_);
  for (int i = 0; i < nProducts_; ++i)
    oldConcs_[i] = MathAlgo::max(0.0, y[i] + h_*(a61*rates1_[i] + a64*rates4_[i] + a65*rates5_[i]));
  calcRates(rates6_);
  for (int i = 0; i < nProducts_; ++i)
    oldConcs_[i] = MathAlgo::max(0.0, y[i] + h_*(a71*rates1_[i] + a74*rates4_[i] + a75*rates5_[i] + a76*rates6_[i]));
  calcRates(rates7_);
  for (int i = 0; i < nProducts_; ++i)
    oldConcs_[i] = MathAlgo::max(0.0, y[i] + h_*(a81*rates1_[i] + a84*rates4_[i] + a85*rates5_[i] + a86*rates6_[i] + a87*rates7_[i]));
  calcRates(rates8_);
  for (int i = 0; i < nProducts_; ++i)
    oldConcs_[i] = MathAlgo::max(0.0, y[i] + h_*(a91*rates1_[i] + a94*rates4_[i] + a95*rates5_[i] + a96*rates6_[i] + a97*rates7_[i] + a98*rates8_[i]));
  calcRates(rates9_);
  for (int i = 0; i < nProducts_; ++i)
    oldConcs_[i] = MathAlgo::max(0.0, y[i] + h_*(a101*rates1_[i] + a104*rates4_[i] + a105*rates5_[i] + a106*rates6_[i] + a107*rates7_[i] + a108*rates8_[i] + a109*rates9_[i]));
  calcRates(rates10_);
  for (int i = 0; i < nProducts_; ++i)
    oldConcs_[i] = MathAlgo::max(0.0, y[i] + h_*(a111*rates1_[i] + a114*rates4_[i] + a115*rates5_[i] + a116*rates6_[i] + a117*rates7_[i] + a118*rates8_[i] + a119*rates9_[i] + a1110*rates10_[i]));
  calcRates(rates2_);
  for (int i = 0; i < nProducts_; ++i)
    oldConcs_[i] = MathAlgo::max(0.0, y[i] + h_*(a121*rates1_[i] + a124*rates4_[i] + a125*rates5_[i] + a126*rates6_[i] + a127*rates7_[i] + a128*rates8_[i] + a129*rates9_[i] + a1210*rates10_[i] + a1211*rates2_[i]));
  calcRates(rates3_);

  double err = 0.0;
  double err2 = 0.0;
  for (int i = 0; i < nProducts_; ++i) {
    rates4_[i] = b1*rates1_[i] + b6*rates6_[i] + b7*rates7_[i] + b8*rates8_[i] + b9*rates9_[i] + b10*rates10_[i] + b11*rates2_[i] + b12*rates3_[i];

    double e1 = rates4_[i] - bhh1*rates1_[i] - bhh2*rates9_[i] - bhh3*rates3_[i];
    double e2 = er1*rates1_[i] + er6*rates6_[i] + er7*rates7_[i] + er8*rates8_[i] + er9*rates9_[i] + er10*rates10_[i] + er11*rates2_[i] + er12*rates3_[i];
    double sk = aTol + rTol*y[i];
    err2 += MathAlgo::sqr(e1 / sk);
    err += MathAlgo::sqr(e2 / sk);
  }

  double deno = err + 0.01*err2;
  if (deno <= 0.0)
    deno = 1.0;

  double errRat = h_*err*sqrt(1.0 / (nProducts_*deno));
  return errRat;
}

// Compute operations using oldConcs_ and saving in regul_
void ModelSimulator::calcRates(double *rates) {
  for (int i = 0; i < nOps_; ++i)
    ops_[i]->compute();

  for (int i = 0; i < nConstRateProducts_; ++i) {
    rates[i] = constRates_[i];
  }

  for (int i = nConstRateProducts_; i < nProducts_; ++i) {
    rates[i] = productions_[i] * (limits_[i] * regul_[i])
      + (degradationFactors_[i] - degradations_[i]) * oldConcs_[i];
  }
}

double ModelSimulator::checkSuccess(double errRat) {
  double hnext;
  double scale;
  if (errRat <= 1.0) {
    if (errRat == 0.0) {
      scale = maxscale;
    } else {
      scale = safe*pow(errRat, -alpha)*pow(errold_, beta);
      if (scale < minscale) scale = minscale;
      if (scale > maxscale) scale = maxscale;
    }
    if (success_) // previous check success
      hnext = h_*scale;
    else
      hnext = h_*MathAlgo::min(scale, 1.0);
    errold_ = MathAlgo::max(errRat, erroldmin);
    success_ = true;
  } else {
    scale = MathAlgo::max(safe*pow(errRat, -alpha), minscale);
    hnext = h_ * scale;
    success_ = false;
  }

  return hnext;
}

void ModelSimulator::setProdRate(int label, double rate) {
  int ind = labels2Ind_.value(label, nProducts_);

  if (ind < nConstRateProducts_)
    constRates_[ind] = rate;

  if (outputLabels_.contains(label))
    constRates_[ind] = rate;
}

void ModelSimulator::blockProductProduction(int label) {
  int ind = labels2Ind_.value(label, nProducts_);

  if (ind < nProducts_)
    productions_[ind] = 0;
}

void ModelSimulator::reset() {
  resetProductProductionBlocks();
  resetDegradationFactors();
}

void ModelSimulator::resetProductProductionBlocks() {
  for (int k = 0; k < nProducts_; ++k)
    productions_[k] = 1;
}

void ModelSimulator::applyDegradationFactor(int label, double factor) {
  int ind = labels2Ind_.value(label, nProducts_);

  if (ind < nProducts_)
    degradationFactors_[ind] += factor;
}

void ModelSimulator::resetDegradationFactors() {
  for (int k = 0; k < nProducts_; ++k)
    degradationFactors_[k] = 0;
}
}
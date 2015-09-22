/**
 * Non-metric Space Library
 *
 * Authors: Bilegsaikhan Naidan (https://github.com/bileg), Leonid Boytsov (http://boytsov.info).
 * With contributions from Lawrence Cayton (http://lcayton.com/) and others.
 *
 * For the complete list of contributors and further details see:
 * https://github.com/searchivarius/NonMetricSpaceLib 
 * 
 * Copyright (c) 2014
 *
 * This code is released under the
 * Apache License Version 2.0 http://www.apache.org/licenses/.
 *
 */

#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <sstream>
#include <memory>
#include <iomanip>
#include <limits>
#include <cctype>

#include "object.h"
#include "utils.h"
#include "logging.h"
#include "distcomp.h"
#include "experimentconf.h"
#include "space/space_word_embed.h"

namespace similarity {

using namespace std;

/** Standard functions to read/write/create objects */ 

template <typename dist_t>
string WordEmbedSpace<dist_t>::CreateStrFromObj(const Object* pObj, const string& externId) const {
  if (HasWhiteSpace(externId)) {
    throw runtime_error("The id '" + externId + "' has the white space (but it shouldn't)");
  }

  string res = VectorSpace<dist_t>::CreateStrFromObj(pObj, "" /* ignored here */);
  if (!externId.empty()) res = externId + " " + res;
  return res;
}

template <typename dist_t>
bool WordEmbedSpace<dist_t>::ReadNextObjStr(DataFileInputState &inpState, string& strObj, LabelType& label, string& externId) const {
  externId.clear();
  if (!inpState.inp_file_) return false;
  if (!getline(inpState.inp_file_, strObj)) return false;
  inpState.line_num_++;
  ssize_t pos = -1;
  for (size_t i = 0; i < strObj.size(); ++i) 
  if (isspace(strObj[i])) { pos = i ; break; }

  if (-1 == pos) {
    PREPARE_RUNTIME_ERR(err) << "No white space in line #" << inpState.line_num_ << " line: 'strObj'";
    THROW_RUNTIME_ERR(err);
  }
  externId = strObj.substr(0, pos);
  strObj = strObj.substr(pos + 1);

  return true;
}

/** End of standard functions to read/write/create objects */ 

template <typename dist_t>
string WordEmbedSpace<dist_t>::ToString() const {
  string distType;
  if (distType_ == kEmbedDistL2) {
    distType=SPACE_WORD_EMBED_DIST_L2;
  } else if (distType_ == kEmbedDistCosine) {
    distType = SPACE_WORD_EMBED_DIST_COSINE;
  } else {
    PREPARE_RUNTIME_ERR(err) << "Bug: Invalid distance type code: " << distType_;
    THROW_RUNTIME_ERR(err);
  }
  return "word embeddings, distance type: " + distType;
}

template <typename dist_t>
dist_t WordEmbedSpace<dist_t>::HiddenDistance(const Object* obj1, const Object* obj2) const {
  CHECK(obj1->datalength() > 0);
  CHECK(obj1->datalength() == obj2->datalength());
  const dist_t* x = reinterpret_cast<const dist_t*>(obj1->data());
  const dist_t* y = reinterpret_cast<const dist_t*>(obj2->data());
  const size_t length = obj1->datalength() / sizeof(dist_t);

  if (distType_ == kEmbedDistL2) 
    return L2NormSIMD(x, y, length);
  if (distType_ == kEmbedDistCosine) 
    return CosineSimilarity(x, y, length);
  PREPARE_RUNTIME_ERR(err) << "Bug: Invalid distance type code: " << distType_;
  THROW_RUNTIME_ERR(err);
}

template <typename dist_t>
Space<dist_t>* WordEmbedSpace<dist_t>::HiddenClone() const {
  return new WordEmbedSpace<dist_t>(distType_);
}

/* 
 * Note that we don't instantiate vector spaces for types other than float & double
 */
template class WordEmbedSpace<float>;
template class WordEmbedSpace<double>;

}  // namespace similarity

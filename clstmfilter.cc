#include "pstring.h"
#include "clstm.h"
#include "clstmhl.h"
#include <assert.h>
#include <iostream>
#include <vector>
#include <memory>
#include <math.h>
#include <Eigen/Dense>
#include <sstream>
#include <fstream>
#include <iostream>
#include <set>

#include "multidim.h"
#include "pymulti.h"
#include "extras.h"

using namespace Eigen;
using namespace ocropus;
using namespace pymulti;
using std::vector;
using std::map;
using std::make_pair;
using std::shared_ptr;
using std::unique_ptr;
using std::cout;
using std::ifstream;
using std::set;
using std::to_string;
using std_string = std::string;
using std_wstring = std::wstring;
#define string std_string
#define wstring std_wstring

struct Sample {
  wstring in, out;
};

void read_samples(vector<Sample> &samples, const string &fname) {
  ifstream stream(fname);
  string line;
  wstring in, out;
  samples.clear();
  while (getline(stream, line)) {
    // skip blank lines and lines starting with a comment
    if (line.substr(0, 1) == "#") continue;
    if (line.size() == 0) continue;
    int where = line.find("\t");
    if (where < 0) THROW("no tab found in input line");
    in = utf8_to_utf32(line.substr(0, where));
    out = utf8_to_utf32(line.substr(where + 1));
    if (in.size() == 0) continue;
    if (out.size() == 0) continue;
    samples.push_back(Sample{in, out});
  }
}

int main_with_gt(int argc, char **argv) {
  string load_name = getsenv("load", "");
  if (load_name == "") THROW("must give load= parameter");
  CLSTMText clstm;
  clstm.load(load_name);

  vector<Sample> test_samples;
  read_samples(test_samples, argv[1]);

  int ch_count = 0;
  int ch_errors = 0;

  double exact_matches = 0.0;
  for (int test = 0; test < test_samples.size(); test++) {
    wstring gt = test_samples[test].out;
    wstring pred = clstm.predict(test_samples[test].in);
    ch_count += gt.size();
    ch_errors += levenshtein(pred, gt);
    if (pred == gt) {
      exact_matches++;
    }
    cout << gt << "\t" << pred << endl;
  }
  double exact_test_error = 1.0 - exact_matches * 1.0 / test_samples.size();
  double ch_test_error = ch_errors * 1.0 / ch_count;
  print("ERROR", ch_test_error, " ", ch_errors, ch_count,
        "exact_test_error", exact_test_error);
  return 0;
}


int main_without_gt(int argc, char **argv) {
  if (argc != 2) THROW("give text file as an argument");
  const char *fname = argv[1];

  string load_name = getsenv("load", "");
  if (load_name == "") THROW("must give load= parameter");
  CLSTMText clstm;
  clstm.load(load_name);

  string line;
  ifstream stream(fname);
  int output = getienv("output", 0);
  while (getline(stream, line)) {
    string orig = line + "";
    int where = line.find("\t");
    if (where >= 0) line = line.substr(0, where);
    string out = clstm.predict_utf8(line);
    if (output == 0)
      cout << out << endl;
    else if (output == 1)
      cout << line << "\t" << out << endl;
    else if (output == 2)
      cout << orig << "\t" << out << endl;
  }
  return 0;
}


int main(int argc, char **argv) {
  int with_gt = getienv("with_gt", 0);

#ifdef NOEXCEPTION
  if (with_gt) {
    return main_with_gt(argc, argv);
  } else {
    return main_without_gt(argc, argv);
  }
#else
  try {
  if (with_gt) {
      return main_with_gt(argc, argv);
    } else {
      return main_without_gt(argc, argv);
    }
  } catch (const char *message) {
    cerr << "FATAL: " << message << endl;
  }
#endif
}

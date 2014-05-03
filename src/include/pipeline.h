#ifndef _PIPELINE_H_
#define _PIPELINE_H_
#include <string>
#include <fstream>
using namespace std;
class Pipeline
{
 private:
  string name;
  int width;
 public:
  Pipeline(){};
 Pipeline(string n,int w):name(n),width(w){};
  void read(ifstream &);
  void output(ofstream &);
};
#endif

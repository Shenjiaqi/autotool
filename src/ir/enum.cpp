#include "enum.h"
#include "def.h"
string Enum::ent_name(int i)
{
  return list[i].first;
}
string Enum::ent_code(int i)
{
  return list[i].second;
}
string Enum::enum_name()
{
  return name;
}
int Enum::width()
{
  int w=0;
  for(int i=1;i<list.size();++w,i<<=1)
    ;
  return w;
}
int Enum::size()
{
  return list.size();
}
void Enum::add(string name,string code)
{
  list.push_back(make_pair(name,code));
}
void Enum::output(ofstream & fout)
{
  fout<<name<<' '<<list.size()<<endl;
  FR(i,list)
    fout<<i->first<<' '<<i->second<<' ';
  // for(int i=0;i<list.size();++i)
  //   fout<<list[i].first<<' '<<list[i].second<<' ';
  fout<<endl;
}
void Enum::read(ifstream & fin)
{
  int num;
  fin>>name>>num;
  for(string t1,t2;num--;)
    {
      fin>>t1>>t2;
      list.push_back(make_pair(t1,t2));
    }
}
void Enum::set_name(string n)
{
  name=n;
}

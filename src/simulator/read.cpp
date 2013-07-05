#include<iostream>
#include <fstream>
#include<cstdio>
#include<vector>
#include<string>
#include "hash.h"
#include <utility>
#include <assert.h>
#include "ir/ir.cpp"
#include "ir/wire.h"
#include "ir/instruction.h"
using namespace std;
//��ʾAsem��ivec��Ч
#define type_is_vector 0

//��ʾAsem��name��Ч
//��ʾû�����ŵ�string�������ǹؼ��ʣ���������
#define type_is_string1 1
//��ʾ�����ŵ�string
#define type_is_string2 2
string reserved[] ={
  "rule",
  "stage","false","true","enum","const","width",
  "size","type","index","read","write","wire","pipe_line",
  "register","like","override","alias","function_unit",
  "instruction","top","select","code","do","binary","switch"};

// ��ϣ�� hash controller
class hash_control hc;

class Asem{
public:
  vector<Asem> ivec;
  string name;
  int type;
  Asem * find(const string &name);
};

// ���ҵ�ǰvector�������ǵ�name��һ��
Asem * Asem::find(const string &name)
{
  if(this->type==type_is_vector)
    for(int i=0;i<this->ivec.size();++i)
      if(this->ivec[i].type==type_is_vector &&
	 this->ivec[i].ivec[0].type==type_is_string1 &&
	 name==this->ivec[i].ivec[0].name)
	return &(this->ivec[i]);
  cout<<name<<endl;
  assert(0);
  return NULL;
}

void gen(Asem &asem){
  char c1;
  while((c1=getchar())!=EOF){
    if(c1==')') return;
    //һ��vector��ʼ
    else if(c1=='('){
      Asem asem_1;
      asem_1.type=type_is_vector;
      asem.ivec.push_back(asem_1);
      gen(asem.ivec[asem.ivec.size()-1]);
    }
    //�������ŵ�string
    else if(c1=='\''){
      Asem asem_2;
      asem_2.type=type_is_string2;
      char c2;
			
      while((c2=getchar())!=EOF&&c2!='\'')
	asem_2.name+=c2;
      asem.ivec.push_back(asem_2);
    }
    //��ͨstring
    else if(c1!=' '&&c1!='\n'){
      Asem asem_3;
      asem_3.type=type_is_string1;
      asem_3.name=c1;
      char c3;

      while((c3=getchar())!=EOF&&c3!=' '&&c3!='\n'&&c3!=')') 
	asem_3.name+=c3;
      asem.ivec.push_back(asem_3);
      if(c3==')')
	return ;
    }
		
  }
}

void display(const Asem &asem,int lev){
  for(int i=0;i<asem.ivec.size();++i){
    if(asem.ivec[i].type==type_is_vector){
      if(i)
	{
	  cout<<endl;
	  for(int j=0;j<lev;++j)
	    cout<<"  ";
	}
      cout<<'(';
      display(asem.ivec[i],lev+1);
      cout<<')';
    }
    else
      {
	if(i && asem.ivec[i-1].type==type_is_vector)
	  {
	    cout<<endl;
	    for(int j=0;j<lev;++j)
	      cout<<"  ";
	  }
	if(asem.ivec[i].type==type_is_string1)
	  cout<<asem.ivec[i].name<<' ';
	else cout<<'\''<<asem.ivec[i].name<<'\''<<' ';
      }
  }
}

// �ݹ�Ĳ���hash��
void dfs_insert_hash(const Asem &asem,string pwd)
{
  // ����Ƕ���ı��������Ǳ�����
  pwd+=".";
  if(asem.ivec[0].type==type_is_string1)
    {
      if(asem.ivec[0].name=="=" || 
	 asem.ivec[0].name=="+" ||
	 asem.ivec[0].name=="if" ||
	 asem.ivec[0].name=="[]")
	return ;
      pwd+=asem.ivec[0].name;
      // ��Ӧ�����ظ�����
      assert(NULL==
	     hc.insert(pwd.c_str(),(int)(&asem)));
      assert(sizeof(int)==sizeof(&asem));
      for(int i=1;i<asem.ivec.size();++i)
	if(asem.ivec[i].type==type_is_vector)
	  dfs_insert_hash(asem.ivec[i],pwd);
    }
  else if(asem.ivec[0].type==type_is_vector)
    {
      for(int i=0;i<asem.ivec.size();++i)
	if(asem.ivec[i].type==type_is_vector)
	  dfs_insert_hash(asem.ivec[i],pwd);
    }
}
// ���ڲ����Ѿ�չ��������
hash_control hc_unfold;
// �����ݽṹ ���ڱ���ģ���� �Զ�����ʱչ��������
class triple
{
public:
  string code;
  string binary;
  vector<Asem *> doo;
  vector<int> off_in_code;
  vector<int> off_in_binary;
  triple(string c,string b){code=c,binary=b;};
  triple(){};
};
vector<vector<triple> > unfolded_list;

string get_full_name(string name,string pwd)
{
  // ��̿��ܵ�ȫ·��
  int minl=name.length()+3;
  name=pwd+"."+name;
  //  ��...a.b..c.dʱ������...a.b..c.d
  // ���û��,��ɾ��c.,��...a..b..d
  // û��,������...a..b.d > ...a..d > ...a.d > ...d
  for(;hc.find(name)==0 && name.length()>=minl;)
    {
      int l=name.length()-1;
      for(;l>=0 && name[l]!='.';--l)
	;
      int t=l;
      for(--l;l>=0 && name[l]!='.';--l)
	;
      name.erase(l+1,t-l);
    }
  // �����ؿ��ַ���
  if(name.length()<minl)
    return "";
  return name;
}

// string get_val(Asem & asem,string pwd)
// {
//   assert(asem.size()==2);
//   string get_full_name(asem.ivec[1].name,pwd);
// }
bool is_instr(string name)
{
  return (hc.find((string)"...instruction."+name)!=0);
}
bool is_type(string name)
{
  return (hc.find((string)"...type."+name)!=0);
}
bool is_enum(string name)
{
  return (hc.find((string)"...enum."+name)!=0);
}
// ����name��Ӧ����һ��asem��ַ�����ڵ�ʵ�������ȼ�����instruction��
// Ȼ����type�����enum������Щ�������β���
Asem *get_asem(string name)
{
  int tmp=hc.find((string)"...instruction."+name);
  if(tmp==0)
    {
      tmp=hc.find((string)"...type."+name);
      if(tmp==0)
	tmp=hc.find((string)"...enum."+name);
    }
  return (Asem*)tmp;
}
int unfold(Asem &asem);

// ÿ�������Ѿ�����ȡֵ�����code��binary��do�����浽r��
void eval_unfold(vector<string> &var_name,
		 vector<vector<string> >&var_choose_name,
		 vector<vector<int> >&var_val,
		 vector<pair<int,int> > &var_choosed_val,
		 Asem &code,
		 Asem &binary,
		 Asem * doo,
		 vector<triple> &r)
{
  // ��¼������code�г��ֵ�λ�ã�Ŀǰ��Ϊһ��������code��ֻ����һ��
  vector<int> var_off_in_code;
  // ��¼������binary�г��ֵ�λ�ã�Ŀǰ��Ϊһ��������binary��ֻ����һ��
  vector<int> var_off_in_bin;

  var_off_in_code.resize(var_name.size());
  var_off_in_bin.resize(var_name.size());

  // r������һ������
  int k=r.size();
  r.resize(k+1);

  // ��code��binary����ֵӦ�ÿ���д�ɺ���
  // ���code
  assert(code.ivec[0].name==(string)"code");
  for(int i=1;i<code.ivec.size();++i)
    {
      if(code.ivec[i].type==type_is_string2)
	{// �Ǵ����ŵ�string
	  r[k].code+=code.ivec[i].name;
	}
      else if(code.ivec[i].type==type_is_string1)
	{
	  // string1������Ŀǰֻ������ʱ����������
	  int j=0;
	  // �ҵ���Ӧ�ı�����var_list��������Ŀǰ��˳�����
	  for(;j<var_name.size() && var_name[j]!=code.ivec[i].name;++j)
	    ;
	  assert(j<var_name.size());

	  // ��¼�ҵ��ı�����code�е�ƫ��
	  var_off_in_code[j]=r[k].code.length();
	  r[k].code+=unfolded_list[var_val[j][var_choosed_val[j].first]][var_choosed_val[j].second].code;
	}
      else assert(0);
    }
  // ���binary
  for(int i=1;i<binary.ivec.size();++i)
    {
      if(binary.ivec[i].type==type_is_string2)
	{
	  r[k].binary+=binary.ivec[i].name;
	}
      else if(binary.ivec[i].type==type_is_string1)
	{
	  int j=0;
	  for(;j<var_name.size() && var_name[j]!=binary.ivec[i].name;++j)
	    ;
	  assert(j<var_name.size());

	  var_off_in_bin[j]=r[k].binary.length();
	  r[k].binary+=unfolded_list[var_val[j][var_choosed_val[j].first]][var_choosed_val[j].second].binary;
	}
      else if(binary.ivec[i].type==type_is_vector)
	{
	  // Ŀǰֻ������switch���
	  assert(binary.ivec[i].ivec[0].name==(string)"switch");
	  
	  // �ҵ�switch��ָ���ı�����var_name�е����
	  int j=0;
	  for(;j<var_name.size() && var_name[j]!=binary.ivec[i].ivec[1].name;++j)
	    ;
	  assert(j<var_name.size());

	  int z=2;
	  // ����switch��ÿһ��ѡ��ҵ���ǰƥ�����һ��
	  for(;z<binary.ivec[i].ivec.size() &&
		var_choose_name[j][var_choosed_val[j].first]!=binary.ivec[i].ivec[z].name;z+=2)
	    assert(binary.ivec[i].ivec[z].type==type_is_string1);
	  assert(z+1<binary.ivec[i].ivec.size());

	  // ������Ϊswitch�����ֵֻ�ǳ���string���Ժ�Ӧ�ô����������
	  assert(binary.ivec[i].ivec[z+1].type==type_is_string2);
	  r[k].binary+=binary.ivec[i].ivec[z+1].name;
	}
    }
  // ���������ɵ�code offset��binary offset ��Ϣ
  for(int i=0;i<var_val.size();++i)
    {
      int a=var_val[i][var_choosed_val[i].first];
      int b=var_choosed_val[i].second;
      int oc=var_off_in_code[i];
      int ob=var_off_in_bin[i];
      for(int j=0;j<unfolded_list[a][b].off_in_code.size();++j)
	{
	  r[k].off_in_code.push_back(unfolded_list[a][b].off_in_code[j]+oc);
	  r[k].off_in_binary.push_back(unfolded_list[a][b].off_in_binary[j]+ob);
	}
    }
  //  ����do����
  for(int i=0;i<var_val.size();++i)
    {
      int a=var_val[i][var_choosed_val[i].first];
      int b=var_choosed_val[i].second;
      for(int j=0;j<unfolded_list[a][b].doo.size();++j)
	r[k].doo.push_back(unfolded_list[a][b].doo[j]);
    }
  if(doo)
    r[k].doo.push_back(doo);
}
void dfs_unfold_instr(vector<string> &var_name, // ��������
		      vector<vector<string> > &var_choose_name, // ������ѡ��ֵ������
		      vector<vector<int> > &var_val,		// ������ѡֵ��Ӧunfolded_list������
		      vector<pair<int,int> > &var_choosed_val,	// ����ȡֵ�����
		      Asem &code,				// ��չ����code��Ӧasem
		      Asem &binary,				// ��չ����binary��Ӧasem
		      Asem * doo,				// ��չ��do��Ӧ��asem
		      int lev,					// ��ǰö�ٵ���lev������
		      vector<triple> &r)			// ����չ�����
{
  if(lev>=var_name.size())
    eval_unfold(var_name,var_choose_name,var_val,var_choosed_val,
		code,binary,doo,r);
  else
    for(int i=0;i<var_val[lev].size();++i)
      {
	// ��lev������ѡ���i��ֵ
	var_choosed_val[lev].first=i;
	for(int j=0;j<unfolded_list[var_val[lev][i]].size();++j)
	  {
	    // ��i��ֵ�е�j��չ����
	    var_choosed_val[lev].second=j;
	    dfs_unfold_instr(var_name,var_choose_name,var_val,var_choosed_val,
			     code,binary,doo,lev+1,r);
	  }
      }
}

// ����v�Ķ����ƣ�����ܱ�ʾw���0��ʼ��
string num2string(int v,int w)
{
  string r;
  for(int i=1;i<w;i<<=1)
    {
      if(i&v)
	r=(string)"1"+r;
      else r=(string)"0"+r;
    }
  return r;
}

// ��enum��������չ��
// ����(enum_name ('aaa' 'xxx') ...)
// ö������ÿһ������Ƕ�Ԫ�飬��ʾ���ֺͶ�Ӧ��code��binary����
// ����λ�þ���
int unfold_enum(Asem &asem)
{
  // ����һ��
  int k=unfolded_list.size();
  unfolded_list.resize(k+1);

  //�������չ�����е�enum������ܴܺ����Ըĳɲ���enumչ����
#define ENUM_NOT_UNFOLDED 1

#ifdef ENUM_NOT_UNFOLDED
  //cout<<"enum_"<<asem.ivec[0].name<<":"<<endl;
  string binary;
  for(int i=0;i<asem.ivec.size()-1;++i)
    binary+="-";
  unfolded_list[k].push_back(triple(asem.ivec[0].name,binary));
  unfolded_list[k][0].off_in_code.push_back(0);
  unfolded_list[k][0].off_in_binary.push_back(0);
#endif
  for(int i=1;i<asem.ivec.size();++i)
    {
      if(asem.ivec[i].type==type_is_vector)
	{
	  // �����ivec��ôֻ������Ԫ��,�Ҷ��������ŵ�string
	  assert(asem.ivec[i].ivec.size()==2 && 
		 asem.ivec[i].ivec[0].type==type_is_string2 && 
		 asem.ivec[i].ivec[1].type==type_is_string2);
	  
#ifndef ENUM_NOT_UNFOLDED
	  unfolded_list[k].push_back(triple(asem.ivec[i].ivec[1].name,num2string(i-1,asem.ivec.size()-1)));
#else
	  // TODO �������ö����Ӧ��lex����ʹ�����
#endif
	}
      else
	{
#ifndef ENUM_NOT_UNFOLDED
	  assert(asem.ivec[i].type==type_is_string2);
	  unfolded_list[k].push_back(triple(asem.ivec[i].name,num2string(i-1,asem.ivec.size()-1)));
#endif
	  // TODO �������ö����Ӧ��lex����ʹ�����
	}
    }
  return k;
}
// ��stringת��int
int string2num(string s)
{
  int v(0);
  // û����������
  for(int i=0;i<s.length();++i)
    v=(int)(s[i]-'0')+v*10;
  return v;
}

// ����type����
// (name (width num) (flag xxx) )
// ���ɵ�code��"name_width_xxx"
// ���ɵ�binary��"-..."(��width��"-")
int unfold_type(Asem &asem)
{
  // type������3������
  assert(asem.ivec.size()==3);

  Asem & tmpw=*asem.find("width");
  assert(tmpw.ivec.size()==2 && tmpw.ivec[1].type==type_is_string1);
  int width=string2num(tmpw.ivec[1].name);

  Asem & tmpf=*asem.find("flag");
  assert(tmpf.ivec.size()==2 && tmpf.ivec[1].type==type_is_string1);

  int k=unfolded_list.size();
  unfolded_list.resize(k+1);

  string binary;
  for(int i=0;i<width;++i)
    binary+=(string)"-";
  unfolded_list[k].push_back(triple((string)"type_"+asem.ivec[0].name+
				    (string)"_"+tmpw.ivec[1].name+
				    (string)"_"+tmpf.ivec[1].name,
				    binary));
  unfolded_list[k][0].off_in_code.push_back(0);
  unfolded_list[k][0].off_in_binary.push_back(0);
  return k;
}
// ��instructionչ��
int unfold_instr(Asem &asem)
{
  // ������
  vector<string> var_name;
  // ������ѡ��ֵ(����������instruction������type������enum)
  vector<vector<string> >var_choose_name;
  // ����ѡ��ֵͬ��Ӧ����unfolded_list�е�����
  vector<vector<int> > var_val;

  // �ҵ����б������� ������Ϊ��������ӵģ�
  // ��= a b c ...��
  // ��ʾa������b��c��...
  // �����������ܼ�,û��Ƕ�׺͵ݹ�
  // ���б��������ڵ�ǰinstruction������������һ��
  for(int i=1;i<asem.ivec.size();++i)
    if(asem.ivec[i].ivec[0].name==(string)"=")
      {// ��һ���Ǳ�������
	// ���ǵ�k������
	int k=var_name.size();
	var_name.resize(k+1);
	var_choose_name.resize(k+1);
	var_val.resize(k+1);

	// ���������a
	var_name[k]=asem.ivec[i].ivec[1].name;
	// j��2��ʼ,�ǵ�һ����ѡ��ֵ
	for(int j=2;j<asem.ivec[i].ivec.size();++j)
	  {
	    const string &name=asem.ivec[i].ivec[j].name;
	    // �����ѡ��ֵ������
	    var_choose_name[k].push_back(name);
	    // ���������Ӧ����һ��instruction ������type������ö�� �ݹ��չ����
	    if(is_instr(name) || is_type(name) || is_enum(name))
	      {
		int val=unfold(*get_asem(name));
		// ����չ��������
		var_val[k].push_back(val);
	      }
	    else
	      {// ��Ȼ����
		assert(0);
	      }
	  }
      }

  // �����չ��instruction���binary,code,do��asem��ַ
  Asem & code=*asem.find("code");
  Asem & binary=*asem.find("binary");
  Asem * doo=asem.find("do");
  assert((&code!=0) && (&binary!=0) && (doo!=NULL));

  // ��unfolded_list������һ��
  int k=unfolded_list.size();
  unfolded_list.resize(k+1);

  // ���浱ǰ����ȡ��ֵ����Ԫ���ʾ��
  // (= a b c)
  // ��һ���ʾaȡb����c
  // �ڶ����ʾȡb����c�еĵڼ�����b��c�ѱ�չ���������ж�����ѡ�
  vector<pair<int,int> > var_choosed_val;
  var_choosed_val.resize(var_name.size());
  // ��Ա���ȡ��ֵͬ��ʱ��õ�չ����code��binary��do
  dfs_unfold_instr(var_name,var_choose_name,var_val,var_choosed_val,
		   code,binary,doo,0,unfolded_list[k]);
  return k;
}


// չ��һ������������ö�٣�ָ�����
int unfold(Asem &asem)
{
  string & name=asem.ivec[0].name;

  // ����Ѿ�չ����
  if(hc_unfold.find(name)!=0)
    {
      return hc_unfold.find(name)-1; // ���ڼ���hash���ʱ��ӹ�1
    }

  //cout<<"unfolding:"<<name<<endl;

  int r;
  // �Բ�ͬ�����ͷֱ�չ��
  if(is_instr(name))
    r=unfold_instr(asem);
  else if(is_enum(name))
    r=unfold_enum(asem);
  else if(is_type(name))
    r=unfold_type(asem);
  else assert(0);
  //cout<<"done:"<<name<<endl;
  // ����hash������r������0����hash.find����0��ʾû���ҵ�
  hc_unfold.insert(name,r+1);
  return r;
}
int string2int(string & s)
{
  int r(0);
  for(int i=0;i<s.length();++i)
    r=r*10+s[i]-'0';
  return r;
}
int main(){
  freopen("in.txt","r",stdin);
  Asem asem;

  asem.type=type_is_vector;
  // �����ļ�
  gen(asem);
  // display(asem,0);

  // �ѱ����ּ���hash��
  for(int i=0;i<sizeof(reserved)/sizeof(reserved[0]);++i)
    hc.insert(reserved[i].c_str(),(int)(void*)reserved);
  
  // ����hash��
  dfs_insert_hash(asem,"");
  // instruction������ж������
  string instr="...instruction.top";
  assert(hc.find(instr)!=0);
  Asem &rule=*(Asem*)hc.find(instr);
  assert(rule.type==type_is_vector && 
	 rule.ivec.size()==2
	 && rule.ivec[1].type==type_is_string1);

  // �õ����������
  instr="...instruction."+rule.ivec[1].name;
  assert(hc.find(instr)!=0);
  rule=*(Asem*)hc.find(instr);
  //�鿴���
  int allrule=unfold(rule);
  cout<<unfolded_list[allrule].size()<<endl;
  fstream out_txt("out.txt");
  hash_control tmp;
  for(int i=0;i<unfolded_list[allrule].size();++i)
    {
      out_txt<<i<<endl<<"\tcode: "<<unfolded_list[allrule][i].code<<endl
	  <<"\tbinary: "<<unfolded_list[allrule][i].binary<<endl;
//       cout<<"do :";
//       for(int j=0;j<unfolded_list[allrule][i].doo.size();++j)
// 	{
// 	  cout<<unfolded_list[allrule][i].doo[j]<<' ';
// 	  display(*unfolded_list[allrule][i].doo[j],1);
// 	}
//       cout<<endl;
    }
  //�������������Ϊir
  //����wire����
  Ir ir;
  Asem &asem_wire=*asem.ivec[0].find((string)"wire");
  assert(asem_wire.type==type_is_vector);
  Asem &asem_type=*asem.ivec[0].find((string)"type");
  assert(asem_type.type==type_is_vector);
  for(int i=1;i<asem_wire.ivec.size();++i)
    {
      string wire_type_name=(*asem_wire.ivec[i].find((string)"width")).ivec[1].name;
      //TODO û�м��type��Ӧ����������Ӧ�����޷��ţ����ҹ��̼��������
      int wire_width=string2int((*(*(asem_type.find(wire_type_name))).find((string) "width")).ivec[1].name);
      ir.add_wire(Wire(asem_wire.ivec[i].ivec[0].name,wire_width));
    }
  ofstream out2_txt("out2.txt");
  ir.output_wire(out2_txt);

  // ��code��binary���뵽ir��
  for(vector<triple>::iterator ite=unfolded_list[allrule].begin();
      ite!=unfolded_list[allrule].end();++ite)
    {
      vector<pp> tmp;
      for(int i=0;i<ite->off_in_code.size();++i)
	tmp.push_back(pp(ite->off_in_code[i],ite->off_in_binary[i]));
      ir.add_instruction(Instruction(ite->code,
				     ite->binary,
				     tmp));
    }
  ir.output_instruction_set(out2_txt);
  return 0;
}

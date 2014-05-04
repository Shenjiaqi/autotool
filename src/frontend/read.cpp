#include<iostream>#include <fstream>#include<cstdio>#include<vector>#include<string>#include <utility>#include <assert.h>#include <ctype.h>#include "hash.h"#include "ir.h"#include "asem.h"#include "triple.h"#include "def.h"using namespace std;static string reserved[] ={  "rule",  "stage","false","true","enum","const","width",  "size","type","index","read","write","wire","pipe_line",  "register","like","override","alias","function_unit",  "instruction","top","select","code","do","binary","switch"};static int string2int(string & s){  int r(0);  for(int i=0;i<(int)s.length();++i)    r=r*10+s[i]-'0';  return r;}bool isnumber(string v){  FRA(i,v)    if(*i<'0' || *i>'9')      return false;  return true;}// hash_control hc_unfold_read;// vector<vector<triple > > unfolded_list_read;// vector<instr_type> u_l_t;// vector<string> unfolded_list_name_read;// name of rule of corresponding unfolded_list// class hash_control hc_read;// vector<Asem> do_content_read;// class hash_control & Asem::hc_unfold=hc_unfold_read;// vector<Asem> & Asem::do_content = do_content_read;// vector<vector<triple > > & Asem::unfolded_list=unfolded_list_read;// vector<instr_type> &Asem::unfolded_list_type=u_l_t;// vector<string> & Asem::unfolded_list_name=unfolded_list_name_read;// name of rule of corresponding unfolded_list// class hash_control & Asem::hc=hc_read;int main(int argc,char *argv[]){  assert(argc==3);  FILE * input=fopen(argv[1],"r");  Asem asem;  asem.type=type_is_vector;  // 读入文件  asem.gen(input);    //asem.display(0);  // 把保留字加入hash表  for(int i=0;i<(int)(sizeof(reserved)/sizeof(reserved[0]));++i)    asem.hc.insert(reserved[i].c_str(),(void*)reserved);    // 插入hash表  asem.dfs_insert_hash("");  // instruction里必须有顶层规则  //string instr="...instruction.top";  //assert(hc.find(instr)!=0);  //找到instruction那一项  Asem &instruction=*(asem.ivec[0].find((string)"instruction"));  //找到instruction中的top  Asem &top=*(instruction.find((string)"top"));  assert(&top!=NULL);  Asem &rule=*instruction.find(top.ivec[1].name);  //查看结果  ofstream yout("yout.y");//.y file name  ofstream dot_c_out("cout.cpp");// .c file name;  ofstream dot_h_out("cout.h");  int allrule=rule.unfold(yout,dot_c_out,dot_h_out);#if READ_COUT==1  cout<<rule.unfolded_list[allrule].size()<<endl;#endif  // fstream out_txt("out.txt");  hash_control tmp;  // for(int i=0;i<rule.unfolded_list[allrule].size();++i)  //   {  //     out_txt<<i<<endl<<"\tcode: "<<rule.unfolded_list[allrule][i].code<<endl  // 	     <<"\tbinary: "<<rule.unfolded_list[allrule][i].binary<<endl;  //     out_txt<<"do: "<<endl;  //     // for(int i=0;i<rule.unfolded_list[allrule][i].doo.size();++i)  //     // 	out_txt<<rule.unfolded_list[allrule][i].doo[i]<<' ';  //     out_txt<<endl;  //     for(int i=0;i<rule.unfolded_list[allrule][i].off_in_code.size();++i)  // 	out_txt<<rule.unfolded_list[allrule][i].off_in_code[i]<<' '<<rule.unfolded_list[allrule][i].off_in_binary[i]<<' ';  //     out_txt<<endl;  //   }  //接下来是输出到为ir  //处理wire描述  Ir ir;  Asem &asem_wire=*asem.ivec[0].find((string)"wire");  assert(asem_wire.type==type_is_vector);  Asem & asem_type=*asem.ivec[0].find("type");  assert(asem_type.type==type_is_vector);  for(int i=1;i<(int)asem_wire.ivec.size();++i)    {      string wire_type_name=(*asem_wire.ivec[i].find((string)"width")).ivec[1].name;      //TODO 没有检查type对应的数据类型应该是无符号，查找过程加入出错检查      int wire_width=string2int((*(*(asem_type.find(wire_type_name))).find((string) "width")).ivec[1].name);      ir.add_wire(Wire(asem_wire.ivec[i].ivec[0].name,wire_width));    }  //add type info to ir  for(int i=1;i<(int)asem_type.ivec.size();++i)    {      ir.add_type(Type(asem_type.ivec[i].ivec[0].name,		       string2int((*asem_type.ivec[i].find("width")).ivec[1].name),		       (*asem_type.ivec[i].find("flag")).ivec[1].name));    }  //add stage info to ir  Asem & asem_stage=*asem.ivec[0].find("stage");  assert(asem_stage.type==type_is_vector);  for(int i=1;i<(int)asem_stage.ivec.size();++i)    {      ir.add_stage(Stage(asem_stage.ivec[i].ivec[0].name));    }  //add enum info to ir  Asem & asem_enum=*asem.ivec[0].find("enum");  assert(asem_enum.type==type_is_vector);  for(int i=1;i<(int)asem_enum.ivec.size();++i)    {      ir.add_enum(asem_enum.ivec[i].ivec[0].name);      int cnt(0);      for(int j=1;j<(int)asem_enum.ivec[i].ivec.size();++j)	{	  string name,code;	  int val(0);	  if(asem_enum.ivec[i].ivec[j].is_string())	    name=code=asem_enum.ivec[i].ivec[j].name,val=cnt;	  else if((int)asem_enum.ivec[i].ivec[j].ivec.size()==2)	    {	      if(isnumber(asem_enum.ivec[i].ivec[j].ivec[1].name))		  name=code=asem_enum.ivec[i].ivec[j].ivec[0].name,		  val=cnt=string2int(asem_enum.ivec[i].ivec[j].ivec[1].name);	      else		name=asem_enum.ivec[i].ivec[j].ivec[0].name,		  code=asem_enum.ivec[i].ivec[j].ivec[1].name;	    }	  else	    {	      code=asem_enum.ivec[i].ivec[j].ivec[0].name;	      if(isnumber(asem_enum.ivec[i].ivec[j].ivec[1].name))		val=cnt=string2int(asem_enum.ivec[i].ivec[j].ivec[1].name),		  name=asem_enum.ivec[i].ivec[j].ivec[2].name;	      else if(isnumber(asem_enum.ivec[i].ivec[j].ivec[2].name))		val=cnt=string2int(asem_enum.ivec[i].ivec[j].ivec[2].name),		  name=asem_enum.ivec[i].ivec[j].ivec[1].name;	      else assert(0);	    }	  ++cnt;	  ir.add_enum_entry(name,code,val);	}    }  //add memory info to ir  Asem & asem_memory=*asem.ivec[0].find("memory");  assert(asem_memory.type==type_is_vector);  for(int i=1;i<(int)asem_memory.ivec.size();++i)    {      string sizes=(*asem_memory.ivec[i].find("size")).ivec[1].name;      int sizei;      if(sizes[0]<='9' && sizes[0]>='0')  	sizei=string2int(sizes);      else//TODO need check here	assert(0);  	// sizei=string2int((*(*asem.ivec[0].find(sizes)).find("value")).ivec[1].name);      // ir.add_memory(Memory(asem_memory.ivec[i].ivec[0].name,      // 			   sizei,      // 			   (*asem_memory.ivec[i].find("type")).ivec[1].name,      // 			   (*asem_memory.ivec[i].find("index")).ivec[1].name));      string widths = (*asem_memory.ivec[i].find("width")).ivec[1].name;      int width = string2int(widths);      ir.add_memory( Memory( asem_memory.ivec[i].ivec[0].name,			     sizei, width ) );    }  //add register info to ir  Asem & asem_register=*asem.ivec[0].find("register");  assert(asem_register.type==type_is_vector);  for(int i=1;i<(int)asem_register.ivec.size();++i)    {      string name=asem_register.ivec[i].ivec[0].name;      int size;      if((asem_register.ivec[i].find("size"))!=NULL)  	size=string2int((*asem_register.ivec[i].find("size")).ivec[1].name);      else size=1;      int width = string2int( (*asem_register.ivec[i].find("width")).			       ivec[1].name );      // string type=(*asem_register.ivec[i].find("type")).ivec[1].name;      // string idx;      // if(asem_register.ivec[i].find("index")!=NULL)      // 	idx=(*asem_register.ivec[i].find("index")).ivec[1].name;      // string alias;      // if((asem_register.ivec[i].find("alias"))!=NULL)      // 	alias=(*asem_register.ivec[i].find("alias")).ivec[1].name;      // string read_port;      // if((asem_register.ivec[i].find("read"))!=NULL)      // 	read_port=(*asem_register.ivec[i].find("read")).ivec[1].name;      // string write_port;      // if((asem_register.ivec[i].find("write"))!=NULL)      // 	write_port=(*asem_register.ivec[i].find("write")).ivec[1].name;      // ir.add_register(Register(name,size,type,idx,alias,read_port,write_port));      ir.add_register( Register( name, size, width) );    }  //add pipeline info to ir  Asem & asem_pipeline=*asem.ivec[0].find("pipe_line");  assert(asem_pipeline.type==type_is_vector);  for(int i=1;i<(int)asem_pipeline.ivec.size();++i)    {      string name = asem_pipeline.ivec[i].ivec[0].name;      string typeName = asem_pipeline.ivec[i].ivec[1].name;      int idx = ir.find_type(typeName);      if( idx < 0 )      	cerr << " cannot find type " << typeName;      int width = ir.get_type_width(i);      ir.add_pipeline(Pipeline( name, width) );    }  ofstream out2_txt;//("out2.txt");  out2_txt.open(argv[2],ofstream::out);  // find vliw mode definition  Asem &vliw_mode=*rule.find("vliw");  if(&vliw_mode!=NULL)    {      assert(vliw_mode.is_vec());      string mode=vliw_mode.ivec[1].get_string();      string off=vliw_mode.ivec[2].get_string();      ir.add_vliw_mode(string2int(mode),string2int(off));    }  // 把code和binary, do, argument list 描述加入到ir里  for(int i=0;i<=allrule;++i)    {      string type;      switch (rule.unfolded_list_type[i])	{	case e_notpack:	  type="e_notpack";	  break;	case e_pack:	  type="e_pack";	  break;	default:	  assert(0);	}      for(vector<triple>::iterator ite=rule.unfolded_list[i].begin();	  ite!=rule.unfolded_list[i].end();++ite)	{	  vector<pp> tmp;	  for(int i=0;i<(int)ite->off_in_code.size();++i)	    tmp.push_back(pp(ite->off_in_code[i],ite->off_in_binary[i]));	  ir.add_instruction(ite->rulename,ite->code,ite->binary// ,ite->doo			     ,tmp,ite->enum_name,			     ite->arg_list,ite->reloc_info,ite->type, ite->var_name, ite->do_list);	}    }  // feed do_content to ir  for( int i = 0; i < Asem::do_content.size(); ++i )    {      ir.add_do_content( Asem::do_content[i] );    }  // add addr info  Asem & asem_addr=*asem.ivec[0].find("address");  assert(asem_addr.type==type_is_vector);  for(int i=1;i<(int)asem_addr.ivec.size();++i)    {      Asem & tmp=asem_addr.ivec[i];      int w=string2int((*tmp.find("width")).ivec[1].name);      bool p=(*tmp.find("pcrel")).ivec[1].name=="TRUE";      string name=tmp.ivec[0].name;      int r=string2int((*tmp.find("right_shift")).ivec[1].name);      ir.add_addr(Addr(w,p,r,name));    }  // ir.output_wire(out2_txt);  // ir.output_type(out2_txt);  // ir.output_stage(out2_txt);  // ir.output_enum(out2_txt);  // ir.output_memory(out2_txt);  // ir.output_register(out2_txt);  // ir.output_pipeline(out2_txt);  // ir.output_instruction_set(out2_txt);  ir.add_top_name(top.ivec[1].name);  ir.output_all(out2_txt);  return 0;}
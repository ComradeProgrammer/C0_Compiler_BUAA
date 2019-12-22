#pragma once

#include<iostream>
#include<fstream>
#include<string>
#include<map>
#include<cstdlib>
using namespace std;

enum FaultType {
	LEXICALERROR = 97,//a.不符合词法规则
	REDEFINED,//b.重定义
	UNDEFINED,//c.未定义符号
	PARANUMERROR,//d.参数个数不匹配
	PARATYPEERROR,//e.参数类型不匹配
	ILLEGALTYPEINCONDITION,//F.条件判断中存在不合法的类型
	ILLEGALRETURNINVOID,//无返回值函数中存在不匹配的return 语句
	ILLEGALRETURNINNONVOID,//有返回值的函数缺少return语句或存在不匹配的return语句
	ILLEGALINDEX,//数组元素的下标只能是整型表达式
	MODIFYCONST,//不能改变常量的值
	NOSEMICN,//k.缺少分号
	NORPARENT,//l.缺少右括号
	NORBRACK,//m缺少右中括号
	NOWHILE,//n.do-while语句缺少while
	ILLEGALCONSTDEFINITION//常量定义中=后面只能是整型或字符型常量
};

class FaultHandler {
public:
	FaultHandler(string filename);
	~FaultHandler();
	void handleCourseFault(int line, FaultType type);
	void handleFault(int line, string information);
	void terminate();
	void debugOn();
	void test();
	bool haveBug();
	
private:
	fstream fout;
	bool debug=false;
	bool hasBug = false;
	map<FaultType, string>messages;
};
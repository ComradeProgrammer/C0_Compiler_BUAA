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
	TYPEERROR,//e.类型不匹配
	NOSEMICN,//f.缺少分号
	NORPARENT,//g.缺少右括号
	NORBRACK//h。缺少右中括号
};

class FaultHandler {
public:
	FaultHandler(string filename);
	~FaultHandler();
	void handleCourseFault(int line, FaultType type);
	void handleFault(int line, string information);
	void terminate();
	void debugOn();
	void debugOff();
	void test();
	
private:
	ofstream fout;
	bool debug;
	map<FaultType, string>messages;
};
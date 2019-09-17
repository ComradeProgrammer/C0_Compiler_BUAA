#pragma once

#include<iostream>
#include<fstream>
#include<string>
#include<map>
#include<cstdlib>
using namespace std;

enum FaultType {
	LEXICALERROR = 97,//a.�����ϴʷ�����
	REDEFINED,//b.�ض���
	UNDEFINED,//c.δ�������
	PARANUMERROR,//d.����������ƥ��
	TYPEERROR,//e.���Ͳ�ƥ��
	NOSEMICN,//f.ȱ�ٷֺ�
	NORPARENT,//g.ȱ��������
	NORBRACK//h��ȱ����������
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
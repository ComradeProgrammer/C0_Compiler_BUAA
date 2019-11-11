#pragma once
#include<map>
#include<string>
#include<vector>
#include<iostream>
#include"SubSymbolTable.h"

using namespace std;

class SymbolTable {
public:
	int mainSymbolId;
	
	SymbolTable();
	~SymbolTable();
	SymbolEntry* addSymbol(string currentScope,string name,bool isFunction);
	SymbolEntry* getSymbolByName(string currentScope,string name);
	SymbolEntry* getSymbolById(int id);
	SymbolEntry* addTmpSymbol(string function, int id);
	SubSymbolTable* getSubSymbolTableByName(string s);
	int addString(string str);
	string getString(int id);

	map<int, vector<int>> summary();

	void debugOn();
	void selfTest();
	void dumpMipsCodeHeader(ostream& f);
	friend ostream& operator<<(ostream& out, SymbolTable& t);
private:
	int count;
	bool debug=false;
	map<string, SubSymbolTable*>scope;
	SubSymbolTable globalScope;
	vector<SymbolEntry*>symbolId;
	map<int,SymbolEntry*>tmpSymbolId;
	int stringCount;
	vector<string> unnamedStrings;

};

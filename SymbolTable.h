#pragma once
#include<map>
#include<string>
#include<vector>
#include<iostream>
#include"SubSymbolTable.h"

using namespace std;

class SymbolTable {
public:
	SymbolTable();
	~SymbolTable();
	SymbolEntry* addSymbol(string currentScope,string name,bool isFunction);
	SymbolEntry* getSymbolByName(string currentScope,string name);
	SymbolEntry* getSymbolById(int id);
	void debugOn();
	void selfTest();
private:
	int count;
	bool debug=false;
	map<string, SubSymbolTable*>scope;
	SubSymbolTable globalScope;
	vector<SymbolEntry*>symbolId;
};

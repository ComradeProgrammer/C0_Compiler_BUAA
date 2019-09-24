#pragma once
#include<vector>
#include<map>
#include<string>
using namespace std;
enum SymbolType {
	TYPEINT,
	TYPECHAR,
	TYPEINTARRAY,
	TYPECHARARRAY,
	TYPEINTCONST,
	TYPECHARCONST,
	TYPEFUNCTION
};
enum ReturnType {
	RETINT,
	RETCHAR,
	RETVOID
};

struct FunctionLink {
	ReturnType returnType;
	int paraNum;
	vector<SymbolType>paras;
	int tmpVarSize;
};

struct SymbolEntry {
	string name;
	int id;
	SymbolType type;
	string scope;
	int initValue;
	unsigned int addr;
	int dimension;
	FunctionLink* link;
};

class SubSymbolTable {
public:
	SubSymbolTable(string _nameScope);
	~SubSymbolTable();
	SymbolEntry* addSymbol(string name,bool isFuntion);
	SymbolEntry* getSymbolByName(string name);

private:
	string nameScope;
	int count;
	map<string, SymbolEntry*>symbolMap;
};
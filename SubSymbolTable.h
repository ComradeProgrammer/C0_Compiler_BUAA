#pragma once
#include<vector>
#include<map>
#include<string>
using namespace std;
/*用来标记符号类型*/
enum SymbolType {
	TYPEINT,
	TYPECHAR,
	TYPEINTARRAY,
	TYPECHARARRAY,
	TYPEINTCONST,
	TYPECHARCONST,
	TYPEFUNCTION
};
/*用来标记若符号为函数时返回类型*/
enum ReturnType {
	RETINT,
	RETCHAR,
	RETVOID
};

/*在变量为函数时的附加域*/
struct FunctionLink {
	ReturnType returnType;
	int paraNum;
	vector<SymbolType>paras;
	vector<int>paraIds;
	int tmpVarSize;
	friend ostream& operator<<(ostream& stream,FunctionLink f);
};

/*符号表项*/
struct SymbolEntry {
	string name;//会自动设置
	int id;//会自动设置
	SymbolType type;
	string scope;//会自动设置
	int initValue;
	unsigned int addr;
	int dimension=0;
	FunctionLink* link=NULL;//会自动设置
	friend ostream& operator<<(ostream& stream,SymbolEntry s);
};

/*子符号表，是为每个作用域所建立的符号表，所有的符号表项依靠动态分配*/
class SubSymbolTable {
public:
	SubSymbolTable(string _nameScope);
	~SubSymbolTable();
	SymbolEntry* addSymbol(string name,bool isFuntion);
	SymbolEntry* getSymbolByName(string name);

	void selfTest();
private:
	string nameScope;
	map<string, SymbolEntry*>symbolMap;
};
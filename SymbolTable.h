#include<map>
#include<string>
#include<vector>
#include"SubSymbolTable.h"
using namespace std;

class SymbolTable {
public:
	SymbolTable();
	~SymbolTable();
	SymbolEntry* addSymbol(string currentScope,string name,bool isFunction);
	SymbolEntry* getSymbolByName(string currentScope,string name);
	SymbolEntry* getSymbolById(int id);
private:
	int count;
	map<string, SubSymbolTable*>scope;
	SubSymbolTable globalScope;
	vector<SymbolEntry*>symbolId;
};
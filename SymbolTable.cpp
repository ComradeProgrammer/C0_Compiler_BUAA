#include"SymbolTable.h"
SymbolTable::SymbolTable():globalScope("") {
	count = 0;
}

SymbolTable::~SymbolTable() {
	map<string, SubSymbolTable*>::iterator itr;
	for (itr = scope.begin(); itr != scope.end(); itr++) {
		delete itr->second;
	}
}

SymbolEntry* SymbolTable::addSymbol(string currentScope, string name,bool isFunction) {
	SymbolEntry* res;
	if (currentScope != "") {
		SubSymbolTable* s = scope[currentScope];
		res = s->addSymbol(name, isFunction);
	}
	else {
		res = globalScope.addSymbol(name, isFunction);
		if(res!=NULL&&isFunction){
			SubSymbolTable* newscope = new SubSymbolTable(name);
			scope[name] = newscope;
		}
	}
	if (res != NULL) {
		res->id = count++;
		symbolId.push_back(res);
	}
	return res;
}

SymbolEntry* SymbolTable::getSymbolByName(string currentScope, string name) {
	SymbolEntry* res;
	if (currentScope != "") {
		SubSymbolTable* s = scope[currentScope];
		res = s->getSymbolByName(name);
		if (res != NULL) {
			return res;
		}
	}
	res = globalScope.getSymbolByName(name);
	return res;
}

SymbolEntry* SymbolTable:: getSymbolById(int id) {
	if (id < 0 || id >= symbolId.size()) {
		return NULL;
	}
	return symbolId[id];
}

void SymbolTable::selfTest() {
	SymbolEntry* tmp1=addSymbol("", "var1", false);
	SymbolEntry* tmp2 = addSymbol("", "func1", true);
	SymbolEntry* tmp3 = addSymbol("", "func2", true);

	SymbolEntry* tmp4 = addSymbol("", "var1", false);
	SymbolEntry* tmp5 = addSymbol("", "func1", true);

	SymbolEntry* tmp6 = addSymbol("func1","var2", false);
	SymbolEntry* tmp7 = addSymbol("func1", "var1", false);


	SymbolEntry* tmp8 = getSymbolByName("func1", "var1");
	SymbolEntry* tmp9 = getSymbolByName("", "var1");

	SymbolEntry* tmp10 = getSymbolByName("", "func1");



}
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
		SubSymbolTable* s = scope[name];
		res = s->addSymbol(name, isFunction);
	}
	else {
		res = globalScope.addSymbol(name, isFunction);
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
		SubSymbolTable* s = scope[name];
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
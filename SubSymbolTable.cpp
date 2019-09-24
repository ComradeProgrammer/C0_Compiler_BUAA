#include"SubSymbolTable.h"
SubSymbolTable::SubSymbolTable(string _nameScope){
	nameScope = _nameScope;
}
SubSymbolTable::~SubSymbolTable() {
	map<string, SymbolEntry*>::iterator itr ;
	for (itr = symbolMap.begin(); itr != symbolMap.end(); itr++) {
		delete itr->second;
	}
}
SymbolEntry* SubSymbolTable::addSymbol(string name,bool isFunction) {
	map<string, SymbolEntry*>::iterator itr = symbolMap.find(name);
	if (itr != symbolMap.end()) {
		return NULL;
	}

	int id = count++;
	SymbolEntry* entry = new SymbolEntry;
	if (isFunction) {
		entry->link = new FunctionLink;
	}
	entry->id = id;
	entry->scope = nameScope;
	symbolMap[name] = entry;
	return entry;
}

SymbolEntry* SubSymbolTable::getSymbolByName(string name) {
	map<string, SymbolEntry*>::iterator itr = symbolMap.find(name);
	if (itr == symbolMap.end()) {
		return NULL;
	}
	else {
		return itr->second;
	}
}

#include"SubSymbolTable.h"
#include<iostream>
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

	SymbolEntry* entry = new SymbolEntry;
	if (isFunction) {
		entry->link = new FunctionLink;
	}
	entry->name = name;
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

void SubSymbolTable::selfTest() {
	SymbolEntry* tmp1=addSymbol("para1", false);
	if (tmp1->name != "para1"||tmp1->link!=NULL) {
		cout << "error1";
	}
	SymbolEntry* tmp2=addSymbol("para2", false);
	if (tmp2->name != "para2"||tmp2->link != NULL) {
		cout << "error2";
	}
	SymbolEntry* tmp3 = addSymbol("para1", false);
	if (tmp3!=NULL ){
		cout << "error3";
	}
	SymbolEntry* tmp4=addSymbol("func1", true);
	if (tmp4->name != "func1" || tmp4->link == NULL) {
		cout << "error4";
	}
	SymbolEntry* tmp5 = getSymbolByName("para1");
	if (tmp5 != tmp1) {
		cout << "error5";
	}
	SymbolEntry* tmp6 = getSymbolByName("para2");
	if (tmp6 != tmp2) {
		cout << "error6";
	}
	SymbolEntry* tmp7 = getSymbolByName("func1");
	if (tmp7 != tmp4) {
		cout << "error7";
	}
}
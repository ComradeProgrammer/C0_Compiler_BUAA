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
	if (debug) {
		cout << "debug@SymbolTable:calling addSymbol" << endl;
		cout << "currentscope:" + currentScope << ";name:" 
			<< name << ";isFunction:" << isFunction << endl;
	}
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
			if (debug) {
				cout << "debug@SymbolTable:new scope added" << endl;
			}
		}
	}
	if (res != NULL) {
		res->id = count++;
		symbolId.push_back(res);
	}
	if (debug&&res==NULL) {
		cout << "debug@SymbolTable:duplicate name found" << endl;
	}
	if (debug) { cout << "==============================" << endl; }
	
	return res;
}

SymbolEntry* SymbolTable::getSymbolByName(string currentScope, string name) {
	if (debug) {
		cout << "debug@SymbolTable:calling getSymbolByName" << endl;
		cout << "currentscope:" + currentScope << ";name:"
			<< name<< endl;
	}
	SymbolEntry* res;
	if (currentScope != "") {
		SubSymbolTable* s = scope[currentScope];
		res = s->getSymbolByName(name);
		if (res != NULL) {
			if (debug) { cout << "variable Type:LOCAL" << endl; }
			return res;
		}
	}
	res = globalScope.getSymbolByName(name);
	if (debug) {
		if (res != NULL) {
			cout << "variable Type:GLOBAL" << endl;
			cout << (*res);
		}
		else {
			cout << "variable doesm't exist" << endl;
		}
		cout << "=============================" << endl;
	}
	return res;
}

SymbolEntry* SymbolTable:: getSymbolById(int id) {
	if (id < 0 || id >= symbolId.size()) {
		return NULL;
	}
	return symbolId[id];
}

void SymbolTable::debugOn() {
	debug = true;
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
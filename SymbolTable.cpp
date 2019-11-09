#include"SymbolTable.h"
SymbolTable::SymbolTable():globalScope("") {
	count = 0;
	stringCount = 0;
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
		if (s == NULL) { return NULL; }
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
	if (name == "main") {
		mainSymbolId = res->id;
	}
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
		if (s == NULL) { return NULL; }
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
	if ( id >= symbolId.size()) {
		return NULL;
	}
	else if (id < 0) {
		return tmpSymbolId[id];
	}
	return symbolId[id];
}

string SymbolTable::getString(int id) {
	return unnamedStrings[id];
}

int SymbolTable::addString(string str) {
	unnamedStrings.push_back(str);
	
	return stringCount++;
}

void SymbolTable::debugOn() {
	debug = true;
}

SubSymbolTable* SymbolTable::getSubSymbolTableByName(string s) {
	if (s == "") {
		return &globalScope;
	}
	return scope[s];
}

SymbolEntry* SymbolTable::addTmpSymbol(string function, int id) {
	SubSymbolTable* subtable = scope[function];
	SymbolEntry*res= subtable->addTmpSymbol(id);
	tmpSymbolId[id] = res;
	return res;
}

ostream& operator<<(ostream& out, SymbolTable& t) {
	out << "global:" << endl;
	out << t.globalScope;
	for (auto& i : t.scope) {
		out << i.first << ":" << endl;
		out << *(i.second);
	}
	return out;
}

map<int, vector<int>> SymbolTable::summary() {
	map<int, vector<int>>res;
	globalScope.summaryAndReport();
	for (auto& i : scope) {
		int id = globalScope.getSymbolByName(i.first)->id;
		vector<int>tmp=i.second->summaryAndReport();
		res[id] = tmp;
	}
	return res;
}

void SymbolTable::selfTest() {
	/*SymbolEntry* tmp1=addSymbol("", "var1", false);
	SymbolEntry* tmp2 = addSymbol("", "func1", true);
	SymbolEntry* tmp3 = addSymbol("", "func2", true);

	SymbolEntry* tmp4 = addSymbol("", "var1", false);
	SymbolEntry* tmp5 = addSymbol("", "func1", true);

	SymbolEntry* tmp6 = addSymbol("func1","var2", false);
	SymbolEntry* tmp7 = addSymbol("func1", "var1", false);


	SymbolEntry* tmp8 = getSymbolByName("func1", "var1");
	SymbolEntry* tmp9 = getSymbolByName("", "var1");

	SymbolEntry* tmp10 = getSymbolByName("", "func1");*/
}
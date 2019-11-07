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
	/*SymbolEntry* tmp1=addSymbol("para1", false);
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
	}*/
}

ostream& operator<<(ostream& o,FunctionLink f) {
	o << "\tsymbol entry link content" << endl;
	string s = f.returnType == RETINT ? "int" :
		f.returnType == RETCHAR ? "char" :
		f.returnType == RETVOID ? "void" :
		"INVALID";
	o << "\t\treturnType:" << s<<endl;
	o << "\t\tparaNum:" << f.paraNum << endl;
	o << "\t\tparas:[";
	for (int i = 0; i < f.paras.size(); i++) {
		string tmp = f.paras[i] == TYPEINT ? "int" :
			f.paras[i] == TYPECHAR ? "char" :
			"INVALID";
		o << tmp << " ";
	}
	o << "]" << endl;
	return o;
}

void SubSymbolTable::dumpMidCode(ostream& out) {
	/*导出变量定义处的中间代码*/
	map<string, SymbolEntry*>::iterator itr = symbolMap.begin();
	for (itr = symbolMap.begin(); itr != symbolMap.end(); itr++) {
		if (itr->second->isParameter == true) {
			continue;
		}
		if (itr->second->type == TYPEINTCONST) {
			out << "const int " << itr->first << endl;
		}
		else if (itr->second->type == TYPECHARCONST) {
			out << "const char " << itr->first << endl;
		}
		else if (itr->second->type == TYPEINT) {
			out << "int " << itr->first << endl;
		}
		else if (itr->second->type == TYPECHAR) {
			out << "int " << itr->first << endl;
		}
		else if (itr->second->type == TYPEINTARRAY) {
			out << "int " << itr->first
				<< "[" << itr->second->dimension << "]" << endl;
		}
		else if (itr->second->type == TYPECHARARRAY) {
			out << "int " << itr->first
				<< "[" << itr->second->dimension << "]" << endl;
		}
	}
}


ostream& operator<<(ostream& o,SymbolEntry s) {
	o << "symbol entry content" << endl;
	o << "\tname:" << s.name;
	o << "\tid:" << s.id;
	string tmp;
	switch(s.type){
	case TYPEINT:
		tmp = "int";
		break;
	case TYPECHAR:
		tmp = "char";
		break;
	case TYPEINTARRAY:
		tmp = "int[]";
		break;
	case TYPECHARARRAY:
		tmp = "char[]";
		break;
	case TYPEINTCONST:
		tmp = "const int";
		break;
	case TYPECHARCONST:
		tmp = "const char";
		break;
	case TYPEFUNCTION:
		tmp = "function";
		break; 
	}
	o << "\ttype:" <<tmp<< endl;
	o << "\tscope:" << s.scope << endl;
	o << "\tinitValue:" << s.initValue << endl;
	o << "\taddr:" << s.addr << endl;
	o << "\tdimension:" << s.dimension << endl;
	if (s.link != NULL){
		o<<(*s.link);
	}
	else {
		cout<<"link:NULL"<<endl;
	}
	return o;
}

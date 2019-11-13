#include"SubSymbolTable.h"
#include"SymbolTable.h"
#include<iostream>
SymbolTable* SubSymbolTable::table = nullptr;
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

SymbolEntry* SubSymbolTable::addTmpSymbol(int id) {
	SymbolEntry* entry = new SymbolEntry();
	entry->type = TYPETMP;
	entry->id = id;
	tmpSymbolMap[id] = entry;
	return entry;
}
vector<int> SubSymbolTable::summaryAndReport() {

	//第一个数是参数+局部变量大小，第二个数是参数大小
	//分配的地址是以sp为界的，char int全是一个字存储
	vector<int>res;
	int size = 0;
	int parasize=0;
	//临时变量
	for (auto& i:tmpSymbolMap) {
		i.second->addr = size;
		size += 4;
		sequence.push_back(i.second->id);
	}
	//int数组
	for (auto& i : symbolMap) {
		if (i.second->type == TYPEINTARRAY&&!i.second->isParameter) {
			i.second->addr = size;
			size += (i.second->dimension) * 4;
			sequence.push_back(i.second->id);
		}
	}
	//char数组
	for (auto& i : symbolMap) {
		if (i.second->type == TYPECHARARRAY && !i.second->isParameter) {
			i.second->addr = size;
			size += (i.second->dimension) * 4;
			sequence.push_back(i.second->id);
		}
	}
	//int变量
	for (auto& i : symbolMap) {
		if (i.second->type == TYPEINT && !i.second->isParameter) {
			i.second->addr = size;
			size += 4;
			sequence.push_back(i.second->id);
		}
	}
	//char变量
	for (auto& i : symbolMap) {
		if (i.second->type == TYPECHAR && !i.second->isParameter) {
			i.second->addr = size;
			size += 4;
			sequence.push_back(i.second->id);
		}
	}
	//整型常数
	for (auto& i : symbolMap) {
		if (i.second->type == TYPEINTCONST && !i.second->isParameter) {
			i.second->addr = size;
			size += 4;
			sequence.push_back(i.second->id);
		}
	}
	//字符型常数
	for (auto& i : symbolMap) {
		if (i.second->type == TYPECHARCONST && !i.second->isParameter) {
			i.second->addr = size;
			size += 4;
			sequence.push_back(i.second->id);
		}
	}

	if (nameScope != "") {
		//参数
		SymbolEntry* functionEntry = SubSymbolTable::table->getSymbolByName("", nameScope);
		for (int i : functionEntry->link->paraIds) {
			SymbolEntry* entry = SubSymbolTable::table->getSymbolById(i);
			entry->addr = size+36;
			size += 4;
			parasize += 4;
		}
	}
	res.push_back(size);
	res.push_back(parasize);
	return res;
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

ostream& operator<<(ostream& o, FunctionLink& f) {
	//o << "\tsymbol entry link content" << endl;
	string s = f.returnType == RETINT ? "int" :
		f.returnType == RETCHAR ? "char" :
		f.returnType == RETVOID ? "void" :
		"INVALID";
	o << "\t\treturnType:" << s ;
	o << "\tparaNum:" << f.paraNum;
	o << "\tparas:[";
	for (int i = 0; i < f.paras.size(); i++) {
		
		o << f.paraIds[i] << " ";
	}
	o << "]" ;
	return o;
}

ostream& operator<<(ostream& o,SymbolEntry& s) {
	//o << "symbol entry content" << endl;
	o << "name:" << s.name;
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
	case TYPETMP:
		tmp = "tmpVariable";
		break;
	}
	o << "\ttype:" <<tmp;
	o << "\tscope:" << s.scope ;
	//o << "\tinitValue:" << s.initValue << endl;
	o << "\taddr:" << s.addr ;
	o << "\tdimension:" << s.dimension;
	if (s.link != NULL){
		o<<"\nlink:"<<(*s.link)<<endl;
	}
	else {
		o<<"link:NULL"<<endl;
	}
	return o;
}

ostream& operator<<(ostream& out, SubSymbolTable& t) {
	for (auto& i : t.symbolMap) {
		out << *(i.second);
	}
	if (t.nameScope != "") {
		for (auto& i : t.tmpSymbolMap) {
			out << "tmp" << -(i.second->id);
			out << " " << "addr:" << i.second->addr<<endl;
		}
	}
	return out;
}
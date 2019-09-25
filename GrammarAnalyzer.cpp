#include"GrammarAnalyzer.h"

GrammarAnalyzer::GrammarAnalyzer(FaultHandler& _f, SymbolTable& _s, LexicalAnalyzer& _l,string file)
:f(_f),table(_s),lex(_l){
	out.open(file);
	currentScope="";
}

Lexical GrammarAnalyzer::getNextSym() {
	if (course) { lex.printResult(out); }
	return lex.getNextSym();
}

void GrammarAnalyzer::toNextSemicon() {
	while (1) {
		Lexical tmp = lex.getNextSym();
		if (tmp == END || tmp == SEMICN) {
			break;
		}
	}
}

void GrammarAnalyzer::homeworkOn() {
	course = true;
}

/*<整数>*/
int GrammarAnalyzer::integer() {
	int sign = 1;
	Lexical tk = lex.sym().type;//检查符号
	if (tk == PLUS || tk == MINU) {
		sign = tk == MINU ? -1 : 1;
		getNextSym();
	}
	if (lex.sym().type != INTCON) {
		f.handleFault(lex.lineNumber(), "需要无符号整数");
		throw 0;
	}
	int value = lex.sym().value;//获取数字
	getNextSym();
	out << "<无符号整数>" << endl;
	out << "<整数>" << endl;
	return value;
}

/*常量声明*/
void GrammarAnalyzer::constDeclearation() {
	bool init = true;
	while (1) {
		try {
			if (lex.sym().type != CONSTTK) {
				break;//检查是否是const
			}
			init = false;
			getNextSym();

			constDefination();//检查常量定义

			if (lex.sym().type != SEMICN) {//检查是否为分号
				f.handleCourseFault(lex.lineNumber(), NOSEMICN);
				f.handleFault(lex.lineNumber(), "缺少 ;");
			}
			else {
				getNextSym();
			}
		}
		catch (int e) {
			toNextSemicon();//跳读
			lex.getNextSym();
			break;
		}
	}
	if (course&&!init) {out << "<常量说明>" << endl;}
}

/*常量定义*/
void GrammarAnalyzer::constDefination() {
	bool init = true;
	if (lex.sym().type != INTTK && lex.sym().type != CHARTK) {//检查int或char
		f.handleFault(lex.lineNumber(), "需要类型标识符");
		throw 0;
	}
	Lexical vartype = lex.sym().type;
	getNextSym();
	
	while (1) {
		if (init) {
			init = false;//第一个变量定义不能没有且不是逗号开头
		}
		else {
			if (lex.sym().type != COMMA) {
				break;//若没有逗号说明定义结束
			}
			else {
				getNextSym();
			}
		}
		if (lex.sym().type != IDENFR) {//检查并获取标识符
			f.handleFault(lex.lineNumber(), "需要变量名称");
			throw 0;
		}
		string varname = lex.sym().str;
		getNextSym();
		
		if (lex.sym().type != ASSIGN) {//获取等号
			f.handleFault(lex.lineNumber(), "需要=");
			throw 0;
		}
		getNextSym();

		if (vartype == INTTK) {
			int intvalue = integer();
			SymbolEntry* entry = table.addSymbol(currentScope, varname, false);
			entry->type = TYPEINT;
			entry->initValue = intvalue;
			entry->isConst = true;
		}
		else {
			char charvalue = (char)(lex.sym().value);
			getNextSym();
			SymbolEntry* entry = table.addSymbol(currentScope, varname, false);
			entry->type = TYPECHAR;
			entry->initValue = charvalue;
			entry->isConst = true;
		}
	}
	if (course) { out << "<常量定义>" << endl; }
}

/*变量说明,供非全局段的变量声明所使用*/
void GrammarAnalyzer::variableDeclearation() {
	bool init = true;
	while (1) {
		try {
			if (lex.sym().type != INTTK && lex.sym().type != CHARTK) {
				break;
			}
			init = false;
			variableDefination();
			if (lex.sym().type != SEMICN) {//检查是否为分号
				f.handleCourseFault(lex.lineNumber(), NOSEMICN);
				f.handleFault(lex.lineNumber(), "缺少 ;");
			}
			else {
				getNextSym();
			}
		}
		catch (int e) {
			toNextSemicon();//跳读
			lex.getNextSym();
			break;
		}
	}
	if (course && !init) { out << "<变量说明>" << endl; }
}

/*变量说明，供declearationHeader调用使用*/
void GrammarAnalyzer::variableDeclearation(Lexical type,string varname) {
		try {
			variableDefination(true,type,varname);
			if (lex.sym().type != SEMICN) {//检查是否为分号
				f.handleCourseFault(lex.lineNumber(), NOSEMICN);
				f.handleFault(lex.lineNumber(), "缺少 ;");
			}
			else {
				getNextSym();
			}
		}
		catch (int e) {
			toNextSemicon();//跳读
			lex.getNextSym();
		}
	if (course ) { out << "<变量说明>" << endl; }
}
/*<变量定义>*/
void GrammarAnalyzer::variableDefination(bool wraper,Lexical type,string name) {
	Lexical vartype;
	if (!wraper) {
		if (lex.sym().type != INTTK && lex.sym().type != CHARTK) {
			f.handleFault(lex.lineNumber(), "需要类型标识符");
			throw 0;
		}
		vartype = lex.sym().type;
		getNextSym();
	}
	else {
		vartype = type;
	}
	bool init = true;
	string varname;
	while (1) {
		if (init) {
			init = false;
			if (wraper) {
				varname = name;
				goto LABEL;
			}
		}
		else {
			if (lex.sym().type != COMMA) {
				break;
			}
			else {
				getNextSym();
			}
		}
		if (lex.sym().type != IDENFR) {
			f.handleFault(lex.lineNumber(), "需要变量名称");
			throw 0;
		}
		varname = lex.sym().str;
		
		getNextSym();

LABEL:	int dimension = 0;
		if (lex.sym().type == LBRACK) {
			getNextSym();
			if (lex.sym().type != INTCON) {
				f.handleFault(lex.lineNumber(), "需要维数");
				throw 0;
			}
			dimension = lex.sym().value;
			getNextSym();
			if(lex.sym().type!=RBRACK){
				f.handleCourseFault(lex.lineNumber(), NORBRACK);
				f.handleFault(lex.lineNumber(), "缺少]");
			}
			else {
				getNextSym();
			}
		}

		SymbolEntry* entry = table.addSymbol(currentScope, varname, false);
		if (entry == NULL) {
			f.handleCourseFault(lex.lineNumber(), REDEFINED);
			f.handleFault(lex.lineNumber(), varname + "重定义");
		}
		else {
			entry->dimension = dimension;
			if (vartype == INTTK) {
				entry->type = dimension == 0 ? TYPEINT : TYPEINTARRAY;
			}
			else if (vartype == CHARTK) {
				entry->type = dimension == 0 ? TYPECHAR : TYPECHARARRAY;
			}
		}
	}
	if (course) { out << "<变量定义>" << endl; }
}

/*有返回值函数定义,供非变量声明区的有返回值函数声明*/
void GrammarAnalyzer::nonVoidFunctionDefination() {
	try {
		Lexical retType;
		declearationHeader(retType);
		if (lex.sym().type != LPARENT) {
			f.handleFault(lex.lineNumber(), "缺少参数表");
			//todo 错误处理待定;
			f.terminate();
		}
		string functionName = lex.sym().str;
		getNextSym();
		SymbolEntry* entry = table.addSymbol(currentScope, functionName, true);
		if (entry == NULL) {
			f.handleCourseFault(lex.lineNumber(), REDEFINED);
			f.handleFault(lex.lineNumber(), "函数名重定义" + functionName);
		}
		entry->link->returnType = retType == INTTK ? RETINT : RETCHAR;
		parameterList(entry);

		if (lex.sym().type != RPARENT) {
			f.handleCourseFault(lex.lineNumber(), NORPARENT);
			f.handleFault(lex.lineNumber(), "缺少)");
		}
		else {
			getNextSym();
		}

		if (lex.sym().type != LBRACE) {
			f.handleFault(lex.lineNumber(), "缺少{");
			throw 0;
		}
		else {
			getNextSym();
		}
		currentScope = functionName;
		compoundSentence();
		currentScope = "";

		if (lex.sym().type != RBRACE) {
			f.handleFault(lex.lineNumber(), "缺少{");
			// TODO HANDLEFAULT
			f.terminate();
		}
	}
	catch (int e) {
		// TODO handlefault;
		f.terminate();
	}
	if (course) { out << "<有返回值函数声明>" << endl; }
}

void GrammarAnalyzer::nonVoidFunctionDefination(Lexical retType,string functionName) {
	try {
		
		if (lex.sym().type != LPARENT) {
			f.handleFault(lex.lineNumber(), "缺少参数表");
			//todo 错误处理待定;
			f.terminate();
		}
		getNextSym();

		SymbolEntry* entry = table.addSymbol(currentScope, functionName, true);
		if (entry == NULL) {
			f.handleCourseFault(lex.lineNumber(), REDEFINED);
			f.handleFault(lex.lineNumber(), "函数名重定义" + functionName);
		}
		entry->link->returnType = retType == INTTK ? RETINT : RETCHAR;
		parameterList(entry);

		if (lex.sym().type != RPARENT) {
			f.handleCourseFault(lex.lineNumber(), NORPARENT);
			f.handleFault(lex.lineNumber(), "缺少)");
		}
		else {
			getNextSym();
		}

		if (lex.sym().type != LBRACE) {
			f.handleFault(lex.lineNumber(), "缺少{");
			throw 0;
		}
		else {
			getNextSym();
		}
		currentScope = functionName;
		compoundSentence();
		currentScope = "";

		if (lex.sym().type != RBRACE) {
			f.handleFault(lex.lineNumber(), "缺少{");
			// TODO HANDLEFAULT
			f.terminate();
		}
	}
	catch (int e) {
		// TODO handlefault;
		f.terminate();
	}
	if (course) { out << "<有返回值函数声明>" << endl; }
}
/*声明头部,供非变量声明区的有返回值函数调用*/
string GrammarAnalyzer::declearationHeader(Lexical& retType) {
	if (lex.sym().type != INTTK && lex.sym().type != CHARTK) {
		f.handleFault(lex.lineNumber(), "缺少类型标识符");
		throw 0;
	}
	retType = lex.sym().type;
	getNextSym();
	if (lex.sym().type != IDENFR) {
		f.handleFault(lex.lineNumber(), "缺少函数名称");
		throw 0;
	}
	string functionName = lex.sym().str;
	if (course) { out << "<声明头部>" << endl; }
	getNextSym();
	
	return functionName;
}

Lexical GrammarAnalyzer::declearationHeader() {
	Lexical type;
	if (lex.sym().type != INTTK && lex.sym().type != CHARTK) {
		f.handleFault(lex.lineNumber(), "缺少类型标识符");
		throw 0;
	}
	type = lex.sym().type;
	getNextSym();
	if (lex.sym().type != IDENFR) {
		f.handleFault(lex.lineNumber(), "缺少函数名称");
		throw 0;
	}
	string name = lex.sym().str;
	getNextSym();

	if (lex.sym().type == LPARENT) {
		if (course) { out << "<声明头部>" << endl; }
		nonVoidFunctionDefination(type, name);
	}
	else if (lex.sym().type == COMMA||lex.sym().type==LBRACK) {
		variableDeclearation(type, name);
	}
	return lex.sym().type;

}

void GrammarAnalyzer::parameterList(SymbolEntry* entry) {
	bool init = true;
	while (1) {
		if (init) {
			if (lex.sym().type != INTTK && lex.sym().type != CHARTK) {
				break;
			}
			init = false;
		}
		else {
			if (lex.sym().type != COMMA) {
				break;
			}
			else {
				getNextSym();
			}
		}
		SymbolType paratype = lex.sym().type == INTTK ? TYPEINT : TYPECHAR;
		getNextSym();
		if (lex.sym().type != IDENFR) {
			f.handleFault(lex.lineNumber(), "需要提供参数名称");
			throw 0;
		}
		string paraname = lex.sym().str;
		entry->link->paras.push_back(paratype);
		entry->link->paraNum++;
		string scope = entry->scope;
		SymbolEntry* paraentry=table.addSymbol(scope, paraname, false);
		if (paraentry == NULL) {
			f.handleCourseFault(lex.lineNumber(), REDEFINED);
			f.handleFault(lex.lineNumber(), "参数名重定义" + paraname);
		}
		getNextSym();	
	}
	if (course) { out << "<参数表>" << endl; }
}

void GrammarAnalyzer::compoundSentence(){

}
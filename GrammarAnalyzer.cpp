#include"GrammarAnalyzer.h"

/*构造器函数*/
GrammarAnalyzer::GrammarAnalyzer(FaultHandler& _f, SymbolTable& _s, LexicalAnalyzer& _l,string file)
:f(_f),table(_s),lex(_l){
	out.open(file);
	currentScope="";
}

/*获取下一词素，并输出上一个词素*/
Lexical GrammarAnalyzer::getNextSym() {
	if (course) { lex.printResult(out); }
	return lex.getNextSym();
}

/*一直跳到下一个分号为止*/
void GrammarAnalyzer::toNextSemicon() {
	while (1) {
		Lexical tmp = lex.getNextSym();
		if (tmp == END || tmp == SEMICN) {
			break;
		}
	}
}

/*输出语法分析作业的输出*/
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

/*<常量声明>*/
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

/*<常量定义>*/
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

		if (vartype == INTTK) {/*根据类型读取值并写入符号表*/
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

/*<变量说明>,供非全局段的变量声明所使用*/
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

/*<变量说明>，供declearationHeader包装调用使用*/
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
/*<变量定义>,供各种情况使用
第一个参数代表是否是被包装器调用的，第二个参数传入包装器预读的变量类型和变量名称
*/
void GrammarAnalyzer::variableDefination(bool wraper,Lexical type,string name) {
	Lexical vartype;
	if (!wraper) {
		if (lex.sym().type != INTTK && lex.sym().type != CHARTK) {
			f.handleFault(lex.lineNumber(), "需要类型标识符");//未被包装：检测变量类型标识符
			throw 0;
		}
		vartype = lex.sym().type;
		getNextSym();
	}
	else {
		vartype = type;//已被包装：检测变量类型标识符
	}
	bool init = true;
	string varname;
	while (1) {
		if (init) {
			init = false;
			if (wraper) {//需要处理被包装器预读的第一个变量
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
			f.handleFault(lex.lineNumber(), "需要变量名称");//读变量名称
			throw 0;
		}
		varname = lex.sym().str;
		
		getNextSym();

LABEL:	int dimension = 0;
		if (lex.sym().type == LBRACK) {//判断是不是数组元素
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

		SymbolEntry* entry = table.addSymbol(currentScope, varname, false);//插入符号表
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

/*<有返回值函数定义>,供非变量声明区的有返回值函数声明*/
void GrammarAnalyzer::nonVoidFunctionDefination() {
	try {
		Lexical retType;
		string functionName=declearationHeader(retType);//获取变量名和返回类型
		if (lex.sym().type != LPARENT) {//参数表的左括号
			f.handleFault(lex.lineNumber(), "缺少参数表");
			//todo 错误处理待定;
			f.terminate();
		}
		getNextSym();
		SymbolEntry* entry = table.addSymbol(currentScope, functionName, true);//插入符号表项
		if (entry == NULL) {
			f.handleCourseFault(lex.lineNumber(), REDEFINED);
			f.handleFault(lex.lineNumber(), "函数名重定义" + functionName);
		}
		entry->link->returnType = retType == INTTK ? RETINT : RETCHAR;
		entry->type = TYPEFUNCTION;
		parameterList(entry);

		if (lex.sym().type != RPARENT) {//参数表右括号
			f.handleCourseFault(lex.lineNumber(), NORPARENT);
			f.handleFault(lex.lineNumber(), "缺少)");
		}
		else {
			getNextSym();
		}

		if (lex.sym().type != LBRACE) {//读取左大括号
			f.handleFault(lex.lineNumber(), "缺少{");
			throw 0;
		}
		else {
			getNextSym();
		}
		currentScope = functionName;//变更当前作用域
		compoundSentence();
		currentScope = "";

		if (lex.sym().type != RBRACE) {
			f.handleFault(lex.lineNumber(), "缺少{");
			// TODO HANDLEFAULT
			f.terminate();
		}
		getNextSym();
	}
	catch (int e) {
		// TODO handlefault;
		f.terminate();
	}
	if (course) { out << "<有返回值函数声明>" << endl; }
}

void GrammarAnalyzer::nonVoidFunctionDefination(Lexical retType,string functionName) {
	try {
		
		if (lex.sym().type != LPARENT) {//参数表的左括号
			f.handleFault(lex.lineNumber(), "缺少参数表");
			//todo 错误处理待定;
			f.terminate();
		}
		getNextSym();

		SymbolEntry* entry = table.addSymbol(currentScope, functionName, true);//插入符号表项
		if (entry == NULL) {
			f.handleCourseFault(lex.lineNumber(), REDEFINED);
			f.handleFault(lex.lineNumber(), "函数名重定义" + functionName);
		}
		entry->link->returnType = retType == INTTK ? RETINT : RETCHAR;
		entry->type = TYPEFUNCTION;
		parameterList(entry);

		if (lex.sym().type != RPARENT) {
			f.handleCourseFault(lex.lineNumber(), NORPARENT);//参数表右括号
			f.handleFault(lex.lineNumber(), "缺少)");
		}
		else {
			getNextSym();
		}

		if (lex.sym().type != LBRACE) {//左大括号
			f.handleFault(lex.lineNumber(), "缺少{");
			throw 0;
		}
		else {
			getNextSym();
		}
		currentScope = functionName;
		compoundSentence();
		currentScope = "";

		if (lex.sym().type != RBRACE) {//右大括号
			f.handleFault(lex.lineNumber(), "缺少{");
			// TODO HANDLEFAULT
			f.terminate();
		}
		getNextSym();
	}
	catch (int e) {
		// TODO handlefault;
		f.terminate();
	}
	
	if (course) { out << "<有返回值函数声明>" << endl; }
}

/*<主函数>*/
void GrammarAnalyzer::voidFunctionDefination() {
	try {
		if (lex.sym().type != VOIDTK) {
			cout << "BUG@void GrammarAnalyzer::voidFunctionDefination()" << endl;
			f.terminate();
		}
		getNextSym();

		if (lex.sym().type != IDENFR) {
			f.handleFault(lex.lineNumber(), "缺少函数名");
			//todo 错误处理待定;
			f.terminate();
		}
		string functionName = lex.sym().str;
		getNextSym();

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
		entry->link->returnType = RETVOID;
		entry->type = TYPEFUNCTION;
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
		getNextSym();
	}
	catch (int e) {
		// TODO handlefault;
		f.terminate();
	}
	if (course) { out << "<无返回值函数声明>" << endl; }
}


void GrammarAnalyzer::mainFunctionDefination() {
	try {
		if (lex.sym().type != VOIDTK) {
			cout << "BUG@void GrammarAnalyzer::voidFunctionDefination()" << endl;
			f.terminate();
		}
		getNextSym();

		if (lex.sym().type != MAINTK) {
			f.handleFault(lex.lineNumber(), "缺少函数名");
			//todo 错误处理待定;
			f.terminate();
		}
		string functionName = "main";
		getNextSym();

		SymbolEntry* entry = table.addSymbol(currentScope, functionName, true);
		if (entry == NULL) {
			f.handleCourseFault(lex.lineNumber(), REDEFINED);
			f.handleFault(lex.lineNumber(), "函数名重定义" + functionName);
		}
		entry->link->returnType = RETVOID;
		entry->type = TYPEFUNCTION;
		if (lex.sym().type != LPARENT) {
			f.handleFault(lex.lineNumber(), "缺少参数表");
			//todo 错误处理待定;
			f.terminate();
		}
		getNextSym();

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
			f.handleFault(lex.lineNumber(), "缺少}");
			// TODO HANDLEFAULT
			f.terminate();
		}
		getNextSym();
	}
	catch (int e) {
		// TODO handlefault;
		f.terminate();
	}
	getNextSym();
	if (course) { out << "<主函数>" << endl; }
}

/*<声明头部>,供非变量声明区的有返回值函数调用*/
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

/*变量声明和有返回值函数定义在尚无法判明的情况下进行预读用的函数*/
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
	else if (lex.sym().type == COMMA||lex.sym().type==LBRACK||lex.sym().type==SEMICN) {
		variableDeclearation(type, name);
	}
	return lex.sym().type;

}

/*<参数表>*/
void GrammarAnalyzer::parameterList(SymbolEntry* entry) {
	bool init = true;
	int paraNum = 0;
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
		if (lex.sym().type != INTTK && lex.sym().type != CHARTK) {
			f.handleFault(lex.lineNumber(), "需要提供参数类型");
			throw 0;
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
		string scope = entry->name;
		SymbolEntry* paraentry=table.addSymbol(scope, paraname, false);
		if (paraentry == NULL) {
			f.handleCourseFault(lex.lineNumber(), REDEFINED);
			f.handleFault(lex.lineNumber(), "参数名重定义" + paraname);
		}
		getNextSym();
		paraNum++;
	}
	entry->link->paraNum = paraNum;
	if (course) { out << "<参数表>" << endl; }
}

/*<复合语句>*/
void GrammarAnalyzer::compoundSentence(){

}

/*<因子>*/
void GrammarAnalyzer::factor() {
	bool error = false;
	if (lex.sym().type == IDENFR) {//标识符，标识符数组，有返回值函数调用语句
		string varname = lex.sym().str;
		getNextSym();
		SymbolEntry* entry = table.getSymbolByName(currentScope, varname);
		if (entry == NULL) {
			f.handleCourseFault(lex.lineNumber(), UNDEFINED);
			f.handleFault(lex.lineNumber(), "未定义的符号" + varname);
			error = true;
		}
		if (lex.sym().type == LBRACK) {//数组
			if (!error && entry->type != TYPECHARARRAY && entry->type != TYPEINTARRAY) {
				f.handleCourseFault(lex.lineNumber(), TYPEERROR);
				f.handleFault(lex.lineNumber(), varname + "变量不是数组类型");
				error = true;
			}
			getNextSym();

			expression();

			if (lex.sym().type != RBRACK) {
				f.handleCourseFault(lex.lineNumber(), NORBRACK);
				f.handleFault(lex.lineNumber(), "缺少]");
				error = true;
			}
			else {
				getNextSym();
			}
		}
		else if (lex.sym().type == LPARENT) {//有返回值函数
			functionCall(varname,true);
		}
		else {//标识符
			//此处应有中间代码生成？
		}
	}
	else if (lex.sym().type == LPARENT) {
		getNextSym();

		expression();
		if (lex.sym().type != RPARENT) {
			f.handleCourseFault(lex.lineNumber(), NORPARENT);
			f.handleFault(lex.lineNumber(),"缺少）");
		}
		else {
			getNextSym();
		}
	}
	else if (lex.sym().type == INTCON|| lex.sym().type == PLUS|| lex.sym().type == MINU) {
		int value = integer();
	}
	else if (lex.sym().type == CHARCON) {
		char c = (char)(lex.sym().value);
		getNextSym();
	}
	else {
		f.handleFault(lex.lineNumber(), "不被识别的表达式");
		// todo handlefault 
		f.terminate();
	}
	if (course) { out << "<因子>" << endl; }
}

/*<项>*/
void GrammarAnalyzer::term() {
	factor();
	while (1) {
		if (lex.sym().type != MULT && lex.sym().type != DIV) {
			break;
		}
		Lexical op = lex.sym().type;
		getNextSym();
		factor();
	}
	if (course) { out << "<项>" << endl; }
}

/*<表达式>*/
void GrammarAnalyzer::expression() {
	int sign = 1;
	if (lex.sym().type == PLUS || lex.sym().type == MINU) {
		if (lex.sym().type == MINU) {
			sign = -1;
		}
		getNextSym();
	}
	term();
	while (1) {
		if (lex.sym().type != PLUS && lex.sym().type != MINU) {
			break;
		}
		Lexical op = lex.sym().type;
		getNextSym();
		term();
	}
	if (course) { out << "<表达式>" << endl; }
}

/*针对赋值语句和函数调用语句的预读分支*/
void GrammarAnalyzer::assignAndCall() {
	if (lex.sym().type != IDENFR) {
		f.handleFault(lex.lineNumber(), "需要标识符");
		// todo handle fault
		f.terminate();
	}
	string name = lex.sym().str;//读到一个标识符
	getNextSym();
	if (lex.sym().type == LBRACK || lex.sym().type == ASSIGN) {//是赋值语句
		assignSentence(name);
	}
	else if (lex.sym().type == LPARENT) {//是函数调用语句
		functionCall(name,false);
	}
	else {
		f.handleFault(lex.lineNumber(), "无意义的标识符");
		// todo handle fault
		f.terminate();
	}
}
/*<赋值语句>，已经过预读*/
void GrammarAnalyzer::assignSentence(string varname) {
	bool error = false;
	SymbolEntry* entry = table.getSymbolByName(currentScope, varname);
	if (entry == NULL) {
		f.handleCourseFault(lex.lineNumber(), UNDEFINED);
		f.handleFault(lex.lineNumber(), "未定义的变量");
		error = true;
	}
	if (lex.sym().type == LBRACK) {//左值是数组
		if (!error && entry->type != TYPECHARARRAY && entry->type != TYPEINTARRAY) {
			f.handleCourseFault(lex.lineNumber(), TYPEERROR);
			f.handleFault(lex.lineNumber(), varname + "变量不是数组类型");
			error = true;
		}
		getNextSym();
		expression();
		if (lex.sym().type != RBRACK) {
			f.handleCourseFault(lex.lineNumber(), NORBRACK);
			f.handleFault(lex.lineNumber(), "缺少]");
		}
		else {
			getNextSym();
		}
	}
	if (lex.sym().type != ASSIGN) {//读取赋值运算符
		f.handleFault(lex.lineNumber(), "需要=");
		// todo handlefault
	}
	else {
		getNextSym();
	}
	expression();//读取所赋的表达式
	if (course) { out << "<赋值语句>"<<endl; }
}

/*函数调用，若mustReturn是真意味着必须有返回值，否则有无返回值皆可，已被预读符号*/
void GrammarAnalyzer::functionCall(string name,bool mustReturn) {
	bool error = false;
	SymbolEntry* entry = table.getSymbolByName(currentScope, name);//读取函数信息
	if (entry == NULL) {//检查是否已定义
		f.handleCourseFault(lex.lineNumber(), UNDEFINED);
		f.handleFault(lex.lineNumber(), "未定义的变量");
		error = true;
	}
	if (!error&&entry->type != TYPEFUNCTION) {//检查是否为函数
		f.handleCourseFault(lex.lineNumber(), TYPEERROR);
		f.handleFault(lex.lineNumber(), "不是函数");
		//todo handle fault
		f.terminate();
	}
	if (!error&&mustReturn && entry->link->returnType == RETVOID) {//检查是否有返回值
		f.handleCourseFault(lex.lineNumber(), TYPEERROR);
		f.handleFault(lex.lineNumber(), "不是有返回值函数");
		// todo handlefault;
		f.terminate();
	}
	if (lex.sym().type != LPARENT) {//读取（
		f.handleFault(lex.lineNumber(), "缺少(");
		//todo handle fault
		f.terminate();
	}
	getNextSym();
	parameterValueList(entry);//读取实参列表
	if (lex.sym().type != RPARENT) {
		f.handleCourseFault(lex.lineNumber(), NORPARENT);
		f.handleFault(lex.lineNumber(), "缺少)");
		//todo handle fault
	}
	getNextSym();
	if (course) {
		if (entry->link->returnType == RETVOID) {
			out << "<无返回值函数调用语句>"<<endl;
		}
		else {
			out << "<有返回值函数调用语句>" << endl;
		}
	}
}

/*<值参数表>*/
void GrammarAnalyzer::parameterValueList(SymbolEntry* entry) {
	int paraNum = 0;
	bool init = true;
	while (1) {
		if (lex.sym().type == RPARENT) {
			break;
		}
		if (!init) {
			if (lex.sym().type != COMMA) {
				f.handleFault(lex.lineNumber(), "缺少,");
				// todo handle fault
				f.terminate();
			}
			else {
				getNextSym();
			}
		}
		else {
			init = false;
		}
		expression();
		paraNum++;
	}
	if (entry->link->paraNum != paraNum) {
		f.handleCourseFault(lex.lineNumber(), PARANUMERROR);
		f.handleFault(lex.lineNumber(), "函数参数个数不正确");
		// todo handle fault 
		f.terminate();
	}
	if (course) { out << "<值参数表>" << endl; }
}
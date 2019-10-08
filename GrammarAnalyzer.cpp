#include"GrammarAnalyzer.h"

/*构造器函数*/
GrammarAnalyzer::GrammarAnalyzer(FaultHandler& _f, SymbolTable& _s, LexicalAnalyzer& _l,string file)
:f(_f),table(_s),lex(_l){
	out.open(file,ios_base::trunc|ios_base::out);
	currentScope="";
}

/*获取下一词素，并输出上一个词素*/
Lexical GrammarAnalyzer::getNextSym() {
	if (lexicalTest) { lex.printResult(out); }
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
void GrammarAnalyzer::homeworkOn(bool _c, bool _l) {
	course = _c;
	lexicalTest = _l;

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
	if (course) {
		out << "<无符号整数>" << endl;
		out << "<整数>" << endl;
	}
	return value;
}

/*<常量声明>*/
void GrammarAnalyzer::constDeclearation() {
	bool init = true;
	while (1) {
		try {
			if (lex.sym().type != CONSTTK) {
				break;//检查是否是const，若不是则结束
			}
			init = false;
			getNextSym();
			//完成读取const符号

			constDefination();//检查常量定义

			if (lex.sym().type != SEMICN) {//检查是否为分号
				f.handleCourseFault(lex.lineNumber(), NOSEMICN);
				f.handleFault(lex.lineNumber(), "缺少 ;");
			}
			else {
				getNextSym();
			}
			//完成读取分号
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
	//读取标识符完成
	
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
			//读取逗号完成
		}
		if (lex.sym().type != IDENFR) {//检查并获取标识符
			f.handleFault(lex.lineNumber(), "需要变量名称");
			throw 0;
		}
		string varname = lex.sym().str;
		getNextSym();
		//读取标识符完成
		
		if (lex.sym().type != ASSIGN) {//获取等号
			f.handleFault(lex.lineNumber(), "需要=");
			throw 0;
		}
		getNextSym();
		//读取等号完成

		if (vartype == INTTK) {/*根据类型读取值并写入符号表*/
			int intvalue = integer();
			//读取整数完成
			SymbolEntry* entry = table.addSymbol(currentScope, varname, false);
			entry->type = TYPEINTCONST;
			entry->initValue = intvalue;
			//写入符号表完成
		}
		else {
			char charvalue = (char)(lex.sym().value);
			getNextSym();
			//读取字符完成
			SymbolEntry* entry = table.addSymbol(currentScope, varname, false);
			entry->type = TYPECHARCONST;
			entry->initValue = charvalue;
			//写入符号表完成
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
			//预读标识符
			init = false;
			variableDefination();
			if (lex.sym().type != SEMICN) {//检查是否为分号
				f.handleCourseFault(lex.lineNumber(), NOSEMICN);
				f.handleFault(lex.lineNumber(), "缺少 ;");
			}
			else {
				getNextSym();
			}
			//读取分号完成
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
	//if (course ) { out << "<变量说明>" << endl; }
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
		//读取标识符完成
	}
	else {
		vartype = type;//已被包装：不需要检测变量类型标识符
	}
	bool init = true;
	string varname;
	while (1) {
		if (init) {//是循环中读取的第一个变量
			init = false;
			if (wraper) {//需要处理被包装器预读的第一个变量
				varname = name;
				goto LABEL;//跳过读取标识符的阶段
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
		//完成标识符地区

LABEL:	int dimension = 0;
		if (lex.sym().type == LBRACK) {//判断是不是数组元素
			getNextSym();
			if (lex.sym().type != INTCON) {
				f.handleFault(lex.lineNumber(), "需要维数");
				throw 0;
			}
			dimension = lex.sym().value;
			getNextSym();
			//读取维数完成
			if (course) { out << "<无符号整数>"<<endl; }//debug added
			if(lex.sym().type!=RBRACK){
				f.handleCourseFault(lex.lineNumber(), NORBRACK);
				f.handleFault(lex.lineNumber(), "缺少]");
			}
			else {
				getNextSym();
			}
			//读取右中括号完成
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
		//向符号表中填入信息
	}
	if (course) { out << "<变量定义>" << endl; }
}

/*<有返回值函数定义>,供非变量声明区的有返回值函数声明*/
void GrammarAnalyzer::nonVoidFunctionDefination() {
	try {
		Lexical retType;
		string functionName=declearationHeader(retType);
		//获取变量名和返回类型完成

		if (lex.sym().type != LPARENT) {//参数表的左括号
			f.handleFault(lex.lineNumber(), "缺少参数表");
			//todo 错误处理待定;
			f.terminate();
		}
		getNextSym();
		//处理左括号完成

		SymbolEntry* entry = table.addSymbol(currentScope, functionName, true);//插入符号表项
		if (entry == NULL) {
			f.handleCourseFault(lex.lineNumber(), REDEFINED);
			f.handleFault(lex.lineNumber(), "函数名重定义" + functionName);
		}
		entry->link->returnType = retType == INTTK ? RETINT : RETCHAR;
		entry->type = TYPEFUNCTION;
		parameterList(entry);
		//填入符号表信息

		if (lex.sym().type != RPARENT) {
			f.handleCourseFault(lex.lineNumber(), NORPARENT);
			f.handleFault(lex.lineNumber(), "缺少)");
		}
		else {
			getNextSym();
		}
		//参数表右括号完成

		if (lex.sym().type != LBRACE) {//读取左大括号
			f.handleFault(lex.lineNumber(), "缺少{");
			throw 0;
		}
		else {
			getNextSym();
		}
		//左大括号完成

		currentScope = functionName;//变更当前作用域
		compoundSentence();//复合语句
		currentScope = "";//作用域变回来

		if (lex.sym().type != RBRACE) {
			f.handleFault(lex.lineNumber(), "缺少{");
			// TODO HANDLEFAULT
			f.terminate();
		}
		getNextSym();
		//获取右大括号
	}
	catch (int e) {
		// TODO handlefault;
		f.terminate();
	}
	if (course) { out << "<有返回值函数定义>" << endl; }
}

void GrammarAnalyzer::nonVoidFunctionDefination(Lexical retType,string functionName) {
	try {
		if (lex.sym().type != LPARENT) {//参数表的左括号
			f.handleFault(lex.lineNumber(), "缺少参数表");
			//todo 错误处理待定;
			f.terminate();
		}
		getNextSym();
		//左括号读取完成

		SymbolEntry* entry = table.addSymbol(currentScope, functionName, true);//插入符号表项
		if (entry == NULL) {
			f.handleCourseFault(lex.lineNumber(), REDEFINED);
			f.handleFault(lex.lineNumber(), "函数名重定义" + functionName);
		}
		entry->link->returnType = retType == INTTK ? RETINT : RETCHAR;
		entry->type = TYPEFUNCTION;
		//写入符号表

		parameterList(entry);
		//参数表

		if (lex.sym().type != RPARENT) {
			f.handleCourseFault(lex.lineNumber(), NORPARENT);//参数表右括号
			f.handleFault(lex.lineNumber(), "缺少)");
		}
		else {
			getNextSym();
		}
		//读取右括号

		if (lex.sym().type != LBRACE) {//左大括号
			f.handleFault(lex.lineNumber(), "缺少{");
			throw 0;
		}
		else {
			getNextSym();
		}
		//读取左大括号完成

		currentScope = functionName;//作用域变过来
		compoundSentence();//复合语句
		currentScope = "";//作用域变回去

		if (lex.sym().type != RBRACE) {//右大括号
			f.handleFault(lex.lineNumber(), "缺少{");
			// TODO HANDLEFAULT
			f.terminate();
		}
		getNextSym();
		//右大括号读取完成
	}
	catch (int e) {
		// TODO handlefault;
		f.terminate();
	}
	
	if (course) { out << "<有返回值函数定义>" << endl; }
}

/*<主函数>*/

void GrammarAnalyzer::voidFunctionDefination() {
	try {
		//这个地方没有void符号因为已经被programme预读掉了
		if (lex.sym().type != IDENFR) {
			f.handleFault(lex.lineNumber(), "缺少函数名");
			//todo 错误处理待定;
			f.terminate();
		}
		string functionName = lex.sym().str;
		getNextSym();
		//函数名读取完成

		if (lex.sym().type != LPARENT) {
			f.handleFault(lex.lineNumber(), "缺少参数表");
			//todo 错误处理待定;
			f.terminate();
		}
		getNextSym();
		//读走了左括号
		SymbolEntry* entry = table.addSymbol(currentScope, functionName, true);
		if (entry == NULL) {
			f.handleCourseFault(lex.lineNumber(), REDEFINED);
			f.handleFault(lex.lineNumber(), "函数名重定义" + functionName);
		}
		entry->link->returnType = RETVOID;
		entry->type = TYPEFUNCTION;
		//写入符号表完成

		parameterList(entry);
		//参数表读取完成

		if (lex.sym().type != RPARENT) {
			f.handleCourseFault(lex.lineNumber(), NORPARENT);
			f.handleFault(lex.lineNumber(), "缺少)");
		}
		else {
			getNextSym();
		}
		//右括号读取完成

		if (lex.sym().type != LBRACE) {
			f.handleFault(lex.lineNumber(), "缺少{");
			throw 0;
		}
		else {
			getNextSym();
		}
		//读取左大括号完成

		currentScope = functionName;
		compoundSentence();
		currentScope = "";

		if (lex.sym().type != RBRACE) {
			f.handleFault(lex.lineNumber(), "缺少{");
			// TODO HANDLEFAULT
			f.terminate();
		}
		getNextSym();
		//读取右大括号完成
	}
	catch (int e) {
		// TODO handlefault;
		f.terminate();
	}
	if (course) { out << "<无返回值函数定义>" << endl; }
}


void GrammarAnalyzer::mainFunctionDefination() {
	try {
		//这个地方没有void符号因为已经被programme预读掉了
		if (lex.sym().type != MAINTK) {
			f.handleFault(lex.lineNumber(), "缺少函数名");
			//todo 错误处理待定;
			f.terminate();
		}
		string functionName = "main";
		getNextSym();
		//读取main标识符

		SymbolEntry* entry = table.addSymbol(currentScope, functionName, true);
		if (entry == NULL) {
			f.handleCourseFault(lex.lineNumber(), REDEFINED);
			f.handleFault(lex.lineNumber(), "main函数重定义" + functionName);
		}
		entry->link->returnType = RETVOID;
		entry->type = TYPEFUNCTION;
		// 写入符号表完成

		if (lex.sym().type != LPARENT) {
			f.handleFault(lex.lineNumber(), "缺少参数表");
			//todo 错误处理待定;
			f.terminate();
		}
		getNextSym();
		//读入左括号完成

		if (lex.sym().type != RPARENT) {
			f.handleCourseFault(lex.lineNumber(), NORPARENT);
			f.handleFault(lex.lineNumber(), "缺少)");
		}
		else {
			getNextSym();
		}
		//读入右括号完成

		if (lex.sym().type != LBRACE) {
			f.handleFault(lex.lineNumber(), "缺少{");
			throw 0;
		}
		else {
			getNextSym();
		}
		//读入左大括号完成
		currentScope = functionName;
		compoundSentence();
		currentScope = "";

		if (lex.sym().type != RBRACE) {
			f.handleFault(lex.lineNumber(), "缺少}");
			// TODO HANDLEFAULT
			f.terminate();
		}
		getNextSym();
		//读入右大括号完成
	}
	catch (int e) {
		// TODO handlefault;
		f.terminate();
	}
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
	// 读取类型完成

	if (lex.sym().type != IDENFR) {
		f.handleFault(lex.lineNumber(), "缺少函数名称");
		throw 0;
	}
	string functionName = lex.sym().str;
	getNextSym();
	//读取函数名完成
	if (course) { out << "<声明头部>" << endl; }
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
	//getNextSym();
	Result tmp1 = lex.sym();
	lex.getNextSym();
	//读取类型标识符完成

	if (lex.sym().type != IDENFR) {
		f.handleFault(lex.lineNumber(), "缺少函数名称");
		throw 0;
	}
	string name = lex.sym().str;
	Result tmp2 = lex.sym();
	lex.getNextSym();
	//读取标识符完成

	if (lex.sym().type == LPARENT) {
		if (globalVariableDeclearation) {
			if (course) { out << "<变量说明>" << endl; }
			globalVariableDeclearation = false;
			//如果这是我们碰见的第一个有返回值函数
			//并且前面出现了全局变量声明，那就要输出变量声明这一问题
		}
		if (lexicalTest) {
			lex.printResult(out, tmp1);
			lex.printResult(out, tmp2);//输出憋住的信息
		}
		if (course) { out << "<声明头部>" << endl; }
		nonVoidFunctionDefination(type, name);
	}
	else if (lex.sym().type == COMMA||lex.sym().type==LBRACK||lex.sym().type==SEMICN) {
		globalVariableDeclearation = true;//设置存在变量声明的标记位
		if (lexicalTest) {
			lex.printResult(out, tmp1);
			lex.printResult(out, tmp2);//输出憋住的信息
		}
		variableDeclearation(type, name);
	}
	else {
		f.handleFault(lex.lineNumber(), "非法的标识符声明");
		//f.terminate();
		throw 0;
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
			//要是在开头而且还没有类型标识符说明是空列表
			init = false;
		}
		else {
			if (lex.sym().type != COMMA) {
				break;
			}
			else {
				getNextSym();
			}
			//读取逗号完成
		}

		if (lex.sym().type != INTTK && lex.sym().type != CHARTK) {
			f.handleFault(lex.lineNumber(), "需要提供参数类型");
			throw 0;
		}
		SymbolType paratype = lex.sym().type == INTTK ? TYPEINT : TYPECHAR;
		getNextSym();
		//读取参数类型

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
		//写入符号表

		getNextSym();
		//读取参数名称完成
		paraNum++;
	}
	entry->link->paraNum = paraNum;
	if (course) { out << "<参数表>" << endl; }
}

/*<复合语句>*/
void GrammarAnalyzer::compoundSentence(){
	constDeclearation();
	variableDeclearation();
	sentenceSeries();
	if (course) { out << "<复合语句>" << endl; }
}

/*<因子>*/
void GrammarAnalyzer::factor() {
	bool error = false;
	if (lex.sym().type == IDENFR) {//标识符，标识符数组，有返回值函数调用语句
		string varname = lex.sym().str;
		getNextSym();
		//标识符读取完成

		SymbolEntry* entry = table.getSymbolByName(currentScope, varname);
		if (entry == NULL) {
			f.handleCourseFault(lex.lineNumber(), UNDEFINED);
			f.handleFault(lex.lineNumber(), "未定义的符号" + varname);
			error = true;
		}
		//查询符号表，检索是否存在这个符号

		if (lex.sym().type == LBRACK) {//数组
			if (!error && entry->type != TYPECHARARRAY && entry->type != TYPEINTARRAY) {
				f.handleCourseFault(lex.lineNumber(), TYPEERROR);
				f.handleFault(lex.lineNumber(), varname + "变量不是数组类型");
				error = true;
			}
			//检索符号表，检查该符号是否为数组类型
			getNextSym();
			//完成读取中括号

			expression();
			//下标表达式

			if (lex.sym().type != RBRACK) {
				f.handleCourseFault(lex.lineNumber(), NORBRACK);
				f.handleFault(lex.lineNumber(), "缺少]");
				error = true;
			}
			else {
				getNextSym();
			}
			//完成读取右括号
		}
		else if (lex.sym().type == LPARENT) {//有返回值函数
			functionCall(varname,true);
		}
		else {//标识符
			//此处应有中间代码生成？
		}
	}
	else if (lex.sym().type == LPARENT) {//（<表达式>）
		getNextSym();
		//完成读取左括号

		expression();
		//读取表达式
		if (lex.sym().type != RPARENT) {
			f.handleCourseFault(lex.lineNumber(), NORPARENT);
			f.handleFault(lex.lineNumber(),"缺少）");
		}
		else {
			getNextSym();
		}
	}
	else if (lex.sym().type == INTCON|| lex.sym().type == PLUS|| lex.sym().type == MINU) {
		//整数（由+-或整数开头）
		int value = integer();
		//读取整数
	}
	else if (lex.sym().type == CHARCON) {
		char c = (char)(lex.sym().value);
		getNextSym();
		//完成读取字符
	}
	else {
		f.handleFault(lex.lineNumber(), "不被识别的表达式");
		// todo handlefault 
		//f.terminate();
		throw 0;
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
		//读取完运算符号
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
	//读取开头的加减号完成

	term();
	//读取第一项
	while (1) {
		if (lex.sym().type != PLUS && lex.sym().type != MINU) {
			break;
		}
		Lexical op = lex.sym().type;
		getNextSym();
		//读取运算符号完成
		term();
	}
	if (course) { out << "<表达式>" << endl; }
}

/*针对赋值语句和函数调用语句的预读分支*/
void GrammarAnalyzer::assignAndCall() {
	if (lex.sym().type != IDENFR) {
		f.handleFault(lex.lineNumber(), "需要标识符");
		// todo handle fault
		//f.terminate();
		throw 0;
	}
	string name = lex.sym().str;
	getNextSym();
	//读取标识符完成

	if (lex.sym().type == LBRACK || lex.sym().type == ASSIGN) {
		//接下来是=或者[的是赋值语句
		assignSentence(name);
	}
	else if (lex.sym().type == LPARENT) {//是函数调用语句
		functionCall(name,false);
	}
	else {
		f.handleFault(lex.lineNumber(), "无意义的符号");
		// todo handle fault
		//f.terminate();
		throw 0;
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
	if (entry->type == TYPECHARCONST || entry->type == TYPEINTCONST ||
		entry->type == TYPEFUNCTION) {
		f.handleFault(lex.lineNumber(), "非法左值");
		error = true;
	}
	//检查变量合法性
	if (lex.sym().type == LBRACK) {//左值是数组
		if (!error && entry->type != TYPECHARARRAY && entry->type != TYPEINTARRAY) {
			f.handleCourseFault(lex.lineNumber(), TYPEERROR);
			f.handleFault(lex.lineNumber(), varname + "变量不是数组类型");
			error = true;
		}
		//检查是不是数组类型的变量
		getNextSym();
		//读取完了左括号

		expression();
		//读取完成右括号

		if (lex.sym().type != RBRACK) {
			f.handleCourseFault(lex.lineNumber(), NORBRACK);
			f.handleFault(lex.lineNumber(), "缺少]");
		}
		else {
			getNextSym();
		}
		//完成读取右括号
	}
	if (lex.sym().type != ASSIGN) {//读取赋值运算符
		f.handleFault(lex.lineNumber(), "需要=");
		// todo handlefault
		throw 0;
	}
	else {
		getNextSym();
	}
	//完成读取赋值运算符
	expression();
	//读取所赋的表达式
	if (course) { out << "<赋值语句>"<<endl; }
}

/*函数调用，若mustReturn是真意味着必须有返回值，否则有无返回值皆可，已被预读符号*/
void GrammarAnalyzer::functionCall(string name,bool mustReturn) {
	bool error = false;

	SymbolEntry* entry = table.getSymbolByName(currentScope, name);
	if (entry == NULL) {//检查是否已定义
		f.handleCourseFault(lex.lineNumber(), UNDEFINED);
		f.handleFault(lex.lineNumber(), "未定义的函数");
		error = true;
	}
	//检查是否有变量信息

	if (!error&&entry->type != TYPEFUNCTION) {//检查是否为函数
		f.handleCourseFault(lex.lineNumber(), TYPEERROR);
		f.handleFault(lex.lineNumber(), "不是函数");
		error = true;
		//todo handle fault
		//f.terminate();
	}
	
	if (!error&&mustReturn && entry->link->returnType == RETVOID) {//检查是否有返回值
		f.handleCourseFault(lex.lineNumber(), TYPEERROR);
		f.handleFault(lex.lineNumber(), "不是有返回值函数");
		// todo handlefault;
		error = true;
		//f.terminate();
	}

	if (lex.sym().type != LPARENT) {//读取（
		f.handleFault(lex.lineNumber(), "缺少(");
		//todo handle fault
		//f.terminate();
		throw 0;
	}
	getNextSym();
	//完成左括号处理

	parameterValueList(entry);//读取实参列表

	if (lex.sym().type != RPARENT) {
		f.handleCourseFault(lex.lineNumber(), NORPARENT);
		f.handleFault(lex.lineNumber(), "缺少)");
	}
	else {
		getNextSym();
	}
	//右大括号读取完成

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
				//f.terminate();
				throw 0;
			}
			else {
				getNextSym();
			}
			//读取逗号
		}
		else {
			init = false;
		}
		expression();
		//读取实参的表达式
		paraNum++;
	}
	if (entry->link->paraNum != paraNum) {
		f.handleCourseFault(lex.lineNumber(), PARANUMERROR);
		f.handleFault(lex.lineNumber(), "函数参数个数不正确");
		// todo handle fault 
		//f.terminate();
		throw 0;
	}
	//检查参数是否对应（个数，应该还有参数类型的标记）
	if (course) { out << "<值参数表>" << endl; }
}

void GrammarAnalyzer::scanSentence() {
	if (lex.sym().type != SCANFTK) {
		f.handleFault(lex.lineNumber(), "缺少scanf标识符");
		// todo handle-fault;
		//f.terminate();
		throw 0;
	}
	getNextSym();
	//读取scanf标识符

	if (lex.sym().type != LPARENT) {
		f.handleFault(lex.lineNumber(), "缺少（");
		// todo handle-fault
		//f.terminate();
		throw 0;
	}
	getNextSym();
	//读取左括号

	bool init = true;
	while (1) {
		if (init) {
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
		if (lex.sym().type != IDENFR) {
			f.handleFault(lex.lineNumber(), "缺少标识符");
			// todo handle-fault
			//f.terminate();
			throw 0;
		}
		string varname = lex.sym().str;
		SymbolEntry* entry = table.getSymbolByName(currentScope, varname);
		if (entry == NULL) {
			f.handleCourseFault(lex.lineNumber(), UNDEFINED);
			f.handleFault(lex.lineNumber(), "未定义的变量");
			throw 0;
		}
		if (entry->type != TYPEINT && entry->type != TYPECHAR) {
			f.handleFault(lex.lineNumber(), "不可读入的数据类型");
			throw 0;
		}
		//依据符号表信息检查变量是否合法
		getNextSym();
		//标识符读取完成
	}

	if (lex.sym().type != RPARENT) {
		f.handleCourseFault(lex.lineNumber(), NORPARENT);
		f.handleFault(lex.lineNumber(), "缺少)");
	}
	else {
		getNextSym();
	}
	//右括号读取完成
	if (course) { out << "<读语句>" << endl; }
}

void GrammarAnalyzer::printSentence() {
	if (lex.sym().type != PRINTFTK) {
		f.handleFault(lex.lineNumber(), "缺少printf标识符");
		// todo handle-fault;
		//f.terminate();
		throw 0;
	}
	getNextSym();
	//读取printf符号完成

	if (lex.sym().type != LPARENT) {
		f.handleFault(lex.lineNumber(), "缺少（");
		// todo handle-fault
		//f.terminate();
		throw 0;
	}
	getNextSym();
	//读取左括号完成

	if (lex.sym().type == STRCON) {
		/*how to handle the string*/
		getNextSym();
		if (course) { out << "<字符串>" << endl; }
		if (lex.sym().type == COMMA) {
			getNextSym();
			//读取逗号完成
			expression();
			//直接读取表达式
		}
	}
	else { 
		expression();
		//直接读取表达式
	}
	
	if (lex.sym().type != RPARENT) {
		f.handleCourseFault(lex.lineNumber(), NORPARENT);
		f.handleFault(lex.lineNumber(), "缺少)");
	}
	else {
		getNextSym();
	}
	if (course) { out << "<写语句>" << endl; }
}

void GrammarAnalyzer::returnSentence() {
	if (lex.sym().type != RETURNTK) {
		f.handleFault(lex.lineNumber(), "缺少RETURN标识符");
		// todo handle-fault;
		//f.terminate();
		throw 0;
	}
	getNextSym();
	//读取return标识符
	if (lex.sym().type == LPARENT) {
		getNextSym();
		//完成读取左括号
		expression();
		if (lex.sym().type != RPARENT) {
			f.handleCourseFault(lex.lineNumber(), NORPARENT);
			f.handleFault(lex.lineNumber(), "缺少)");
		}
		else {
			getNextSym();
		}
		//完成读取右括号
	}
	if (course) { out << "<返回语句>" << endl; }
}

void GrammarAnalyzer::ifSentence() {
	if (lex.sym().type != IFTK) {
		f.handleFault(lex.lineNumber(), "缺少if标识符");
		// todo handlefault
		//f.terminate();
		throw 0;
	}
	getNextSym();
	//完成if标识符读取

	if (lex.sym().type != LPARENT) {
		f.handleFault(lex.lineNumber(), "缺少（");
		// todo handle-fault
		//f.terminate();
		throw 0;
	}
	getNextSym();
	//完成读取左括号

	condition();
	//读取条件

	if (lex.sym().type != RPARENT) {
		f.handleCourseFault(lex.lineNumber(), NORPARENT);
		f.handleFault(lex.lineNumber(), "缺少)");
	}
	else {
		getNextSym();
	}
	//读取右括号

	sentence();
	//语句
	if (lex.sym().type == ELSETK) {
		getNextSym();
		//读取else完成
		sentence();
		//读取语句
	}
	if (course) { out << "<条件语句>" << endl; }
}

void GrammarAnalyzer::condition() {
	expression();
	//表达式
	if (lex.sym().type == LSS || lex.sym().type == LEQ
		|| lex.sym().type == GRE || lex.sym().type == GEQ
		|| lex.sym().type == EQL || lex.sym().type == NEQ) {
		getNextSym();
		//读取关系运算符完成
		expression();
	}
	if (course) { out << "<条件>" << endl; }
}

void GrammarAnalyzer::loopSentence() {
	bool error = false;
	if (lex.sym().type == WHILETK) {
		getNextSym();
		//读取while符号

		if (lex.sym().type != LPARENT) {
			f.handleFault(lex.lineNumber(), "缺少（");
			// todo handle-fault
			//f.terminate();
			throw 0;
		}
		getNextSym();
		//读取左括号

		condition();
		//读取条件语句
		if (lex.sym().type != RPARENT) {
			f.handleCourseFault(lex.lineNumber(), NORPARENT);
			f.handleFault(lex.lineNumber(), "缺少)");
		}
		else {
			getNextSym();
		}
		//读取右括号完成
		sentence();
	}
	else if (lex.sym().type == DOTK) {
		getNextSym();
		//读取do符号完成
		sentence();
		//读取句子
		if (lex.sym().type != WHILETK) {
			f.handleFault(lex.lineNumber(), "缺少while符号");
			// todo handle fault
			//f.terminate();
			throw 0;
		}
		else {
			getNextSym();
		}
		//读取while符号
		if (lex.sym().type != LPARENT) {
			f.handleFault(lex.lineNumber(), "缺少（");
			// todo handle-fault
			//f.terminate();
			throw 0;
		}
		getNextSym();
		//读取左括号
		condition();
		//读取条件语句
		if (lex.sym().type != RPARENT) {
			f.handleCourseFault(lex.lineNumber(), NORPARENT);
			f.handleFault(lex.lineNumber(), "缺少)");
		}
		else {
			getNextSym();
		}
		//读取右括号
	}
	else if (lex.sym().type == FORTK) {
		getNextSym();
		//读取for符号完成
		if (lex.sym().type != LPARENT) {
			f.handleFault(lex.lineNumber(), "缺少（");
			// todo handle-fault
			//f.terminate();
			throw 0;
		}
		getNextSym();
		//读取左括号完成
		if (lex.sym().type != IDENFR) {
			f.handleFault(lex.lineNumber(), "需要标识符");
			// todo handle-fault
			//f.terminate();
			throw 0;
		}
		string varname1 = lex.sym().str;
		getNextSym();
		//读取标识符完成
		SymbolEntry* entry1 = table.getSymbolByName(currentScope, varname1);
		if (entry1 == NULL) {
			f.handleCourseFault(lex.lineNumber(), UNDEFINED);
			f.handleFault(lex.lineNumber(), "未定义的标识符" + varname1);
			error = true;
		}
		if (!error && entry1->type != TYPEINT && entry1->type != TYPECHAR) {
			f.handleCourseFault(lex.lineNumber(), TYPEERROR);
			f.handleFault(lex.lineNumber(), "不合法的变量类型");
			error = true;
		}
		if (lex.sym().type != ASSIGN) {//读取赋值运算符
			f.handleFault(lex.lineNumber(), "需要=");
			// todo handlefault
			throw 0;
		}
		else {
			getNextSym();
		}
		//读取赋值符完成
		expression();
		//读取所赋的表达式
		if (lex.sym().type != SEMICN) {
			f.handleCourseFault(lex.lineNumber(), NOSEMICN);
			f.handleFault(lex.lineNumber(), "需要;");
		}
		else {
			getNextSym();
		}
		//读取分号完成
		condition();
		//读取条件完成
		if (lex.sym().type != SEMICN) {
			f.handleCourseFault(lex.lineNumber(), NOSEMICN);
			f.handleFault(lex.lineNumber(), "需要;");
		}
		else {
			getNextSym();
		}
		//读取分号完成
		if (lex.sym().type != IDENFR) {
			f.handleFault(lex.lineNumber(), "需要标识符");
			// todo handle-fault
			//f.terminate();
			throw 0;
		}
		string varname2 = lex.sym().str;
		getNextSym();
		//读取标识符完成
		SymbolEntry* entry2 = table.getSymbolByName(currentScope, varname2);
		if (entry2 == NULL) {
			f.handleCourseFault(lex.lineNumber(), UNDEFINED);
			f.handleFault(lex.lineNumber(), "未定义的标识符" + varname2);
			error = true;
		}
		if (!(entry2==NULL) && entry2->type != TYPEINT && entry2->type != TYPECHAR) {
			f.handleCourseFault(lex.lineNumber(), TYPEERROR);
			f.handleFault(lex.lineNumber(), "不合法的变量类型");
			error = true;
		}
		if (lex.sym().type != ASSIGN) {//读取赋值运算符
			f.handleFault(lex.lineNumber(), "需要=");
			// todo handlefault
			//f.terminate();
			throw 0;
		}
		else {
			getNextSym();
		}
		//读取赋值运算符完成
		if (lex.sym().type != IDENFR) {
			f.handleFault(lex.lineNumber(), "需要标识符");
			// todo handle-fault
			//f.terminate();
			throw 0;
		}
		string varname3 = lex.sym().str;
		SymbolEntry* entry3 = table.getSymbolByName(currentScope, varname3);
		if (entry3 == NULL) {
			f.handleCourseFault(lex.lineNumber(), UNDEFINED);
			f.handleFault(lex.lineNumber(), "未定义的标识符" + varname3);
			error = true;
		}
		if (!(entry3 == NULL) && entry3->type != TYPEINT && entry3->type != TYPECHAR) {
			f.handleCourseFault(lex.lineNumber(), TYPEERROR);
			f.handleFault(lex.lineNumber(), "不合法的变量类型");
			error = true;
		}
		getNextSym();
		//读取标识符完成
		if (lex.sym().type != PLUS && lex.sym().type != MINU) {
			f.handleFault(lex.lineNumber(), "需要运算符");
			// todo handlefault
			//f.terminate();
			throw 0;
		}
		Lexical op = lex.sym().type;
		getNextSym();
		//读取加减号完成
		if (lex.sym().type != INTCON) {
			f.handleFault(lex.lineNumber(), "需要步长");
			// todo handlefault
			//f.terminate();
			throw 0;
		}
		int step = lex.sym().value;
		getNextSym();
		//读取步长完成
		if (course) { out << "<无符号整数>" << endl; }
		if (course) { out << "<步长>" << endl; }
		if (lex.sym().type != RPARENT) {
			f.handleCourseFault(lex.lineNumber(), NORPARENT);
			f.handleFault(lex.lineNumber(), "缺少)");
		}
		else {
			getNextSym();
		}
		//读取右括号完成
		sentence();
	}
	else {
		f.handleFault(lex.lineNumber(), "非法的循环语句");
		// todo handle fault;
		//f.terminate();
		throw 0;
	}
	if (course) { out << "<循环语句>" << endl; }
}

void GrammarAnalyzer::sentence() {
	try {
		if (lex.sym().type == IFTK) {
			ifSentence();
		}
		else if (lex.sym().type == WHILETK || lex.sym().type == DOTK ||
			lex.sym().type == FORTK) {
			loopSentence();
		}
		else if (lex.sym().type == LBRACE) {
			getNextSym();
			//读取左大括号完成
			sentenceSeries();
			if (lex.sym().type != RBRACE) {
				f.handleFault(lex.lineNumber(), "缺少}");
				// TODO HANDLEFAULT
				f.terminate();
			}
			getNextSym();
			//读取右大括号完成
		}
		else {
			if (lex.sym().type == IDENFR) {
				assignAndCall();
			}
			else if (lex.sym().type == PRINTFTK) {
				printSentence();
			}
			else if (lex.sym().type == SCANFTK) {
				scanSentence();
			}
			else if (lex.sym().type == RETURNTK) {
				returnSentence();
			}
			else if (lex.sym().type != SEMICN) {
				f.handleFault(lex.lineNumber(), "非法的语句开头");
				// handle fault
				f.terminate();
			}
			if (lex.sym().type != SEMICN) {
				f.handleCourseFault(lex.lineNumber(), NOSEMICN);
				f.handleFault(lex.lineNumber(), "需要;");
			}
			else {
				getNextSym();
			}
		}
	}
	catch (int e) {
		toNextSemicon();
		getNextSym();
	}
	if (course) { out << "<语句>" << endl; }
}

void GrammarAnalyzer::sentenceSeries() {
	while (1) {
		if (lex.sym().type == IFTK ||
			lex.sym().type == WHILETK || lex.sym().type == DOTK || lex.sym().type == FORTK||
			lex.sym().type == LBRACE ||
			lex.sym().type == IDENFR ||
			lex.sym().type == PRINTFTK ||
			lex.sym().type == SCANFTK ||
			lex.sym().type == RETURNTK ||
			lex.sym().type == SEMICN) {
			sentence();
		}
		else {
			break;
		}
	}
	if (course) { out << "<语句列>" << endl; }
}

void GrammarAnalyzer::programme() {
	constDeclearation();
	while (1) {
		if (lex.sym().type == INTTK || lex.sym().type == CHARTK) {
			Lexical judge = declearationHeader();
			if (judge == LPARENT) {
				break;
			}
		}
		else {
			break;
		}
	}
	while (1) {
		if (lex.sym().type == VOIDTK) {
			if (globalVariableDeclearation) {
				if (course) { out << "<变量说明>" << endl; }
				globalVariableDeclearation = false;
				//如果这是我们碰见的第一个函数
				//并且前面出现了全局变量声明，那就要输出变量声明这一问题
			}
			getNextSym();
			if (lex.sym().type == IDENFR) {
				voidFunctionDefination();
			}
			else if (lex.sym().type == MAINTK) {
				mainFunctionDefination();
			}
			else {
				// todo handle fault
				f.handleFault(lex.lineNumber(), "非法");
				f.terminate();
			}
		}
		else if (lex.sym().type == CHARTK || lex.sym().type == INTTK) {
			nonVoidFunctionDefination();
		}
		else {
			break;
		}
	}
	if (course) { out << "<程序>" << endl; }
}

#include"GrammarAnalyzer.h"

/*构造器函数*/
GrammarAnalyzer::GrammarAnalyzer(FaultHandler& _f, SymbolTable& _s, LexicalAnalyzer& _l,MidCodeFramework& _raw,string file)
:f(_f),table(_s),raw(_raw), lex(_l) {
	if (file == "") {
		file = "/dev/null";
	}
	out.open(file,ios_base::trunc|ios_base::out);
	currentScope="";
}

/*获取下一词素，并输出上一个词素*/
Lexical GrammarAnalyzer::getNextSym() {
	if (lexicalTest) { lex.printResult(out); }
	return lex.getNextSym();
}

/*一直跳到下一个分号为止,用于局部化处理*/
void GrammarAnalyzer::toNextSemicon() {
	while (1) {
		Lexical tmp = lex.getNextSym();
		if (tmp == END || tmp == SEMICN) {
			break;
		}
	}
}
/*一直跳到下一个分号为止,用于局部化处理*/
void GrammarAnalyzer::toNextBrace() {
	while (1) {
		Lexical tmp = lex.getNextSym();
		if (tmp == END || tmp == RBRACE) {
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
	return value*sign;
}

/*<常量声明>
不会抛出异常，在此函数中进行局部化处理*/
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
		}
	}
	if (course&&!init) {out << "<常量说明>" << endl;}
}

/*<常量定义>*/
void GrammarAnalyzer::constDefination() {
	bool init = true, error = false;;
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
		SymbolEntry* entry = table.addSymbol(currentScope, varname, false);
		if (entry == NULL) {
			f.handleCourseFault(lex.lineNumber(), REDEFINED);
			f.handleFault(lex.lineNumber(), "常量定义中有符号重定义");
			error = true;
		}//检查是否为重定义
		
		if (lex.sym().type != ASSIGN) {//获取等号
			f.handleFault(lex.lineNumber(), "需要=");
			throw 0;
		}
		getNextSym();
		//读取等号完成
		if (vartype == INTTK) {/*根据类型读取值并写入符号表*/
			int intvalue;
			try {
				intvalue = integer();
			}
			catch (int e) {
				f.handleCourseFault(lex.lineNumber(), ILLEGALCONSTDEFINITION);
				f.handleFault(lex.lineNumber(), "常量定义中=后面只能是整型或字符型常量");
				throw 0;
			}
			//读取整数完成
			if (!error) {
				entry->type = TYPEINTCONST;
				entry->initValue = intvalue;
				//写入符号表完成
			}
		}
		else {
			if (lex.sym().type != CHARCON) {
				f.handleCourseFault(lex.lineNumber(), ILLEGALCONSTDEFINITION);
				f.handleFault(lex.lineNumber(), "常量定义中=后面只能是整型或字符型常量");
				throw 0;
			}
			char charvalue = (char)(lex.sym().value);
			getNextSym();
			//读取字符完成
			if (!error) {
				entry->type = TYPECHARCONST;
				entry->initValue = charvalue;
				//写入符号表完成
			}
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
}
/*<变量定义>,供各种情况使用
第一个参数代表是否是被包装器调用的，第二个参数传入包装器预读的变量类型和变量名称
*/
void GrammarAnalyzer::variableDefination(bool wraper,Lexical type,string name) {
	Lexical vartype;
	int error = false;
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
		//完成标识符读取

	LABEL:	
		SymbolEntry* entry = table.addSymbol(currentScope, varname, false);//插入符号表
		if (entry == NULL) {
			f.handleCourseFault(lex.lineNumber(), REDEFINED);
			f.handleFault(lex.lineNumber(), varname + "重定义");
			error = true;
		}
		int dimension = 0;
		if (lex.sym().type == LBRACK) {//判断是不是数组元素
			if (currentScope != "") {
				inlineable = false;
			}
			getNextSym();
			if (lex.sym().type != INTCON) {
				f.handleCourseFault(lex.lineNumber(), ILLEGALINDEX);
				f.handleFault(lex.lineNumber(), "需要维数:数组元素的下标只能是整型表达式");
				throw 0;
			}
			dimension = lex.sym().value;
			getNextSym();
			//读取维数完成
			if (course) { out << "<无符号整数>"<<endl; }
			if(lex.sym().type!=RBRACK){
				f.handleCourseFault(lex.lineNumber(), NORBRACK);
				f.handleFault(lex.lineNumber(), "缺少]");
			}
			else {
				getNextSym();
			}
			//读取右中括号完成
		}

		
		if(!error){
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
	hasReturned = false;
	try {
		Lexical retType;
		string functionName;
		declearationHeader(retType,functionName);
		//获取变量名和返回类型完成
		if (lex.sym().type != LPARENT) {//参数表的左括号
			f.handleFault(lex.lineNumber(), "缺少参数表");
			//todo 错误处理待定;
			f.terminate();
		}
		getNextSym();
		//处理左括号完成
		raw.functionStart(functionName);
		SymbolEntry* entry = table.addSymbol(currentScope, functionName, true);//插入符号表项
		if (entry == NULL) {
			f.handleCourseFault(lex.lineNumber(), REDEFINED);
			f.handleFault(lex.lineNumber(), "函数名重定义" + functionName);
		}
		else {
			entry->link->returnType = retType == INTTK ? RETINT : RETCHAR;
			entry->type = TYPEFUNCTION;
		}
		if (entry != NULL) {
			raw.midCodeInsert(MIDFUNC, MIDUNUSED,
				entry->id, false,
				(retType == INTTK ? MIDINT : MIDCHAR), false, MIDNOLABEL);
		}
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
		if (!hasReturned) {
			f.handleCourseFault(lex.lineNumber(), ILLEGALRETURNINNONVOID);
			f.handleFault(lex.lineNumber(), "有返回值函数缺少返回语句");
		}

		if (lex.sym().type != RBRACE) {
			f.handleFault(lex.lineNumber(), "缺少}");
			throw 0;
		}
		getNextSym();
		//获取右大括号
		raw.functionEnd();
	}
	catch (int e) {
		toNextBrace();
		getNextSym();
	}
	if (course) { out << "<有返回值函数定义>" << endl; }
}
/*有返回值函数定义，供包装器调用使用，不会抛出异常*/
void GrammarAnalyzer::nonVoidFunctionDefination(Lexical retType,string functionName) {
	hasReturned = false;
	raw.functionStart(functionName);
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
		else {
			entry->link->returnType = retType == INTTK ? RETINT : RETCHAR;
			entry->type = TYPEFUNCTION;
			//写入符号表
		}
		if (entry != NULL) {
			raw.midCodeInsert(MIDFUNC, MIDUNUSED,
				entry->id, false,
				(retType == INTTK ? MIDINT : MIDCHAR), false, MIDNOLABEL);
		}
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
		if (!hasReturned) {
			f.handleCourseFault(lex.lineNumber(), ILLEGALRETURNINNONVOID);
			f.handleFault(lex.lineNumber(), "有返回值函数缺少返回语句");
		}
		if (lex.sym().type != RBRACE) {//右大括号
			f.handleFault(lex.lineNumber(), "缺少{");
			throw 0;
		}
		getNextSym();
		//右大括号读取完成
		raw.functionEnd();
	}
	catch (int e) {
		toNextBrace();
		getNextSym();
	}
	
	if (course) { out << "<有返回值函数定义>" << endl; }
}

/*无返回值定义，不会抛出异常*/
void GrammarAnalyzer::voidFunctionDefination() {
	hasReturned = false;
	try {
		//这个地方没有void符号因为已经被programme预读掉了
		if (lex.sym().type != IDENFR) {
			f.handleFault(lex.lineNumber(), "缺少函数名");
			throw 0;
		}
		string functionName = lex.sym().str;
		getNextSym();
		//函数名读取完成
		SymbolEntry* entry = table.addSymbol(currentScope, functionName, true);
		if (entry == NULL) {
			f.handleCourseFault(lex.lineNumber(), REDEFINED);
			f.handleFault(lex.lineNumber(), "函数名重定义" + functionName);
		}
		else {
			entry->link->returnType = RETVOID;
			entry->type = TYPEFUNCTION;
		}
		raw.functionStart(functionName);
		if (lex.sym().type != LPARENT) {
			f.handleFault(lex.lineNumber(), "缺少参数表");
			throw 0;
		}

		getNextSym();
		//读走了左括号
		if (entry != NULL) {
			raw.midCodeInsert(MIDFUNC, MIDUNUSED,
				entry->id, false,
				MIDVOID, false, MIDNOLABEL);
		}
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
		if (entry != NULL) {
			raw.midCodeInsert(MIDRET, MIDUNUSED,
				MIDUNUSED, false,
				MIDUNUSED, false, MIDNOLABEL);
		}
		if (lex.sym().type != RBRACE) {
			f.handleFault(lex.lineNumber(), "缺少}");
			throw 0;
		}
		getNextSym();
		//读取右大括号完成
		raw.functionEnd();
	}
	catch (int e) {
		toNextBrace();
		getNextSym();
	}
	if (course) { out << "<无返回值函数定义>" << endl; }
}

/*<主函数>，不会抛出异常*/
void GrammarAnalyzer::mainFunctionDefination() {
	hasReturned = false;
	try {
		//这个地方没有void符号因为已经被programme预读掉了
		if (lex.sym().type != MAINTK) {
			f.handleFault(lex.lineNumber(), "缺少函数名");
			throw 0;
		}
		string functionName = "main";
		getNextSym();
		//读取main标识符
		raw.functionStart(functionName);
		SymbolEntry* entry = table.addSymbol(currentScope, functionName, true);
		if (entry == NULL) {
			f.handleCourseFault(lex.lineNumber(), REDEFINED);
			f.handleFault(lex.lineNumber(), "main函数重定义" + functionName);
		}
		else {
			entry->link->returnType = RETVOID;
			entry->type = TYPEFUNCTION;
			// 写入符号表完成
		}

		if (entry != NULL) {
			raw.midCodeInsert(MIDFUNC, MIDUNUSED,
				entry->id, false,
				MIDVOID, false, MIDNOLABEL);
		}

		if (lex.sym().type != LPARENT) {
			f.handleFault(lex.lineNumber(), "缺少参数表");
			throw 0;
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
			throw 0;
		}
		getNextSym();
		//读入右大括号完成
		raw.functionEnd();
	}
	catch (int e) {
		toNextBrace();
		getNextSym();
	}
	if (course) { out << "<主函数>" << endl; }
}

/*<声明头部>,供非变量声明区的有返回值函数调用,会抛出异常*/
void GrammarAnalyzer::declearationHeader(Lexical& retType,string&functionName) {
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
	functionName = lex.sym().str;
	getNextSym();
	//读取函数名完成
	if (course) { out << "<声明头部>" << endl; }
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
		f.handleFault(lex.lineNumber(), "缺少标识符");
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
		try {
			variableDeclearation(type, name);
		}
		catch (int e) {
			toNextSemicon();
			getNextSym();
		}
	}
	else {
		f.handleFault(lex.lineNumber(), "非法的标识符声明");
		throw 0;
	}
	return lex.sym().type;
}

/*<参数表>*/
void GrammarAnalyzer::parameterList(SymbolEntry* entry) {
	bool init = true, error = false;
	int paraNum = 0;
	if (entry == NULL) { error = true; }
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
		SymbolEntry* paraentry=NULL;
		if (!error) {
			entry->link->paras.push_back(paratype);//存储参数类型
			string scope = entry->name;//获取函数作用域
			 paraentry = table.addSymbol(scope, paraname, false);
			if (paraentry == NULL) {
				f.handleCourseFault(lex.lineNumber(), REDEFINED);
				f.handleFault(lex.lineNumber(), "参数名重定义" + paraname);
			}
			else {
				paraentry->type = paratype;
				paraentry->isParameter = true;
				entry->link->paraIds.push_back(paraentry->id);//存储参数是谁
				//写入符号表
			}
		}
		getNextSym();
		//读取参数名称完成
		paraNum++;
		if (paraentry != NULL) {
			raw.midCodeInsert(MIDPARA, MIDUNUSED,
				paraentry->id, false,
				paratype == TYPEINT ? MIDINT : MIDCHAR, false, MIDNOLABEL);
		}
	}
	if (!error) { entry->link->paraNum = paraNum; }
	if (course) { out << "<参数表>" << endl; }
}

/*<复合语句>*/
 void GrammarAnalyzer::compoundSentence(){
	inlineable = inlineSwitch;
	if (currentScope == "main") {
		inlineable = false;
	}
	constDeclearation();
	variableDeclearation();
	sentenceSeries();
	SymbolEntry* entry = table.getSymbolByName("", currentScope);
	if (inlineable == true) {
		entry->link->inlineable = true;
	}
	if (course) { out << "<复合语句>" << endl; }
}

/*<因子>*/
ReturnBundle GrammarAnalyzer::factor() {
	bool error = false;
	ReturnBundle res;
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

		if (lex.sym().type == LBRACK) {
			//数组
			inlineable = false;
			if (!error && entry->type != TYPECHARARRAY && entry->type != TYPEINTARRAY) {
				f.handleFault(lex.lineNumber(), varname + "变量不是数组类型");
				error = true;
			}
			//检索符号表，检查该符号是否为数组类型
			getNextSym();
			//完成读取中括号
			ReturnBundle index;
			try {
				index=expression();
				if (index.isChar) {
					throw 0;
				}
			}
			catch (int e) {
				f.handleCourseFault(lex.lineNumber(), ILLEGALINDEX);
				f.handleFault(lex.lineNumber(), "数组元素的下标只能是整型表达式");
			}
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
			if (!error&&entry->type == TYPECHARARRAY) {
				res.isChar = true;
			}
			if (!error) {
				res.id = MidCode::tmpVarAlloc();
				res.isImmediate = false;
			}
			if (!error) {
				raw.midCodeInsert(MIDARRAYGET, res.id,
					entry->id, false, index.id, index.isImmediate, MIDNOLABEL);
				//生成中间代码
			}
		}
		else if (lex.sym().type == LPARENT) {
			//有返回值函数
			res=functionCall(varname,true);
		}
		else {//标识符
			if (!error && !(entry->type == TYPECHAR || entry->type == TYPEINT ||
				entry->type == TYPECHARCONST || entry->type == TYPEINTCONST)) {
				f.handleCourseFault(lex.lineNumber(), PARATYPEERROR);
				f.handleFault(lex.lineNumber(), "参数中非法的类型");
				error = true;
			}
			if (!error&&(entry->type == TYPECHAR || entry->type == TYPECHARCONST)) {
				res.isChar = true;
			}
			if (!error) {
				if (constantSubstitutionSwitch&&(entry->type == TYPECHARCONST || entry->type == TYPEINTCONST)) {
					res.id = entry->initValue;
					res.isImmediate = true;
				}
				else {
					res.id = entry->id;
					res.isImmediate = false;
				}
			}
			//生成中间代码
		}
	}
	else if (lex.sym().type == LPARENT) {//（<表达式>）
		getNextSym();
		//完成读取左括号

		ReturnBundle tmp=expression();
		//读取表达式
		if (lex.sym().type != RPARENT) {
			f.handleCourseFault(lex.lineNumber(), NORPARENT);
			f.handleFault(lex.lineNumber(),"缺少）");
		}
		else {
			getNextSym();
		}
		res.id = tmp.id;
		res.isImmediate = tmp.isImmediate;
	}
	else if (lex.sym().type == INTCON|| lex.sym().type == PLUS|| lex.sym().type == MINU) {
		//整数（由+-或整数开头）
		int value = integer();
		res.id = value;
		res.isImmediate = true;
		//读取整数
	}
	else if (lex.sym().type == CHARCON) {
		char c = (char)(lex.sym().value);
		getNextSym();
		res.isChar = true;
		res.id = c;
		res.isImmediate = true;
		//完成读取字符
	}
	else {
		f.handleFault(lex.lineNumber(), "不被识别的表达式");
		throw 0;
	}
	if (course) { out << "<因子>" << endl; }
	return res;
}

/*<项>*/
ReturnBundle GrammarAnalyzer::term() {
	ReturnBundle res;
	ReturnBundle res1=factor();
	res.isChar = res1.isChar;
	res.id = res1.id;
	res.isImmediate = res1.isImmediate;
	while (1) {
		if (lex.sym().type != MULT && lex.sym().type != DIV) {
			break;
		}
		res.isChar = false;
		MidCodeOp op = lex.sym().type==MULT?MIDMULT:MIDDIV;
		getNextSym();
		//读取完运算符号
		ReturnBundle res2=factor();
		//读取因子
		int tmpVar = MidCode::tmpVarAlloc();
		raw.midCodeInsert(op, tmpVar, res.id, res.isImmediate,
			res2.id, res2.isImmediate, MIDNOLABEL);
		res.id = tmpVar;
		res.isImmediate = false;
	}
	if (course) { out << "<项>" << endl; }
	return res;
}

/*<表达式>*/
ReturnBundle GrammarAnalyzer::expression() {
	ReturnBundle res;
	int sign = 1;
	bool calculated = false;//标记是不是只有一项，供类型判断使用
	if (lex.sym().type == PLUS || lex.sym().type == MINU) {
		if (lex.sym().type == MINU) {
			sign = -1;
		}
		getNextSym();
		calculated = true;
	}
	//读取开头的加减号完成

	ReturnBundle res1= term();
	res.isChar = res1.isChar;

	res.id = res1.id;
	res.isImmediate = res1.isImmediate;
	if (sign == -1) {
		if (res.isImmediate) {
			res.id = -res.id;
		}
		else {
			int tmpVar = MidCode::tmpVarAlloc();
			raw.midCodeInsert(MIDNEGATE, tmpVar,
				res.id, res.isImmediate, MIDUNUSED, false, MIDNOLABEL);
			res.id = tmpVar;
			res.isImmediate = false;
		}
	}
	//读取第一项
	while (1) {
		if (lex.sym().type != PLUS && lex.sym().type != MINU) {
			break;
		}
		MidCodeOp op = lex.sym().type==PLUS?MIDADD:MIDSUB;
		getNextSym();
		//读取运算符号完成
		ReturnBundle res2=term();
		calculated = true;

		int tmpVar = MidCode::tmpVarAlloc();
		raw.midCodeInsert(op, tmpVar,
			res.id, res.isImmediate, res2.id, res2.isImmediate, MIDNOLABEL);
		res.id = tmpVar;
		res.isImmediate = false;
	}
	res.isChar = res.isChar&&!calculated;
	if (course) { out << "<表达式>" << endl; }
	return res;
}

/*针对赋值语句和函数调用语句的预读分支*/
void GrammarAnalyzer::assignAndCall() {
	if (lex.sym().type != IDENFR) {
		f.handleFault(lex.lineNumber(), "需要标识符");
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
		f.handleFault(lex.lineNumber(), "assignAndCall无意义的符号");
		throw 0;
	}
}
/*<赋值语句>，已经过预读*/
void GrammarAnalyzer::assignSentence(string varname) {
	bool error = false, leftIsChar = false;
	SymbolEntry* entry = table.getSymbolByName(currentScope, varname);
	if (entry == NULL) {
		f.handleCourseFault(lex.lineNumber(), UNDEFINED);
		f.handleFault(lex.lineNumber(), "未定义的变量" + varname);
		error = true;
	}
	else if (!error && (entry->type == TYPECHARCONST || entry->type == TYPEINTCONST)) {
		f.handleCourseFault(lex.lineNumber(), MODIFYCONST);
		f.handleFault(lex.lineNumber(), "试图修改常量的值");
		error = true;
	}
	else if (!error && entry->type == TYPEFUNCTION) {
		f.handleFault(lex.lineNumber(), "该文法中函数调用不能作为左值");
		error = true;
	}
	int target = -1;
	if (entry!=NULL) {
		target = entry->id;
	}
	bool isArray = false;
	ReturnBundle indexBundle;
	//检查变量合法性
	if (lex.sym().type == LBRACK) {//左值是数组
		inlineable = false;
		if (!error && entry->type != TYPECHARARRAY && entry->type != TYPEINTARRAY) {
			f.handleFault(lex.lineNumber(), varname + "变量不是数组类型");
			error = true;
		}
		//检查是不是数组类型的变量
		getNextSym();
		
		//读取完了左括号
		try {
			indexBundle = expression();
			if (indexBundle.isChar) {
				throw 0;
			}
		}
		catch (int e) {
			f.handleCourseFault(lex.lineNumber(), ILLEGALINDEX);
			f.handleFault(lex.lineNumber(), "数组元素的下标只能是整型表达式");
		}
		//读取完成下标

		if (lex.sym().type != RBRACK) {
			f.handleCourseFault(lex.lineNumber(), NORBRACK);
			f.handleFault(lex.lineNumber(), "缺少]");
		}
		else {
			getNextSym();
		}
		//完成读取右括号
		isArray = true;
	}
	if (!error&&(entry->type == TYPECHARARRAY || entry->type == TYPECHAR)) {
		leftIsChar = true;
	}
	if (lex.sym().type != ASSIGN) {//读取赋值运算符
		f.handleFault(lex.lineNumber(), "需要=");
		throw 0;
	}
	else {
		getNextSym();
	}
	//完成读取赋值运算符
	ReturnBundle res=expression();
	//读取所赋的表达式
	if (res.isChar != leftIsChar) {
		f.handleFault(lex.lineNumber(), "赋值语句中类型错误");
	}

	if (isArray) {
		raw.midCodeInsert(MIDARRAYWRITE, target,
			indexBundle.id, indexBundle.isImmediate, res.id, res.isImmediate, MIDNOLABEL);
	}
	else {
		if (!res.isImmediate&& res.id < -1) {
			raw.container.v[raw.container.v.size() - 1].target = target;
		}
		else {
			raw.midCodeInsert(MIDASSIGN, target,
				res.id, res.isImmediate, MIDUNUSED, false, MIDNOLABEL);
		}
		
	}
	//生成中间代码
	if (course) { out << "<赋值语句>"<<endl; }
}

/*函数调用，若mustReturn是真意味着必须有返回值，否则有无返回值皆可，已被预读符号*/
ReturnBundle GrammarAnalyzer::functionCall(string name,bool mustReturn) {
	inlineable = false;
	bool error = false;
	ReturnBundle res;
	SymbolEntry* entry = table.getSymbolByName(currentScope, name);
	if (entry == NULL) {//检查是否已定义
		f.handleCourseFault(lex.lineNumber(), UNDEFINED);
		f.handleFault(lex.lineNumber(), "未定义的函数"+name);
		error = true;
	}
	//检查是否有变量信息

	if (!error&&entry->type != TYPEFUNCTION) {//检查是否为函数
		f.handleFault(lex.lineNumber(), "不是函数");
		error = true;
		
	}
	
	if (!error&&mustReturn && entry->link->returnType == RETVOID) {//检查是否有返回值
		f.handleFault(lex.lineNumber(), "不是有返回值函数");
		error = true;
	}
	if (!error && entry->link->returnType == RETCHAR) {
		res.isChar = true;
	}
	if (lex.sym().type != LPARENT) {//读取（
		f.handleFault(lex.lineNumber(), "缺少(");
		throw 0;
	}
	getNextSym();
	//完成左括号处理


	vector<ReturnBundle>paras;
	if (!error) {
		paras=parameterValueList(entry, entry->link->inlineable);//读取实参列表
	}

	if (lex.sym().type != RPARENT) {
		f.handleCourseFault(lex.lineNumber(), NORPARENT);
		f.handleFault(lex.lineNumber(), "缺少)");
		throw 0;
	}
	else {
		getNextSym();
	}
	//右大括号读取完成
	if (!error) {
		if (!entry->link->inlineable) {
			raw.midCodeInsert(MIDCALL, MIDUNUSED,
				entry->id, false, MIDUNUSED, false, MIDNOLABEL);
			if (entry->link->returnType != RETVOID) {
				res.id = MidCode::tmpVarAlloc();
				raw.midCodeInsert(MIDASSIGN, res.id, -1, false,
					MIDUNUSED, false, MIDNOLABEL);
				res.isImmediate = false;
			}
		}
		else {
			//内联
			//	todo:返回类型??
			res.id = MidCode::tmpVarAlloc();
			res.isImmediate = false;
			int returnTmp= MidCode::tmpVarAlloc();
			vector<MidCode>midcode = raw.inlinedSimpleFunction(entry->name, paras, returnTmp);
			raw.midCodeInsert(midcode);
			raw.midCodeInsert(MIDASSIGN, res.id,
				returnTmp, false, MIDUNUSED, false, MIDNOLABEL);
		}
	}
	if (course) {
		if (!error&&entry->link->returnType == RETVOID) {
			out << "<无返回值函数调用语句>"<<endl;
		}
		else {
			out << "<有返回值函数调用语句>" << endl;
		}
	}
	return res;
}

/*<值参数表>*/
vector<ReturnBundle> GrammarAnalyzer::parameterValueList(SymbolEntry* entry,bool inlined) {
	int paraNum = 0;
	bool init = true, error = false;
	vector<vector<int>> codeIndex;//存储中间代码的下标
	vector<ReturnBundle>returnBundles;//存储returnBundles
	if (entry == NULL) {
		error = true;
	}
	while (1) {
		if (lex.sym().type == RPARENT) {
			break;
		}
		if (!init) {
			if (lex.sym().type != COMMA) {
				f.handleFault(lex.lineNumber(), "缺少,");
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
		int probe1 = raw.getIndex();//获取参数值计算开始的代码（超尾式下标）
		ReturnBundle res1=expression();
		returnBundles.push_back(res1);//获取参数值计算结束的代码（超尾式下标）
		int probe2 = raw.getIndex();

		codeIndex.push_back({ probe1,probe2 });//记录起止点
		//读取实参的表达式
		bool leftIsChar=false;
		if (!error&&paraNum < entry->link->paraNum) {
			if (entry->link->paras[paraNum] == TYPECHAR) {
				leftIsChar = true;
			}
			if (leftIsChar != res1.isChar) {
				f.handleCourseFault(lex.lineNumber(), PARATYPEERROR);
				f.handleFault(lex.lineNumber(), "实参类型不正确");
			}
		}
		paraNum++;
	}
	if (!error&&entry->link->paraNum != paraNum) {
		f.handleCourseFault(lex.lineNumber(), PARANUMERROR);
		f.handleFault(lex.lineNumber(), "函数参数个数不正确");
		throw 0;
	}

	if (codeIndex.size() > 0) {
		vector<vector<MidCode>>codeSegment;//保存切出来的代码片段
		for (int i = 0; i < codeIndex.size(); i++) {
			vector<MidCode>paraCode(raw.getIterator(codeIndex[i][0]), raw.getIterator(codeIndex[i][1]));
			codeSegment.push_back(paraCode);
		}
		int eraseStart = codeIndex[0][0];
		int eraseEnd = codeIndex[codeIndex.size() - 1][1];
		raw.erase(eraseStart, eraseEnd);
		//调整参数计算顺序
		for (int i = codeSegment.size() - 1; i >= 0; i--) {
			raw.midCodeInsert(codeSegment[i]);
		}
	}
	if (!inlined) {
		for (int i = 0; i < returnBundles.size(); i++) {
			raw.midCodeInsert(MIDPUSH, MIDUNUSED,
				returnBundles[i].id, returnBundles[i].isImmediate,
				MIDUNUSED, false, MIDNOLABEL);
		}
		//检查参数是否对应（个数，应该还有参数类型的标记）
	}
	if (course) { out << "<值参数表>" << endl; }
	return returnBundles;
}

void GrammarAnalyzer::scanSentence() {
	inlineable = false;
	if (lex.sym().type != SCANFTK) {
		f.handleFault(lex.lineNumber(), "缺少scanf标识符");
		throw 0;
	}
	getNextSym();
	//读取scanf标识符

	if (lex.sym().type != LPARENT) {
		f.handleFault(lex.lineNumber(), "缺少（");
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
			throw 0;
		}
		string varname = lex.sym().str;
		SymbolEntry* entry = table.getSymbolByName(currentScope, varname);
		if (entry == NULL) {
			f.handleCourseFault(lex.lineNumber(), UNDEFINED);
			f.handleFault(lex.lineNumber(), "未定义的变量"+varname);
			throw 0;
		}
		if (entry != NULL&&( entry->type == TYPEINTCONST || entry->type == TYPECHARCONST)) {
			f.handleCourseFault(lex.lineNumber(), MODIFYCONST);
			f.handleFault(lex.lineNumber(), "试图修改常量的值");
			throw 0;
		}
		else if (entry != NULL&&(entry->type != TYPEINT && entry->type != TYPECHAR)) {
			f.handleFault(lex.lineNumber(), "不可读入的数据类型");
			throw 0;
		}
		//依据符号表信息检查变量是否合法
		getNextSym();
		//标识符读取完成
		if (entry != NULL){
			MidCodeOp op = entry->type == TYPEINT ? MIDREADINTEGER : MIDREADCHAR;
			raw.midCodeInsert(op, entry->id, MIDUNUSED, false, MIDUNUSED, false, MIDNOLABEL);
		}
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
	//inlineable = false;
	if (lex.sym().type != PRINTFTK) {
		f.handleFault(lex.lineNumber(), "缺少printf标识符");
		throw 0;
	}
	getNextSym();
	//读取printf符号完成

	if (lex.sym().type != LPARENT) {
		f.handleFault(lex.lineNumber(), "缺少（");
		throw 0;
	}
	getNextSym();
	//读取左括号完成

	if (lex.sym().type == STRCON) {
		/* handle the string*/
		string constString = lex.sym().str;
		int stringNo=table.addString(constString);
		raw.midCodeInsert(MIDPRINTSTRING, MIDUNUSED,
			stringNo, false, MIDUNUSED, false, MIDNOLABEL);

		getNextSym();
		if (course) { out << "<字符串>" << endl; }
		if (lex.sym().type == COMMA) {
			getNextSym();
			//读取逗号完成
			ReturnBundle res=expression();
			//直接读取表达式
			if (res.isChar) {
				raw.midCodeInsert(MIDPRINTCHAR, MIDUNUSED,
					res.id, res.isImmediate, MIDUNUSED, false, MIDNOLABEL);
			}
			else {
				raw.midCodeInsert(MIDPRINTINT, MIDUNUSED,
					res.id, res.isImmediate, MIDUNUSED, false, MIDNOLABEL);
			}
		}
	}
	else { 
		ReturnBundle res = expression();
		//直接读取表达式
		if (res.isChar) {
			raw.midCodeInsert(MIDPRINTCHAR, MIDUNUSED,
				res.id, res.isImmediate, MIDUNUSED, false, MIDNOLABEL);
		}
		else {
			raw.midCodeInsert(MIDPRINTINT, MIDUNUSED,
				res.id, res.isImmediate, MIDUNUSED, false, MIDNOLABEL);
		}
	}
	
	if (lex.sym().type != RPARENT) {
		f.handleCourseFault(lex.lineNumber(), NORPARENT);
		f.handleFault(lex.lineNumber(), "缺少)");
	}
	else {
		getNextSym();
	}
	raw.midCodeInsert(MIDPRINTCHAR, MIDUNUSED, '\n', true, MIDUNUSED, false, MIDNOLABEL);
	if (course) { out << "<写语句>" << endl; }
}

void GrammarAnalyzer::returnSentence() {
	bool hasValue = false;
	ReturnBundle res;
	if (lex.sym().type != RETURNTK) {
		f.handleFault(lex.lineNumber(), "缺少RETURN标识符");
		throw 0;
	}
	getNextSym();
	//读取return标识符
	if (lex.sym().type == LPARENT) {
		hasValue = true;
		getNextSym();
		//完成读取左括号
		 res=expression();
		if (lex.sym().type != RPARENT) {
			f.handleCourseFault(lex.lineNumber(), NORPARENT);
			f.handleFault(lex.lineNumber(), "缺少)");
		}
		else {
			getNextSym();
		}
		//完成读取右括号
	}
	SymbolEntry* entry = table.getSymbolByName("",currentScope);
	if (entry != NULL&& entry->link!=NULL) {
		if (entry->link->returnType == RETVOID && hasValue) {
			f.handleCourseFault(lex.lineNumber(), ILLEGALRETURNINVOID);
			f.handleFault(lex.lineNumber(), "无返回值函数里有含值的返回语句");
		}
		else if ((entry->link->returnType == RETINT || entry->link->returnType == RETCHAR) && !hasValue) {
			f.handleCourseFault(lex.lineNumber(), ILLEGALRETURNINNONVOID);
			f.handleFault(lex.lineNumber(), "有返回值函数里类型不正确");
		}
		else if (entry->link->returnType == RETINT && res.isChar) {
			f.handleCourseFault(lex.lineNumber(), ILLEGALRETURNINNONVOID);
			f.handleFault(lex.lineNumber(), "有返回值函数里类型不正确");
		}
		else if (entry->link->returnType == RETCHAR && !res.isChar) {
			f.handleCourseFault(lex.lineNumber(), ILLEGALRETURNINNONVOID);
			f.handleFault(lex.lineNumber(), "有返回值函数里类型不正确");
		}
	}
	hasReturned = true;
	if (hasValue) {
		raw.midCodeInsert(MIDRET, MIDUNUSED, res.id, res.isImmediate,
			MIDUNUSED, false, MIDNOLABEL);
	}
	else {
		raw.midCodeInsert(MIDRET, MIDUNUSED, MIDUNUSED, false,
			MIDUNUSED, false, MIDNOLABEL);
	}
	if (course) { out << "<返回语句>" << endl; }
}

void GrammarAnalyzer::ifSentence() {
	inlineable = false;
	if (lex.sym().type != IFTK) {
		f.handleFault(lex.lineNumber(), "缺少if标识符");
		throw 0;
	}
	getNextSym();
	//完成if标识符读取

	if (lex.sym().type != LPARENT) {
		f.handleFault(lex.lineNumber(), "缺少（");
		throw 0;
	}
	getNextSym();
	//完成读取左括号
	vector<int>tmp;
	//tmp.clear();
	ReturnBundle conditionBundle=condition(tmp);
	//读取条件

	if (lex.sym().type != RPARENT) {
		f.handleCourseFault(lex.lineNumber(), NORPARENT);
		f.handleFault(lex.lineNumber(), "缺少)");
	}
	else {
		getNextSym();
	}
	//读取右括号
	int label1 = MidCode::labelAlloc();
	if (tmp.size() == 0) {
		raw.midCodeInsert(MIDBZ, MIDUNUSED,
			conditionBundle.id, conditionBundle.isImmediate, label1, false, MIDNOLABEL);
	}
	else {
		raw.midCodeInsert(MIDBNZ, MIDUNUSED,
			conditionBundle.id, conditionBundle.isImmediate, label1, false, MIDNOLABEL);
	}
	//生成if的头部
	sentence();
	//语句
	if (lex.sym().type == ELSETK) {
		int label2 = MidCode::labelAlloc();
		raw.midCodeInsert(MIDGOTO, MIDUNUSED, label2, false, MIDUNUSED, false, MIDNOLABEL);
		raw.midCodeInsert(MIDNOP, MIDUNUSED, MIDUNUSED, false, MIDUNUSED, false, label1);
		getNextSym();
		//读取else完成
		sentence();
		//读取语句
		raw.midCodeInsert(MIDNOP, MIDUNUSED, MIDUNUSED, false, MIDUNUSED, false, label2);
	}
	else {
		raw.midCodeInsert(MIDNOP, MIDUNUSED, MIDUNUSED, false, MIDUNUSED, false, label1);
	}
	if (course) { out << "<条件语句>" << endl; }
}

ReturnBundle GrammarAnalyzer::condition() {
	
	ReturnBundle res1=expression();

	ReturnBundle res;
	res.isChar = false;
	res.isImmediate = res1.isImmediate;
	res.id = res1.id;
	if (res1.isChar) {
		f.handleCourseFault(lex.lineNumber(), ILLEGALTYPEINCONDITION);
		f.handleFault(lex.lineNumber(), "条件中必须使用整型");
	}
	//表达式
	if (lex.sym().type == LSS || lex.sym().type == LEQ
		|| lex.sym().type == GRE || lex.sym().type == GEQ
		|| lex.sym().type == EQL || lex.sym().type == NEQ) {
		MidCodeOp op;
		switch (lex.sym().type) {
		case LSS:
			op = MIDLSS;
			break;
		case LEQ:
			op = MIDLEQ;
			break;
		case GRE:
			op = MIDGRE;
			break;
		case GEQ:
			op = MIDGEQ;
			break;
		case EQL:
			op = MIDEQL;
			break;
		case NEQ:
			op = MIDNEQ;
			break;
		}
		getNextSym();
		//读取关系运算符完成
		ReturnBundle res2=expression();
		if (res2.isChar) {
			f.handleCourseFault(lex.lineNumber(), ILLEGALTYPEINCONDITION);
			f.handleFault(lex.lineNumber(), "条件中必须使用整型");
		}
		int tmpVar = MidCode::tmpVarAlloc();
		raw.midCodeInsert(op, tmpVar,
			res.id, res.isImmediate,
			res2.id, res2.isImmediate, MIDNOLABEL);
		res.id = tmpVar;
		res.isImmediate = false;
	}
	if (course) { out << "<条件>" << endl; }
	return res;
}

//tmp如果为空证明跳转指令无法优化；tmp如果说明条件的第二操作数是0，可以优化，返回第一操作数
//同时tmp变成第一操作数
ReturnBundle GrammarAnalyzer::condition(vector<int>& tmp) {

	ReturnBundle res1 = expression();

	ReturnBundle res;
	res.isChar = false;
	res.isImmediate = res1.isImmediate;
	res.id = res1.id;
	if (res1.isChar) {
		f.handleCourseFault(lex.lineNumber(), ILLEGALTYPEINCONDITION);
		f.handleFault(lex.lineNumber(), "条件中必须使用整型");
	}
	//表达式
	if (lex.sym().type == LSS || lex.sym().type == LEQ
		|| lex.sym().type == GRE || lex.sym().type == GEQ
		|| lex.sym().type == EQL || lex.sym().type == NEQ) {
		MidCodeOp op;
		switch (lex.sym().type) {
		case LSS:
			op = MIDLSS;
			break;
		case LEQ:
			op = MIDLEQ;
			break;
		case GRE:
			op = MIDGRE;
			break;
		case GEQ:
			op = MIDGEQ;
			break;
		case EQL:
			op = MIDEQL;
			break;
		case NEQ:
			op = MIDNEQ;
			break;
		}
		getNextSym();
		//读取关系运算符完成
		ReturnBundle res2 = expression();
		if (res2.isChar) {
			f.handleCourseFault(lex.lineNumber(), ILLEGALTYPEINCONDITION);
			f.handleFault(lex.lineNumber(), "条件中必须使用整型");
		}
		if (res2.isImmediate && res2.id == 0&&(op==MIDEQL||op==MIDNEQ)) {
			if (op == MIDNEQ) {
				return res1;
			}
			else {
				tmp.push_back(op);
				return res1;
			}
		}
		int tmpVar = MidCode::tmpVarAlloc();
		raw.midCodeInsert(op, tmpVar,
			res.id, res.isImmediate,
			res2.id, res2.isImmediate, MIDNOLABEL);
		res.id = tmpVar;
		res.isImmediate = false;
	}
	if (course) { out << "<条件>" << endl; }
	return res;
}

void GrammarAnalyzer::loopSentence() {
	inlineable = false;
	bool error = false;
	if (lex.sym().type == WHILETK) {
		getNextSym();
		//读取while符号

		if (lex.sym().type != LPARENT) {
			f.handleFault(lex.lineNumber(), "缺少（");
			throw 0;
		}
		getNextSym();
		int label1 = MidCode::labelAlloc();
		int label2 = MidCode::labelAlloc();
		
		//读取左括号

		int probe1 = raw.getIndex();
		ReturnBundle conditionBundle = condition();
		int probe2 = raw.getIndex();
		vector<MidCode>paraCode(raw.getIterator(probe1), raw.getIterator(probe2));
		//读取条件语句
		
		
		if (lex.sym().type != RPARENT) {
			f.handleCourseFault(lex.lineNumber(), NORPARENT);
			f.handleFault(lex.lineNumber(), "缺少)");
		}
		else {
			getNextSym();
		}
		//读取右括号完成

		
		//生成while头部
		raw.midCodeInsert(MIDGOTO, MIDUNUSED, label2, false,
			MIDUNUSED, false, MIDNOLABEL);
		raw.midCodeInsert(MIDNOP, MIDUNUSED, MIDUNUSED, false, MIDUNUSED, false, label1);
		sentence();

		raw.midCodeInsert(MIDNOP, MIDUNUSED, MIDUNUSED, false, MIDUNUSED, false, label2);
		raw.midCodeInsert(paraCode);
		raw.midCodeInsert(MIDBNZ, MIDUNUSED,
			conditionBundle.id, conditionBundle.isImmediate,
			label1, false, MIDNOLABEL);
		
	}
	else if (lex.sym().type == DOTK) {
		getNextSym();
		//读取do符号完成

		int label1 = MidCode::labelAlloc();
		raw.midCodeInsert(MIDNOP, MIDUNUSED, MIDUNUSED, false, MIDUNUSED, false, label1);
		
		sentence();
		//读取句子
		if (lex.sym().type != WHILETK) {
			f.handleCourseFault(lex.lineNumber(), NOWHILE);
			f.handleFault(lex.lineNumber(), "DO-WHILE缺少while符号");
			throw 0;
		}
		else {
			getNextSym();
		}
		//读取while符号
		if (lex.sym().type != LPARENT) {
			f.handleFault(lex.lineNumber(), "缺少（");
			throw 0;
		}
		getNextSym();
		//读取左括号
		ReturnBundle conditionBundle=condition();
		//读取条件语句
		if (lex.sym().type != RPARENT) {
			f.handleCourseFault(lex.lineNumber(), NORPARENT);
			f.handleFault(lex.lineNumber(), "缺少)");
		}
		else {
			getNextSym();
		}
		//读取右括号
		raw.midCodeInsert(MIDBNZ, MIDUNUSED,
			conditionBundle.id, conditionBundle.isImmediate,
			label1, false, MIDNOLABEL);
	}
	else if (lex.sym().type == FORTK) {
		getNextSym();
		//读取for符号完成
		if (lex.sym().type != LPARENT) {
			f.handleFault(lex.lineNumber(), "缺少（");
			throw 0;
		}
		getNextSym();
		//读取左括号完成
		if (lex.sym().type != IDENFR) {
			f.handleFault(lex.lineNumber(), "需要标识符");
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
		if (!error && (entry1->type == TYPEINTCONST || entry1->type == TYPECHARCONST)) {
			f.handleCourseFault(lex.lineNumber(), MODIFYCONST);
			f.handleFault(lex.lineNumber(), "不能改变常量的值");
		}
		else if (!error && entry1->type != TYPEINT && entry1->type != TYPECHAR) {
			f.handleFault(lex.lineNumber(), "不合法的变量类型");
			error = true;
		}
		if (lex.sym().type != ASSIGN) {//读取赋值运算符
			f.handleFault(lex.lineNumber(), "需要=");
			throw 0;
		}
		else {
			getNextSym();
		}
		//读取赋值符完成
		ReturnBundle resA= expression();
		//读取所赋的表达式
		if (entry1 != NULL) {
			raw.midCodeInsert(MIDASSIGN, entry1->id,
				resA.id, resA.isImmediate, MIDUNUSED, false, MIDNOLABEL);
		}
		int label1 = MidCode::labelAlloc();
		int label2 = MidCode::labelAlloc();

		//生成第一部分中间代码
		if (lex.sym().type != SEMICN) {
			f.handleCourseFault(lex.lineNumber(), NOSEMICN);
			f.handleFault(lex.lineNumber(), "需要;");
		}
		else {
			getNextSym();
		}
		//读取分号完成
		int probe1 = raw.getIndex();
		ReturnBundle conditionBundle = condition();
		int probe2 = raw.getIndex();
		vector<MidCode>paraCode(raw.getIterator(probe1), raw.getIterator(probe2));

		
		//生成条件部分代码
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
		if (!error && (entry2->type == TYPEINTCONST || entry2->type == TYPECHARCONST)) {
			f.handleCourseFault(lex.lineNumber(), MODIFYCONST);
			f.handleFault(lex.lineNumber(), "不能改变常量的值");
		}
		else if (!(entry2==NULL) && entry2->type != TYPEINT && entry2->type != TYPECHAR) {
			f.handleFault(lex.lineNumber(), "不合法的变量类型");
			error = true;
		}
		if (lex.sym().type != ASSIGN) {//读取赋值运算符
			f.handleFault(lex.lineNumber(), "需要=");
			throw 0;
		}
		else {
			getNextSym();
		}
		//读取赋值运算符完成
		if (lex.sym().type != IDENFR) {
			f.handleFault(lex.lineNumber(), "需要标识符");
			throw 0;
		}
		string varname3 = lex.sym().str;
		SymbolEntry* entry3 = table.getSymbolByName(currentScope, varname3);
		if (entry3 == NULL) {
			f.handleCourseFault(lex.lineNumber(), UNDEFINED);
			f.handleFault(lex.lineNumber(), "未定义的标识符" + varname3);
			error = true;
		}
		 if (!(entry3 == NULL) && entry3->type != TYPEINT && entry3->type != TYPECHAR
			 &&entry3->type!=TYPEINTCONST&&entry3->type!=TYPECHARCONST) {
			f.handleFault(lex.lineNumber(), "不合法的变量类型");
			error = true;
		}
		getNextSym();
		//读取标识符完成
		if (lex.sym().type != PLUS && lex.sym().type != MINU) {
			f.handleFault(lex.lineNumber(), "需要运算符");
			throw 0;
		}
		MidCodeOp op = lex.sym().type==PLUS?MIDADD:MIDSUB;
		getNextSym();
		//读取加减号完成
		if (lex.sym().type != INTCON) {
			f.handleFault(lex.lineNumber(), "需要步长");
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
		raw.midCodeInsert(MIDGOTO, MIDUNUSED, label2, false,
			MIDUNUSED, false, MIDNOLABEL);
		raw.midCodeInsert(MIDNOP, MIDUNUSED, MIDUNUSED, false, MIDUNUSED, false, label1);
		sentence();



		
		if (entry2 != NULL&&entry3!=NULL) {
			if (constantSubstitutionSwitch&&(entry3->type == TYPECHARCONST || entry3->type == TYPEINTCONST)) {
				raw.midCodeInsert(op, entry2->id,
					entry3->initValue, true, step, true, MIDNOLABEL);
			}
			else {
				raw.midCodeInsert(op, entry2->id,
					entry3->id, false, step, true, MIDNOLABEL);
			}
			raw.midCodeInsert(MIDNOP, MIDUNUSED, MIDUNUSED, false, MIDUNUSED, false, label2);
			raw.midCodeInsert(paraCode);
			raw.midCodeInsert(MIDBNZ, MIDUNUSED,
				conditionBundle.id, conditionBundle.isImmediate, label1, false, MIDNOLABEL);
	
			//生成头部第三部分
		}
	}
	else {
		f.handleFault(lex.lineNumber(), "非法的循环语句");
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
				throw 0;
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
	try {
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
	}
	catch (int e) {
		toNextBrace();
		getNextSym();
	}
	while (1) {
		try {
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
					f.handleFault(lex.lineNumber(), "programme:void后非法字符");
					throw 0;
				}
			}
			else if (lex.sym().type == CHARTK || lex.sym().type == INTTK) {
				nonVoidFunctionDefination();
			}
			else if (lex.sym().type == END) {
				break;
			}
			else {
				f.handleFault(lex.lineNumber(), "程序开头非法字符");
				throw 0;
			}
		}
		catch (int e) {
			toNextBrace();
			getNextSym();
		}
	}
	if (course) { out << "<程序>" << endl; }
}

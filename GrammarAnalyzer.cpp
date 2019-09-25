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

/*<����>*/
int GrammarAnalyzer::integer() {
	int sign = 1;
	Lexical tk = lex.sym().type;//������
	if (tk == PLUS || tk == MINU) {
		sign = tk == MINU ? -1 : 1;
		getNextSym();
	}
	if (lex.sym().type != INTCON) {
		f.handleFault(lex.lineNumber(), "��Ҫ�޷�������");
		throw 0;
	}
	int value = lex.sym().value;//��ȡ����
	getNextSym();
	out << "<�޷�������>" << endl;
	out << "<����>" << endl;
	return value;
}

/*��������*/
void GrammarAnalyzer::constDeclearation() {
	bool init = true;
	while (1) {
		try {
			if (lex.sym().type != CONSTTK) {
				break;//����Ƿ���const
			}
			init = false;
			getNextSym();

			constDefination();//��鳣������

			if (lex.sym().type != SEMICN) {//����Ƿ�Ϊ�ֺ�
				f.handleCourseFault(lex.lineNumber(), NOSEMICN);
				f.handleFault(lex.lineNumber(), "ȱ�� ;");
			}
			else {
				getNextSym();
			}
		}
		catch (int e) {
			toNextSemicon();//����
			lex.getNextSym();
			break;
		}
	}
	if (course&&!init) {out << "<����˵��>" << endl;}
}

/*��������*/
void GrammarAnalyzer::constDefination() {
	bool init = true;
	if (lex.sym().type != INTTK && lex.sym().type != CHARTK) {//���int��char
		f.handleFault(lex.lineNumber(), "��Ҫ���ͱ�ʶ��");
		throw 0;
	}
	Lexical vartype = lex.sym().type;
	getNextSym();
	
	while (1) {
		if (init) {
			init = false;//��һ���������岻��û���Ҳ��Ƕ��ſ�ͷ
		}
		else {
			if (lex.sym().type != COMMA) {
				break;//��û�ж���˵���������
			}
			else {
				getNextSym();
			}
		}
		if (lex.sym().type != IDENFR) {//��鲢��ȡ��ʶ��
			f.handleFault(lex.lineNumber(), "��Ҫ��������");
			throw 0;
		}
		string varname = lex.sym().str;
		getNextSym();
		
		if (lex.sym().type != ASSIGN) {//��ȡ�Ⱥ�
			f.handleFault(lex.lineNumber(), "��Ҫ=");
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
	if (course) { out << "<��������>" << endl; }
}

/*����˵��,����ȫ�ֶεı���������ʹ��*/
void GrammarAnalyzer::variableDeclearation() {
	bool init = true;
	while (1) {
		try {
			if (lex.sym().type != INTTK && lex.sym().type != CHARTK) {
				break;
			}
			init = false;
			variableDefination();
			if (lex.sym().type != SEMICN) {//����Ƿ�Ϊ�ֺ�
				f.handleCourseFault(lex.lineNumber(), NOSEMICN);
				f.handleFault(lex.lineNumber(), "ȱ�� ;");
			}
			else {
				getNextSym();
			}
		}
		catch (int e) {
			toNextSemicon();//����
			lex.getNextSym();
			break;
		}
	}
	if (course && !init) { out << "<����˵��>" << endl; }
}

/*����˵������declearationHeader����ʹ��*/
void GrammarAnalyzer::variableDeclearation(Lexical type,string varname) {
		try {
			variableDefination(true,type,varname);
			if (lex.sym().type != SEMICN) {//����Ƿ�Ϊ�ֺ�
				f.handleCourseFault(lex.lineNumber(), NOSEMICN);
				f.handleFault(lex.lineNumber(), "ȱ�� ;");
			}
			else {
				getNextSym();
			}
		}
		catch (int e) {
			toNextSemicon();//����
			lex.getNextSym();
		}
	if (course ) { out << "<����˵��>" << endl; }
}
/*<��������>*/
void GrammarAnalyzer::variableDefination(bool wraper,Lexical type,string name) {
	Lexical vartype;
	if (!wraper) {
		if (lex.sym().type != INTTK && lex.sym().type != CHARTK) {
			f.handleFault(lex.lineNumber(), "��Ҫ���ͱ�ʶ��");
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
			f.handleFault(lex.lineNumber(), "��Ҫ��������");
			throw 0;
		}
		varname = lex.sym().str;
		
		getNextSym();

LABEL:	int dimension = 0;
		if (lex.sym().type == LBRACK) {
			getNextSym();
			if (lex.sym().type != INTCON) {
				f.handleFault(lex.lineNumber(), "��Ҫά��");
				throw 0;
			}
			dimension = lex.sym().value;
			getNextSym();
			if(lex.sym().type!=RBRACK){
				f.handleCourseFault(lex.lineNumber(), NORBRACK);
				f.handleFault(lex.lineNumber(), "ȱ��]");
			}
			else {
				getNextSym();
			}
		}

		SymbolEntry* entry = table.addSymbol(currentScope, varname, false);
		if (entry == NULL) {
			f.handleCourseFault(lex.lineNumber(), REDEFINED);
			f.handleFault(lex.lineNumber(), varname + "�ض���");
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
	if (course) { out << "<��������>" << endl; }
}

/*�з���ֵ��������,���Ǳ������������з���ֵ��������*/
void GrammarAnalyzer::nonVoidFunctionDefination() {
	try {
		Lexical retType;
		declearationHeader(retType);
		if (lex.sym().type != LPARENT) {
			f.handleFault(lex.lineNumber(), "ȱ�ٲ�����");
			//todo ���������;
			f.terminate();
		}
		string functionName = lex.sym().str;
		getNextSym();
		SymbolEntry* entry = table.addSymbol(currentScope, functionName, true);
		if (entry == NULL) {
			f.handleCourseFault(lex.lineNumber(), REDEFINED);
			f.handleFault(lex.lineNumber(), "�������ض���" + functionName);
		}
		entry->link->returnType = retType == INTTK ? RETINT : RETCHAR;
		parameterList(entry);

		if (lex.sym().type != RPARENT) {
			f.handleCourseFault(lex.lineNumber(), NORPARENT);
			f.handleFault(lex.lineNumber(), "ȱ��)");
		}
		else {
			getNextSym();
		}

		if (lex.sym().type != LBRACE) {
			f.handleFault(lex.lineNumber(), "ȱ��{");
			throw 0;
		}
		else {
			getNextSym();
		}
		currentScope = functionName;
		compoundSentence();
		currentScope = "";

		if (lex.sym().type != RBRACE) {
			f.handleFault(lex.lineNumber(), "ȱ��{");
			// TODO HANDLEFAULT
			f.terminate();
		}
	}
	catch (int e) {
		// TODO handlefault;
		f.terminate();
	}
	if (course) { out << "<�з���ֵ��������>" << endl; }
}

void GrammarAnalyzer::nonVoidFunctionDefination(Lexical retType,string functionName) {
	try {
		
		if (lex.sym().type != LPARENT) {
			f.handleFault(lex.lineNumber(), "ȱ�ٲ�����");
			//todo ���������;
			f.terminate();
		}
		getNextSym();

		SymbolEntry* entry = table.addSymbol(currentScope, functionName, true);
		if (entry == NULL) {
			f.handleCourseFault(lex.lineNumber(), REDEFINED);
			f.handleFault(lex.lineNumber(), "�������ض���" + functionName);
		}
		entry->link->returnType = retType == INTTK ? RETINT : RETCHAR;
		parameterList(entry);

		if (lex.sym().type != RPARENT) {
			f.handleCourseFault(lex.lineNumber(), NORPARENT);
			f.handleFault(lex.lineNumber(), "ȱ��)");
		}
		else {
			getNextSym();
		}

		if (lex.sym().type != LBRACE) {
			f.handleFault(lex.lineNumber(), "ȱ��{");
			throw 0;
		}
		else {
			getNextSym();
		}
		currentScope = functionName;
		compoundSentence();
		currentScope = "";

		if (lex.sym().type != RBRACE) {
			f.handleFault(lex.lineNumber(), "ȱ��{");
			// TODO HANDLEFAULT
			f.terminate();
		}
	}
	catch (int e) {
		// TODO handlefault;
		f.terminate();
	}
	if (course) { out << "<�з���ֵ��������>" << endl; }
}
/*����ͷ��,���Ǳ������������з���ֵ��������*/
string GrammarAnalyzer::declearationHeader(Lexical& retType) {
	if (lex.sym().type != INTTK && lex.sym().type != CHARTK) {
		f.handleFault(lex.lineNumber(), "ȱ�����ͱ�ʶ��");
		throw 0;
	}
	retType = lex.sym().type;
	getNextSym();
	if (lex.sym().type != IDENFR) {
		f.handleFault(lex.lineNumber(), "ȱ�ٺ�������");
		throw 0;
	}
	string functionName = lex.sym().str;
	if (course) { out << "<����ͷ��>" << endl; }
	getNextSym();
	
	return functionName;
}

Lexical GrammarAnalyzer::declearationHeader() {
	Lexical type;
	if (lex.sym().type != INTTK && lex.sym().type != CHARTK) {
		f.handleFault(lex.lineNumber(), "ȱ�����ͱ�ʶ��");
		throw 0;
	}
	type = lex.sym().type;
	getNextSym();
	if (lex.sym().type != IDENFR) {
		f.handleFault(lex.lineNumber(), "ȱ�ٺ�������");
		throw 0;
	}
	string name = lex.sym().str;
	getNextSym();

	if (lex.sym().type == LPARENT) {
		if (course) { out << "<����ͷ��>" << endl; }
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
			f.handleFault(lex.lineNumber(), "��Ҫ�ṩ��������");
			throw 0;
		}
		string paraname = lex.sym().str;
		entry->link->paras.push_back(paratype);
		entry->link->paraNum++;
		string scope = entry->scope;
		SymbolEntry* paraentry=table.addSymbol(scope, paraname, false);
		if (paraentry == NULL) {
			f.handleCourseFault(lex.lineNumber(), REDEFINED);
			f.handleFault(lex.lineNumber(), "�������ض���" + paraname);
		}
		getNextSym();	
	}
	if (course) { out << "<������>" << endl; }
}

void GrammarAnalyzer::compoundSentence(){

}
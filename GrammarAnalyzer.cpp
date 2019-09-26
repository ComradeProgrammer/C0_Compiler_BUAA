#include"GrammarAnalyzer.h"

/*����������*/
GrammarAnalyzer::GrammarAnalyzer(FaultHandler& _f, SymbolTable& _s, LexicalAnalyzer& _l,string file)
:f(_f),table(_s),lex(_l){
	out.open(file);
	currentScope="";
}

/*��ȡ��һ���أ��������һ������*/
Lexical GrammarAnalyzer::getNextSym() {
	if (course) { lex.printResult(out); }
	return lex.getNextSym();
}

/*һֱ������һ���ֺ�Ϊֹ*/
void GrammarAnalyzer::toNextSemicon() {
	while (1) {
		Lexical tmp = lex.getNextSym();
		if (tmp == END || tmp == SEMICN) {
			break;
		}
	}
}

/*����﷨������ҵ�����*/
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

/*<��������>*/
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

/*<��������>*/
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

		if (vartype == INTTK) {/*�������Ͷ�ȡֵ��д����ű�*/
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

/*<����˵��>,����ȫ�ֶεı���������ʹ��*/
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

/*<����˵��>����declearationHeader��װ����ʹ��*/
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
/*<��������>,���������ʹ��
��һ�����������Ƿ��Ǳ���װ�����õģ��ڶ������������װ��Ԥ���ı������ͺͱ�������
*/
void GrammarAnalyzer::variableDefination(bool wraper,Lexical type,string name) {
	Lexical vartype;
	if (!wraper) {
		if (lex.sym().type != INTTK && lex.sym().type != CHARTK) {
			f.handleFault(lex.lineNumber(), "��Ҫ���ͱ�ʶ��");//δ����װ�����������ͱ�ʶ��
			throw 0;
		}
		vartype = lex.sym().type;
		getNextSym();
	}
	else {
		vartype = type;//�ѱ���װ�����������ͱ�ʶ��
	}
	bool init = true;
	string varname;
	while (1) {
		if (init) {
			init = false;
			if (wraper) {//��Ҫ������װ��Ԥ���ĵ�һ������
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
			f.handleFault(lex.lineNumber(), "��Ҫ��������");//����������
			throw 0;
		}
		varname = lex.sym().str;
		
		getNextSym();

LABEL:	int dimension = 0;
		if (lex.sym().type == LBRACK) {//�ж��ǲ�������Ԫ��
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

		SymbolEntry* entry = table.addSymbol(currentScope, varname, false);//������ű�
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

/*<�з���ֵ��������>,���Ǳ������������з���ֵ��������*/
void GrammarAnalyzer::nonVoidFunctionDefination() {
	try {
		Lexical retType;
		string functionName=declearationHeader(retType);//��ȡ�������ͷ�������
		if (lex.sym().type != LPARENT) {//�������������
			f.handleFault(lex.lineNumber(), "ȱ�ٲ�����");
			//todo ���������;
			f.terminate();
		}
		getNextSym();
		SymbolEntry* entry = table.addSymbol(currentScope, functionName, true);//������ű���
		if (entry == NULL) {
			f.handleCourseFault(lex.lineNumber(), REDEFINED);
			f.handleFault(lex.lineNumber(), "�������ض���" + functionName);
		}
		entry->link->returnType = retType == INTTK ? RETINT : RETCHAR;
		entry->type = TYPEFUNCTION;
		parameterList(entry);

		if (lex.sym().type != RPARENT) {//������������
			f.handleCourseFault(lex.lineNumber(), NORPARENT);
			f.handleFault(lex.lineNumber(), "ȱ��)");
		}
		else {
			getNextSym();
		}

		if (lex.sym().type != LBRACE) {//��ȡ�������
			f.handleFault(lex.lineNumber(), "ȱ��{");
			throw 0;
		}
		else {
			getNextSym();
		}
		currentScope = functionName;//�����ǰ������
		compoundSentence();
		currentScope = "";

		if (lex.sym().type != RBRACE) {
			f.handleFault(lex.lineNumber(), "ȱ��{");
			// TODO HANDLEFAULT
			f.terminate();
		}
		getNextSym();
	}
	catch (int e) {
		// TODO handlefault;
		f.terminate();
	}
	if (course) { out << "<�з���ֵ��������>" << endl; }
}

void GrammarAnalyzer::nonVoidFunctionDefination(Lexical retType,string functionName) {
	try {
		
		if (lex.sym().type != LPARENT) {//�������������
			f.handleFault(lex.lineNumber(), "ȱ�ٲ�����");
			//todo ���������;
			f.terminate();
		}
		getNextSym();

		SymbolEntry* entry = table.addSymbol(currentScope, functionName, true);//������ű���
		if (entry == NULL) {
			f.handleCourseFault(lex.lineNumber(), REDEFINED);
			f.handleFault(lex.lineNumber(), "�������ض���" + functionName);
		}
		entry->link->returnType = retType == INTTK ? RETINT : RETCHAR;
		entry->type = TYPEFUNCTION;
		parameterList(entry);

		if (lex.sym().type != RPARENT) {
			f.handleCourseFault(lex.lineNumber(), NORPARENT);//������������
			f.handleFault(lex.lineNumber(), "ȱ��)");
		}
		else {
			getNextSym();
		}

		if (lex.sym().type != LBRACE) {//�������
			f.handleFault(lex.lineNumber(), "ȱ��{");
			throw 0;
		}
		else {
			getNextSym();
		}
		currentScope = functionName;
		compoundSentence();
		currentScope = "";

		if (lex.sym().type != RBRACE) {//�Ҵ�����
			f.handleFault(lex.lineNumber(), "ȱ��{");
			// TODO HANDLEFAULT
			f.terminate();
		}
		getNextSym();
	}
	catch (int e) {
		// TODO handlefault;
		f.terminate();
	}
	
	if (course) { out << "<�з���ֵ��������>" << endl; }
}

/*<������>*/
void GrammarAnalyzer::voidFunctionDefination() {
	try {
		if (lex.sym().type != VOIDTK) {
			cout << "BUG@void GrammarAnalyzer::voidFunctionDefination()" << endl;
			f.terminate();
		}
		getNextSym();

		if (lex.sym().type != IDENFR) {
			f.handleFault(lex.lineNumber(), "ȱ�ٺ�����");
			//todo ���������;
			f.terminate();
		}
		string functionName = lex.sym().str;
		getNextSym();

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
		entry->link->returnType = RETVOID;
		entry->type = TYPEFUNCTION;
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
		getNextSym();
	}
	catch (int e) {
		// TODO handlefault;
		f.terminate();
	}
	if (course) { out << "<�޷���ֵ��������>" << endl; }
}


void GrammarAnalyzer::mainFunctionDefination() {
	try {
		if (lex.sym().type != VOIDTK) {
			cout << "BUG@void GrammarAnalyzer::voidFunctionDefination()" << endl;
			f.terminate();
		}
		getNextSym();

		if (lex.sym().type != MAINTK) {
			f.handleFault(lex.lineNumber(), "ȱ�ٺ�����");
			//todo ���������;
			f.terminate();
		}
		string functionName = "main";
		getNextSym();

		SymbolEntry* entry = table.addSymbol(currentScope, functionName, true);
		if (entry == NULL) {
			f.handleCourseFault(lex.lineNumber(), REDEFINED);
			f.handleFault(lex.lineNumber(), "�������ض���" + functionName);
		}
		entry->link->returnType = RETVOID;
		entry->type = TYPEFUNCTION;
		if (lex.sym().type != LPARENT) {
			f.handleFault(lex.lineNumber(), "ȱ�ٲ�����");
			//todo ���������;
			f.terminate();
		}
		getNextSym();

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
			f.handleFault(lex.lineNumber(), "ȱ��}");
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
	if (course) { out << "<������>" << endl; }
}

/*<����ͷ��>,���Ǳ������������з���ֵ��������*/
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

/*�����������з���ֵ�������������޷�����������½���Ԥ���õĺ���*/
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
	else if (lex.sym().type == COMMA||lex.sym().type==LBRACK||lex.sym().type==SEMICN) {
		variableDeclearation(type, name);
	}
	return lex.sym().type;

}

/*<������>*/
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
			f.handleFault(lex.lineNumber(), "��Ҫ�ṩ��������");
			throw 0;
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
		string scope = entry->name;
		SymbolEntry* paraentry=table.addSymbol(scope, paraname, false);
		if (paraentry == NULL) {
			f.handleCourseFault(lex.lineNumber(), REDEFINED);
			f.handleFault(lex.lineNumber(), "�������ض���" + paraname);
		}
		getNextSym();
		paraNum++;
	}
	entry->link->paraNum = paraNum;
	if (course) { out << "<������>" << endl; }
}

/*<�������>*/
void GrammarAnalyzer::compoundSentence(){

}

/*<����>*/
void GrammarAnalyzer::factor() {
	bool error = false;
	if (lex.sym().type == IDENFR) {//��ʶ������ʶ�����飬�з���ֵ�����������
		string varname = lex.sym().str;
		getNextSym();
		SymbolEntry* entry = table.getSymbolByName(currentScope, varname);
		if (entry == NULL) {
			f.handleCourseFault(lex.lineNumber(), UNDEFINED);
			f.handleFault(lex.lineNumber(), "δ����ķ���" + varname);
			error = true;
		}
		if (lex.sym().type == LBRACK) {//����
			if (!error && entry->type != TYPECHARARRAY && entry->type != TYPEINTARRAY) {
				f.handleCourseFault(lex.lineNumber(), TYPEERROR);
				f.handleFault(lex.lineNumber(), varname + "����������������");
				error = true;
			}
			getNextSym();

			expression();

			if (lex.sym().type != RBRACK) {
				f.handleCourseFault(lex.lineNumber(), NORBRACK);
				f.handleFault(lex.lineNumber(), "ȱ��]");
				error = true;
			}
			else {
				getNextSym();
			}
		}
		else if (lex.sym().type == LPARENT) {//�з���ֵ����
			functionCall(varname,true);
		}
		else {//��ʶ��
			//�˴�Ӧ���м�������ɣ�
		}
	}
	else if (lex.sym().type == LPARENT) {
		getNextSym();

		expression();
		if (lex.sym().type != RPARENT) {
			f.handleCourseFault(lex.lineNumber(), NORPARENT);
			f.handleFault(lex.lineNumber(),"ȱ�٣�");
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
		f.handleFault(lex.lineNumber(), "����ʶ��ı��ʽ");
		// todo handlefault 
		f.terminate();
	}
	if (course) { out << "<����>" << endl; }
}

/*<��>*/
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
	if (course) { out << "<��>" << endl; }
}

/*<���ʽ>*/
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
	if (course) { out << "<���ʽ>" << endl; }
}

/*��Ը�ֵ���ͺ�����������Ԥ����֧*/
void GrammarAnalyzer::assignAndCall() {
	if (lex.sym().type != IDENFR) {
		f.handleFault(lex.lineNumber(), "��Ҫ��ʶ��");
		// todo handle fault
		f.terminate();
	}
	string name = lex.sym().str;//����һ����ʶ��
	getNextSym();
	if (lex.sym().type == LBRACK || lex.sym().type == ASSIGN) {//�Ǹ�ֵ���
		assignSentence(name);
	}
	else if (lex.sym().type == LPARENT) {//�Ǻ����������
		functionCall(name,false);
	}
	else {
		f.handleFault(lex.lineNumber(), "������ı�ʶ��");
		// todo handle fault
		f.terminate();
	}
}
/*<��ֵ���>���Ѿ���Ԥ��*/
void GrammarAnalyzer::assignSentence(string varname) {
	bool error = false;
	SymbolEntry* entry = table.getSymbolByName(currentScope, varname);
	if (entry == NULL) {
		f.handleCourseFault(lex.lineNumber(), UNDEFINED);
		f.handleFault(lex.lineNumber(), "δ����ı���");
		error = true;
	}
	if (lex.sym().type == LBRACK) {//��ֵ������
		if (!error && entry->type != TYPECHARARRAY && entry->type != TYPEINTARRAY) {
			f.handleCourseFault(lex.lineNumber(), TYPEERROR);
			f.handleFault(lex.lineNumber(), varname + "����������������");
			error = true;
		}
		getNextSym();
		expression();
		if (lex.sym().type != RBRACK) {
			f.handleCourseFault(lex.lineNumber(), NORBRACK);
			f.handleFault(lex.lineNumber(), "ȱ��]");
		}
		else {
			getNextSym();
		}
	}
	if (lex.sym().type != ASSIGN) {//��ȡ��ֵ�����
		f.handleFault(lex.lineNumber(), "��Ҫ=");
		// todo handlefault
	}
	else {
		getNextSym();
	}
	expression();//��ȡ�����ı��ʽ
	if (course) { out << "<��ֵ���>"<<endl; }
}

/*�������ã���mustReturn������ζ�ű����з���ֵ���������޷���ֵ�Կɣ��ѱ�Ԥ������*/
void GrammarAnalyzer::functionCall(string name,bool mustReturn) {
	bool error = false;
	SymbolEntry* entry = table.getSymbolByName(currentScope, name);//��ȡ������Ϣ
	if (entry == NULL) {//����Ƿ��Ѷ���
		f.handleCourseFault(lex.lineNumber(), UNDEFINED);
		f.handleFault(lex.lineNumber(), "δ����ı���");
		error = true;
	}
	if (!error&&entry->type != TYPEFUNCTION) {//����Ƿ�Ϊ����
		f.handleCourseFault(lex.lineNumber(), TYPEERROR);
		f.handleFault(lex.lineNumber(), "���Ǻ���");
		//todo handle fault
		f.terminate();
	}
	if (!error&&mustReturn && entry->link->returnType == RETVOID) {//����Ƿ��з���ֵ
		f.handleCourseFault(lex.lineNumber(), TYPEERROR);
		f.handleFault(lex.lineNumber(), "�����з���ֵ����");
		// todo handlefault;
		f.terminate();
	}
	if (lex.sym().type != LPARENT) {//��ȡ��
		f.handleFault(lex.lineNumber(), "ȱ��(");
		//todo handle fault
		f.terminate();
	}
	getNextSym();
	parameterValueList(entry);//��ȡʵ���б�
	if (lex.sym().type != RPARENT) {
		f.handleCourseFault(lex.lineNumber(), NORPARENT);
		f.handleFault(lex.lineNumber(), "ȱ��)");
		//todo handle fault
	}
	getNextSym();
	if (course) {
		if (entry->link->returnType == RETVOID) {
			out << "<�޷���ֵ�����������>"<<endl;
		}
		else {
			out << "<�з���ֵ�����������>" << endl;
		}
	}
}

/*<ֵ������>*/
void GrammarAnalyzer::parameterValueList(SymbolEntry* entry) {
	int paraNum = 0;
	bool init = true;
	while (1) {
		if (lex.sym().type == RPARENT) {
			break;
		}
		if (!init) {
			if (lex.sym().type != COMMA) {
				f.handleFault(lex.lineNumber(), "ȱ��,");
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
		f.handleFault(lex.lineNumber(), "����������������ȷ");
		// todo handle fault 
		f.terminate();
	}
	if (course) { out << "<ֵ������>" << endl; }
}
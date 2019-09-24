#include"GrammarAnalyzer.h"

GrammarAnalyzer::GrammarAnalyzer(FaultHandler& _f, SymbolTable& _s, LexicalAnalyzer& _l,string file)
:f(_f),table(_s),lex(_l){
	out.open(file);
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

/*��������*/
void GrammarAnalyzer::constDeclearation() {
	while (1) {
		try {
			if (lex.sym().type != CONSTTK) {
				break;//����Ƿ���const
			}
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
	if (course) {out << "<����˵��>" << endl;}
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
			// todo insert into table
		}
		else {
			char charvalue = (char)(lex.sym().value);
			getNextSym();
			// todo insert into table
		}
	}
	out << "<��������>" << endl;
}

int GrammarAnalyzer::integer() {
	int sign = 1;
	Lexical tk = lex.sym().type;
	if (tk == PLUS || tk == MINU) {
		sign = tk == MINU ? -1 : 1;
		getNextSym();
	}
	if (lex.sym().type != INTCON) {
		f.handleFault(lex.lineNumber(), "��Ҫ�޷�������");
		throw 0;
	}
	int value = lex.sym().value;
	getNextSym();
	out << "<�޷�������>" << endl;
	out << "<����>" << endl;

}


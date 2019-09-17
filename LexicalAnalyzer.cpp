#include"LexicalAnalyzer.h"

LexicalAnalyzer::LexicalAnalyzer(FaultHandler& f):faultHandler(f) {
	text = "";
	currentSym = {UNKNOWN};
	ptr = 0;
	line = 1;
	column = 1;
	reservedKey["const"] = CONSTTK;
	reservedKey["int"] = INTTK;
	reservedKey["char"] = CHARTK;
	reservedKey["void"] = VOIDTK;
	reservedKey["main"] = MAINTK;
	reservedKey["if"] = IFTK;
	reservedKey["else"] = ELSETK;
	reservedKey["do"] = DOTK;
	reservedKey["while"] = WHILETK;
	reservedKey["for"] = FORTK;
	reservedKey["scanf"] = SCANFTK;
	reservedKey["printf"] = PRINTFTK;
	reservedKey["return"] = RETURNTK;
}

void LexicalAnalyzer::readAll(string filename) {
	ifstream fin;
	fin.open(filename,ios_base::in);
	while (1) {
		string tmp;
		getline(fin, tmp);
		if (tmp == "") {
			break;
		}
		tmp += '\n';
		text += tmp;
	}
}

Result LexicalAnalyzer::sym() {
	return currentSym;
}
int LexicalAnalyzer::getUnsignedInteger() {
	int res = 0;
	while (ptr<text.size()&&isdigit(text[ptr])) {
		res *= 10;
		res += text[ptr] - '0';
		ptr++;
		column ++ ;
	}
	return res;
}

bool LexicalAnalyzer::isValidChar(char c) {
	if ( c=='+'||c=='-'||c=='*'||c=='/'||
		c == '_' || isdigit(c) || isalpha(c)) {
		return true;
	}
	return false;
}

string LexicalAnalyzer::getString() {
	string res = "";
	while (ptr < text.size() && text[ptr] >= 32 && text[ptr] <= 126 && text[ptr] != 34) {
		res += text[ptr];
		ptr++;
		column++;
	}
	return res;
}

string LexicalAnalyzer::getIdentifier() {
	string res = "";
	while (ptr < text.size() && 
		(isalpha(text[ptr])||isdigit(text[ptr])||text[ptr]=='_')) {
		res += text[ptr];
		ptr++;
		column++;
	}
	return res;
}

Lexical LexicalAnalyzer::getNextSym() {
	Result result;
	while (ptr<text.size()&&isspace(text[ptr])) {
		if (text[ptr] == '\n') {
			line++;
			column = 1;
		}
		else {
			column++;
		}
		ptr++;
	}
	if (ptr == text.size()) {
		result.str = END;
		return END;
	}
	Lexical res;
	switch (text[ptr]) {
	case '+':
		res = PLUS;
		ptr++;
		column++;
		break;
	case '-':
		res = MINU;
		ptr++;
		column++;
		break;
	case '*':
		res = MULT;
		ptr++;
		column++;
		break;
	case '/':
		res = DIV;
		ptr++;
		column++;
		break;
	case '<':
		ptr++;
		column++;
		if (ptr < text.size() && text[ptr] == '=') {
			res = LEQ;
			ptr++;
			column++;
		}
		else {
			res = LSS;
		}
		break;
	case '>':
		ptr++;
		column++;
		if (ptr < text.size() && text[ptr] == '=') {
			res = GEQ;
			ptr++;
			column++;
		}
		else {
			res = GRE;
		}
		break;
	case '=':
		ptr++;
		column++;
		if (ptr < text.size() && text[ptr] == '=') {
			res = EQL;
			ptr++;
			column++;
		}
		else {
			res = ASSIGN;
		}
		break;
	case '!':
		ptr++;
		column++;
		if (ptr < text.size() && text[ptr] == '=') {
			res = NEQ;
			ptr++;
			column++;
		}
		else {
			faultHandler.handleCourseFault(line, LEXICALERROR);
			faultHandler.handleFault(line, "illegal character '!'");
			res = UNKNOWN;
			break;
		}
	case ';':
		res = SEMICN;
		ptr++;
		column++;
		break;
	case ',':
		res = COMMA;
		ptr++;
		column++;
		break;
	case '(':
		res = LPARENT;
		ptr++;
		column++;
		break;
	case ')':
		res = RPARENT;
		ptr++;
		column++;
		break;
	case '{':
		res = LBRACE;
		ptr++;
		column++;
		break;
	case '}':
		res = RBRACE;
		ptr++;
		column++;
		break;
	case '[':
		res = LBRACK;
		ptr++;
		column++;
		break;
	case ']':
		res = RBRACK;
		ptr++;
		column++;
		break;
	case '\'':
		ptr++;
		column++;
		if (ptr >= text.size()) {
			faultHandler.handleCourseFault(line,LEXICALERROR);
			faultHandler.handleFault(line, "unexpected termination:missing \'");
			res = UNKNOWN;
			break;
		}
		result.value = text[ptr];
		if (!isValidChar(text[ptr])) {
			faultHandler.handleCourseFault(line, LEXICALERROR);
			faultHandler.handleFault(line, "illegal character");
			res = UNKNOWN;
			break;
		}
		ptr++;
		column++;
		if (ptr >= text.size()) {
			faultHandler.handleCourseFault(line, LEXICALERROR);
			faultHandler.handleFault(line, "unexpected termination:missing \'");
			res = UNKNOWN;
			break;
		}
		if (text[ptr] != '\'') {
			faultHandler.handleCourseFault(line, LEXICALERROR);
			faultHandler.handleFault(line, "missing \'");
			res = UNKNOWN;
			break;
		}
		ptr++;
		column++;
		res = CHARCON;
		break;
	case '\"':
		ptr++;
		column++;
		result.str = getString();
		if (ptr >= text.size()) {
			faultHandler.handleCourseFault(line, LEXICALERROR);
			faultHandler.handleFault(line, "unexpected termination:missing \"");
			res = UNKNOWN;
			break;
		}
		ptr++;
		column++;
		res = STRCON;
		break;
	default:
		if (isdigit(text[ptr])) {
			int temp = getUnsignedInteger();
			res = INTCON;
			result.value = temp;
		}
		else if (isalpha(text[ptr])) {
			string idfy = getIdentifier();
			map<string, Lexical>::iterator itr = reservedKey.find(idfy);
			if (itr != reservedKey.end()) {
				res = reservedKey[idfy];
				result.str = idfy;
			}
			else {
				res = IDENFR;
				result.str = idfy;
			}
		}
		else {
			faultHandler.handleCourseFault(line, LEXICALERROR);
			faultHandler.handleFault(line, "Illegal lexical");
			res = UNKNOWN;
			break;
		}
	}
	result.type = res;
	currentSym = result;
	return res;
}

void LexicalAnalyzer::homework() {
	map<Lexical, string>lexicalName;
	map<Lexical, string>lexicalSymbol;
	{
		lexicalName[IDENFR] = "IDENFR";
		lexicalName[INTCON] = "INTCON";
		lexicalName[CHARCON] = "CHARCON";
		lexicalName[STRCON] = "STRCON";
		lexicalName[CONSTTK] = "CONSTTK";
		lexicalName[INTTK] = "INTTK";
		lexicalName[CHARTK] = "CHARTK";
		lexicalName[VOIDTK] = "VOIDTK";
		lexicalName[MAINTK] = "MAINTK";
		lexicalName[IFTK] = "IFTK";
		lexicalName[ELSETK] = "ELSETK";
		lexicalName[DOTK] = "DOTK";
		lexicalName[WHILETK] = "WHILETK";
		lexicalName[FORTK] = "FORTK";
		lexicalName[SCANFTK] = "SCANFTK";
		lexicalName[PRINTFTK] = "PRINTFTK";
		lexicalName[RETURNTK] = "RETURNTK";
		lexicalName[PLUS] = "PLUS";
		lexicalName[MINU] = "MINU";
		lexicalName[MULT] = "MULT";
		lexicalName[DIV] = "DIV";
		lexicalName[LSS] = "LSS";
		lexicalName[LEQ] = "LEQ";
		lexicalName[GRE] = "GRE";
		lexicalName[GEQ] = "GEQ";
		lexicalName[EQL] = "EQL";
		lexicalName[NEQ] = "NEQ";
		lexicalName[ASSIGN] = "ASSIGN";
		lexicalName[SEMICN] = "SEMICN";
		lexicalName[COMMA] = "COMMA";
		lexicalName[LPARENT] = "LPARENT";
		lexicalName[RPARENT] = "RPARENT";
		lexicalName[LBRACK] = "LBRACK";
		lexicalName[RBRACK] = "RBRACK";
		lexicalName[LBRACE] = "LBRACE";
		lexicalName[RBRACE] = "RBRACE";

		lexicalSymbol[PLUS] = "+";
		lexicalSymbol[MINU] = "-";
		lexicalSymbol[MULT] = "*";
		lexicalSymbol[DIV] = "/";
		lexicalSymbol[LSS] = "<";
		lexicalSymbol[LEQ] = "<=";
		lexicalSymbol[GRE] = ">";
		lexicalSymbol[GEQ] = ">=";
		lexicalSymbol[EQL] = "==";
		lexicalSymbol[NEQ] = "!=";
		lexicalSymbol[ASSIGN] = "=";
		lexicalSymbol[SEMICN] = ";";
		lexicalSymbol[COMMA] = ",";
		lexicalSymbol[LPARENT] = "(";
		lexicalSymbol[RPARENT] = ")";
		lexicalSymbol[LBRACK] = "[";
		lexicalSymbol[RBRACK] = "]";
		lexicalSymbol[LBRACE] = "{";
		lexicalSymbol[RBRACE] = "}";
	}
	while (1) {
		Lexical tmp = getNextSym();
		if (tmp == END) {
			break;
		}
		else if (tmp == UNKNOWN) {
			cout << "UNKNOWN??????????????????????????" << endl;
		}
		else {
			Result result = sym();
			cout << lexicalName[tmp]<<" ";
			if (result.type == IDENFR||result.type==STRCON) {
				cout << result.str;
			}
			else if (result.type == CHARCON) {
				cout << (char)result.value;
			}
			else if (result.type == INTCON) {
				cout << result.value;
			}
			else if (result.type == CONSTTK||result.type == INTTK || result.type == CHARTK || result.type == VOIDTK ||
				result.type == MAINTK || result.type == IFTK || result.type == ELSETK || result.type == DOTK ||
				result.type == WHILETK || result.type == FORTK || result.type == SCANFTK || result.type == PRINTFTK
				|| result.type == RETURNTK) {
				cout << result.str;
			}
			else {
				cout << lexicalSymbol[result.type];
			}
			cout << endl;
		}
	}
}
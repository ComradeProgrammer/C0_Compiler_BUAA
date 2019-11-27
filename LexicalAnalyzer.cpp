#include"LexicalAnalyzer.h"
#include<cstdlib>
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
}

void LexicalAnalyzer::readAll(string filename) {
	ifstream fin;
	fin.open(filename,ios_base::in);
	string tmp;
	while (getline(fin, tmp)) {
		tmp += '\n';
		text += tmp;
	}
}

Result LexicalAnalyzer::sym() {
	return currentSym;
}
int LexicalAnalyzer::getUnsignedInteger() {
	int res = 0;

	if (text[ptr] == '0') {
		ptr++;
		column++;
		return 0;
	}
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
		if (text[ptr] == '\\') {
			res += text[ptr];
		}
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
	newLine = false;
	while (ptr<text.size()&&isspace(text[ptr])) {
		if (text[ptr] == '\n') {
			line++;
			column = 1;
			newLine = true;
		}
		else {
			column++;
		}
		ptr++;
	}
	if (ptr >= text.size()) {
		result.type = END;
		currentSym = result;
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
			break;
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
			int temp=0;
			temp = getUnsignedInteger();
			res = INTCON;
			result.value = temp;
		}
		else if (isalpha(text[ptr])||text[ptr]=='_') {
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
			ptr++;
			column++;
			res = UNKNOWN;
			break;
		}
	}
	result.type = res;
	currentSym = result;
	return res;
}

int LexicalAnalyzer::lineNumber() {
	if (newLine) { return line - 1; }
	return line;
}

void LexicalAnalyzer::printResult(ostream& out) {
	if (currentSym.type == END || currentSym.type == UNKNOWN) {
		return;
	}
	out << lexicalName[currentSym.type] << " ";
	if (currentSym.type == IDENFR || currentSym.type == STRCON) {
		out << currentSym.str;
	}
	else if (currentSym.type == CHARCON) {
		out << (char)currentSym.value;
	}
	else if (currentSym.type == INTCON) {
		out << currentSym.value;
	}
	else if (currentSym.type == CONSTTK || currentSym.type == INTTK || currentSym.type == CHARTK || currentSym.type == VOIDTK ||
		currentSym.type == MAINTK || currentSym.type == IFTK || currentSym.type == ELSETK || currentSym.type == DOTK ||
		currentSym.type == WHILETK || currentSym.type == FORTK || currentSym.type == SCANFTK || currentSym.type == PRINTFTK
		|| currentSym.type == RETURNTK) {
		out << currentSym.str;
	}
	else {
		out << lexicalSymbol[currentSym.type];
	}
	out << endl;
}
void LexicalAnalyzer::homework() {
	
	ofstream fout;
	fout.open("output.txt", ios_base::out|ios_base::trunc);
	
	while (1) {
		Lexical tmp = getNextSym();
		if (tmp == END) {
			break;
		}
		else if (tmp == UNKNOWN) {
			fout << "UNKNOWN??????????????????????????" << endl;
		}
		else {
			printResult(fout);
		}
	}
	fout.close();
}
void LexicalAnalyzer::printResult(ostream& out,Result currentSym) {
	if (currentSym.type == END || currentSym.type == UNKNOWN) {
		return;
	}
	out << lexicalName[currentSym.type] << " ";
	if (currentSym.type == IDENFR || currentSym.type == STRCON) {
		out << currentSym.str;
	}
	else if (currentSym.type == CHARCON) {
		out << (char)currentSym.value;
	}
	else if (currentSym.type == INTCON) {
		out << currentSym.value;
	}
	else if (currentSym.type == CONSTTK || currentSym.type == INTTK || currentSym.type == CHARTK || currentSym.type == VOIDTK ||
		currentSym.type == MAINTK || currentSym.type == IFTK || currentSym.type == ELSETK || currentSym.type == DOTK ||
		currentSym.type == WHILETK || currentSym.type == FORTK || currentSym.type == SCANFTK || currentSym.type == PRINTFTK
		|| currentSym.type == RETURNTK) {
		out << currentSym.str;
	}
	else {
		out << lexicalSymbol[currentSym.type];
	}
	out << endl;
}
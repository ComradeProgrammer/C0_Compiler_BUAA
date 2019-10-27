#pragma once
#include<cctype>
#include<string>
#include<fstream>
#include<iostream>
#include"FaultHandler.h"
using namespace std;
enum Lexical {
	IDENFR = 0,//标识符
	INTCON,//整形变量
	CHARCON,//字符常量
	STRCON,//字符串
	CONSTTK,//const
	INTTK,//int
	CHARTK,//char
	VOIDTK,//void 
	MAINTK,//main
	IFTK,//if 
	ELSETK,//else
	DOTK,//do
	WHILETK,//while
	FORTK,//for
	SCANFTK,//scanf
	PRINTFTK,//printf
	RETURNTK,//return
	PLUS,//+
	MINU,//-
	MULT,//*
	DIV,// /
	LSS,//<
	LEQ,//<=
	GRE,//>
	GEQ,//>=
	EQL,//==
	NEQ,//!=
	ASSIGN,//=
	SEMICN,//;
	COMMA,//,
	LPARENT,//(
	RPARENT,//)
	LBRACK,//[
	RBRACK,//]
	LBRACE,//{
	RBRACE,//}
	UNKNOWN,
	END
};

struct Result {
	Lexical type;
	int value;//value for int or char
	string str;//value for string or symbol
};

class LexicalAnalyzer {
public:
	LexicalAnalyzer(FaultHandler& f) ;
	void readAll(string filename);

	Result sym();
	Lexical getNextSym();
	int lineNumber();

	void printResult(ostream& out);
	void printResult(ostream& out,Result currentSym);
	void homework();
private:
	string text;
	int ptr;
	int line;
	int column;
	bool newLine;
	map<string, Lexical>reservedKey;
	Result currentSym;
	FaultHandler& faultHandler;
	int getUnsignedInteger();
	bool isValidChar(char c);
	string getString();
	string getIdentifier();

	map<Lexical, string>lexicalName;
	map<Lexical, string>lexicalSymbol;
};

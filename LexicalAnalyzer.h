#pragma once
#include<string>
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
	RBRACE//}
};
/*
class LexicalAnalyzer {
public:
	LexicalAnalyzer(FaultHandler f) ;
	void readAll(string filename);
	Lexical sym();
	Lexical getNextSym();
private:
	string text;
	Lexical currentSym;
	FaultHandler& faultHandler;
};
*/
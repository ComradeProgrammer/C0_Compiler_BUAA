#pragma once
#include<iostream>
#include<vector>
#include"LexicalAnalyzer.h"
#include"SymbolTable.h"
using namespace std;
class GrammarAnalyzer {
public:
	GrammarAnalyzer(FaultHandler& f,SymbolTable& s,LexicalAnalyzer& l,string file);
	Lexical getNextSym();
	void homeworkOn();
	int integer();

	void constDeclearation();
	void constDefination();
	void variableDeclearation();
	void variableDeclearation(Lexical type, string varname);
	void variableDefination(bool wraper = true, Lexical type = UNKNOWN, string name = "");

	void nonVoidFunctionDefination();
	void nonVoidFunctionDefination(Lexical retType, string functionName);
	void voidFunctionDefination();
	void mainFunctionDefination();

	string declearationHeader(Lexical& retType);
	Lexical declearationHeader();

	void parameterList(SymbolEntry* entry);
	void compoundSentence();//未实现

	void factor();//缺一个有返回值的因子没处理
	void term();
	void expression();

	void assignAndCall();
	void assignSentence(string varname);
	void functionCall(string name, bool mustReturn);
	void parameterValueList(SymbolEntry* entry);

	
private:
	FaultHandler& f;
	SymbolTable& table;
	LexicalAnalyzer lex;
	bool course=false;
	fstream out;
	string currentScope ;
	void toNextSemicon();
};

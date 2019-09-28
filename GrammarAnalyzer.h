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
	void homeworkOn(bool _c,bool _l);
	int integer();

	void constDeclearation();
	void constDefination();
	void variableDeclearation();
	void variableDeclearation(Lexical type, string varname);
	void variableDefination(bool wraper = false, Lexical type = UNKNOWN, string name = "");

	void nonVoidFunctionDefination();
	void nonVoidFunctionDefination(Lexical retType, string functionName);
	void voidFunctionDefination();
	void mainFunctionDefination();

	string declearationHeader(Lexical& retType);
	Lexical declearationHeader();

	void parameterList(SymbolEntry* entry);
	void compoundSentence();

	void factor();//缺一个有返回值的因子没处理
	void term();
	void expression();

	void assignAndCall();
	void assignSentence(string varname);//预读了名称
	void functionCall(string name, bool mustReturn);//这个函数应该带走两括号
	void parameterValueList(SymbolEntry* entry);

	void scanSentence();
	void printSentence();
	void returnSentence();

	void ifSentence();
	void condition();
	void loopSentence();

	void sentence();
	void sentenceSeries();

	void programme();
private:
	FaultHandler& f;
	SymbolTable& table;
	LexicalAnalyzer& lex;
	bool course=false;
	fstream out;
	string currentScope ;
	void toNextSemicon();
	bool globalVariableDeclearation = false;
	bool lexicalTest = false;
};

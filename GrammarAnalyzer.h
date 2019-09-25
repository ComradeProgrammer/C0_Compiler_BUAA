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



	void constDeclearation();
	void constDefination();
	void variableDeclearation();
	void variableDeclearation(Lexical type, string varname);
	void variableDefination(bool wraper = true, Lexical type = UNKNOWN, string name = "");

	void nonVoidFunctionDefination();
	void nonVoidFunctionDefination(Lexical retType, string functionName);
	//void voidFunctionDefination();

	string declearationHeader(Lexical& retType);
	Lexical declearationHeader();

	void parameterList(SymbolEntry* entry);
	void compoundSentence();

	int integer();
	
private:
	FaultHandler& f;
	SymbolTable& table;
	LexicalAnalyzer lex;
	bool course=false;
	fstream out;
	string currentScope ;
	void toNextSemicon();

};
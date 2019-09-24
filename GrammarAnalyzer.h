#pragma once
#include<iostream>
#include"LexicalAnalyzer.h"
#include"SymbolTable.h"
using namespace std;
class GrammarAnalyzer {
public:
	GrammarAnalyzer(FaultHandler& f,SymbolTable& s,LexicalAnalyzer& l,string file);

	void constDeclearation();
	void constDefination();

	int integer();
	Lexical getNextSym();
	void homeworkOn();
private:
	FaultHandler& f;
	SymbolTable& table;
	LexicalAnalyzer lex;
	bool course=false;
	fstream out;
	
	void toNextSemicon();

};

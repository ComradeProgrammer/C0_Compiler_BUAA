#pragma once
#include<iostream>
#include<vector>
#include"LexicalAnalyzer.h"
#include"SymbolTable.h"
#include"MidCode.h"
#include"MidCodeFramework.h"
#include"OptimizeSwitch.h"
using namespace std;
struct ReturnBundle {
	bool isChar=false;
	int id=-1;
	int isImmediate = false;
};
class GrammarAnalyzer {
public:
	GrammarAnalyzer(FaultHandler& f,SymbolTable& s,LexicalAnalyzer& l,MidCodeFramework& _raw, string file);
	Lexical getNextSym();
	void homeworkOn(bool _c,bool _l);
	/*整数，会抛出异常*/
	int integer();

	/*常量声明，不会抛出异常*/
	void constDeclearation();

	/*常量定义，会抛出异常*/
	void constDefination();

	/*<变量说明>,供非全局段的变量声明所使用，不会抛出异常*/
	void variableDeclearation();

	/*<变量说明>，供declearationHeader包装调用使用，不会抛出异常*/
	void variableDeclearation(Lexical type, string varname);

	/*<变量定义>,供各种情况使用，会抛出异常*/
	void variableDefination(bool wraper = false, Lexical type = UNKNOWN, string name = "");

	/*<有返回值函数定义>,供非变量声明区的有返回值函数声明,不会抛出异常*/
	void nonVoidFunctionDefination();

	/*有返回值函数定义，供包装器调用使用，不会抛出异常*/
	void nonVoidFunctionDefination(Lexical retType, string functionName);

	/*无返回值定义，不会抛出异常*/
	void voidFunctionDefination();

	/*<主函数>，不会抛出异常*/
	void mainFunctionDefination();

	/*<声明头部>,供非变量声明区的有返回值函数调用,会抛出异常*/
	void declearationHeader(Lexical& retType,string& functionName);

	/*变量声明和有返回值函数定义在尚无法判明的情况下进行预读用的函数，会抛出没法解决的异常*/
	Lexical declearationHeader();

	/*参数表，会抛出异常*/
	void parameterList(SymbolEntry* entry);

	/*复合语句*/
	void compoundSentence();

	/*因子，会抛出异常*/
	ReturnBundle factor();

	/*项，会让异常上溢*/
	ReturnBundle term();

	/*表达式，会让异常上溢*/
	ReturnBundle expression();

	/*针对赋值语句和函数调用语句的预读分支，会让异常上溢*/
	void assignAndCall();

	/*赋值语句，会抛出异常*/
	void assignSentence(string varname);//预读了名称

	/*函数调用语句，会抛出异常*/
	ReturnBundle functionCall(string name, bool mustReturn);//这个函数应该带走两括号

	/*值参数表，会抛出异常*/
	vector<ReturnBundle> parameterValueList(SymbolEntry* entry,bool inlined);

	/*会抛出异常*/
	void scanSentence();

	void printSentence();
	void returnSentence();

	void ifSentence();
	ReturnBundle condition();
	ReturnBundle condition(vector<int>&tmp);
	void loopSentence();
	/*会在此处理各种语句抛出的异常*/
	void sentence();
	void sentenceSeries();

	void programme();
private:
	MidCodeFramework& raw;
	void toNextSemicon();
	void toNextBrace();

	FaultHandler& f;
	SymbolTable& table;
	LexicalAnalyzer& lex;
	
	string currentScope ;
	bool globalVariableDeclearation = false;
	bool hasReturned;
	bool inlineable;
	

	bool course = false;
	fstream out;
	bool lexicalTest = false;
};

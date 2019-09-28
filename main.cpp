#include"main.h"
using namespace std;
int main() {
    FaultHandler faultHandler("error.txt");
	//faultHandler.debugOn();
	LexicalAnalyzer lexicalAnalyzer(faultHandler);
	lexicalAnalyzer.readAll("testfile.txt");
	SymbolTable test;
	//test.selfTest();
	lexicalAnalyzer.getNextSym();
	GrammarAnalyzer g(faultHandler,test,lexicalAnalyzer,"output.txt");
	g.homeworkOn(true,true);
	//g.constDeclearation();
	//g.mainFunctionDefination();
	//g.declearationHeader();
	//g.declearationHeader();
	//g.declearationHeader();
	//g.voidFunctionDefination();
	//g.declearationHeader();
	//g.declearationHeader();
	//g.declearationHeader();
	//g.expression();
	//g.assignAndCall();
	//g.scanSentence();
	//g.returnSentence();
	//g.sentence();
	//g.mainFunctionDefination();
	g.programme();
	//system("pause");
}


#include"main.h"
using namespace std;
int main() {
    FaultHandler faultHandler("error.txt");
	faultHandler.debugOn();
	LexicalAnalyzer lexicalAnalyzer(faultHandler);
	lexicalAnalyzer.readAll("testfile.txt");
	//lexicalAnalyzer.homework();
	SymbolTable test;
	//test.selfTest();
	lexicalAnalyzer.getNextSym();
	GrammarAnalyzer g(faultHandler,test,lexicalAnalyzer,"output.txt");
	g.homeworkOn();
	g.constDeclearation();
	g.declearationHeader();
	//g.declearationHeader();
	system("pause");
}
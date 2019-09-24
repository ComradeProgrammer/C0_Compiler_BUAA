#include"main.h"
using namespace std;
int main() {
    FaultHandler faultHandler("error.txt");
	LexicalAnalyzer lexicalAnalyzer(faultHandler);
	lexicalAnalyzer.readAll("testfile.txt");
	//lexicalAnalyzer.homework();
	SymbolTable test;
	//test.selfTest();
	lexicalAnalyzer.getNextSym();
	GrammarAnalyzer g(faultHandler,test,lexicalAnalyzer,"output.txt");
	g.homeworkOn();
	g.constDeclearation();
	system("pause");
}
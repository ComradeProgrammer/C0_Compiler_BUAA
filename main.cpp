#include"main.h"
using namespace std;
int main() {
    /*FaultHandler faultHandler("error.txt");
	LexicalAnalyzer lexicalAnalyzer(faultHandler);
	lexicalAnalyzer.readAll("testfile.txt");
	lexicalAnalyzer.homework();*/
	SymbolTable test;
	test.selfTest();
	system("pause");
}
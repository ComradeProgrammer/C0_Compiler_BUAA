#include"main.h"
using namespace std;
int main() {
    FaultHandler faultHandler("error.txt");
	faultHandler.debugOn();
	LexicalAnalyzer lexicalAnalyzer(faultHandler);
	lexicalAnalyzer.readAll("testfile.txt");
	SymbolTable symbol;
	//test.selfTest();
	lexicalAnalyzer.getNextSym();
	GrammarAnalyzer g(faultHandler,symbol,lexicalAnalyzer,"output.txt");
	g.homeworkOn(true,true);
	//symbol.debugOn();
	
	g.programme();
	system("pause");
	return 0;
}
/*词法分析中001应该是3个数但是我好像没改*/

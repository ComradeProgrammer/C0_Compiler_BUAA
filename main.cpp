#include"main.h"
using namespace std;

int main() {
	fstream f;
	f.open("debug.txt", ios_base::trunc | ios_base::out);

    FaultHandler faultHandler("error.txt");
	faultHandler.debugOn();

	LexicalAnalyzer lexicalAnalyzer(faultHandler);
	lexicalAnalyzer.readAll("testfile.txt");
	lexicalAnalyzer.getNextSym();

	SymbolTable symbolTable;
	//	symbolTable.debugOn();
	MidCode::table = &symbolTable;
	SubSymbolTable::table = &symbolTable;

	MipsTranslator mips("mips.txt");

	MidCodeFramework frame(mips);
	GrammarAnalyzer grammarAnalyzer(faultHandler,symbolTable,lexicalAnalyzer,frame,"output.txt");
	//grammarAnalyzer.homeworkOn(true,true);

	grammarAnalyzer.programme();

	frame.optimize();
	f << frame;
	f << endl << endl;
	f << symbolTable;
	f << endl << endl;
	frame.generateMips();
	
	system("pause");

	return 0;
}
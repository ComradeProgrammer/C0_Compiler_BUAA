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
	/*
	MipsGenerator mips;
	mips.outputFile("mips.txt");
	FlowChart flowchart(container, mips);
	flowchart.optimize();
	flowchart.summarize();
	f << flowchart << endl << endl;
	f << symbolTable << endl << endl;
	flowchart.conflictEdgeAnalyze();
	mips.printConflictMap(f);
	f << endl << endl;
	mips.globalRegisterAlloc();
	mips.printRegisterAllocStatus(f);
	mips.generateProgramHeader();
	flowchart.go();*/
	system("pause");

	return 0;
}
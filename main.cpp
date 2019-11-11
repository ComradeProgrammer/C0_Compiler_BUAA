#include"main.h"
using namespace std;

int main() {
    FaultHandler faultHandler("error.txt");
	faultHandler.debugOn();

	LexicalAnalyzer lexicalAnalyzer(faultHandler);
	lexicalAnalyzer.readAll("testfile.txt");
	lexicalAnalyzer.getNextSym();

	SymbolTable symbolTable;
	//	symbolTable.debugOn();
	MidCode::table = &symbolTable;
	SubSymbolTable::table = &symbolTable;
	MidCodeContainer container;
	GrammarAnalyzer grammarAnalyzer(faultHandler,symbolTable,lexicalAnalyzer,container,"output.txt");
	//grammarAnalyzer.homeworkOn(true,true);

	grammarAnalyzer.programme();
	container.removeNops();
	fstream f;
	f.open("debug.txt", ios_base::trunc | ios_base::out);
	f << container;
	f << endl << endl;
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
	flowchart.go();
	system("pause");
	return 0;
}
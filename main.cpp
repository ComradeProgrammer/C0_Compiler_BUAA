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

	//=============测试===================
	//grammarAnalyzer.variableDeclearation();
	//grammarAnalyzer.nonVoidFunctionDefination();
	//grammarAnalyzer.declearationHeader();	
	//grammarAnalyzer.declearationHeader();
	//grammarAnalyzer.assignAndCall();
	//grammarAnalyzer.ifSentence();
	//grammarAnalyzer.loopSentence();
	//grammarAnalyzer.voidFunctionDefination();
	//grammarAnalyzer.sentenceSeries();
	//====================================
	container.removeNops();
	fstream f;
	f.open("debug.txt", ios_base::trunc | ios_base::out);
	f << container;
	f << endl << endl;
	FlowChart flowchart(container);
	//flowchart.activeVariableAnalyze();
	//f << flowchart;
	flowchart.summarize();
	symbolTable.summary();
	cout << symbolTable;
	system("pause");
	return 0;
}
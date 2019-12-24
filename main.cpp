#include"main.h"
using namespace std;

bool constantSubstitutionSwitch=false;
bool inlineSwitch=false;
bool propagationSwitch=false;
bool dagMapSwitch=false;
bool deadCodeEliminateSwitch=false;
bool PeepHoleSwitch=false;

int main(int argc, char* argv[]) {
	string inputFile = "testfile.txt";
	string outputFile = "mips.txt";
	string errorFile = "";
	string debugFile = "";
	string recursiveDescendInformationFile = "";
	cout << "Final project of 'Complier Design' Course(Autumn2019),SCSE BUAA" << endl;
	
	/*添加命令行参数支持与学校验收测试的支持*/
	if (argc==1) {
		//如果是学校验收的话无参数
		cout << "HomeWork Acceptance Test Mode" << endl;
		errorFile = "error.txt";
		debugFile = "debug.txt";
		//recursiveDescendInformationFile = "output.txt";
		constantSubstitutionSwitch = true;
		inlineSwitch = true;
		propagationSwitch = true;
		dagMapSwitch = true;
		deadCodeEliminateSwitch = true;
		PeepHoleSwitch = true;
	}
	else {
		//使用了命令行参数
		inputFile = argv[1];
		for (int i = 2; i < argc; i++) {
			string header = argv[i];
			if (header == "-opt") {
				constantSubstitutionSwitch = true;
				inlineSwitch = true;
				propagationSwitch = true;
				dagMapSwitch = true;
				deadCodeEliminateSwitch = true;
				PeepHoleSwitch = true;
				continue;
			}

			if (i + 1 >= argc) {
				cout << "invalid parameter for operand " << header << endl;
				exit(0);
			}
			string para = argv[i + 1];
			if (header == "-o") {
				outputFile = para;
			}
			else if (header == "-d") {
				debugFile = para;
			}
			else if (header == "-rdi") {
				recursiveDescendInformationFile = para;
			}
			else if (header == "-h") {
				/*准备说明文档*/
				cout << "usage: C0compiler.exe sourcefile " << endl;
				cout << "[-o outputfile] " << endl;
				cout << "[-d debugInformation]" << endl;
				cout << "[-rdi recursiveDescendInformationFile]" << endl;
				cout << "[-h] help document" << endl;
				cout << "[-opt] turn on the optimization" << endl;

			}
			else {
				cout << "invalid operand " << header << endl;
				exit(0);
			}
			i++;
		}
	}
	/*实例化各个组件===========================*/
	//符号表
	SymbolTable symbolTable;
	MidCode::table = &symbolTable;
	SubSymbolTable::table = &symbolTable;
	//错误处理
    FaultHandler faultHandler(errorFile);
	faultHandler.debugOn();
	//词法分析
	LexicalAnalyzer lexicalAnalyzer(faultHandler);
	lexicalAnalyzer.readAll(inputFile);
	lexicalAnalyzer.getNextSym();
	//目标代码生成
	MipsTranslator mips(outputFile);
	//编译器框架
	MidCodeFramework frame(mips);
	//语法分析
	GrammarAnalyzer grammarAnalyzer(faultHandler,symbolTable,lexicalAnalyzer,
		frame,recursiveDescendInformationFile);
	if (recursiveDescendInformationFile != "") {
		grammarAnalyzer.homeworkOn(true, true);
	}
	/*开始编译*/
	fstream f;
	if (debugFile != "") {
		f.open(debugFile, ios_base::trunc | ios_base::out);
	}
	grammarAnalyzer.programme();

	if (faultHandler.haveBug()) {
		faultHandler.terminate();
	}
	else {
		frame.optimize();
		if (debugFile != "") {
			f << "BEFORE BACKEND OPTIMIZATION" << endl;
			f << frame;
			f << endl << endl;
		}
		if (debugFile != "") {
			f << "BEFORE BACKEND OPTIMIZATION" << endl;
			frame.dumpNewMidCode(f);
			f << endl << endl;
			f << "SYMBOLTABLE" << endl;
			f << symbolTable;
			f << endl << endl;
		}
		frame.generateMips();
		system("pause");
	}
	return 0;
}
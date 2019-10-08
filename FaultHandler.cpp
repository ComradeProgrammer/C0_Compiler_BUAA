#include"FaultHandler.h"

FaultHandler::FaultHandler(string filename) {
	debug = false;
	fout.open(filename,ios_base::out|ios_base::trunc);
	messages[LEXICALERROR] = "lexical error";
	messages[REDEFINED] = "redefined symbol";
	messages[UNDEFINED] = "undifined symbol";
	messages[PARANUMERROR] = "number of parameters does not match";
	messages[TYPEERROR] = "type does not match";
	messages[NOSEMICN] = " ; required";
	messages[NORPARENT] = ") required";
	messages[NORBRACK] = "] required";
}

FaultHandler::~FaultHandler() {
	fout.close();
}

void FaultHandler::handleCourseFault(int line, FaultType type) {
	fout << line << " "<<(char)type<<endl;
	//handleFault(line, messages[type]);
}

void FaultHandler::handleFault(int line, string information) {
	if (debug) {
		cout << "Error @ line " << line << ": " << information << endl;
	}
}

void FaultHandler::debugOn() {
	debug = true;
}


void FaultHandler::terminate() {
	cout << endl << "Compilation Terminated"<<endl;
	system("pause");
	exit(0);
}

void FaultHandler::test() {
	debugOn();
	handleCourseFault(15, LEXICALERROR);
	handleCourseFault(107,NORPARENT);
}
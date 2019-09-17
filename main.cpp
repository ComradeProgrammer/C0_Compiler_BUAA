#include"main.h"
using namespace std;
int main() {
    FaultHandler faultHandler("error.txt");
	LexicalAnalyzer lexicalAnalyzer(faultHandler);
	lexicalAnalyzer.readAll("input.txt");
	lexicalAnalyzer.homework();
	system("pause");
}
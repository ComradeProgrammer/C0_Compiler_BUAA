#include"MidCodeFramework.h"

MidCodeFramework::MidCodeFramework(MipsTranslator& _mips):mips(_mips) {
}

void MidCodeFramework::functionStart(string name) {
	container = MidCodeContainer();
	container.functionName = name;
}

void MidCodeFramework::functionEnd() {
	functionContainer.push_back(container);
}

int MidCodeFramework::midCodeInsert(MidCodeOp op, int target,
	int operand1, bool isImmediate1,
	int operand2, bool isImmediate2,
	int label) {
	return container.midCodeInsert(op, target,
		operand1, isImmediate1, operand2, isImmediate2,label);
}

void MidCodeFramework::midCodeInsert(vector<MidCode>&segment) {
	container.midCodeInsert(segment);
}

int MidCodeFramework::getIndex() {
	return container.getIndex();
}

void MidCodeFramework::erase(int start, int end) {
	container.erase(start, end);
}

vector<MidCode>::iterator MidCodeFramework::getIterator(int index) {
	return container.getIterator(index);
}

void MidCodeFramework::removeNops() {
	for (MidCodeContainer& i : functionContainer) {
		i.removeNops();
	}
}

ostream& operator<<(ostream& out, MidCodeFramework frame) {
	MidCode::table->getSubSymbolTableByName("")->dumpMidCode(out);
	for (MidCodeContainer& c : frame.functionContainer) {
		out << c;
	}
	out << endl;
	for (FlowGraph g : frame.graph) {
		out << g;
	}
	return out;
}

void MidCodeFramework::optimize() {
	removeNops();
	for (MidCodeContainer& c : functionContainer) {
		graph.push_back(FlowGraph(c));
	}
	for (FlowGraph& g : graph) {
		g.optimize();
	}
	report = MidCode::table->summary();
	mips.setReport(report);


}

void MidCodeFramework::generateMips() {
	mips.generateProgramHeader();
	for (FlowGraph g : graph) {
		mips.translateFunction(g);
	}
}
#include"MidCodeFramework.h"
#include"GrammarAnalyzer.h"
MidCodeFramework::MidCodeFramework(MipsTranslator& _mips):mips(_mips) {
}

MidCodeFramework::~MidCodeFramework(){
	for (Block* b : FlowGraph::allGraph) {
		delete b;
	}
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

ostream& operator<<(ostream& out, MidCodeFramework& frame) {
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

void MidCodeFramework::dumpNewMidCode(ostream&out) {
	MidCode::table->getSubSymbolTableByName("")->dumpMidCode(out);
	for (FlowGraph& g : graph) {
		string functionName = MidCode::table->getSymbolById(g.functionId)->name;
		
		for (Block* b : g.graph) {
			for (MidCode& c : b->v) {
				if(c.op==MIDFUNC)
					MidCode::table->getSubSymbolTableByName(functionName)->dumpMidCode(out);
				out << c;
			}
		}
	}
}

vector<MidCode>MidCodeFramework::inlinedSimpleFunction(string functionName
	, vector<ReturnBundle>parameters, int returnVar=-1) {
	//在目前版本的内联中，被内联的函数不能有函数调用，不能有跳转，不能有读写,不能有数组
	vector<MidCode>res;
	map<int, int>fakeTable;
	SymbolEntry* entry = MidCode::table->getSymbolByName("",functionName);
	//处理符号表中已知的所有变量
	SubSymbolTable* table = MidCode::table->getSubSymbolTableByName(functionName);
	for (auto& i : table->symbolMap) {
		int tmpVar = MidCode::tmpVarAlloc();
		fakeTable[i.second->id] = tmpVar;
	}
	//处理函数参数
	for (int i = 0; i < entry->link->paraNum; i++) {
		//还需要插入赋值语句令新生成的临时变量等于参数
		MidCode tmp = MidCode::generateMidCode(MIDASSIGN, 
			fakeTable[entry->link->paraIds[i]],
			parameters[i].id,parameters[i].isImmediate,
			MIDUNUSED, false, MIDNOLABEL);
		res.push_back(tmp);
	}
	
	MidCodeContainer func;
	//找出对应的函数中间代码
	for (MidCodeContainer& c : functionContainer) {
		if (c.functionName == functionName) {
			func = c;
		}
	}
	for (MidCode& c : func.v) {
		//逐句处理
		MidCode newMidCode = c;
		switch (c.op) {
			case MIDFUNC:
			case MIDPARA:
				break;
			//这都是不应该出现的指令
			case MIDCALL:
			case MIDARRAYGET:
			case MIDARRAYWRITE:
			case MIDGOTO:
			case MIDBNZ:
			case MIDBZ:
			case MIDREADCHAR:
			case MIDREADINTEGER:
				cout << "bug @ inlinedFunction" << endl;
				break;
			case MIDRET:
				if (entry->link->returnType == RETVOID) {
					break;
				}
				else {
					//当一个变量是全局变量时不应替换，局部变量应该替换
					int newVar;
					if (c.isImmediate1) {
						newVar = c.operand1;
					}
					else {
						SymbolEntry* varEntry = MidCode::table->getSymbolById(c.operand1);
						if (!c.operand1<0&&varEntry->scope == "") {
							newVar = c.operand1;
						}
						else {
							newVar = fakeTable[c.operand1];
						}
					}
					MidCode tmp = MidCode::generateMidCode(MIDASSIGN, returnVar,
						newVar, c.isImmediate1,
						MIDUNUSED, false, MIDNOLABEL);//赋返回值
					res.push_back(tmp);
					break;
				}
			case MIDADD:
			case MIDSUB:
			case MIDMULT:
			case MIDDIV:
			case MIDLSS:
			case MIDLEQ:
			case MIDGRE:
			case MIDGEQ:
			case MIDEQL:
			case MIDNEQ:
			{
				SymbolEntry* targetEntry = MidCode::table->getSymbolById(c.target);
				if (c.target<0||targetEntry->scope != "") {
					//凡非全局变量均应当替换,未分配替换变量者应当分配
					if (fakeTable.find(c.target) == fakeTable.end()) {
						fakeTable[c.target] = MidCode::tmpVarAlloc();
					}
					newMidCode.target = fakeTable[c.target];
				}
				if (!c.isImmediate1) {
					//若为常量不需要替换
					SymbolEntry* op1Entry = MidCode::table->getSymbolById(c.operand1);
					if (c.operand1<0||op1Entry->scope != "") {
						if (fakeTable.find(c.operand1) == fakeTable.end()) {
							fakeTable[c.operand1] = MidCode::tmpVarAlloc();
						}
						newMidCode.operand1 = fakeTable[c.operand1];
					}
				}
				if (!c.isImmediate2) {
					//若为常量不需要替换
					SymbolEntry* op2Entry = MidCode::table->getSymbolById(c.operand2);
					if (c.operand2<0||op2Entry->scope != "") {
						if (fakeTable.find(c.operand2) == fakeTable.end()) {
							fakeTable[c.operand2] = MidCode::tmpVarAlloc();
						}
						newMidCode.operand2 = fakeTable[c.operand2];
					}
				}
				res.push_back(newMidCode);
				break;
			}
			case MIDNEGATE:
			case MIDASSIGN:
			case MIDPRINTCHAR:
			case MIDPRINTINT:
			{
				SymbolEntry* targetEntry = MidCode::table->getSymbolById(c.target);
				if (c.target < 0 || targetEntry->scope != "") {
					//凡非全局变量均应当替换,未分配替换变量者应当分配
					if (fakeTable.find(c.target) == fakeTable.end()) {
						fakeTable[c.target] = MidCode::tmpVarAlloc();
					}
					newMidCode.target = fakeTable[c.target];
				}
				if (!c.isImmediate1) {
					//若为常量不需要替换
					SymbolEntry* op1Entry = MidCode::table->getSymbolById(c.operand1);
					if (c.operand1 < 0 || op1Entry->scope != "") {
						if (fakeTable.find(c.operand1) == fakeTable.end()) {
							fakeTable[c.operand1] = MidCode::tmpVarAlloc();
						}
						newMidCode.operand1 = fakeTable[c.operand1];
					}
				}
				res.push_back(newMidCode);
				break;
			}
			
			case MIDPRINTSTRING: 
			case MIDNOP:
				res.push_back(newMidCode);
				break;			
		}
	}
	return res;
}
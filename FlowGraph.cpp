#include"FlowGraph.h"
vector<Block*>FlowGraph::allGraph;
FlowGraph::FlowGraph(MidCodeContainer& c) {
	map<int, Block*>codeToBlock;
	map<int, Block*>labelToBlock;
	
	functionName = c.functionName;
	functionId = MidCode::table->getSymbolByName("", functionName)->id;
	Block* block = new Block(functionId);
	allGraph.push_back(block);
	for (int i = 0; i < c.v.size(); i++) {
		if (c.v[i].labelNo != MIDNOLABEL||
			(i != 0 && ( c.v[i - 1].op == MIDBNZ ||c.v[i - 1].op == MIDBZ||c.v[i-1].op==MIDCALL
				|| c.v[i - 1].op == MIDREADINTEGER|| c.v[i - 1].op == MIDREADCHAR))) {
			Block* oldBlock = block;
			block = new Block(functionId);
			allGraph.push_back(block);
			graph.push_back(oldBlock);
			if (!(i != 0 && (c.v[i - 1].op == MIDGOTO || c.v[i - 1].op == MIDRET))) {
				addLink(oldBlock, block);
			}
		}
		else if (i != 0 &&( c.v[i - 1].op == MIDGOTO||c.v[i-1].op==MIDRET)) {
			Block* oldBlock = block;
			block = new Block(functionId);
			graph.push_back(oldBlock);
			allGraph.push_back(block);
		}
		block->insert(c.v[i]);
		codeToBlock[i] = block;
		if (c.v[i].labelNo != MIDNOLABEL) {
			labelToBlock[c.v[i].labelNo] = block;
		}
	}
	graph.push_back(block);
	for (int i = 0; i < c.v.size(); i++) {
		if (c.v[i].op == MIDBNZ || c.v[i].op == MIDBZ) {
			Block* fromBlock = codeToBlock[i];
			Block* toBlock1 = labelToBlock[c.v[i].operand2];
			addLink(fromBlock, toBlock1);
		}
		else if (c.v[i].op == MIDGOTO) {
			Block* fromBlock = codeToBlock[i];
			Block* toBlock1 = labelToBlock[c.v[i].operand1];
			addLink(fromBlock, toBlock1);
		}
	}
}
FlowGraph::~FlowGraph() {
	
}

void FlowGraph::addLink(Block* from, Block* to) {
	if (from == NULL || to == NULL) {
		return;
	}
	from->addNext(to);
	to->addPrev(from);
}

void FlowGraph::optimize() {
	if (dagMapSwitch) {
		activeVariableAnalyze();
		DAGoptimize();
	}

	if (propagationSwitch) {
		activeVariableAnalyze();
		blockOptimize();
	}

	if (deadCodeEliminateSwitch) {
		activeVariableAnalyze();
		eliminateDeadCode();
	}

	if (PeepHoleSwitch) {
		activeVariableAnalyze();
		activeVariablePerLine();
		peepholeOptimize();
	}

	activeVariableAnalyze();
	variableSummary();
	activeVariablePerLine();
	conflictEdgeAnalyze();
}

void FlowGraph::activeVariableAnalyze() {
	for (Block* i : graph) {
		i->def.clear();
		i->use.clear();
		i->activeIn.clear();
		i->activeOut.clear();
	}
	for (Block* i : graph) {
		i->useDefScan();
	}
	int n = graph.size();
	while (1) {
		bool changed = false;
		for (int i = n - 1; i >= 0; i--) {
			bool status = graph[i]->activeVariableAnalyzeEpoch();
			changed = changed || status;
		}
		if (!changed) {
			break;
		}
	}
}

void FlowGraph::variableSummary() {
	if (graph.size() == 1) {
		SymbolEntry* currentFunction = MidCode::table->getSymbolById(functionId);
		currentFunction->link->inlineable = true;
	}
	for (Block* i : graph) {
		//所有跨基本块生存的变量
		for (int active: i->activeIn) {
			globalVariable.insert(active);
		}
		for (MidCode& j : i->v) {
			switch (j.op) {
				//operand1 only
			case MIDPUSH:
			case MIDRET:
			case MIDBNZ:
			case MIDBZ:
			case MIDPRINTINT:
			case MIDPRINTCHAR:
				if (!j.isImmediate1 && j.operand1 < -1) {
					tmpVariable.insert(j.operand1);
				}
				if (j.operand1 != -1) {
					allVariable.insert(j.operand1);
				}
				break;
				//all
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
			case MIDREM:
				if (!j.isImmediate1 && j.operand1 < -1) {
					tmpVariable.insert(j.operand1);
				}
				if (!j.isImmediate2 && j.operand2 < -1) {
					tmpVariable.insert(j.operand2);
				}
				if (j.target < -1) {
					tmpVariable.insert(j.target);
				}
				if (j.operand1 != -1) {
					allVariable.insert(j.operand1);
				}
				if (j.operand2 != -1) {
					allVariable.insert(j.operand1);
				}
				if (j.target != -1) {
					allVariable.insert(j.target);
				}
				break;
				//target and operand1 only
			case MIDNEGATE:
			case MIDASSIGN:
				if (j.target < -1) {
					tmpVariable.insert(j.target);
				}
				if (!j.isImmediate1 && j.operand1 < -1) {
					tmpVariable.insert(j.operand1);
				}
				if (j.target != -1) {
					allVariable.insert(j.target);
				}
				if (j.operand1 != -1) {
					allVariable.insert(j.operand1);
				}
				break;
				//target and operand2
			case MIDARRAYGET:
				if (j.target < -1) {
					tmpVariable.insert(j.target);
				}
				if (!j.isImmediate2 && j.operand2 < -1) {
					tmpVariable.insert(j.operand2);
				}
				//数组变量不会被作为临时变量但是需要考虑进全部变量
				if (j.target != -1) {
					allVariable.insert(j.target);
				}
				if (j.operand1 != -1) {
					allVariable.insert(j.operand1);
				}
				if (j.operand2 != -1) {
					allVariable.insert(j.operand1);
				}
				break;
				//operand1 and operand2
			case MIDARRAYWRITE:
				if (!j.isImmediate1 && j.operand1 < -1) {
					tmpVariable.insert(j.operand1);
				}
				if (!j.isImmediate2 && j.operand2 < -1) {
					tmpVariable.insert(j.operand2);
				}
				if (j.target != -1) {
					allVariable.insert(j.target);
				}
				if (j.operand1 != -1) {
					allVariable.insert(j.operand1);
				}
				if (j.operand2 != -1) {
					allVariable.insert(j.operand1);
				}
				break;
				// TARGET ONLY
			case MIDREADINTEGER:
			case MIDREADCHAR:
				if (j.target < -1) {
					tmpVariable.insert(j.target);
				}
				if (j.target != -1) {
					allVariable.insert(j.target);
				}
				break;
			}
		}
	}

	for (int j : tmpVariable) {
		MidCode::table->addTmpSymbol(functionName, j);
	}

	/*去除不参与全局寄存器分配的变量*/
	set<int>del;
	for (int i : globalVariable) {
		SymbolEntry* tmp = MidCode::table->getSymbolById(i);
		//这些变量不参与全局寄存器分配是因为带有初值
		if(tmp->scope==""|| tmp->type == TYPECHARCONST || tmp->type == TYPEINTCONST
			|| tmp->type == TYPEINTARRAY || tmp->type == TYPECHARARRAY || tmp->isParameter) {
			del.insert(i);
		}
	}
	for (int i : del) {
		globalVariable.erase(i);
	}
	
}

void FlowGraph::conflictEdgeAnalyze() {
	for (Block* i : graph) {
		vector<vector<int>>conflict=i->conflictEdgeAnalyze();
		for (vector<int>& j : conflict) {
			set<int>::iterator itr1 =globalVariable.find(j[0]);
			set<int>::iterator itr2 =globalVariable.find(j[1]);
			if (itr1 != globalVariable.end() && itr2 != globalVariable.end()) {
				conflictEdges.push_back(j);
			}
		}
	}
}

void FlowGraph::DAGoptimize() {
	for (Block* i : graph) {
		i->DAGoptimize();
	}
}

void FlowGraph::eliminateDeadCode() {
	for (Block* i : graph) {
		i->eliminateDeadCode();
	}
}

void FlowGraph::blockOptimize() {
	for (Block* i : graph) {
		i->blockOptimize();
	}
}

void FlowGraph::activeVariablePerLine() {
	for (Block* i : graph) {
		i->activeVariableAnalyzePerLine();
	}
}
ostream& operator<<(ostream& out, FlowGraph& f) {
	for (Block* i : f.graph) {
		out << (*i);
	}
	return out;
}

void FlowGraph::peepholeOptimize() {
	for (Block* i : graph) {
		i->peepholeOptimize();
	}
}

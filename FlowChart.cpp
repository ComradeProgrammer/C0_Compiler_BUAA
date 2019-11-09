#include"FlowChart.h"

void FlowChart::addLink(Block* from, Block* to) {
	if (from == NULL || to == NULL) {
		return;
	}
	from->addNext(to);
	to->addPrev(from);
}

FlowChart::FlowChart(MidCodeContainer& c) {
	map<int, Block*> codeToBlock;
	map<int, Block*>labelToBlock;
	map<int, Block*>callToBlock;
	map<int, set<Block*>>retToBlock;
	int currentFunction = -1;
	Block* block = NULL;
	//第一趟划分基本块并建立基本块的基本连接
	for (int i = 0; i < c.v.size(); i++) {
		//可以是跳转的目标语句或者函数开始
		if (c.v[i].labelNo != MIDNOLABEL || c.v[i].op == MIDFUNC){
			if (c.v[i].op == MIDFUNC&&currentFunction!=-1) {
				retToBlock[currentFunction].insert(block);
			}
			Block* oldBlock = block;
			chart.push_back(block);
			block = new Block();
			if (c.v[i].op == MIDFUNC) {
				currentFunction = c.v[i].operand1;
				callToBlock[currentFunction] = block;
				block->functionId = currentFunction;
				/*if (c.v[i].operand1 == MidCode::table->mainSymbolId) {
					start = block;
				}*/
			}
			if ((c.v[i].op != MIDFUNC) &&
				(i != 0 && c.v[i - 1].op != MIDRET && c.v[i - 1].op != MIDGOTO&&
					c.v[i - 1].op != MIDCALL)) {
				addLink(oldBlock, block);
			}
		}
		//前一句控制流转移
		else if (i!=0&&(c.v[i - 1].op == MIDCALL || c.v[i - 1].op == MIDRET ||
			c.v[i - 1].op == MIDGOTO || c.v[i - 1].op == MIDBNZ ||
			c.v[i - 1].op == MIDBZ)){
			chart.push_back(block);
			Block* oldBlock = block;
			block = new Block();
			if ((c.v[i].op != MIDFUNC) &&
				(i != 0 && c.v[i - 1].op != MIDRET && c.v[i - 1].op != MIDGOTO &&
					c.v[i - 1].op != MIDCALL)) {
				addLink(oldBlock, block);
			}
		}
		block->insert(c.v[i]);
		//记录标签所在的块
		if (c.v[i].labelNo != MIDNOLABEL) {
			labelToBlock[c.v[i].labelNo] = block;
		}
		if (c.v[i].op == MIDRET) {
			retToBlock[currentFunction].insert(block);
		}
		codeToBlock[i] = block;
	}
	chart.push_back(block);
	//第二趟建立基本块的链接
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
		else if (c.v[i].op == MIDCALL) {
			Block* currentBlock = codeToBlock[i];
			Block* targetHead = callToBlock[c.v[i].operand1];
			addLink(currentBlock, targetHead);
			if (i + 1 < c.v.size()) {
				Block* returnTarget = codeToBlock[i + 1];
				for (Block* ret : retToBlock[c.v[i].operand1]) {
					addLink(ret, returnTarget);
				}
			}
		}
	}
	chart.erase(chart.begin());//删掉第一个NULL
	/*end = *(chart.end() - 1);*/
}

void FlowChart::activeVariableAnalyze() {
	for (Block* i : chart) {
		i->useDefScan();
	}
	int n = chart.size();
	while (1) {
		bool changed = false;
		for(int i=n-1;i>=0;i--){
			bool status=chart[i]->activeVariableAnalyzeEpoch();
			changed = changed || status;
		}
		if (!changed) {
			break;
		}
	}
}

void FlowChart::summarize() {
	map<int, set<int>>tmpVar;
	for (Block* i : chart) {
		int functionId = i->functionId;
		for (MidCode& j : i->v) {
			switch (j.op) {
				//operand1 only
				case MIDPUSH:
				case MIDRET:
				case MIDBNZ:
				case MIDBZ:
				case MIDPRINTINT:
				case MIDPRINTCHAR:
					if (!j.isImmediate1 && j.operand1!= -1) {
						tmpVar[functionId].insert(j.operand1);
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
					if (!j.isImmediate1&&j.operand1<0) {
						tmpVar[functionId].insert(j.operand1);
					}
					if (!j.isImmediate2 && j.operand2 < 0) {
						tmpVar[functionId].insert(j.operand2);
					}
					if (j.target < 0) {
						tmpVar[functionId].insert(j.target);
					}
					break;
				//target and operand1 only
				case MIDNEGATE:
				case MIDASSIGN:
					if (j.target < 0) {
						tmpVar[functionId].insert(j.target);
					}
					if (!j.isImmediate1 && j.operand1 != -1&&j.operand1<0) {
						tmpVar[functionId].insert(j.operand1);
					}
					break;
				//target and operand2
				case MIDARRAYGET:
					if (j.target < 0) {
						tmpVar[functionId].insert(j.target);
					}
					if (!j.isImmediate2&&j.operand2<0) {
						tmpVar[functionId].insert(j.operand2);
					}
					break;
					//operand1 and operand2
				case MIDARRAYWRITE:
					if (!j.isImmediate1 && j.operand1 < 0) {
						tmpVar[functionId].insert(j.operand1);
					}
					if (!j.isImmediate2 && j.operand2 < 0) {
						tmpVar[functionId].insert(j.operand2);
					}
					break;
				// TARGET ONLY
				case MIDREADINTEGER:
				case MIDREADCHAR:
					if (j.target < 0) {
						tmpVar[functionId].insert(j.target);
					}
					break;
			}
		}
	}
	for (map<int, set<int>>::iterator i = tmpVar.begin(); i != tmpVar.end(); i++) {
		string name = MidCode::table->getSymbolById(i->first)->name;
		for (int j : i->second) {
			MidCode::table->addTmpSymbol(name, j);
		}
	}
}
ostream& operator<<(ostream& out, FlowChart f) {
	for (Block* i : f.chart) {
		out << (*i);
	}
	return out;
}


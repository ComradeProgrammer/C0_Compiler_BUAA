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


ostream& operator<<(ostream& out, FlowChart f) {
	for (Block* i : f.chart) {
		out << (*i);
	}
	return out;
}


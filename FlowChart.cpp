#include"FlowChart.h"

void FlowChart::addLink(Block* from, Block* to) {
	if (from == NULL || to == NULL) {
		return;
	}
	from->addNext(to);
	to->addPrev(from);
}

FlowChart::FlowChart(MidCodeContainer& c, MipsGenerator& m):mips(m) {
	//根据给出的中间代码建立流图
	map<int, Block*> codeToBlock;//建立代码在容器中下标到所在块的关系
	map<int, Block*>labelToBlock;//建立标签编号到所在块的关系
	map<int, Block*>callToBlock;//记录函数的开头，方便在调用时使用
	map<int, set<Block*>>retToBlock;//记录一个函数所有发生返回的块
	int currentFunction = -1;//记录目前的函数编号
	Block* block = NULL;//当前的基本块
	//第一趟划分基本块并建立基本块的基本连接
	for (int i = 0; i < c.v.size(); i++) {
		//跳转的目标语句或者函数开始，应当切换基本块
		if (c.v[i].labelNo != MIDNOLABEL || c.v[i].op == MIDFUNC){
			//这里的意思是，如果看到了新的函数，那么上一块就是上一个函数的返回块
			//这是因为对于if语句导致的，无return多处返回，我的中间代码里最后会带个nop
			if (c.v[i].op == MIDFUNC&&currentFunction!=-1) {
				retToBlock[currentFunction].insert(block);
			}
			Block* oldBlock = block;
			chart.push_back(block);
			block = new Block();
			if (c.v[i].op == MIDFUNC) {
				currentFunction = c.v[i].operand1;
				callToBlock[currentFunction] = block;
			}
			block->functionId = currentFunction;
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
			block->functionId = currentFunction;
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
			if (i + 1 < c.v.size()&&c.v[i+1].op!=MIDFUNC) {
				//call函数调用结束之后肯定回到下一个块里，除非下一个块是什么函数声明
				Block* returnTarget = codeToBlock[i + 1];
				for (Block* ret : retToBlock[c.v[i].operand1]) {
					addLink(ret, returnTarget);
				}
			}
		}
	}
	chart.erase(chart.begin());//删掉第一个NULL
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
/*第二个被调用，他会分函数扫描所有临时变量然后添加到符号表里面去
同时还会统计所有跨基本块生存的变量，他结束之后应该立刻调用符号表的summary获得各个
函数栈大小的report*/
void FlowChart::summarize() {
	map<int, set<int>>tmpVar;
	for (Block* i : chart) {
		//所有跨基本块生存的变量
		for (int globalVariable : i->activeIn) {
			allGlobalVariables.insert(globalVariable);
		}
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
					if (!j.isImmediate1 && j.operand1<-1) {
						tmpVar[functionId].insert(j.operand1);
					}
					if (j.operand1 != -1) {
						allVariables.insert(j.operand1);
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
					if (!j.isImmediate1&&j.operand1<-1) {
						tmpVar[functionId].insert(j.operand1);
					}
					if (!j.isImmediate2 && j.operand2 < -1) {
						tmpVar[functionId].insert(j.operand2);
					}
					if (j.target < -1) {
						tmpVar[functionId].insert(j.target);
					}
					if (j.operand1 != -1) {
						allVariables.insert(j.operand1);
					}
					if (j.operand2 != -1) {
						allVariables.insert(j.operand1);
					}
					if (j.target != -1) {
						allVariables.insert(j.target);
					}
					break;
				//target and operand1 only
				case MIDNEGATE:
				case MIDASSIGN:
					if (j.target < -1) {
						tmpVar[functionId].insert(j.target);
					}
					if (!j.isImmediate1 && j.operand1<-1) {
						tmpVar[functionId].insert(j.operand1);
					}
					if (j.target != -1) {
						allVariables.insert(j.target);
					}
					if (j.operand1 != -1) {
						allVariables.insert(j.operand1);
					}
					break;
				//target and operand2
				case MIDARRAYGET:
					if (j.target < -1) {
						tmpVar[functionId].insert(j.target);
					}
					if (!j.isImmediate2&&j.operand2<-1) {
						tmpVar[functionId].insert(j.operand2);
					}
					//数组变量不会被作为临时变量但是需要考虑进全部变量
					if (j.target != -1) {
						allVariables.insert(j.target);
					}
					if (j.operand1 != -1) {
						allVariables.insert(j.operand1);
					}
					if (j.operand2 != -1) {
						allVariables.insert(j.operand1);
					}
					break;
					//operand1 and operand2
				case MIDARRAYWRITE:
					if (!j.isImmediate1 && j.operand1 < -1) {
						tmpVar[functionId].insert(j.operand1);
					}
					if (!j.isImmediate2 && j.operand2 < -1) {
						tmpVar[functionId].insert(j.operand2);
					}
					if (j.target != -1) {
						allVariables.insert(j.target);
					}
					if (j.operand1 != -1) {
						allVariables.insert(j.operand1);
					}
					if (j.operand2 != -1) {
						allVariables.insert(j.operand1);
					}
					break;
				// TARGET ONLY
				case MIDREADINTEGER:
				case MIDREADCHAR:
					if (j.target < -1) {
						tmpVar[functionId].insert(j.target);
					}
					if (j.target != -1) {
						allVariables.insert(j.target);
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
	/*去除不参与全局寄存器分配的变量*/
	set<int>del;
	for (int i : allGlobalVariables) {
		SymbolEntry* tmp = MidCode::table->getSymbolById(i);
		if (tmp->scope == "" || tmp->type == TYPECHARCONST || tmp->type == TYPEINTCONST
			|| tmp->type == TYPEINTARRAY || tmp->type == TYPECHARARRAY||tmp->isParameter) {
			del.insert(i);
		}
	}
	for (int i : del) {
		allGlobalVariables.erase(i);
	}

	map<int, vector<int>>report=MidCode::table->summary();
	mips.init(allGlobalVariables, allVariables,report);

}

void FlowChart::conflictEdgeAnalyze() {
	for (Block* i : chart) {
		vector<vector<int>>res = i->conflictEdgeAnalyze();
		
		for (vector<int>& j : res) {
			set<int>::iterator itr1 = allGlobalVariables.find(j[0]);
			set<int>::iterator itr2 = allGlobalVariables.find(j[1]);
			if (itr1 != allGlobalVariables.end() && itr2 != allGlobalVariables.end()) {
				mips.addConflictEdge(j[0], j[1]);
			}
		}
	}
}

/*最先被调用*/
void FlowChart::optimize() {
	//todo implement;
	activeVariableAnalyze();
}

void FlowChart::go() {
	for (Block* i : chart) {
		for (int j = 0; j < i->v.size();j++) {
			MidCode c = i->v[j];
			if (c.op != MIDPUSH) {
				mips.parseToMips(c);
			}
			else {
				vector<MidCode>res;
				while (i->v[j].op == MIDPUSH) {
					res.push_back(i->v[j]);
					j++;
				}
				j--;
				mips.pushToMips(res);
			}
		}
	}

}

ostream& operator<<(ostream& out, FlowChart f) {
	for (Block* i : f.chart) {
		out << (*i);
	}
	return out;
}


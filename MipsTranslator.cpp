#include"MipsTranslator.h"

MipsTranslator::MipsTranslator(string name) {
	currentFunction = -1;
	out.open(name, ios_base::trunc | ios_base::out);
}

void MipsTranslator::generateProgramHeader() {
	out << ".globl main" << endl;
	out << ".data:" << endl;
	MidCode::table->dumpMipsCodeHeader(out);
	out << ".text:" << endl;
}

void MipsTranslator::translateFunction(FlowGraph& g) {
	for (int i = 0; i < GLOBALREG; i++) {
		Sstatus[i] = REGFREE;
	}
	currentFunction = g.functionId;
	SregisterUser.clear();
	globalVariable = g.globalVariable;
	tmpVariable = g.tmpVariable;
	allVariable = g.allVariable;
	conflictMap.clear();
	varReg.clear();
	for (int i : allVariable) {
		varReg[i] = -1;
	}
	for (int i = 0; i < g.conflictEdges.size(); i++) {
		if (g.conflictEdges[i][0] != g.conflictEdges[i][1]) {
			conflictMap[g.conflictEdges[i][0]].insert(g.conflictEdges[i][1]);
			conflictMap[g.conflictEdges[i][1]].insert(g.conflictEdges[i][0]);
		}
	}
	SregisterAlloc();
	//应为参数登记寄存器,但是这个事再说，参数先都不分配寄存器
	for (Block* b : g.graph) {
		translateBlock(b);
	}
}

void MipsTranslator::translateBlock(Block* b) {
	for (int i = 0; i < TMPREG; i++) {
		Tstatus[i] = REGFREE;
		Tuser[i] = -1;
	}
	//todo implement
	for (int i = 0; i < b->v.size(); i++) {
		if (b->v[i].op != MIDPUSH) {
			translate(b->v[i]);
		}
		else {
			vector<MidCode>res;
			while (b->v[i].op == MIDPUSH) {
				res.push_back(b->v[i]);
				i++;
			}
			i--;
			translate(res);
		}
	}
	for (int i = 0; i < TMPREG; i++) {
		if (Tstatus[i] == REGVAR && Tuser[i] != -1) {
			varReg[Tuser[i]] = -1;
		}
	}
}


void MipsTranslator::SregisterAlloc() {
	if (globalVariable.size() == 0) {
		return;
	}
	else if (globalVariable.size() == 1) {
		varReg[*(globalVariable.begin())] = 16;
		Sstatus[0] = REGVAR;
		SregisterUser[16].insert(  *(globalVariable.begin())  );
		return;
	}
	else {
		vector<int>q;
		set<int>var=globalVariable;
		map<int, set<int>>m = conflictMap;
		while (var.size() > 1) {
			int remove = -1;
			for (int i : var) {
				if (m[i].size() < GLOBALREG) {
					//选取一个连接边小于k的并删除
					q.push_back(i);
					for (int j : m[i]) {
						m[j].erase(i);
					}
					remove = i;
				}
			}
			if (remove != -1) {
				var.erase(remove);
			}
			else {
				//若没有找到满足条件的
				//此处如何选择可进行优化
				int chosen = *(var.begin());
				for (int j : m[chosen]) {
					m[j].erase(chosen);
				}
				var.erase(chosen);
			}
		}
		varReg[*(globalVariable.begin())] = 16;
		Sstatus[0] = REGVAR;
		SregisterUser[16].insert(*(globalVariable.begin()));
		//剩余的那个直接分 16寄存器
		for (int i : q) {
			for (int j = 0; j < GLOBALREG; j++) {
				bool ok = true;
				for (int k : SregisterUser[j + 16]) {
					if (conflictMap[i].find(k) != conflictMap[i].end()) {
						ok = false;
						break;
					}
				}
				if (ok) {
					varReg[i] = Sregister[j];
					Sstatus[j] = REGVAR;
					SregisterUser[j + 16].insert(i);
					break;
				}
			}
		}
	}
	/*
	for (int i : globalVariable) {
		cout << i << ":" << varReg[i] << endl;
	}*/
}

vector<int> MipsTranslator::TregisterAlloc(int var, int isImmediate
	, vector<int>conflictVar, vector<int> conflictReg) {
	//返回第一个数是返回的寄存器编号，第二个数是需要写回的变量，如果没有就是-1
	//返回时寄存器已被注册，var是-1代表临时存数使用，不登记只分出
	//如果已经分配了寄存器
	if (!isImmediate&&var!=-1&&varReg[var] >0) {
		return { varReg[var],-1 };
	}
	else{
		//寻找寄存器
		for (int i = 0; i < TMPREG; i++) {
			if (Tstatus[i] == REGFREE) {
				if (isImmediate||var==-1) {
					Tstatus[i] = REGOCCUPY;//改状态
					Tuser[i] = -1;//改使用者
				}
				else {
					Tstatus[i] = REGVAR;
					Tuser[i] = var;
					varReg[var] = Tregister[i];//登记
				}
				return { Tregister[i],-1 };
			}
		}
		//此处可选择进行优化
		for (int i = 0; i < TMPREG; i++) {
			if (Tstatus[i]==REGVAR&&
				find(conflictVar.begin(), conflictVar.end(), Tuser[i]) != conflictVar.end()) {
				continue;//不分配相关的寄存器
			}
			else if (Tstatus[i] == REGOCCUPY &&
				find(conflictReg.begin(), conflictReg.end(), Tregister[i]) != conflictReg.end()) {
				continue;//不分配相关变量占用的寄存器
			}
			else {

				if (Tstatus[i] == REGOCCUPY) {
					if (isImmediate || var == -1) {
						Tstatus[i] = REGOCCUPY;//改状态
						Tuser[i] = -1;//改使用者
					}
					else {
						Tstatus[i] = REGVAR;
						Tuser[i] = var;
						varReg[var] = Tregister[i];//登记
					}
					return { Tregister[i],-1 };
				}
				else {
					int old = Tuser[i];
					if (isImmediate || var == -1) {
						Tstatus[i] = REGOCCUPY;//改状态
						Tuser[i] = -1;//改使用者
					}
					else {
						Tstatus[i] = REGVAR;
						Tuser[i] = var;
						varReg[var] = Tregister[i];//登记
					}
					varReg[old] = -1;
					return{ Tregister[i],old };
				}
			}
		}
		cout << "bug at allocating the tmp register";
		return { -1,-1 };
	}
}


void MipsTranslator::setReport(map<int, vector<int>>_report) {
	report = _report;
}

int MipsTranslator::loadOperand(int var, int isImmediate
	, vector<int>conflictVar, vector<int> conflictReg) {
	if (var!=-1&&!isImmediate && varReg[var] >0) {
		//已分配寄存器
		return varReg[var];
	}
	else if (isImmediate) {
		vector<int>res = TregisterAlloc(var, isImmediate, conflictVar, conflictReg);
		//todo implement
		if (res[1] != -1) { writeback(res[1], res[0]); }
		out << "li " << name[res[0]] << "," << var;
		out << "# load immediate" << var;
		out << endl;
		return res[0];
	}
	else {
		vector<int>res = TregisterAlloc(var, isImmediate, conflictVar, conflictReg);
		if (res[1] != -1) { writeback( res[1],res[0] ); }
		if (var == -1) { return res[0]; }
		SymbolEntry* entry = MidCode::table->getSymbolById(var);
		if (entry->scope != "") {
			if (entry->type == TYPEINT || entry->type == TYPECHAR || entry->type == TYPETMP
				||entry->type==TYPEINTCONST||entry->type==TYPECHARCONST) {
				int bias = entry->addr;
				out << "lw " << name[res[0]] << "," << bias << "($sp)";
				out<<"#load variable" <<MidCode::getOperandName(var, false);
				out << endl;
			}
			else if (entry->type == TYPEINTARRAY || entry->type == TYPECHARARRAY) {
				int bias = entry->addr;
				out << "addiu " << name[res[0]] << ",$sp," << bias;
				out << "# load address of " << MidCode::getOperandName(var, false);
				out << endl;
			}
		}
		else {
			if(entry->type == TYPEINT || entry->type == TYPECHAR
				|| entry->type == TYPEINTCONST || entry->type == TYPECHARCONST) {
				out << "lw " << name[res[0]] << "," << entry->name;
				out << "#load global variable " << entry->name;
				out << endl;
			}
			else if (entry->type == TYPEINTARRAY || entry->type == TYPECHARARRAY) {
				out << "la " << name[res[0]] << "," << entry->name;
				out << "# load address of " << entry->name;
				out << endl;
			}
		}
		return res[0];
	}
}

void MipsTranslator::writeback(int var,int reg) {
	SymbolEntry* entry = MidCode::table->getSymbolById(var);
	if ((entry->type == TYPEINT || entry->type == TYPECHAR||entry->type==TYPETMP) && entry->scope != "") {
		//int char类型的局部变量
		int bias = entry->addr;
		out << "sw " << name[reg] << "," << bias << "($sp)";
		out << "#write back temporary variable " << MidCode::getOperandName(var,false);
		out << endl;
	}

	else if (entry->scope == "" && (entry->type == TYPEINT || entry->type == TYPECHAR || entry->type == TYPETMP)) {
		out << "sw " << name[reg] << "," << entry->name;
		out << "#write back global variable " << entry->name;
		out << endl;
	}
	//数组类型不需要写回
}

void MipsTranslator::translate(MidCode c) {
	if (c.labelNo != MIDNOLABEL) {
		out << "label$" << -c.labelNo << ":" << endl;
	}
	switch (c.op) {
		case MIDFUNC:
		{
			SymbolEntry* s = MidCode::table->getSymbolById(c.operand1);
			out << s->name << ":" << endl;
			if (s->name != "main") {
				out << "sw $ra," << report[s->id][0] - report[s->id][1] + 32 << "($sp)";
				out << "#save the return value" << endl;
				for (int i = 0; i < GLOBALREG; i++) {
					out << "sw " << name[i + 16] << "," << report[s->id][0] - report[s->id][1] + i * 4 << "($sp)" << endl;
				}
			}
			else {
				out << "addiu $sp,$sp," << -report[s->id][0] - report[s->id][1] << endl;
			}
			SubSymbolTable* tmp = MidCode::table->getSubSymbolTableByName(s->name);
			for (auto& i : tmp->symbolMap) {
				if (i.second->type == TYPEINTCONST || i.second->type == TYPECHARCONST) {
					//bug
					out << "li $v1," << i.second->initValue << endl;
					out << "sw $v1," << i.second->addr << "($sp)" << endl;
				}
			}
			
			break;
		}
		case MIDCALL:
		{
			SymbolEntry* s = MidCode::table->getSymbolById(c.operand1);
			out << "addiu $sp,$sp," << -(report[s->id][1] + 36) << endl;
			out << "jal " << s->name<<endl;
			break;
		}
		case MIDRET:
		{
			SymbolEntry* func = MidCode::table->getSymbolById(currentFunction);
			if (func->name != "main") {
				if (c.operand1 != -1 && !c.isImmediate1) {
					//返回值不是空且不是立即数
					if (varReg[c.operand1] > 0) {
						out << "move $v0," << name[varReg[c.operand1]];
						//已经保存在寄存器中
					}
					else {
						//返回的肯定不能是数组名；
						SymbolEntry* s = MidCode::table->getSymbolById(c.operand1);
						if (s->scope == "") {
							out << "lw $v0," << s->name << endl;
						}
						else {
							out << "lw $v0," << s->addr << "($sp)" << endl;
						}
					}
				}
				else if (c.isImmediate1) {
					out << "li $v0," << c.operand1 << endl;
				}
				for (int i = 0; i < GLOBALREG; i++) {
					out << "lw " << name[i + 16] << "," << report[currentFunction][0] -
						report[currentFunction][1] + i * 4 << "($sp)" << endl;
				}
				out << "lw $ra," << report[currentFunction][0]
					- report[currentFunction][1] + 32 << "($sp)" << endl;
				out << "addiu $sp,$sp," << report[currentFunction][1] + 36 << endl;
				out << "jr $ra" << endl;
			}
			break;
		}
		case MIDADD:
		{
			vector<int>conflictVar;
			if (!c.isImmediate1) {conflictVar.push_back(c.operand1);}
			if (!c.isImmediate2) { conflictVar.push_back(c.operand2); }
			int target = loadOperand(c.target, false, conflictVar, {});

			conflictVar.clear();
			conflictVar.push_back(c.target);
			if (!c.isImmediate2) { conflictVar.push_back(c.operand2); }
			int operand1 = loadOperand(c.operand1, c.isImmediate1, conflictVar,
				{ target });

			conflictVar.clear();
			conflictVar.push_back(c.target);
			if (!c.isImmediate1) { conflictVar.push_back(c.operand1); }
			int operand2 = loadOperand(c.operand2, c.isImmediate2, conflictVar,
				{ target,operand1 });

			out << "addu " << name[target] << "," << name[operand1] << "," << name[operand2];
			out << "#" << c<<endl;
			specialVarwriteback(c.target, false);
			break;
		}
		case MIDSUB:
		{
			vector<int>conflictVar;
			if (!c.isImmediate1) { conflictVar.push_back(c.operand1); }
			if (!c.isImmediate2) { conflictVar.push_back(c.operand2); }
			int target = loadOperand(c.target, false, conflictVar, {});

			conflictVar.clear();
			conflictVar.push_back(c.target);
			if (!c.isImmediate2) { conflictVar.push_back(c.operand2); }
			int operand1 = loadOperand(c.operand1, c.isImmediate1, conflictVar,
				{ target });

			conflictVar.clear();
			conflictVar.push_back(c.target);
			if (!c.isImmediate1) { conflictVar.push_back(c.operand1); }
			int operand2 = loadOperand(c.operand2, c.isImmediate2, conflictVar,
				{ target,operand1 });

			out << "subu " << name[target] << "," << name[operand1] << "," << name[operand2];
			out << "#" << c << endl;
			specialVarwriteback(c.target, false);
			break;
		}
		case MIDMULT:
		{
			vector<int>conflictVar;
			if (!c.isImmediate1) { conflictVar.push_back(c.operand1); }
			if (!c.isImmediate2) { conflictVar.push_back(c.operand2); }
			int target = loadOperand(c.target, false, conflictVar, {});

			conflictVar.clear();
			conflictVar.push_back(c.target);
			if (!c.isImmediate2) { conflictVar.push_back(c.operand2); }
			int operand1 = loadOperand(c.operand1, c.isImmediate1, conflictVar,
				{ target });

			conflictVar.clear();
			conflictVar.push_back(c.target);
			if (!c.isImmediate1) { conflictVar.push_back(c.operand1); }
			int operand2 = loadOperand(c.operand2, c.isImmediate2, conflictVar,
				{ target,operand1 });

			out << "mul " << name[target] << "," << name[operand1] << "," << name[operand2];
			out << "#" << c << endl;
			specialVarwriteback(c.target, false);
			break;
		}
		case MIDDIV:
		{
			vector<int>conflictVar;
			if (!c.isImmediate1) { conflictVar.push_back(c.operand1); }
			if (!c.isImmediate2) { conflictVar.push_back(c.operand2); }
			int target = loadOperand(c.target, false, conflictVar, {});

			conflictVar.clear();
			conflictVar.push_back(c.target);
			if (!c.isImmediate2) { conflictVar.push_back(c.operand2); }
			int operand1 = loadOperand(c.operand1, c.isImmediate1, conflictVar,
				{ target });

			conflictVar.clear();
			conflictVar.push_back(c.target);
			if (!c.isImmediate1) { conflictVar.push_back(c.operand1); }
			int operand2 = loadOperand(c.operand2, c.isImmediate2, conflictVar,
				{ target,operand1 });

			out << "div "  << name[operand1] << "," << name[operand2]<<endl;
			out << "mflo " << name[target];
			out << "#" << c << endl;
			specialVarwriteback(c.target, false);
			break;
		}
		case MIDLSS:
		{
			vector<int>conflictVar;
			if (!c.isImmediate1) { conflictVar.push_back(c.operand1); }
			if (!c.isImmediate2) { conflictVar.push_back(c.operand2); }
			int target = loadOperand(c.target, false, conflictVar, {});

			conflictVar.clear();
			conflictVar.push_back(c.target);
			if (!c.isImmediate2) { conflictVar.push_back(c.operand2); }
			int operand1 = loadOperand(c.operand1, c.isImmediate1, conflictVar,
				{ target });

			conflictVar.clear();
			conflictVar.push_back(c.target);
			if (!c.isImmediate1) { conflictVar.push_back(c.operand1); }
			int operand2 = loadOperand(c.operand2, c.isImmediate2, conflictVar,
				{ target,operand1 });

			out << "slt " << name[target] << "," << name[operand1] << "," << name[operand2];
			out << "#" << c << endl;
			specialVarwriteback(c.target, false);
			break;
		}
		case MIDLEQ:
		{
			vector<int>conflictVar;
			if (!c.isImmediate1) { conflictVar.push_back(c.operand1); }
			if (!c.isImmediate2) { conflictVar.push_back(c.operand2); }
			int target = loadOperand(c.target, false, conflictVar, {});

			conflictVar.clear();
			conflictVar.push_back(c.target);
			if (!c.isImmediate2) { conflictVar.push_back(c.operand2); }
			int operand1 = loadOperand(c.operand1, c.isImmediate1, conflictVar,
				{ target });

			conflictVar.clear();
			conflictVar.push_back(c.target);
			if (!c.isImmediate1) { conflictVar.push_back(c.operand1); }
			int operand2 = loadOperand(c.operand2, c.isImmediate2, conflictVar,
				{ target,operand1 });

			out << "sle " << name[target] << "," << name[operand1] << "," << name[operand2];
			out << "#" << c << endl;
			specialVarwriteback(c.target, false);
			break;
		}
		case MIDGRE:
		{
			vector<int>conflictVar;
			if (!c.isImmediate1) { conflictVar.push_back(c.operand1); }
			if (!c.isImmediate2) { conflictVar.push_back(c.operand2); }
			int target = loadOperand(c.target, false, conflictVar, {});

			conflictVar.clear();
			conflictVar.push_back(c.target);
			if (!c.isImmediate2) { conflictVar.push_back(c.operand2); }
			int operand1 = loadOperand(c.operand1, c.isImmediate1, conflictVar,
				{ target });

			conflictVar.clear();
			conflictVar.push_back(c.target);
			if (!c.isImmediate1) { conflictVar.push_back(c.operand1); }
			int operand2 = loadOperand(c.operand2, c.isImmediate2, conflictVar,
				{ target,operand1 });

			out << "sgt " << name[target] << "," << name[operand1] << "," << name[operand2];
			out << "#" << c << endl;
			specialVarwriteback(c.target, false);
			break;
		}
		case MIDGEQ:
		{
			vector<int>conflictVar;
			if (!c.isImmediate1) { conflictVar.push_back(c.operand1); }
			if (!c.isImmediate2) { conflictVar.push_back(c.operand2); }
			int target = loadOperand(c.target, false, conflictVar, {});

			conflictVar.clear();
			conflictVar.push_back(c.target);
			if (!c.isImmediate2) { conflictVar.push_back(c.operand2); }
			int operand1 = loadOperand(c.operand1, c.isImmediate1, conflictVar,
				{ target });

			conflictVar.clear();
			conflictVar.push_back(c.target);
			if (!c.isImmediate1) { conflictVar.push_back(c.operand1); }
			int operand2 = loadOperand(c.operand2, c.isImmediate2, conflictVar,
				{ target,operand1 });

			out << "sge " << name[target] << "," << name[operand1] << "," << name[operand2];
			out << "#" << c << endl;
			specialVarwriteback(c.target, false);
			break;
		}
		case MIDEQL:
		{
			vector<int>conflictVar;
			if (!c.isImmediate1) { conflictVar.push_back(c.operand1); }
			if (!c.isImmediate2) { conflictVar.push_back(c.operand2); }
			int target = loadOperand(c.target, false, conflictVar, {});

			conflictVar.clear();
			conflictVar.push_back(c.target);
			if (!c.isImmediate2) { conflictVar.push_back(c.operand2); }
			int operand1 = loadOperand(c.operand1, c.isImmediate1, conflictVar,
				{ target });

			conflictVar.clear();
			conflictVar.push_back(c.target);
			if (!c.isImmediate1) { conflictVar.push_back(c.operand1); }
			int operand2 = loadOperand(c.operand2, c.isImmediate2, conflictVar,
				{ target,operand1 });

			out << "seq " << name[target] << "," << name[operand1] << "," << name[operand2];
			out << "#" << c << endl;
			specialVarwriteback(c.target, false);
			break;
		}
		case MIDNEQ:
		{
			vector<int>conflictVar;
			if (!c.isImmediate1) { conflictVar.push_back(c.operand1); }
			if (!c.isImmediate2) { conflictVar.push_back(c.operand2); }
			int target = loadOperand(c.target, false, conflictVar, {});

			conflictVar.clear();
			conflictVar.push_back(c.target);
			if (!c.isImmediate2) { conflictVar.push_back(c.operand2); }
			int operand1 = loadOperand(c.operand1, c.isImmediate1, conflictVar,
				{ target });

			conflictVar.clear();
			conflictVar.push_back(c.target);
			if (!c.isImmediate1) { conflictVar.push_back(c.operand1); }
			int operand2 = loadOperand(c.operand2, c.isImmediate2, conflictVar,
				{ target,operand1 });

			out << "sne " << name[target] << "," << name[operand1] << "," << name[operand2];
			out << "#" << c << endl;
			specialVarwriteback(c.target, false);
			break;
		}
		case MIDNEGATE:
		{
			vector<int>conflictVar;
			if (!c.isImmediate1) { conflictVar.push_back(c.operand1); }
			int target = loadOperand(c.target, false, conflictVar, {});

			conflictVar.clear();
			conflictVar.push_back(c.target);
			int operand1 = loadOperand(c.operand1, c.isImmediate1, conflictVar,
				{ target });

			out << "subu "<<name[target]<<",$0," << name[operand1] ;
			out << "#" << c << endl;
			specialVarwriteback(c.target, false);
			break;
		}
		case MIDARRAYGET: 
		{
			vector<int>conflictVar;
			if (!c.isImmediate1) { conflictVar.push_back(c.operand1); }
			if (!c.isImmediate2) { conflictVar.push_back(c.operand2); }
			int target = loadOperand(c.target, false, conflictVar, {});
			
			conflictVar.clear();
			conflictVar.push_back(c.target);
			if (!c.isImmediate1) { conflictVar.push_back(c.operand1); }
			int operand2 = loadOperand(c.operand2, c.isImmediate2, conflictVar,
				{ target });

			SymbolEntry* s = MidCode::table->getSymbolById(c.operand1);
			if (s->scope == "") {
				conflictVar.clear();
				conflictVar.push_back(c.target);
				if (!c.isImmediate2) { conflictVar.push_back(c.operand2); }
				int tmpreg = loadOperand(-1, false, conflictVar, { target,operand2 });
				out << "sll " << name[tmpreg] << "," << name[operand2] << ",2" << endl;
				out << "lw " << name[target] << "," << s->name << "(" << name[tmpreg] << ")" << endl;;
				out << "#" << c << endl;
			}
			else {
				conflictVar.clear();
				conflictVar.push_back(c.target);
				if (!c.isImmediate2) { conflictVar.push_back(c.operand2); }
				int tmpreg = loadOperand(-1, false, conflictVar, { target,operand2 });
				out << "sll " << name[tmpreg] << "," << name[operand2] << ",2" << endl;
				out << "addu " << name[tmpreg] << "," << name[tmpreg] << ",$sp" << endl;
				out << "lw " << name[target] << "," << s->addr<<"("<<name[tmpreg]<<")";
				out << "#" << c << endl;
			}
			specialVarwriteback(c.target, false);
			break;
		}
		case MIDARRAYWRITE:
		{
			vector<int>conflictVar;
			conflictVar.clear();
			conflictVar.push_back(c.target);
			if (!c.isImmediate2) { conflictVar.push_back(c.operand2); }
			int operand1 = loadOperand(c.operand1, c.isImmediate1, conflictVar,{});

			conflictVar.clear();
			conflictVar.push_back(c.target);
			if (!c.isImmediate1) { conflictVar.push_back(c.operand1); }
			int operand2 = loadOperand(c.operand2, c.isImmediate2, conflictVar,
				{ operand1 });
			
			SymbolEntry* s = MidCode::table->getSymbolById(c.target);
			if (s->scope == "") {
				conflictVar.clear();
				if (!c.isImmediate1) { conflictVar.push_back(c.operand1); }
				if (!c.isImmediate2) { conflictVar.push_back(c.operand2); }
				int tmpreg = loadOperand(-1, false, conflictVar, { operand1,operand2 });
				out << "sll " << name[tmpreg] << "," << name[operand1] << ",2" << endl;
				out << "sw " << name[operand2] << "," << s->name << "("<<name[tmpreg] << ")"<<endl;
				out << "#" << c << endl;
			}
			else {
				conflictVar.clear();
				if (!c.isImmediate1) { conflictVar.push_back(c.operand1); }
				if (!c.isImmediate2) { conflictVar.push_back(c.operand2); }
				int tmpreg = loadOperand(-1, false, conflictVar, { operand1,operand2 });
				out << "sll " << name[tmpreg] << "," << name[operand1] << ",2" << endl;
				out << "addu " << name[tmpreg] << "," << name[tmpreg] << ",$sp" << endl;
				out << "sw " << name[operand2] << "," << s->addr << "(" << name[tmpreg] << ")";
				out << "#" << c << endl;
			}
			break;
		}
		case MIDASSIGN:
		{
			vector<int>conflictVar;
			if (!c.isImmediate1) { conflictVar.push_back(c.operand1); }
			int target = loadOperand(c.target, false, conflictVar, {});
			if (c.isImmediate1) {
				out << "li " << name[target] << "," << c.operand1<<endl;
			}
			else {
				if (c.operand1 == -1) {
					out << "move " << name[target] << ",$v0" ;
					out << "#" << c << endl;
				}
				else {
					SymbolEntry* s = MidCode::table->getSymbolById(c.operand1);
					if (varReg[c.operand1] <=0) {
						if (s->scope != "") {
							out << "lw " << name[target] << "," << s->addr << "($sp)";
						}
						else {
							out << "lw " << name[target] << "," << s->name<<endl;
						}
					}
					else {
						out << "move " << name[target] << "," << name[varReg[c.operand1]];
					}
					out << "#" << c << endl;
				}
			}
			specialVarwriteback(c.target, false);
			break;
		}
		case MIDGOTO:
		{
			out << "j label$" << -c.operand1;
			out << "#" << c << endl;
			break;
		}
		case MIDBNZ:
		{
			int operand1 = loadOperand(c.operand1, c.isImmediate1, {}, {});
			out << "bgtz " << name[operand1] << ",label$" << -c.operand2;
			out << "#" << c << endl;
			break;
		}
		case MIDBZ:
		{
			int operand1 = loadOperand(c.operand1, c.isImmediate1, {}, {});
			out << "blez " << name[operand1] << ",label$" << -c.operand2;
			out << "#" << c << endl;
			break;
		}
		case MIDPRINTINT:
		{
			if (!c.isImmediate1) {
				//不是立即数
				if (varReg[c.operand1] >0) {
					out << "move $a0," << name[varReg[c.operand1]]<<endl;
					//已经保存在寄存器中
				}
				else {
					//返回的肯定不能是数组名；
					SymbolEntry* s = MidCode::table->getSymbolById(c.operand1);
					if (s->scope == "") {
						out << "lw $a0," << s->name << endl;
					}
					else {
						out << "lw $a0," << s->addr << "($sp)" << endl;
					}
				}
			}
			else if (c.isImmediate1) {
				out << "li $a0," << c.operand1 << endl;
			}
			out << "li $v0,1" << endl;
			out << "syscall #printint" << endl;
			break;
		}
		case MIDPRINTCHAR:
		{
			if (!c.isImmediate1) {
				//不是立即数
				if (varReg[c.operand1]>0) {
					out << "move $a0," << name[varReg[c.operand1]]<<endl;
					//已经保存在寄存器中
				}
				else {
					//返回的肯定不能是数组名；
					SymbolEntry* s = MidCode::table->getSymbolById(c.operand1);
					if (s->scope == "") {
						out << "lw $a0," << s->name << endl;
					}
					else {
						out << "lw $a0," << s->addr << "($sp)" << endl;
					}
				}
			}
			else if (c.isImmediate1) {
				out << "li $a0," << c.operand1 << endl;
			}
			out << "li $v0,11" << endl;
			out << "syscall #printchar" << endl;
			break;
		}
		case MIDPRINTSTRING:
		{
			out << "la $a0,string$" << c.operand1 << endl;
			out << "li $v0,4" << endl;
			out << "syscall #printstring" << endl;
			break;
		}
		case MIDREADCHAR:
		{
			int target = loadOperand(c.target, false, {}, {});
			out << "li $v0,12" << endl;
			out << "syscall" << endl;
			out << "move " << name[target] <<",$v0"<< endl;
			specialVarwriteback(c.target, false);
			break;
		}
		case MIDREADINTEGER:
		{
			int target = loadOperand(c.target, false, {}, {});
			out << "li $v0,5" << endl;
			out << "syscall" << endl;
			out << "move " << name[target] << ",$v0" << endl;
			specialVarwriteback(c.target, false);
			break;
		}
		case MIDNOP:
		{
			out << "nop" << endl;
			break;
		}
		case MIDPARA:
			break;
		default:
			cout << "bug" << endl;
	}
	//out << endl;
}

void MipsTranslator::translate(vector<MidCode>c) {
	int n = c.size();
	for (int i = 0; i < n; i++) {
		int tmpreg = loadOperand(c[i].operand1,c[i].isImmediate1, {}, {});
		out << "sw " << name[tmpreg] << "," << (-n + i) * 4 << "($sp)" << endl;
		/*if (i >= 0 && i <= 3) {
			out << "move $a" << i << "," << name[tmpreg] << endl;
		}*/
	}
}

void MipsTranslator::specialVarwriteback(int var, bool isImmediate) {
	if (isImmediate||var==-1) {
		return;
	}
	SymbolEntry* e = MidCode::table->getSymbolById(var);
	int reg = varReg[var];
	if ((e->scope == "" || e->isParameter)) {
		writeback(var,varReg[var]);
		Tuser[getTmpRegIndex(varReg[var])] = -1;
		Tstatus[getTmpRegIndex(varReg[var])] =REGFREE;
		varReg[var] = -1;
	}
}
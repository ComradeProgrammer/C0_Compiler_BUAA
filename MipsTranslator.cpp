#include"MipsTranslator.h"

MipsTranslator::MipsTranslator(string name) {
	currentFunction = -1;
	out.open(name, ios_base::trunc | ios_base::out);
}

void MipsTranslator::generateProgramHeader() {
	out << ".data:" << endl;
	MidCode::table->dumpMipsCodeHeader(out);
	out << ".text:" << endl;
	out << "jal main"<<endl;
}

void MipsTranslator::translateFunction(FlowGraph& g) {
	//所有s寄存器清空
	for (int i = 0; i < GLOBALREG; i++) {
		Sstatus[i] = REGFREE;
	}
	SregisterUser.clear();
	//设置当前状态参数
	currentFunction = g.functionId;//函数编号
	globalVariable = g.globalVariable;//所有跨基本块
	tmpVariable = g.tmpVariable;//所有的临时变量
	allVariable = g.allVariable;//所有变量
	conflictMap.clear();//冲突图
	varReg.clear();//重新把寄存器分配全部置-1避免发生分配0号寄存器的事情
	for (int i : allVariable) {
		varReg[i] = -1;
	}
	//添加冲突边
	for (int i = 0; i < g.conflictEdges.size(); i++) {
		if (g.conflictEdges[i][0] != g.conflictEdges[i][1]) {
			conflictMap[g.conflictEdges[i][0]].insert(g.conflictEdges[i][1]);
			conflictMap[g.conflictEdges[i][1]].insert(g.conflictEdges[i][0]);
		}
	}
	//根据冲突边分配s寄存器
	SregisterAlloc();
	for (Block* b : g.graph) {
		translateBlock(b);//逐块翻译
	}
}

void MipsTranslator::translateBlock(Block* b) {
	currentBlock = b;//设置当前块
	//基本块中所有临时寄存器全部释放
	for (int i = 0; i < TMPREG; i++) {
		Tstatus[i] = REGFREE;
		Tuser[i] = -1;
	}
	for (int i = 0; i < b->v.size(); i++) {
		
		if (b->v[i].op != MIDPUSH) {
			translate(b->v[i]);
		}
		else {
			//所有的push语句集中处理
			vector<MidCode>res;
			while (b->v[i].op == MIDPUSH) {
				res.push_back(b->v[i]);
				i++;
			}
			i--;
			translate(res);
		}
	}
	//这里：暂时是把所有临时变量全部写回了，但是其实没这个必要，稍后可以加以改进
	for (int i = 0; i < TMPREG; i++) {
		varReg[Tuser[i]] = -1;
		Tstatus[i] = REGFREE;
		Tuser[i] = -1;
	}
}


void MipsTranslator::SregisterAlloc() {
	if (globalVariable.size() == 0) {
		//若没有跨基本块变量
		return;
	}
	else if (globalVariable.size() == 1) {
		//若是只有一个，直接分配16号寄存器
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
				//若没有找到满足条件的，暂时是从头找一个
				//此处如何选择可进行优化
				int chosen = *(var.begin());
				cout << "remove var No." << chosen << endl;
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
			//对q中的每一个节点
			for (int j = 0; j < GLOBALREG; j++) {
				//检查每一个s寄存器
				bool ok = true;
				for (int k : SregisterUser[j + 16]) {
					if (conflictMap[i].find(k) != conflictMap[i].end()) {
						//检查使用该寄存器的变量中有没有和自己冲突的变量
						ok = false;//找到了就肯定不行
						break;
					}
				}
				if (ok) {
					varReg[i] = Sregister[j];
					Sstatus[j] = REGVAR;
					SregisterUser[j + 16].insert(i);
					cout << "assign reg $s" << j << "to var No." << i << endl;
					break;
				}
			}
		}
	}
}
	/*
	@param
	var:待分配寄存器的变量编号/立即数值，若为-1则意味着分配一个临时寄存器
	isImmediate：是否是立即数
	conflictVar：与之有冲突的变量：不能把同一指令里使用的其他寄存器分出去
	conflictReg:与之有冲突的寄存器编号：也不能把在同一指令里使用的立即数寄存器分出去
	@return
	返回值：第一个数是返回的寄存器编号，第二个数是需要写回的变量，如果没有就是-1
	在返回时寄存器将已被进行所有注册，只有写回操作需要完成（？？为啥这么干来着我忘了）
	*/
vector<int> MipsTranslator::TregisterAlloc(int var, int isImmediate
	, vector<int>conflictVar, vector<int> conflictReg) {
	//如果是变量（不是立即数）且已经分配了寄存器，直接返回
	if (!isImmediate&&var!=-1&&varReg[var] >0) {
		return { varReg[var],-1 };
	}
	else{
		//寻找寄存器，这里寻找寄存器的策略可以优化？
		for (int i = 0; i < TMPREG; i++) {
			if (Tstatus[i] == REGFREE) {
				//如果找到了free状态的直接使用即可
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
		//此处分配策略可选择进行优化
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
				//找到了一个T寄存器
				if (Tstatus[i] == REGOCCUPY) {
					//若是occupy状态无需写回
					if (isImmediate || var == -1) {
						//若是立即数/临时寄存器
						Tstatus[i] = REGOCCUPY;//改状态
						Tuser[i] = -1;//改使用者
					}
					else {
						//若是变量
						Tstatus[i] = REGVAR;
						Tuser[i] = var;
						varReg[var] = Tregister[i];//登记
					}
					return { Tregister[i],-1 };
				}
				else {
					//需要写回
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
		//是立即数
		vector<int>res = TregisterAlloc(var, isImmediate, conflictVar, conflictReg);
		if (res[1] != -1) { writeback(res[1], res[0]); }//写回
		out << "li " << name[res[0]] << "," << var;
		out << "	# load immediate " << var;
		out << endl;
		return res[0];
	}
	else {
		if (var == -1) { 
			vector<int>res = TregisterAlloc(var, isImmediate, conflictVar, conflictReg);
			if (res[1] != -1) { writeback(res[1], res[0]); }
			return res[0]; 
		}
		SymbolEntry* entry = MidCode::table->getSymbolById(var);
		vector<int>res;
		if (entry->isParameter) {
			SymbolEntry* func = MidCode::table->getSymbolById(currentFunction);
			int order = -1;
			for (int i = 0; i < 4 && i < func->link->paraNum; i++) {
				if (var == func->link->paraIds[i]) {
					order = i;
					Astatus[i] = REGVAR;
					Auser[i] = var;
					break;
				}
			}
			if (order != -1) {
				int bias = entry->addr;
				out << "lw " << name[order+4] << "," << bias << "($sp)";
				out << "	#load variable" << MidCode::getOperandName(var, false);
				out << endl;
				return order+4;
			}
		}
		res = TregisterAlloc(var, isImmediate, conflictVar, conflictReg);
		if (res[1] != -1) { writeback( res[1],res[0] ); }
		
		
		if (entry->scope != "") {
			//是局部变量
			if (entry->type == TYPEINT || entry->type == TYPECHAR || entry->type == TYPETMP
				||entry->type==TYPEINTCONST||entry->type==TYPECHARCONST) {
				//此处可以建立机制防止未初始化的内存被加载进入寄存器，可以节省访存
				if (!(var < 0 && globalVariable.find(var) == globalVariable.end())) {
					int bias = entry->addr;
					out << "lw " << name[res[0]] << "," << bias << "($sp)";
					out << "	#load variable" << MidCode::getOperandName(var, false);
					out << endl;
					loaded.insert(var);
				}
			}
			//在传入变量是数组时返回数组地址，不过这段代码现在应该永远不会被执行
			else if (entry->type == TYPEINTARRAY || entry->type == TYPECHARARRAY) {
				int bias = entry->addr;
				out << "addiu " << name[res[0]] << ",$sp," << bias;
				out << "# load address of " << MidCode::getOperandName(var, false);
				out << endl;
			}
		}
		else {
			//是全局变量
			if(entry->type == TYPEINT || entry->type == TYPECHAR
				|| entry->type == TYPEINTCONST || entry->type == TYPECHARCONST) {
				out << "lw " << name[res[0]] << "," << entry->name;
				out << "	#load global variable " << entry->name;
				out << endl;
			}
			//在传入变量是数组时返回数组地址，不过这段代码现在应该永远不会被执行
			else if (entry->type == TYPEINTARRAY || entry->type == TYPECHARARRAY) {
				out << "la " << name[res[0]] << "," << entry->name;
				out << "	#load address of " << entry->name;
				out << endl;
			}
		}
		return res[0];
	}
}

void MipsTranslator::writeback(int var,int reg) {
	SymbolEntry* entry = MidCode::table->getSymbolById(var);
	if ((entry->type == TYPEINT || entry->type == TYPECHAR||entry->type==TYPETMP) 
		&& entry->scope != "") {
		//int char类型的局部变量
		int bias = entry->addr;
		out << "sw " << name[reg] << "," << bias << "($sp)";
		out << "#write back temporary variable " << MidCode::getOperandName(var,false);
		out << endl;
	}

	else if (entry->scope == "" && (entry->type == TYPEINT || entry->type == TYPECHAR )) {
		out << "sw " << name[reg] << "," << entry->name;
		out << "#write back global variable " << entry->name;
		out << endl;
	}
	//其他类型不需要写回
}

void MipsTranslator::translate(MidCode c) {
	if (c.labelNo != MIDNOLABEL) {
		out << "label$" << -c.labelNo << ":" << endl;
	}
	switch (c.op) {
		case MIDFUNC:
		{
			SymbolEntry* s = MidCode::table->getSymbolById(c.operand1);
			out << s->name << ":" << endl;//函数标签
			if (s->name != "main") {
				//若不是main函数，保存s寄存器，返回地址，不需要下移sp因为call时候会移动的
				out << "sw $ra," << report[s->id][0] - report[s->id][1] + 32 << "($sp)";
				out << "#save the return value" << endl;
				/*for (int i = 0; i < GLOBALREG; i++) {
					out << "sw " << name[i + 16] << "," << report[s->id][0] - report[s->id][1] + i * 4 << "($sp)" << endl;
				}*/
				for (int i = 0; i < 4 && i < s->link->paraNum; i++) {
					varReg[s->link->paraIds[i]] = Aregister[i];
					Astatus[i] = REGVAR;
					Auser[i] = s->link->paraIds[i];
				}
			}
			else {
				//若是main函数则应下移sp来为局部变量开辟空间
				out << "addiu $sp,$sp," << -report[s->id][0] - report[s->id][1] << endl;
			}
			SubSymbolTable* tmp = MidCode::table->getSubSymbolTableByName(s->name);
			//此处在语法分析实现了常量替换的情况下应该可以删掉
			for (auto& i : tmp->symbolMap) {
				if (i.second->type == TYPEINTCONST || i.second->type == TYPECHARCONST) {
					//需要为常量进行赋值
					out << "li $v1," << i.second->initValue << endl;
					out << "sw $v1," << i.second->addr << "($sp)" << endl;
				}
			}
			
			break;
		}
		case MIDCALL:
		{
			SymbolEntry* s = MidCode::table->getSymbolById(c.operand1);
			writeBackAfterBlock();//每一个基本块的结束都要进行写回操作
			for (int i =s->link->paraNum ; i < 4; i++) {
				if (Astatus[i] == REGVAR) {
					SymbolEntry* tmp = MidCode::table->getSymbolById(Auser[i]);
					out << "sw " << name[Aregister[i]] << "," << tmp->addr << "($sp)" << endl;
				}
			}

			if (!s->link->inlineable) {
				for (int i = 0; i < GLOBALREG; i++) {
					if (Sstatus[i] != REGFREE) {
						out << "sw " << name[i + 16] << "," << -report[s->id][1] - 32 + i * 4 << "($sp)" << endl;
					}
				}
			}

			out << "addiu $sp,$sp," << -(report[s->id][0] + 36) << endl;
			out << "jal " << s->name<<endl;
			if (!s->link->inlineable) {
				for (int i = 0; i < GLOBALREG; i++) {
					if (Sstatus[i] != REGFREE) {
						out << "lw " << name[i + 16] << "," << -report[s->id][1] - 32 + i * 4 << "($sp)" << endl;
					}
				}
			}
			for (int i = 0; i < 4; i++) {
				if (Astatus[i] == REGVAR) {
					SymbolEntry* tmp = MidCode::table->getSymbolById(Auser[i]);
					out << "lw " << name[Aregister[i]] << "," << tmp->addr << "($sp)" << endl;
				}
			}
			break;
		}
		case MIDRET:
		{

			SymbolEntry* func = MidCode::table->getSymbolById(currentFunction);
			writeBackAfterBlock();
			if (func->name != "main") {
				//不是main函数，如果是main函数就没啥需要干的了,向v0寄存器写入返回值
				if (c.operand1 != -1 && !c.isImmediate1) {
					//返回值不是空且不是立即数
					if (varReg[c.operand1] > 0) {
						//已经保存在寄存器中
						out << "move $v0," << name[varReg[c.operand1]]<<endl;
					}
					else {
						//返回的肯定不能是数组名
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
				/*for (int i = 0; i < GLOBALREG; i++) {
					out << "lw " << name[i + 16] << "," << report[currentFunction][0] -
						report[currentFunction][1] + i * 4 << "($sp)" << endl;
				}*/
				out << "lw $ra," << report[currentFunction][0]
					- report[currentFunction][1] + 32 << "($sp)" << endl;
				out << "addiu $sp,$sp," << report[currentFunction][0] + 36 << endl;
				out << "jr $ra" << endl;
			}
			break;
		}
		case MIDADD:
		{
			if (c.isImmediate2) {
				vector<int>conflictVar;
				if (!c.isImmediate1) { conflictVar.push_back(c.operand1); }
				int target = loadOperand(c.target, false, conflictVar, {});

				conflictVar.clear();
				conflictVar.push_back(c.target);
				int operand1 = loadOperand(c.operand1, c.isImmediate1, conflictVar,
					{ target });

				out << "addiu " << name[target] << "," << name[operand1] << "," << c.operand2;
				out << "#" << c << endl;
				specialVarwriteback(c.target, false);
				break;
				
			}

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
			if (c.isImmediate2) {
				vector<int>conflictVar;
				if (!c.isImmediate1) { conflictVar.push_back(c.operand1); }
				int target = loadOperand(c.target, false, conflictVar, {});

				conflictVar.clear();
				conflictVar.push_back(c.target);
				int operand1 = loadOperand(c.operand1, c.isImmediate1, conflictVar,
					{ target });

				out << "subiu " << name[target] << "," << name[operand1] << "," << c.operand2;
				out << "#" << c << endl;
				specialVarwriteback(c.target, false);
				break;
			}
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

			if (c.isImmediate2&&c.operand2>=0&&c.operand2<=32767) {
				vector<int>conflictVar;
				if (!c.isImmediate1) { conflictVar.push_back(c.operand1); }
				int target = loadOperand(c.target, false, conflictVar, {});

				conflictVar.clear();
				conflictVar.push_back(c.target);
				int operand1 = loadOperand(c.operand1, c.isImmediate1, conflictVar,
					{ target });

				out << "slti " << name[target] << "," << name[operand1] << "," << c.operand2;
				out << "#" << c << endl;
				specialVarwriteback(c.target, false);
				break;
			}
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
			if (c.isImmediate2) {
				vector<int>conflictVar;
				if (!c.isImmediate1) { conflictVar.push_back(c.operand1); }
				int target = loadOperand(c.target, false, conflictVar, {});

				conflictVar.clear();
				conflictVar.push_back(c.target);
				int operand1 = loadOperand(c.operand1, c.isImmediate1, conflictVar,
					{ target });

				out << "sle " << name[target] << "," << name[operand1] << "," << c.operand2;
				out << "#" << c << endl;
				specialVarwriteback(c.target, false);
				break;
			}
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
			if (c.isImmediate2) {
				vector<int>conflictVar;
				if (!c.isImmediate1) { conflictVar.push_back(c.operand1); }
				int target = loadOperand(c.target, false, conflictVar, {});

				conflictVar.clear();
				conflictVar.push_back(c.target);
				int operand1 = loadOperand(c.operand1, c.isImmediate1, conflictVar,
					{ target });

				out << "sgt " << name[target] << "," << name[operand1] << "," << c.operand2;
				out << "#" << c << endl;
				specialVarwriteback(c.target, false);
				break;
			}
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
			if (c.isImmediate2) {
				vector<int>conflictVar;
				if (!c.isImmediate1) { conflictVar.push_back(c.operand1); }
				int target = loadOperand(c.target, false, conflictVar, {});

				conflictVar.clear();
				conflictVar.push_back(c.target);
				int operand1 = loadOperand(c.operand1, c.isImmediate1, conflictVar,
					{ target });

				out << "sge " << name[target] << "," << name[operand1] << "," << c.operand2;
				out << "#" << c << endl;
				specialVarwriteback(c.target, false);
				break;
			}
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
			if (c.isImmediate2) {
				vector<int>conflictVar;
				if (!c.isImmediate1) { conflictVar.push_back(c.operand1); }
				int target = loadOperand(c.target, false, conflictVar, {});

				conflictVar.clear();
				conflictVar.push_back(c.target);
				int operand1 = loadOperand(c.operand1, c.isImmediate1, conflictVar,
					{ target });

				out << "seq " << name[target] << "," << name[operand1] << "," << c.operand2;
				out << "#" << c << endl;
				specialVarwriteback(c.target, false);
				break;
			}
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
			if (c.isImmediate2) {
				vector<int>conflictVar;
				if (!c.isImmediate1) { conflictVar.push_back(c.operand1); }
				int target = loadOperand(c.target, false, conflictVar, {});

				conflictVar.clear();
				conflictVar.push_back(c.target);
				int operand1 = loadOperand(c.operand1, c.isImmediate1, conflictVar,
					{ target });

				out << "sne " << name[target] << "," << name[operand1] << "," << c.operand2;
				out << "#" << c << endl;
				specialVarwriteback(c.target, false);
				break;
			}
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
			writeBackAfterBlock();
			out << "j label$" << -c.operand1;
			out << "#" << c << endl;
			break;
		}
		case MIDBNZ:
		{
			writeBackAfterBlock();
			int operand1 = loadOperand(c.operand1, c.isImmediate1, {}, {});
			out << "bgtz " << name[operand1] << ",label$" << -c.operand2;
			out << "#" << c << endl;
			break;
		}
		case MIDBZ:
		{
			writeBackAfterBlock();
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
					if (varReg[c.operand1] != 4) {
						revokeAregister(4);//若当前寄存器不是a0就收回a0
						out << "move $a0," << name[varReg[c.operand1]] << endl;
						//已经保存在寄存器中
					}
					
				}
				else {
					revokeAregister(4);
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
			SymbolEntry* func = MidCode::table->getSymbolById(currentFunction);
			if (func->link->paraNum > 0) {
				//restore a0
				loadOperand(func->link->paraIds[0], false, {}, {});
			}
			break;
		}
		case MIDPRINTCHAR:
		{
			if (!c.isImmediate1) {
				//不是立即数
				if (varReg[c.operand1]>0) {
					if (varReg[c.operand1] != 4) {
						revokeAregister(4);//若当前寄存器不是a0就收回a0
						out << "move $a0," << name[varReg[c.operand1]] << endl;
						//已经保存在寄存器中
					}
				}
				else {
					revokeAregister(4);
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
			SymbolEntry* func = MidCode::table->getSymbolById(currentFunction);
			if (func->link->paraNum > 0) {
				//restore a0
				loadOperand(func->link->paraIds[0], false, {}, {});
			}
			break;
		}
		case MIDPRINTSTRING:
		{
			revokeAregister(4);
			out << "la $a0,string$" << c.operand1 << endl;
			out << "li $v0,4" << endl;
			out << "syscall #printstring" << endl;
			SymbolEntry* func = MidCode::table->getSymbolById(currentFunction);
			if (func->link->paraNum > 0) {
				//restore a0
				loadOperand(func->link->paraIds[0], false, {}, {});
			}
			break;
		}
		case MIDREADCHAR:
		{
			int target = loadOperand(c.target, false, {}, {});
			out << "li $v0,12" << endl;
			out << "syscall" << endl;
			out << "move " << name[target] <<",$v0"<< endl;
			specialVarwriteback(c.target, false);
			writeBackAfterBlock();
			break;
		}
		case MIDREADINTEGER:
		{
			int target = loadOperand(c.target, false, {}, {});
			out << "li $v0,5" << endl;
			out << "syscall" << endl;
			out << "move " << name[target] << ",$v0" << endl;
			specialVarwriteback(c.target, false);
			writeBackAfterBlock();
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
		if (c[i].labelNo != MIDNOLABEL) {
			out << "label$" << -c[i].labelNo << ":" << endl;
		}
		int tmpreg = loadOperand(c[i].operand1,c[i].isImmediate1, {}, {});
		
		if (i >= 0 && i <= 3) {
			if (Astatus[i] == REGVAR) {
				SymbolEntry* tmp = MidCode::table->getSymbolById(Auser[i]);
				out << "sw " << name[Aregister[i]] << "," << tmp->addr << "($sp)" << endl;
			}
			out << "move $a" << i << "," << name[tmpreg] << endl;
		}
		out << "sw " << name[tmpreg] << "," << (-n + i) * 4 << "($sp)" << endl;
	}
}

void MipsTranslator::specialVarwriteback(int var, bool isImmediate) {
	if (isImmediate||var==-1) {
		return;
	}
	SymbolEntry* e = MidCode::table->getSymbolById(var);
	int reg = varReg[var];
	if ((e->scope == "" )) {
		writeback(var,varReg[var]);
		Tuser[getTmpRegIndex(varReg[var])] = -1;
		Tstatus[getTmpRegIndex(varReg[var])] =REGFREE;
		varReg[var] = -1;
	}
}

void MipsTranslator::revokeAregister(int reg) {
	int i = reg - 4;
	if (Auser[i] != -1) {
		int user = Auser[i];
		varReg[user] = -1;
		Auser[i] = -1;
		Astatus[i] = REGFREE;
		SymbolEntry* e = MidCode::table->getSymbolById(user);
		out << "sw $a" << i << "," << e->addr << "($sp)" << endl;
	}
}

inline int MipsTranslator::getTmpRegIndex(int i) {
	if (i == 24) {
		return 8;
	}
	else if (i == 25) {
		return 9;
	}
	else {
		return i - 8;
	}
}

void MipsTranslator::writeBackAfterBlock() {
	for (int i = 0; i < TMPREG; i++) {
		/*if (Tstatus[i] == REGVAR && Tuser[i] != -1) {
			SymbolEntry* e = MidCode::table->getSymbolById(Tuser[i]);
			if (e->scope == "" || e->isParameter) {
				writeback(Tuser[i], Tregister[i]);
				varReg[Tuser[i]] = -1;
				Tstatus[i] = REGFREE;
				Tuser[i] = -1;
				continue;
			}
		}
		*/
		if (Tstatus[i] == REGVAR && Tuser[i] != -1&&
			currentBlock->activeOut.find(Tuser[i])!=currentBlock->activeOut.end()) {
			writeback(Tuser[i], Tregister[i]);
			varReg[Tuser[i]] = -1;
			Tstatus[i] = REGFREE;
			Tuser[i] = -1;
		}
	}
}
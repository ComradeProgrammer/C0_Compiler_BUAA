#include"MipsTranslator.h"

MipsTranslator::MipsTranslator(string name) {
	currentFunction = -1;
	out.open(name, ios_base::trunc | ios_base::out);
}

void MipsTranslator::generateProgramHeader() {
	out << ".data:" << endl;
	MidCode::table->dumpMipsCodeHeader(out);
	out << ".text:" << endl;
	out << "j main"<<endl;
}

void MipsTranslator::translateFunction(FlowGraph& g) {
	
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
	//根据冲突边分配s寄存器
	for (Block* b : g.graph) {
		translateBlock(b);//逐块翻译
	}
}

void MipsTranslator::translateBlock(Block* b) {
	currentBlock = b;//设置当前块
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
}



	
void MipsTranslator::setReport(map<int, vector<int>>_report) {
	report = _report;
}

int MipsTranslator::loadOperand(int var,bool isImmediate, int reg) {
	if (isImmediate) {
		//是立即数;
		out << "li " << name[reg] << "," << var;
		out << "	# load immediate " << var;
		out << endl;
		return reg;
	}
	else {
		SymbolEntry* entry = MidCode::table->getSymbolById(var);
		if (entry->scope != "") {
			//是局部变量
			if (entry->type == TYPEINT || entry->type == TYPECHAR || entry->type == TYPETMP
				||entry->type==TYPEINTCONST||entry->type==TYPECHARCONST) {
					int bias = entry->addr;
					out << "lw " << name[reg] << "," << bias << "($sp)";
					out << "	#load variable" << MidCode::getOperandName(var, false);
					out << endl;
			}
			//在传入变量是数组时返回数组地址，不过这段代码现在应该永远不会被执行
			else if (entry->type == TYPEINTARRAY || entry->type == TYPECHARARRAY) {
				int bias = entry->addr;
				out << "addiu " << name[reg] << ",$sp," << bias;
				out << "# load address of " << MidCode::getOperandName(var, false);
				out << endl;
			}
		}
		else {
			//是全局变量
			if(entry->type == TYPEINT || entry->type == TYPECHAR
				|| entry->type == TYPEINTCONST || entry->type == TYPECHARCONST) {
				out << "lw " << name[reg] << "," << entry->name;
				out << "	#load global variable " << entry->name;
				out << endl;
			}
			//在传入变量是数组时返回数组地址，不过这段代码现在应该永远不会被执行
			else if (entry->type == TYPEINTARRAY || entry->type == TYPECHARARRAY) {
				out << "la " << name[reg] << "," << entry->name;
				out << "	#load address of " << entry->name;
				out << endl;
			}
		}
		return reg;
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
			out << "addiu $sp,$sp," << -(report[s->id][0] + 36) << endl;
			out << "jal " << s->name<<endl;
			break;
		}
		case MIDRET:
		{

			SymbolEntry* func = MidCode::table->getSymbolById(currentFunction);
			if (func->name != "main") {
				//不是main函数，如果是main函数就没啥需要干的了,向v0寄存器写入返回值
				if (c.operand1 != -1 && !c.isImmediate1) {
					SymbolEntry* s = MidCode::table->getSymbolById(c.operand1);
					if (s->scope == "") {
						out << "lw $v0," << s->name << endl;
					}
					else {
						out << "lw $v0," << s->addr << "($sp)" << endl;
					}
				}
				else if (c.isImmediate1) {
					out << "li $v0," << c.operand1 << endl;
				}
				out << "lw $ra," << report[currentFunction][0]
					- report[currentFunction][1] + 32 << "($sp)" << endl;
				out << "addiu $sp,$sp," << report[currentFunction][0] + 36 << endl;
				out << "jr $ra" << endl;
			}
			break;
		}
		case MIDADD:
		{
			int target = loadOperand(c.target, false, 8);
			int operand1 = loadOperand(c.operand1, c.isImmediate1, 9);	
			int operand2 = loadOperand(c.operand2, c.isImmediate2, 10);
			out << "addu " << name[target] << "," << name[operand1] << "," << name[operand2];
			out << "#" << c<<endl;
			writeback(c.target, 8);
			break;
		}
		case MIDSUB:
		{
			int target = loadOperand(c.target, false, 8);
			int operand1 = loadOperand(c.operand1, c.isImmediate1, 9);
			int operand2 = loadOperand(c.operand2, c.isImmediate2, 10);
			out << "subu " << name[target] << "," << name[operand1] << "," << name[operand2];
			out << "#" << c << endl;
			writeback(c.target, 8);
			break;
		}
		case MIDMULT:
		{
			int target = loadOperand(c.target, false, 8);
			int operand1 = loadOperand(c.operand1, c.isImmediate1, 9);
			int operand2 = loadOperand(c.operand2, c.isImmediate2, 10);
			out << "mul " << name[target] << "," << name[operand1] << "," << name[operand2];
			out << "#" << c << endl;
			writeback(c.target, 8);
			break;
		}
		case MIDDIV:
		{
			int target = loadOperand(c.target, false, 8);
			int operand1 = loadOperand(c.operand1, c.isImmediate1, 9);
			int operand2 = loadOperand(c.operand2, c.isImmediate2, 10);
			out << "div "  << name[operand1] << "," << name[operand2]<<endl;
			out << "mflo " << name[target];
			out << "#" << c << endl;
			writeback(c.target,8);
			break;
		}
		case MIDLSS:
		{
			int target = loadOperand(c.target, false, 8);
			int operand1 = loadOperand(c.operand1, c.isImmediate1, 9);
			int operand2 = loadOperand(c.operand2, c.isImmediate2, 10);
			out << "slt " << name[target] << "," << name[operand1] << "," << name[operand2];
			out << "#" << c << endl;
			writeback(c.target, 8);
			break;
		}
		case MIDLEQ:
		{
			int target = loadOperand(c.target, false, 8);
			int operand1 = loadOperand(c.operand1, c.isImmediate1, 9);
			int operand2 = loadOperand(c.operand2, c.isImmediate2, 10);
			out << "sle " << name[target] << "," << name[operand1] << "," << name[operand2];
			out << "#" << c << endl;
			writeback(c.target, 8);
			break;
		}
		case MIDGRE:
		{
			int target = loadOperand(c.target, false, 8);
			int operand1 = loadOperand(c.operand1, c.isImmediate1, 9);
			int operand2 = loadOperand(c.operand2, c.isImmediate2, 10);
			out << "sgt " << name[target] << "," << name[operand1] << "," << name[operand2];
			out << "#" << c << endl;
			writeback(c.target, 8);
			break;
		}
		case MIDGEQ:
		{
			int target = loadOperand(c.target, false, 8);
			int operand1 = loadOperand(c.operand1, c.isImmediate1, 9);
			int operand2 = loadOperand(c.operand2, c.isImmediate2, 10);
			out << "sge " << name[target] << "," << name[operand1] << "," << name[operand2];
			out << "#" << c << endl;
			writeback(c.target, 8);
			break;
		}
		case MIDEQL:
		{
			int target = loadOperand(c.target, false, 8);
			int operand1 = loadOperand(c.operand1, c.isImmediate1, 9);
			int operand2 = loadOperand(c.operand2, c.isImmediate2, 10);
			out << "seq " << name[target] << "," << name[operand1] << "," << name[operand2];
			out << "#" << c << endl;
			writeback(c.target, 8);
			break;
		}
		case MIDNEQ:
		{
			int target = loadOperand(c.target, false, 8);
			int operand1 = loadOperand(c.operand1, c.isImmediate1, 9);
			int operand2 = loadOperand(c.operand2, c.isImmediate2, 10);
			out << "sne " << name[target] << "," << name[operand1] << "," << name[operand2];
			out << "#" << c << endl;
			writeback(c.target, 8);
			break;
		}
		case MIDNEGATE:
		{
			int target = loadOperand(c.target, false, 8);
			int operand1 = loadOperand(c.operand1, c.isImmediate1, 9);
			out << "subu "<<name[target]<<",$0," << name[operand1] ;
			out << "#" << c << endl;
			writeback(c.target, 8);
			break;
		}
		case MIDARRAYGET: 
		{
			
			int target = loadOperand(c.target,false, 8);
			int operand2 = loadOperand(c.operand2, c.isImmediate2, 10);

			SymbolEntry* s = MidCode::table->getSymbolById(c.operand1);
			if (s->scope == "") {
				int tmpreg = 11;
				out << "sll " << name[tmpreg] << "," << name[operand2] << ",2" << endl;
				out << "lw " << name[target] << "," << s->name << "(" << name[tmpreg] << ")" << endl;;
				out << "#" << c << endl;
			}
			else {
				
				int tmpreg = 11;
				out << "sll " << name[tmpreg] << "," << name[operand2] << ",2" << endl;
				out << "addu " << name[tmpreg] << "," << name[tmpreg] << ",$sp" << endl;
				out << "lw " << name[target] << "," << s->addr<<"("<<name[tmpreg]<<")";
				out << "#" << c << endl;
			}
			writeback(c.target,8);
			break;
		}
		case MIDARRAYWRITE:
		{
			int operand1 = loadOperand(c.operand1, c.isImmediate1,9);
			int operand2 = loadOperand(c.operand2, c.isImmediate2, 10);	
			SymbolEntry* s = MidCode::table->getSymbolById(c.target);
			if (s->scope == "") {
				int tmpreg = 11;
				out << "sll " << name[tmpreg] << "," << name[operand1] << ",2" << endl;
				out << "sw " << name[operand2] << "," << s->name << "("<<name[tmpreg] << ")"<<endl;
				out << "#" << c << endl;
			}
			else {
				int tmpreg = 11;
				out << "sll " << name[tmpreg] << "," << name[operand1] << ",2" << endl;
				out << "addu " << name[tmpreg] << "," << name[tmpreg] << ",$sp" << endl;
				out << "sw " << name[operand2] << "," << s->addr << "(" << name[tmpreg] << ")";
				out << "#" << c << endl;
			}
			break;
		}
		case MIDASSIGN:
		{
			int target = loadOperand(c.target, false,8);
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
					if (s->scope != "") {
						out << "lw " << name[target] << "," << s->addr << "($sp)";
					}
					else {
						out << "lw " << name[target] << "," << s->name<<endl;
					}
					out << "#" << c << endl;
				}
			}
			writeback(c.target, 8);
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
			int operand1 = loadOperand(c.operand1, c.isImmediate1,8);
			out << "bgtz " << name[operand1] << ",label$" << -c.operand2;
			out << "#" << c << endl;
			break;
		}
		case MIDBZ:
		{
			int operand1 = loadOperand(c.operand1, c.isImmediate1,8);
			out << "blez " << name[operand1] << ",label$" << -c.operand2;
			out << "#" << c << endl;
			break;
		}
		case MIDPRINTINT:
		{
			if (!c.isImmediate1) {
					//返回的肯定不能是数组名；
					SymbolEntry* s = MidCode::table->getSymbolById(c.operand1);
					if (s->scope == "") {
						out << "lw $a0," << s->name << endl;
					}
					else {
						out << "lw $a0," << s->addr << "($sp)" << endl;
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
				//返回的肯定不能是数组名；
				SymbolEntry* s = MidCode::table->getSymbolById(c.operand1);
				if (s->scope == "") {
					out << "lw $a0," << s->name << endl;
				}
				else {
					out << "lw $a0," << s->addr << "($sp)" << endl;
				}

			}
			else if (c.isImmediate1) {
				out << "li $a0," << c.operand1 << endl;
			}
			out << "li $v0,11" << endl;
			out << "syscall #printint" << endl;
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
			int target = loadOperand(c.target, false,8);
			out << "li $v0,12" << endl;
			out << "syscall" << endl;
			out << "move " << name[target] <<",$v0"<< endl;
			writeback(c.target, 8);
			break;
		}
		case MIDREADINTEGER:
		{
			int target = loadOperand(c.target, false,8);
			out << "li $v0,5" << endl;
			out << "syscall" << endl;
			out << "move " << name[target] << ",$v0" << endl;
			writeback(c.target, 8);
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
	
}

void MipsTranslator::translate(vector<MidCode>c) {
	int n = c.size();
	for (int i = 0; i < n; i++) {
		if (c[i].labelNo != MIDNOLABEL) {
			out << "label$" << -c[i].labelNo << ":" << endl;
		}
		int tmpreg = loadOperand(c[i].operand1,c[i].isImmediate1, 11);
		out << "sw " << name[tmpreg] << "," << (-n + i) * 4 << "($sp)" << endl;
	}
}

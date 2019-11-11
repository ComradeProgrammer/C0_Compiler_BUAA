#include"MipsGenerator.h"

void MipsGenerator::init(set<int>globalVar, set<int>allVar, map<int, vector<int>>report) {
	globalVariables = globalVar;
	allVariables = allVar;
	for (int i : allVar) {
		regAlloc[i] = UNASSIGNED;
	}
	summaryReport = report;
}

void MipsGenerator::addConflictEdge(int i, int j) {
	//debug
	set<int>::iterator itr = conflictMap[i].find(j);
	/*if(i!=j&&itr==conflictMap[i].end()){
		cout << "(" << i << "," << j << ")" << endl;
	}*/
	if (i != j) {
		conflictMap[i].insert(j);
		conflictMap[j].insert(i);
	}
	
}

void MipsGenerator::globalRegisterAlloc() {
	/*
	vector<int>q;
	set<int>globalVar = globalVariables;
	map<int, set<int>>conflict = conflictMap;
	if (globalVar.size() == 0) {
		return;
	}
	else if (globalVar.size() == 1) {
		regAlloc[*(globalVar.begin())] = 16;//只有一个全局变量直接分配寄存器
	}
	while (globalVar.size() > 1) {
		bool find = false;
		for (int i : globalVar) {
			if (conflict[i].size() < GLOBALREG) {
				q.push_back(i);
				for (int j : conflict[i]) {
					conflict[j].erase(i);
				}
				globalVar.erase(i);
				find = true;
				break;
			}
		}
		if (find) {
			continue;
		}
		else {
			//可以添加优化策略
			int chosen = *(globalVar.begin());
			regAlloc[chosen] = NOREG;
			for (int j : conflict[chosen]) {
				conflict[j].erase(chosen);
			}
			globalVar.erase(chosen);
		}
	}
	//alloc
	regAlloc[*(globalVar.begin())] = 16;
	for (int i : globalVariables) {
		if (regAlloc[i] == (NOREG)) {
			continue;
		}
		else {
			if (regAlloc[i] == (UNASSIGNED)) {
				vector<int>used=vector<int>(8,0);
				for (int j : conflictMap[i]) {
					if (regAlloc[j] > 0) {
						used[regAlloc[j]-16]=1;
					}
				}
				for (int j : globalRegs) {
					if (used[j - 16] == 0) {
						regAlloc[i] = j;
						break;
					}
				}
			}
		}
	}*/
}

void  MipsGenerator::outputFile(string s) {
	out.open(s, ios_base::out | ios_base::trunc);
}
void MipsGenerator::generateProgramHeader() {
	out << ".globl main"<<endl;
	out << ".data:" << endl;
	MidCode::table->dumpMipsCodeHeader(out);
	out << ".text:" << endl;

}

void MipsGenerator::printRegisterAllocStatus(ostream& out) {
	for (auto& i : regAlloc) {
		out << i.first << ":" << i.second<<endl;
	}
}

vector<int> MipsGenerator::tmpRegisterAlloc(int var,int associate1,int associate2) {
	/*
	for (int i = 0; i < TMPREG; i++) {
		if (tmpRegValid[i] == 0) {
			regAlloc[var] = tmpRegs[i];
			tmpRegValid[i] = 1;
			return {tmpRegs[i], -1};
		}
	}
	for (int i = 0; i < TMPREG; i++) {
		vector<int >res;
		if ((associate1 == -1 || tmpRegPoolVar[i] != associate1)
			&& (associate2 == -1 || tmpRegPoolVar[i] != associate2)) {
			res.push_back(tmpRegs[i]);//分配的寄存器
			res.push_back(tmpRegPoolVar[i]);//需要写回的变量
			regAlloc[tmpRegPoolVar[i]] =UNASSIGNED;//修改regAlloc,解除写回变量
			regAlloc[var] = tmpRegs[i];
			return res;
		}
	}
	*/
	if (regAlloc[var] > 0) {
		return { regAlloc[var],-1 };
	}
	for (int i = 0; i < TMPREG; i++) {
		if (tmpRegValid[i] == 0) {
			//regAlloc[var] = tmpRegs[i];
			tmpRegValid[i] = 1;
			return { tmpRegs[i], -1 };
		}
	}
}

void MipsGenerator::printConflictMap(ostream&out) {
	for (auto& i : conflictMap) {
		for (int j : i.second) {
			out << "(" << i.first << "," << j << ")" << endl;
		}
	}
}

void MipsGenerator::parseToMips(MidCode c) {
	if (c.labelNo != MIDNOLABEL) {
		out << "label$" << -c.labelNo << ":" << endl;
	}
	switch (c.op) {
		
		case MIDFUNC:
		{
			SymbolEntry* s = MidCode::table->getSymbolById(c.operand1);
			out << s->name << ":" << endl;
			currentFunction = c.operand1;
			if (s->name != "main") {
				out << "sw $ra," << summaryReport[s->id][0]- summaryReport[s->id][1] << "($sp)" << endl;
			}
			break;
		}
			//midpara 不需要生成，midpush统一生成
		case MIDCALL:
		{
			SymbolEntry* s = MidCode::table->getSymbolById(c.operand1);
			int id = s->id;
			int totalSize = summaryReport[id][0];
			out << "addi $sp,$sp," << -(summaryReport[id][0] +4 )<< endl;
			out << "jal " << s->name<<endl;
			//out << "sw $ra," << summaryReport[id][0] << "($sp)" << endl;
			break;
		}
		case MIDRET:
		{
			if (regAlloc[c.operand1] > 0) {
				out << "move $v0," << getRegName(regAlloc[c.operand1]) << endl;
			}
			else if(c.operand1!=-1) {
				int tmp2 = genLoadMips(c.operand1, c.isImmediate1, c.target,
					c.isImmediate2 ? -1 : c.operand2);
				//SymbolEntry* tmp = MidCode::table->getSymbolById(c.operand1);
				out << "move $v0," << getRegName(tmp2) <<endl;
			}
			SymbolEntry* s = MidCode::table->getSymbolById(currentFunction);
			out << "lw $ra," << summaryReport[currentFunction][0]- summaryReport[currentFunction][1] << "($sp)" << endl;
			out << "addi $sp,$sp," << summaryReport[currentFunction][0] + 4 << endl;
			out << "jr $ra"<<endl;
			break;
		}
		case MIDADD:
		{
			int tmp1 = genLoadMips(c.target, false,
				c.isImmediate1 ? -1 : c.operand1,
				c.isImmediate2 ? -1 : c.operand2);
			int tmp2 = genLoadMips(c.operand1, c.isImmediate1,c.target,
				c.isImmediate2?-1:c.operand2);
			int tmp3 = genLoadMips(c.operand2, c.isImmediate2,c.target,
				c.isImmediate1 ? -1 : c.operand1);
			out << "addu " << getRegName(tmp1) << "," << getRegName(tmp2)
				<< "," << getRegName(tmp3) << endl;
			writeback(c.target, false);
			//writeback(c.operand1, c.isImmediate1);
			//writeback(c.operand2, c.isImmediate2);
			release(c.target,false);
			release(c.operand1, c.isImmediate1);
			release(c.operand2, c.isImmediate2);
		}
		break;
		case MIDSUB:
		{
			int tmp1 = genLoadMips(c.target, false,
				c.isImmediate1 ? -1 : c.operand1,
				c.isImmediate2 ? -1 : c.operand2);
			int tmp2 = genLoadMips(c.operand1, c.isImmediate1, c.target,
				c.isImmediate2 ? -1 : c.operand2);
			int tmp3 = genLoadMips(c.operand2, c.isImmediate2, c.target,
				c.isImmediate1 ? -1 : c.operand1);
			out << "subu " << getRegName(tmp1) << "," << getRegName(tmp2)
				<< "," << getRegName(tmp3) << endl;
			writeback(c.target, false);
			//writeback(c.operand1, c.isImmediate1);
			//writeback(c.operand2, c.isImmediate2);
			release(c.target, false);
			release(c.operand1, c.isImmediate1);
			release(c.operand2, c.isImmediate2);
		}
		break;
		case MIDMULT:
		{
			int tmp1 = genLoadMips(c.target, false,
				c.isImmediate1 ? -1 : c.operand1,
				c.isImmediate2 ? -1 : c.operand2);
			int tmp2 = genLoadMips(c.operand1, c.isImmediate1, c.target,
				c.isImmediate2 ? -1 : c.operand2);
			int tmp3 = genLoadMips(c.operand2, c.isImmediate2, c.target,
				c.isImmediate1 ? -1 : c.operand1);
			out << "mul " << getRegName(tmp1) << "," << getRegName(tmp2)
				<< "," << getRegName(tmp3) << endl;
			writeback(c.target, false);
			//writeback(c.operand1, c.isImmediate1);
			//writeback(c.operand2, c.isImmediate2);
			release(c.target, false);
			release(c.operand1, c.isImmediate1);
			release(c.operand2, c.isImmediate2);
		}
		break;
		case MIDDIV:
		{
			int tmp1 = genLoadMips(c.target, false,
				c.isImmediate1 ? -1 : c.operand1,
				c.isImmediate2 ? -1 : c.operand2);
			int tmp2 = genLoadMips(c.operand1, c.isImmediate1, c.target,
				c.isImmediate2 ? -1 : c.operand2);
			int tmp3 = genLoadMips(c.operand2, c.isImmediate2, c.target,
				c.isImmediate1 ? -1 : c.operand1);
			out << "div "  << getRegName(tmp2)
				<< "," << getRegName(tmp3) << endl;
			out << "mflo " << getRegName(tmp1) << endl;
			writeback(c.target, false);
			//writeback(c.operand1, c.isImmediate1);
			//writeback(c.operand2, c.isImmediate2);
			release(c.target, false);
			release(c.operand1, c.isImmediate1);
			release(c.operand2, c.isImmediate2);
		}
		break;
		case MIDLSS:
		{
			int tmp1 = genLoadMips(c.target, false,
				c.isImmediate1 ? -1 : c.operand1,
				c.isImmediate2 ? -1 : c.operand2);
			int tmp2 = genLoadMips(c.operand1, c.isImmediate1, c.target,
				c.isImmediate2 ? -1 : c.operand2);
			int tmp3 = genLoadMips(c.operand2, c.isImmediate2, c.target,
				c.isImmediate1 ? -1 : c.operand1);
			out << "slt " << getRegName(tmp1) << "," << getRegName(tmp2)
				<< "," << getRegName(tmp3) << endl;
			writeback(c.target, false);
			//writeback(c.operand1, c.isImmediate1);
			//writeback(c.operand2, c.isImmediate2);
			release(c.target, false);
			release(c.operand1, c.isImmediate1);
			release(c.operand2, c.isImmediate2);
		}
		break;
		case MIDLEQ:
		{
			int tmp1 = genLoadMips(c.target, false,
				c.isImmediate1 ? -1 : c.operand1,
				c.isImmediate2 ? -1 : c.operand2);
			int tmp2 = genLoadMips(c.operand1, c.isImmediate1, c.target,
				c.isImmediate2 ? -1 : c.operand2);
			int tmp3 = genLoadMips(c.operand2, c.isImmediate2, c.target,
				c.isImmediate1 ? -1 : c.operand1);
			out << "sle " << getRegName(tmp1) << "," << getRegName(tmp2)
				<< "," << getRegName(tmp3) << endl;
			writeback(c.target, false);
			//writeback(c.operand1, c.isImmediate1);
			//writeback(c.operand2, c.isImmediate2);
			release(c.target, false);
			release(c.operand1, c.isImmediate1);
			release(c.operand2, c.isImmediate2);
		}
		break;
		case MIDGRE:
		{
			int tmp1 = genLoadMips(c.target, false,
				c.isImmediate1 ? -1 : c.operand1,
				c.isImmediate2 ? -1 : c.operand2);
			int tmp2 = genLoadMips(c.operand1, c.isImmediate1, c.target,
				c.isImmediate2 ? -1 : c.operand2);
			int tmp3 = genLoadMips(c.operand2, c.isImmediate2, c.target,
				c.isImmediate1 ? -1 : c.operand1);
			out << "sgt " << getRegName(tmp1) << "," << getRegName(tmp2)
				<< "," << getRegName(tmp3) << endl;
			writeback(c.target, false);
			//writeback(c.operand1, c.isImmediate1);
			//writeback(c.operand2, c.isImmediate2);
			release(c.target, false);
			release(c.operand1, c.isImmediate1);
			release(c.operand2, c.isImmediate2);
		}
		break;
		case MIDGEQ:
		{
			int tmp1 = genLoadMips(c.target, false,
				c.isImmediate1 ? -1 : c.operand1,
				c.isImmediate2 ? -1 : c.operand2);
			int tmp2 = genLoadMips(c.operand1, c.isImmediate1, c.target,
				c.isImmediate2 ? -1 : c.operand2);
			int tmp3 = genLoadMips(c.operand2, c.isImmediate2, c.target,
				c.isImmediate1 ? -1 : c.operand1);
			out << "sge " << getRegName(tmp1) << "," << getRegName(tmp2)
				<< "," << getRegName(tmp3) << endl;
			writeback(c.target, false);
			//writeback(c.operand1, c.isImmediate1);
			//writeback(c.operand2, c.isImmediate2);
			release(c.target, false);
			release(c.operand1, c.isImmediate1);
			release(c.operand2, c.isImmediate2);
		}
		break;
		case MIDEQL:
		{
			int tmp1 = genLoadMips(c.target, false,
				c.isImmediate1 ? -1 : c.operand1,
				c.isImmediate2 ? -1 : c.operand2);
			int tmp2 = genLoadMips(c.operand1, c.isImmediate1, c.target,
				c.isImmediate2 ? -1 : c.operand2);
			int tmp3 = genLoadMips(c.operand2, c.isImmediate2, c.target,
				c.isImmediate1 ? -1 : c.operand1);
			out << "seq " << getRegName(tmp1) << "," << getRegName(tmp2)
				<< "," << getRegName(tmp3) << endl;
			writeback(c.target, false);
			//writeback(c.operand1, c.isImmediate1);
			//writeback(c.operand2, c.isImmediate2);
			release(c.target, false);
			release(c.operand1, c.isImmediate1);
			release(c.operand2, c.isImmediate2);
		}
		break;
		case MIDNEQ:
		{
			int tmp1 = genLoadMips(c.target, false,
				c.isImmediate1 ? -1 : c.operand1,
				c.isImmediate2 ? -1 : c.operand2);
			int tmp2 = genLoadMips(c.operand1, c.isImmediate1, c.target,
				c.isImmediate2 ? -1 : c.operand2);
			int tmp3 = genLoadMips(c.operand2, c.isImmediate2, c.target,
				c.isImmediate1 ? -1 : c.operand1);
			out << "sne " << getRegName(tmp1) << "," << getRegName(tmp2)
				<< "," << getRegName(tmp3) << endl;
			writeback(c.target, false);
			//writeback(c.operand1, c.isImmediate1);
			//writeback(c.operand2, c.isImmediate2);
			release(c.target, false);
			release(c.operand1, c.isImmediate1);
			release(c.operand2, c.isImmediate2);
		}
		break;
		case MIDNEGATE:
		{
			int tmp1 = genLoadMips(c.target, false,
				c.isImmediate1 ? -1 : c.operand1,
				c.isImmediate2 ? -1 : c.operand2);
			int tmp2 = genLoadMips(c.operand1, c.isImmediate1, c.target,
				-1);
			int tmp3 = genLoadMips(0, true, c.target,
				c.isImmediate1 ? -1 : c.operand1);
			out << "sub " << getRegName(tmp1) << "," << getRegName(tmp3)
				<< "," << getRegName(tmp2) << endl;
			writeback(c.target, false);
			release(c.target, false);
			release(c.operand1, c.isImmediate1);
		}
		break;
		case MIDARRAYGET:
		{
			int tmp1 = genLoadMips(c.target, false,
				c.isImmediate1 ? -1 : c.operand1,
				c.isImmediate2 ? -1 : c.operand2);
			int tmp2 = genLoadMips(c.operand1, c.isImmediate1, c.target,
				c.isImmediate2 ? -1 : c.operand2);
			int tmp3 = genLoadMips(c.operand2, c.isImmediate2, c.target,
				c.isImmediate1 ? -1 : c.operand1);
			out << "sll " << "$at" << "," << getRegName(tmp3) << ",2" << endl;
			out << "addu " << "$at" << "," << getRegName(tmp2) << "," <<
				"$at"<<endl;
			out << "lw " << getRegName(tmp1) << ",0(" << "$at"<<")"<<endl;
			writeback(c.target, false);
			//writeback(c.operand1, c.isImmediate1);
			//writeback(c.operand2, c.isImmediate2);
			release(c.target, false);
			release(c.operand1, c.isImmediate1);
			release(c.operand2, c.isImmediate2);
		}
		break;
		case MIDARRAYWRITE: {
			int tmp1 = genLoadMips(c.target, false,
				c.isImmediate1 ? -1 : c.operand1,
				c.isImmediate2 ? -1 : c.operand2);
			int tmp2 = genLoadMips(c.operand1, c.isImmediate1, c.target,
				c.isImmediate2 ? -1 : c.operand2);
			int tmp3 = genLoadMips(c.operand2, c.isImmediate2, c.target,
				c.isImmediate1 ? -1 : c.operand1);
			out << "sll " << "$at" << "," << getRegName(tmp2) << ",2" << endl;
			out << "addu " << "$at" << "," << getRegName(tmp1) << "," <<
				"$at"<<endl;
			out << "sw " << getRegName(tmp3) << ",0(" << "$at" << ")" << endl;
			//writeback(c.target, false);
			release(c.target, false);
			release(c.operand1, c.isImmediate1);
			release(c.operand2, c.isImmediate2);
		}
		break;
		case MIDASSIGN: {
			int tmp1 = genLoadMips(c.target, false,
				c.isImmediate1 ? -1 : c.operand1,-1);
			if (c.operand1 == -1) {
				out << "move " << getRegName(tmp1) << "," << "$v0" << endl;
				writeback(c.target, false);
				release(c.target, false);
				break;
				//break;
			}
			else {
				int tmp2 = genLoadMips(c.operand1, c.isImmediate1, c.target, -1);
				out << "move " << getRegName(tmp1) << "," << getRegName(tmp2) << endl;

			}
			writeback(c.target, false);
			//writeback(c.operand1, c.isImmediate1);
			//writeback(c.operand2, c.isImmediate2);
			release(c.target, false);
			release(c.operand1, c.isImmediate1);
			//release(c.operand2, c.isImmediate2);
			//writeback(c.operand2, c.isImmediate2);
		}
		break;
		case MIDGOTO: {
			out << "j label$" << -c.operand1 << endl;
		}
		break;
		case MIDBNZ: {
			int tmp2 = genLoadMips(c.operand1, c.isImmediate1, -1,-1);
			out << "bgtz " << getRegName(tmp2) << ",label$" << -c.operand2 << endl;
			release(c.operand1, c.isImmediate1);
		}
		break;
		case MIDBZ: {
			int tmp2 = genLoadMips(c.operand1, c.isImmediate1, -1, -1);
			out << "blez " << getRegName(tmp2) << ",label$" << -c.operand2 << endl;
			release(c.operand1, c.isImmediate1);
		}
		break;
		case MIDPRINTINT:{
			int tmp2 = genLoadMips(c.operand1, c.isImmediate1, -1, -1);
			out << "move $a0," << getRegName(tmp2)<<endl;
			out << "li $v0,1" << endl;
			out << "syscall"<<endl;
			//writeback(c.operand1, c.isImmediate1);
			release(c.operand1, c.isImmediate1);
		}
		break;
		case MIDPRINTCHAR: {
			int tmp2 = genLoadMips(c.operand1, c.isImmediate1, -1, -1);
			out << "move $a0," << getRegName(tmp2)<<endl;
			out << "li $v0,11" << endl;
			out << "syscall" << endl;
			//writeback(c.operand1, c.isImmediate1);
			release(c.operand1, c.isImmediate1);
		}
		break;
		case MIDPRINTSTRING:
		{
			out << "la $a0,string$"<<c.operand1<<endl;
			out << "li $v0,4" << endl;
			out << "syscall" << endl;
		}
		break;
		case MIDREADINTEGER:
		{
			int tmp1 = genLoadMips(c.target, false,-1 ,-1);
			out << "li $v0,5" << endl;
			out << "syscall" << endl;
			out << "move " << getRegName(tmp1) << ",$v0" << endl;
			writeback(c.target, false);
			release(c.target, false);

		}
		break;
		case MIDREADCHAR:
		{
			int tmp1 = genLoadMips(c.target, false, -1, -1);
			out << "li $v0,12" << endl;
			out << "syscall" << endl;
			out << "move " << getRegName(tmp1)<<",$v0"<<endl;
			writeback(c.target, false);
			release(c.target, false);
		}
		break;
		case MIDNOP:
			out << "nop" << endl;
			break;

		case MIDPARA:
			break;
		default:

			cout << "bug!";
	}
	out << endl;
}

string MipsGenerator::getRegName(int id) {
	string tmp[32] = {
	"$0","$at","$v0","$v1","$a0",
	"$a1","$a2","$a3","$t0","$t1",
	"$t2","$t3","$t4","$t5","$t6",
	"$t7","$s0","$s1","$s2","$s3",
	"$s4","$s5","$s6","$s7","$t8",
	"$t9","$k0","$k1","$gp","$sp",
	"$fp","$ra"};
	return tmp[id];

}

int MipsGenerator::genLoadMips(int varid1,bool isImmediate,int varid2,int varid3) {

	if (isImmediate) {
		int reg = immReg ? 24 : 25;
		immReg = 1 - immReg;
		out << "li " << getRegName(reg) << "," << varid1 << endl;
		return reg;
	}
	/*
	if (!isImmediate&&regAlloc[varid1] > 0) {
		return regAlloc[varid1];
	}
	
	SymbolEntry* functionEntry = MidCode::table->getSymbolById(currentFunction);
	vector<int>info = tmpRegisterAlloc(varid1,varid2,varid3);
	if (info[1] != -1) {
		SymbolEntry* tmp1 = MidCode::table->getSymbolById(info[1]);
		if (tmp1->scope == functionEntry->name) {
			out << "sw " << getRegName(info[0]) << "," << tmp1->addr << "($sp)";
		}
		regAlloc[info[1]] = UNASSIGNED;
	}

	SymbolEntry* tmp2 = MidCode::table->getSymbolById(varid1);
	if (tmp2->scope == "") {
		out << "lw " << getRegName(info[0]) << "," << tmp2->name << endl;
	}
	else {
		out << "lw " << getRegName(info[0]) << "," << tmp2->addr << "($sp)"<<endl;
	}
	regAlloc[varid1] = info[0];
	tmpRegPoolVar[info[0]-8] = varid1;
	return info[0];
	*/
	SymbolEntry* functionEntry = MidCode::table->getSymbolById(currentFunction);
	vector<int>info = tmpRegisterAlloc(varid1, varid2, varid3);
	SymbolEntry* tmp2 = MidCode::table->getSymbolById(varid1);
	if (tmp2->scope == "") {
		if (tmp2->type == TYPECHARARRAY || tmp2->type == TYPEINTARRAY) {
			out << "la " << getRegName(info[0]) << "," << tmp2->name << endl;
		}
		else
		out << "lw " << getRegName(info[0]) << "," << tmp2->name << endl;
	}
	else {

		if (tmp2->type == TYPECHARARRAY || tmp2->type == TYPEINTARRAY) {
			out << "addiu " << getRegName(info[0]) << "," " $sp," << tmp2->addr<< endl;
		}

		else
		out << "lw " << getRegName(info[0]) << "," << tmp2->addr << "($sp)" << endl;
	}
	regAlloc[varid1] = info[0];
	return info[0];
}

void MipsGenerator::writeback(int i, bool isImmediate) {
	if (!isImmediate) {
		SymbolEntry* tmp2 = MidCode::table->getSymbolById(i);
		/*if (tmp2->scope == ""&& regAlloc[i]>0) {
			out << "sw " << getRegName(regAlloc[i]) << "," << tmp2->name << endl;
			regAlloc[i] = UNASSIGNED;
		}*/
		if (tmp2->scope == "") {
			out << "sw " << getRegName(regAlloc[i]) << "," << tmp2->name << endl;
		}
		else {
			out << "sw " << getRegName(regAlloc[i]) << "," << tmp2->addr << "($sp)" << endl;
		}
	}
}

void MipsGenerator::release(int i, bool isImmediate) {
	if (!isImmediate) {
		SymbolEntry* tmp2 = MidCode::table->getSymbolById(i);
		if (regAlloc[i] - 8 >= 0) {
			tmpRegValid[regAlloc[i] - 8] = 0;
			regAlloc[i] = UNASSIGNED;
		}
	}

}

void MipsGenerator::pushToMips(vector<MidCode>c) {
	int n = c.size();
	for (int i = 0; i < n; i++) {
		int tmp2 = genLoadMips(c[i].operand1, c[i].isImmediate1, -1,-1);
		out << "sw " << getRegName(tmp2) << ", " << (-n + i) * 4 << "($sp)" << endl;
		if (i >= 0 && i <= 3) {
			out << "move "<<"$a"<<i <<","<< getRegName(tmp2) << endl;
		}
	}

}
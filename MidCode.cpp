#include"MidCode.h"
#include<map>
SymbolTable* MidCode::table = nullptr;
int MidCode::labelCount = -2;
int MidCode::tmpVarNo = -2;

int MidCode::tmpVarAlloc() {
	return tmpVarNo--;
}

int MidCode::labelAlloc() {
	return labelCount--;
}
string MidCode::getOperandName(int n,bool isImmediate) {
	if (isImmediate) {
		return to_string(n);
	}
	if (n >= 0) {
		SymbolEntry* entry = table->getSymbolById(n);
		return entry->name;
	}
	else if (n == -1) {
		return "RET";
	}
	else {
		return string("#TMP") + to_string(-n);
	}
}

string MidCode::getLabelName(int n) {
	if (n >= 0) {
		SymbolEntry* entry = table->getSymbolById(n);
		return entry->name;
	}
	else {
		string tmp = "_label";
		return tmp + to_string(-n);
	}
}
ostream& operator<<(ostream& out, MidCode c) {
	vector<string> type = { "int","char","void" };
	map<MidCodeOp, string>ops;
	ops[MIDADD] = "+";
	ops[MIDSUB] = "-";
	ops[MIDMULT] = "*";
	ops[MIDDIV] = "/";
	ops[MIDLSS] = "<";
	ops[MIDLEQ] = "<=";
	ops[MIDGRE] = ">";
	ops[MIDGEQ] = ">=";
	ops[MIDEQL] = "==";
	ops[MIDNEQ] = "!=";
	ops[MIDREM] = "mod";
	if (c.labelNo < -1) {
		out << "_label" << -(c.labelNo) << ": ";
	}
	else if(c.labelNo>=0) {
		out <<MidCode::getOperandName(c.labelNo,false) << ": " << endl;
	}
	switch (c.op) {
		case MIDFUNC:
			out << type[c.operand2] << " ";
			out << MidCode::getOperandName(c.operand1,c.isImmediate1)<< "()";
			break;
		case MIDPARA:
			out << "para " << type[c.operand2]<<" ";
			out << MidCode::getOperandName(c.operand1, c.isImmediate1);
			break;
		case MIDPUSH:
			out << "push ";
			out << MidCode::getOperandName(c.operand1, c.isImmediate1);
			break;
		case MIDCALL:
			out << "call ";
			out << MidCode::getOperandName(c.operand1, c.isImmediate1);
			break;
		case MIDRET:	
			out << "ret ";
			if (c.operand1 == -1) {
				break;
			}
			else {
				out << MidCode::getOperandName(c.operand1, c.isImmediate1);
			}
			break;
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
			out << MidCode::getOperandName(c.target,false);
			out << " " << "=" << " ";
			out << MidCode::getOperandName(c.operand1, c.isImmediate1);
			out << " "<<ops[c.op]<<" ";
			out << MidCode::getOperandName(c.operand2, c.isImmediate2);
			break;
		case MIDNEGATE:
			out << MidCode::getOperandName(c.target, false);
			out << " " << "=" << " negate ";
			out << MidCode::getOperandName(c.operand1, c.isImmediate1);
			break;
		case MIDARRAYGET:
			out << MidCode::getOperandName(c.target,false);
			out << " " << "=" << " ";
			out << MidCode::getOperandName(c.operand1, c.isImmediate1);
			out << " [ " << MidCode::getOperandName(c.operand2, c.isImmediate2) << " ] ";
			break;
		case MIDARRAYWRITE:
			out << MidCode::getOperandName(c.target,false);
			out << "[" << MidCode::getOperandName(c.operand1, c.isImmediate1) << "] =";
			out << MidCode::getOperandName(c.operand2, c.isImmediate2);
			break;
		case MIDASSIGN:
			out << MidCode::getOperandName(c.target,false);
			out << "=" << MidCode::getOperandName(c.operand1, c.isImmediate1);
			break;
		case MIDPRINTINT:
			out << "syscall-1 " << MidCode::getOperandName(c.operand1,c.isImmediate1);
			break;
		case MIDPRINTCHAR:
			out << "syscall-11 " << MidCode::getOperandName(c.operand1, c.isImmediate1);
			break;
		case MIDPRINTSTRING:
			out << "syscall-4 ";
			out << MidCode::table->getString(c.operand1);
			break;
		case MIDREADINTEGER:
			out << MidCode::getOperandName(c.target, false) <<" = syscall-5 ";
			break;
		case MIDREADCHAR:
			out << MidCode::getOperandName(c.target, false) << " = syscall-12 " ;
			break;
		case MIDGOTO:
			out << "GOTO "<<MidCode::getLabelName(c.operand1);
			break;
		case MIDBNZ:
		case MIDBZ:
			out << ((c.op == MIDBZ) ? "BZ " : "BNZ ");
			out << MidCode::getOperandName(c.operand1,c.isImmediate1) << " " << MidCode::getLabelName(c.operand2);
			break;
		case MIDNOP:
			out << "nop";
			break;
		default:
			out << "unknown instruction";
	}
	/*out << "   active[";
	for (int i : c.activeVariable) {
		out << i << " ";
	}
	out <<"]"<< endl;*/
	out << endl;
	return out;
}
MidCode MidCode::generateMidCode(MidCodeOp op, int target,
	int operand1, bool isImmediate1,
	int operand2, bool isImmediate2,
	int label = -1) {
	MidCode tmp;
	tmp.op = op;
	tmp.target = target;
	tmp.operand1 = operand1;
	tmp.operand2 = operand2;
	tmp.isImmediate1 = isImmediate1;
	tmp.isImmediate2 = isImmediate2;
	tmp.labelNo = label;
	return tmp;
}

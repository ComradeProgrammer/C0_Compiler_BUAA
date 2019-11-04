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
	if (c.labelNo < -1) {
		cout << "label" << c.labelNo << ": "<<endl;
	}
	else if(c.labelNo>=0) {
		cout <<MidCode::getOperandName(c.labelNo,false) << ": " << endl;
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
			out << "syscal-l1 " << MidCode::getOperandName(c.operand1,c.isImmediate1);
			break;
		case MIDPRINTCHAR:
			out << "syscall-11 " << MidCode::getOperandName(c.operand1, c.isImmediate1);
			break;
		case MIDPRINTSTRING:
			out << "syscall-4 ";
			out << MidCode::table->getString(c.operand1);
			break;
		case MIDREADINTEGER:
			out<<"syscall-5 "<< MidCode::getOperandName(c.operand1,c.isImmediate1);
			break;
		case MIDREADCHAR:
			out << "syscall-12 " << MidCode::getOperandName(c.operand1,c.isImmediate1);
			break;
		default:
			out << "unknown instruction";
	}

	out << endl;
	return out;
}


#include"MidCodeContainer.h"

int MidCodeContainer::midCodeInsert(MidCodeOp op, int target,
	int operand1,bool isImmediate1,
	int operand2, bool isImmediate2,
	int label=-1 ) {
	MidCode tmp;
	tmp.op = op;
	tmp.target = target;
	tmp.operand1 = operand1;
	tmp.operand2 = operand2;
	tmp.isImmediate1 = isImmediate1;
	tmp.isImmediate2 = isImmediate2;
	tmp.labelNo = label;
	int no = v.size();
	v.push_back(tmp);
	return no;
}

void MidCodeContainer::midCodeInsert(vector<MidCode>& tmp) {
	for (MidCode& i : tmp) {
		v.push_back(i);
	}
}
ostream& operator<<(ostream& out, MidCodeContainer c) {
	MidCode::table->getSubSymbolTableByName(c.functionName)->dumpMidCode(out);
	for (MidCode& code : c.v) {
		out << code;
	}

	return out;
}

int MidCodeContainer::getIndex() {
	return v.size();
}

vector<MidCode>::iterator MidCodeContainer::getIterator(int index) {
	return v.begin() + index;
}

void MidCodeContainer::erase(int start,int end) {
	v.erase(v.begin() + start, v.begin() + end);
}


void MidCodeContainer::removeNops() {
	for (int i = 0; i < v.size(); i++) {
		if (v[i].op == MIDNOP) {
			if (i + 1 >= v.size()||v[i+1].op==MIDFUNC) {
				continue;
			}
			else if (v[i + 1].labelNo != MIDNOLABEL) {
				int oldLabel = v[i].labelNo;
				int newLabel = v[i + 1].labelNo;
				for (int j = 0; j < v.size(); j++) {
					if (v[j].op == MIDGOTO && v[j].operand1 == oldLabel) {
						v[j].operand1 = newLabel;
					}
					else if ((v[j].op == MIDBNZ || v[j].op == MIDBZ) && v[j].operand2 == oldLabel) {
						v[j].operand2 =newLabel;
					}
				}
				v.erase(v.begin() + i, v.begin() + i + 1);
				i--;
			}
			else {
				v[i + 1].labelNo = v[i].labelNo;
				v.erase(v.begin() + i, v.begin() + i + 1);
				i--;
			}
		}
	}
}


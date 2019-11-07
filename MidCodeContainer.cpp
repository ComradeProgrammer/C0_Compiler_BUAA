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
	MidCode::table->getSubSymbolTableByName("")->dumpMidCode(out);
	for (int i = 0; i < c.v.size(); i++) {
		out << c.v[i];
		if (c.v[i].op == MIDFUNC) {
			SymbolEntry* tmp = MidCode::table->getSymbolById(c.v[i].operand1);
			MidCode::table->getSubSymbolTableByName(tmp->name)->dumpMidCode(out);
		}
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
			if (i + 1 >= v.size()) {
				continue;
			}
			else if (v[i + 1].labelNo != MIDNOLABEL) {
				int oldLabel = v[i].labelNo;
				int newLabel = v[i + 1].labelNo;
				for (int j = 0; j < v.size(); j++) {
					if (v[j].labelNo == oldLabel) {
						v[j].labelNo = newLabel;
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
#include"DeadCodeEliminator.h"

DeadCodeEliminator::DeadCodeEliminator( set<int>& _activeOut) {
	
	activeOut = _activeOut;
}

vector<MidCode> DeadCodeEliminator::eliminateDeadCode(vector<MidCode>& v) {
	set<int>localActive=activeOut;
	for (int line = v.size() - 1; line >= 0; line--) {
		MidCode i = v[line];
		if (i.labelNo != -1) {
			label = i.labelNo;
		}
		//三地址运算
		if (i.op == MIDADD || i.op == MIDSUB || i.op == MIDMULT
			|| i.op == MIDDIV || i.op == MIDLSS || i.op == MIDLEQ
			|| i.op == MIDGRE || i.op == MIDGEQ || i.op == MIDEQL
			|| i.op == MIDNEQ || i.op == MIDARRAYGET||i.op==MIDREM) {
			SymbolEntry* entry = MidCode::table->getSymbolById(i.target);
			if (localActive.find(i.target) == localActive.end()
				&& (i.target < 0 || entry->scope != "")) {
				del.push_back(line);
			}
			else {
				localActive.erase(i.target);
				if (!i.isImmediate1) {
					localActive.insert(i.operand1);
				}
				if (!i.isImmediate2) {
					localActive.insert(i.operand2);
				}
			}
		}
		//只使用operand1
		else if (i.op == MIDPUSH || i.op == MIDRET || i.op == MIDBNZ || i.op == MIDBZ
			|| i.op == MIDPRINTINT || i.op == MIDPRINTCHAR) {
			if (!i.isImmediate1 && i.operand1 != -1) {
				localActive.insert(i.operand1);
			}
		}
		//使用operand1并返回值，其中assign需要考虑排除-1
		else if (i.op == MIDNEGATE || i.op == MIDASSIGN) {
			SymbolEntry* entry = MidCode::table->getSymbolById(i.target);
			if (localActive.find(i.target) == localActive.end()
				&&(i.target<0||entry->scope!="")) {
				del.push_back(line);
			}
			else {
				localActive.erase(i.target);
				if (!i.isImmediate1 && i.operand1 != -1) {
					localActive.insert(i.operand1);
				}
			}
		}
		else if (i.op == MIDARRAYWRITE) {
			if (!i.isImmediate1) {
				localActive.insert(i.operand1);
			}
			if (!i.isImmediate2) {
				localActive.insert(i.operand2);
			}
			
		}
		else if (i.op == MIDREADCHAR || i.op == MIDREADINTEGER) {
			localActive.erase(i.target);
			//这句就算是死代码咱也不能删
		}
	}
	vector<MidCode>res;
	for (int i = 0, j = 0; i < v.size();i++) {
		if (j<del.size()&&del[j] == i) {
			j++;
			continue;
		}
		else {
			res.push_back(v[i]);
		}
	}
	if (label != -1) {
		if (res.size() != 0) {
			res[0].labelNo = label;
		}
		else {
			MidCode tmp;
			tmp.op = MIDNOP;
			tmp.target = tmp.operand1 = tmp.operand2 = -1;
			tmp.isImmediate1 = tmp.isImmediate2 = false;
			tmp.labelNo = label;
			res.push_back(tmp);
		}
	}
	return res;
}


#include"BlockOptimization.h"
BlockOptimization::BlockOptimization(set<int>&_activeOut) {
	activeOut = _activeOut;
}
vector<MidCode>BlockOptimization::propagationInBlock(vector<MidCode>& v) {
	map<int, Item>substitution;
	vector<MidCode>res;
	vector<MidCode>resEnd;
	for (int i = 0; i < v.size(); i++) {
		MidCode c = v[i];
		if (c.labelNo != -1) {
			label = c.labelNo;
		}
		switch (c.op) {
			//只使用op1,且只会出现在最后
			case MIDPUSH:
			case MIDRET:
			case MIDBNZ:
			case MIDBZ:
			{
				MidCode tmp = c;
				if (c.operand1 != -1 && !c.isImmediate1 &&
					substitution.find(c.operand1) != substitution.end()) {
					tmp.operand1 = substitution[c.operand1].id;
					tmp.isImmediate1 = substitution[c.operand1].isImmediate;
				}
				resEnd.push_back(tmp);
				break;
			}
			//只使用op1
			case MIDPRINTINT:
			case MIDPRINTCHAR:
			{
				MidCode tmp = c;
				if (c.operand1 != -1 && !c.isImmediate1 &&
					substitution.find(c.operand1) != substitution.end()) {	
					tmp.operand1 = substitution[c.operand1].id;
					tmp.isImmediate1 = substitution[c.operand1].isImmediate;	
				}
				res.push_back(tmp);
				break;
			}
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
			{
				MidCode tmp = c;
				if (c.operand1 != -1 && !c.isImmediate1 &&
					substitution.find(c.operand1) != substitution.end()) {
					tmp.operand1 = substitution[c.operand1].id;
					tmp.isImmediate1 = substitution[c.operand1].isImmediate;
				}
				if (c.operand2 != -1 && !c.isImmediate2 &&
					substitution.find(c.operand2) != substitution.end()) {
					tmp.operand2 = substitution[c.operand2].id;
					tmp.isImmediate2 = substitution[c.operand2].isImmediate;
				}
				set<int>del;
				for (map<int, Item>::iterator itr = substitution.begin();
					itr != substitution.end(); itr++) {
					if (!itr->second.isImmediate && itr->second.id == c.target) {
						MidCode tmp2;
						tmp2.op = MIDASSIGN; tmp2.target = itr->first;
						tmp2.operand1 = itr->second.id; tmp2.isImmediate1 = itr->second.isImmediate;
						tmp2.operand2 = -1; tmp2.isImmediate2 = false;
						tmp2.labelNo = -1;
						res.push_back(tmp2);
						del.insert(itr->first);
					}
				}
				for (int j : del) {
					substitution.erase(j);
				}
				res.push_back(tmp);
				break;
			}
			case MIDNEGATE:
			{
				MidCode tmp = c;
				if (c.operand1 != -1 && !c.isImmediate1 &&
					substitution.find(c.operand1) != substitution.end()) {
					tmp.operand1 = substitution[c.operand1].id;
					tmp.isImmediate1 = substitution[c.operand1].isImmediate;
				}
				set<int>del;
				for (map<int, Item>::iterator itr = substitution.begin();
					itr != substitution.end(); itr++) {
					if (!itr->second.isImmediate && itr->second.id == c.target) {
						MidCode tmp2;
						tmp2.op = MIDASSIGN; tmp2.target = itr->first;
						tmp2.operand1 = itr->second.id; tmp2.isImmediate1 = itr->second.isImmediate;
						tmp2.operand2 = -1; tmp2.isImmediate2 = false;
						tmp2.labelNo = -1;
						res.push_back(tmp2); 
						del.insert(itr->first);
					}
				}
				for (int j : del) {
					substitution.erase(j);
				}
				res.push_back(tmp);
				break;
			}
			case MIDASSIGN:
			{
				if (!c.isImmediate1 && c.operand1 == -1) {
					//ret
					res.push_back(c);
					break;
				}

				set<int>del;
				for (map<int, Item>::iterator itr = substitution.begin();
					itr != substitution.end(); itr++) {
					if (!itr->second.isImmediate && itr->second.id == c.target) {
						MidCode tmp2;
						tmp2.op = MIDASSIGN; tmp2.target = itr->first;
						tmp2.operand1 = itr->second.id; tmp2.isImmediate1 = itr->second.isImmediate;
						tmp2.operand2 = -1; tmp2.isImmediate2 = false;
						tmp2.labelNo = -1;
						res.push_back(tmp2);
						del.insert(itr->first);
					}
				}
				for (int j : del) {
					substitution.erase(j);
				}

				Item tmp;
				tmp.id=  c.operand1;
				tmp.isImmediate = c.isImmediate1;
				substitution[c.target] = tmp;
				break;
			}
			case MIDARRAYGET:
			{
				MidCode tmp = c;
				if (c.operand2 != -1 && !c.isImmediate2 &&
					substitution.find(c.operand2) != substitution.end()) {
					tmp.operand2 = substitution[c.operand2].id;
					tmp.isImmediate2 = substitution[c.operand2].isImmediate;
				}
				set<int>del;
				for (map<int, Item>::iterator itr = substitution.begin();
					itr != substitution.end(); itr++) {
					if (!itr->second.isImmediate && itr->second.id == c.target) {
						MidCode tmp2;
						tmp2.op = MIDASSIGN; tmp2.target = itr->first;
						tmp2.operand1 = itr->second.id; tmp2.isImmediate1 = itr->second.isImmediate;
						tmp2.operand2 = -1; tmp2.isImmediate2 = false;
						tmp2.labelNo = -1;
						res.push_back(tmp2); 
						del.insert(itr->first);
					}
				}
				for (int j : del) {
					substitution.erase(j);
				}
				res.push_back(tmp);
				break;
			}
			case MIDARRAYWRITE:
			{
				MidCode tmp = c;
				if (c.operand1 != -1 && !c.isImmediate1 &&
					substitution.find(c.operand1) != substitution.end()) {
					tmp.operand1 = substitution[c.operand1].id;
					tmp.isImmediate1 = substitution[c.operand1].isImmediate;
				}
				if (c.operand2 != -1 && !c.isImmediate2 &&
					substitution.find(c.operand2) != substitution.end()) {
					tmp.operand2 = substitution[c.operand2].id;
					tmp.isImmediate2 = substitution[c.operand2].isImmediate;
				}
				res.push_back(tmp);
				break;
			}
			case MIDREADINTEGER:
			case MIDREADCHAR:
			{
				MidCode tmp=c;
				set<int>del;
				for (map<int, Item>::iterator itr = substitution.begin();
					itr != substitution.end(); itr++) {
					if (!itr->second.isImmediate && itr->second.id == c.target) {
						MidCode tmp2;
						tmp2.op = MIDASSIGN; tmp2.target = itr->first;
						tmp2.operand1 = itr->second.id; tmp2.isImmediate1 = itr->second.isImmediate;
						tmp2.operand2 = -1; tmp2.isImmediate2 = false;
						tmp2.labelNo = -1;
						res.push_back(tmp2); 
						del.insert(itr->first);
					}
				}
				for (int j : del) {
					substitution.erase(j);
				}
				res.push_back(tmp);
				break;
			}
			case MIDFUNC:
			case MIDPARA:
			case MIDNOP:
				res.push_back(c);
				break;
			case MIDGOTO:
			case MIDCALL:
				resEnd.push_back(c);
				break;
			case MIDPRINTSTRING:
				res.push_back(c);
		}
		
	}
	//todo implement
	for (auto& i : substitution) {
		if (activeOut.find(i.first) != activeOut.end()) {
			MidCode tmp2;
			tmp2.op = MIDASSIGN; tmp2.target = i.first;
			tmp2.operand1 = i.second.id; tmp2.isImmediate1 = i.second.isImmediate;
			tmp2.operand2 = -1; tmp2.isImmediate2 = false;
			tmp2.labelNo = -1;
			res.push_back(tmp2);
		}
	}
	res.insert(res.end(), resEnd.begin(), resEnd.end());
	if (label != -1) {
		if (res.size() != 0) {
			res[0].labelNo = label;
		}
		else {
			MidCode tmp;
			tmp.op = MIDNOP;
			tmp.target = tmp.operand1 = tmp.operand2 = -1;
			tmp.isImmediate1 = tmp.isImmediate2 = false;
			tmp.labelNo=label;
			res.push_back(tmp);
		}
	}
	return res;
}
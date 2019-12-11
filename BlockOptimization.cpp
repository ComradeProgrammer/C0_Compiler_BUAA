#include"BlockOptimization.h"
BlockOptimization::BlockOptimization(set<int>&_activeOut) {
	activeOut = _activeOut;
}

/*实现的是变量传播*/
vector<MidCode>BlockOptimization::propagationInBlock(vector<MidCode>& v) {
	map<int, Item>substitution;
	vector<MidCode>res;
	vector<MidCode>resEnd;
	for (int i = 0; i < v.size(); i++) {
		MidCode c = v[i];
		if (c.labelNo != -1) {
			label = c.labelNo;//一个基本块最多有一个标签，
			//考虑到这个带标签语句可能被优化掉就保存起来
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
					//不是立即数是个变量而且替换表中是有这个变量的
					tmp.operand1 = substitution[c.operand1].id;
					tmp.isImmediate1 = substitution[c.operand1].isImmediate;
				}
				resEnd.push_back(tmp);//都是只能在最后出现的中间代码
				//push只能连着最后跟着call伪指令，这一串指令肯定在基本块最后
				//为了给最后的写回留下机会，不能在跳转之后写回
				break;
			}
			//只使用op1还不再最后的指令
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
			case MIDREM:
			{
				MidCode tmp = c;
				//如果两个立即数都是常量的话
				if (c.isImmediate1 && c.isImmediate2) {
					int res = 0;
					switch (c.op) {
						case MIDADD:
							res = c.operand1 + c.operand2;
							break;
						case MIDSUB:
							res = c.operand1 - c.operand2;
							break;
						case MIDMULT:
							res = c.operand1 * c.operand2;
							break;
						case MIDDIV:
							res = c.operand1 / c.operand2;
							break;
						case MIDLEQ:
							res = c.operand1 <= c.operand2;
							break;
						case MIDLSS:
							res = c.operand1 < c.operand2;
							break;
						case MIDGRE:
							res = c.operand1 > c.operand2;
							break;
						case MIDGEQ:
							res = c.operand1 >= c.operand2;
							break;
						case MIDEQL:
							res = c.operand1 == c.operand2;
							break;
						case MIDNEQ:
							res = c.operand1 != c.operand2;
							break;
					}
					Item item;
					item.isImmediate = true;
					item.id = res;
					substitution[c.target] = item;
					break;

				}
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
				//如果在标记表中有被标记var1=var2然后var2被改了，那么var1必须在此之前被明确赋值
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
				substitution.erase(c.target);
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
				substitution.erase(c.target);
				res.push_back(tmp);
				break;
			}
			case MIDASSIGN:
			{
				if (!c.isImmediate1 && c.operand1 == -1) {
					//返回值赋值的事情已经在这里解决了
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
				//a=b,若是此前已经记录b=c那么此时也应记录a=c
				if (!c.isImmediate1&&substitution.find(c.operand1) != substitution.end()) {
					tmp.id = substitution[c.operand1].id;
					tmp.isImmediate = substitution[c.operand1].isImmediate;
				}
				
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
				substitution.erase(c.target);
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
				substitution.erase(c.target);
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
		SymbolEntry* e = MidCode::table->getSymbolById(i.first);
		//此时临时变量尚未加入符号表，e可以是null
		if (activeOut.find(i.first) != activeOut.end()||(i.first>0&&e->scope=="")) {
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
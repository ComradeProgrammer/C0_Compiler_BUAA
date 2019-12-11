#include"PeepHoleOptimization.h"
vector<MidCode> PeepHoleOptimization::peepHoleOptimization(vector<MidCode>& v) {
	vector<MidCode>res;
	/*针对取余操作的优化*/
	for (int i = 0; i < v.size(); i++) {
		if (i > (int)(v.size()) - 3) {
			res.push_back(v[i]);
			continue;
		}
		if (v[i].op == MIDDIV && v[i + 1].op == MIDMULT && v[i+2].op == MIDSUB) {
			int a0 = v[i].operand1;
			bool a1 = v[i].isImmediate1;
			int b0 = v[i].operand2;
			bool b1 = v[i].isImmediate2;
			int c0 = v[i].target;
			bool c1 = false;
			if (!(v[i + 1].operand1 == c0 && v[i + 1].isImmediate1 == c1)) {
				res.push_back(v[i]);
				continue;
			}
			if (!(v[i + 1].operand2 == b0 && v[i + 1].isImmediate2 == b1)) {
				res.push_back(v[i]);
				continue;
			}
			int d0 = v[i+1].target;
			bool d1 = false;
			if (!(v[i + 2].operand1 == a0 && v[i + 2].isImmediate1 == a1)) {
				res.push_back(v[i]);
				continue;
			}
			if (!(v[i + 2].operand2 == d0 && v[i + 2].isImmediate2 == d1)) {
				res.push_back(v[i]);
				continue;
			}
			/*因为这里会取消对c d的赋值，所以必须保证以后cd不再有人使用*/
			if (!c1 && v[i + 2].activeVariable.find(c0) != v[i + 2].activeVariable.end()) {
				res.push_back(v[i]);
				continue;
			}
			if (!d1 && v[i + 2].activeVariable.find(d0) != v[i + 2].activeVariable.end()) {
				res.push_back(v[i]);
				continue;
			}
			MidCode tmp = MidCode::generateMidCode(MIDREM, v[i + 2].target,
				a0, a1, b0, b1, v[i].labelNo);
			i += 2;
			res.push_back(tmp);
		}
		else {
			res.push_back(v[i]);
			continue;
		}
	}
	return res;
}
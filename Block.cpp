#include"Block.h"
int Block::count = 0;

Block::Block() {
	id = count++;
}

void Block::insert(MidCode c) {
	v.push_back(c);
}

void Block::addPrev(Block* b) {
	prev.push_back(b);
}

void Block::addNext(Block* b) {
	next.push_back(b);
}

void Block::useDefScan() {
	for (int ii = v.size() - 1; ii >= 0;ii--) {
		MidCode i = v[ii];
		//正常三地址运算,注意立即数问题
		if (i.op == MIDADD || i.op == MIDSUB || i.op == MIDMULT
			|| i.op == MIDDIV || i.op == MIDLSS || i.op == MIDLEQ
			|| i.op == MIDGRE || i.op == MIDGEQ || i.op == MIDEQL
			|| i.op == MIDNEQ||i.op==MIDARRAYGET) {
			use.erase(i.target);
			def.insert(i.target);
			if (!i.isImmediate1) {
				def.erase(i.operand1);
				use.insert(i.operand1);
			}
			if (!i.isImmediate2) {
				def.erase(i.operand2);
				use.insert(i.operand2);
			}
		}
		//只使用operand1
		else if (i.op == MIDPUSH || i.op == MIDRET||i.op==MIDBNZ||i.op==MIDBZ
			||i.op==MIDPRINTINT||i.op==MIDPRINTCHAR) {
			if (!i.isImmediate1) {
				def.erase(i.operand1);
				def.erase(i.operand2);
			}
		}
		//使用operand1并返回值，其中assign需要考虑排除-1
		else if (i.op == MIDNEGATE || i.op == MIDASSIGN) {
			use.erase(i.target);
			def.insert(i.target);
			if (!i.isImmediate1&&i.operand1!=-1) {
				def.erase(i.operand1);
				def.erase(i.operand2);
			}
			
		}
		//数组写操作只会使用2个操作数，但是不会对target造成影响，因为数组地址没变
		else if (i.op == MIDARRAYWRITE) {
			if (!i.isImmediate1) {
				def.erase(i.operand1);
				use.insert(i.operand1);
			}
			if (!i.isImmediate2) {
				def.erase(i.operand2);
				use.insert(i.operand2);
			}
		}//只使用target
		else if (i.op == MIDREADCHAR || i.op == MIDREADINTEGER) {
			use.erase(i.target);
			def.insert(i.target);
		}

	}
}


bool Block::activeVariableAnalyzeEpoch() {
	int oldsize = activeIn.size();
	for (Block* i : next) {
		activeOut = setUnion(activeOut, i->activeIn);
	}
	set<int>tmp = setDifference(activeOut, def);
	activeIn = setUnion(activeIn, use);
	return oldsize == activeIn.size();

}

set<int>Block::setUnion(set<int> a, set<int> b) {
	set<int>res;
	for (int i : a) {
		res.insert(i);
	}
	for (int i : b) {
		res.insert(i);
	}
	return res;
}

set<int>Block::setDifference(set<int> a, set<int> b) {
	set<int>res;
	for (int i : a) {
		res.insert(i);
	}
	for (int i : b) {
		res.erase(i);
	}
	return res;
}
ostream& operator<<(ostream& out, Block b) {
	out << "====Block"<<b.id<<"====" << endl;
	out << "prev:[";
	for (Block* i : b.prev) {
		out << i->id << ",";
	}
	out << "]" << endl;
	for (MidCode& i : b.v) {
		out << i;
	}
	out << "next:[";
	for (Block* i : b.next) {
		out << i->id << ",";
	}
	out << "]" << endl;
	out << "=============="<<endl;
	return out;
}

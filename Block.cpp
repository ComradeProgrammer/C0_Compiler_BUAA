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
		//��������ַ����,ע������������
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
		//ֻʹ��operand1
		else if (i.op == MIDPUSH || i.op == MIDRET||i.op==MIDBNZ||i.op==MIDBZ
			||i.op==MIDPRINTINT||i.op==MIDPRINTCHAR) {
			if (!i.isImmediate1) {
				def.erase(i.operand1);
				def.erase(i.operand2);
			}
		}
		//ʹ��operand1������ֵ������assign��Ҫ�����ų�-1
		else if (i.op == MIDNEGATE || i.op == MIDASSIGN) {
			use.erase(i.target);
			def.insert(i.target);
			if (!i.isImmediate1&&i.operand1!=-1) {
				def.erase(i.operand1);
				def.erase(i.operand2);
			}
			
		}
		//����д����ֻ��ʹ��2�������������ǲ����target���Ӱ�죬��Ϊ�����ַû��
		else if (i.op == MIDARRAYWRITE) {
			if (!i.isImmediate1) {
				def.erase(i.operand1);
				use.insert(i.operand1);
			}
			if (!i.isImmediate2) {
				def.erase(i.operand2);
				use.insert(i.operand2);
			}
		}//ֻʹ��target
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

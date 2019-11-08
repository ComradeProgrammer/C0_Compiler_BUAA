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
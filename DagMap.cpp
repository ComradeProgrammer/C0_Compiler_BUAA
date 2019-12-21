#include"DagMap.h"

//叶子节点构造函数
DagNode::DagNode(int _leafInformation, bool _isImmediate) {
	isLeaf = true;
	leafOperand = _leafInformation;
	isImmediate = _isImmediate;
	if (!isImmediate) {
		possibleNames.insert(leafOperand);
	}
}

//非叶子节点构造函数
DagNode::DagNode(MidCodeOp _op, DagNode* _left, DagNode* _right) {
	isLeaf = false;
	op = _op;
	left = _left;
	right = _right;
	if (nullptr != left) {
		left->father.insert(this);
		son.insert(left);
	}
	if (nullptr != right) {
		right->father.insert(this);
		son.insert(right);
	}
}

DagMap::DagMap(set<int> _mustOut,set<int>_shouldAssign){
	label=MIDNOLABEL;
	mustOut=_mustOut;
	shouldAssign=_shouldAssign;
	fakeOutputStream=MidCode::tmpVarAlloc();
}

DagMap::~DagMap(){
	for(DagNode* i:nodes){
		delete i;
	}
}

DagNode* DagMap::getNodeByVar(int id,bool isImmediate){
	if(isImmediate){
		if(immToNode.find(id)!=immToNode.end()){
			return immToNode[id];
		}
		else {
			DagNode* tmp=new DagNode(id,isImmediate);
			immToNode[id]=tmp;
			nodes.push_back(tmp);
			return tmp;
		}
	}
	else{
		if(varToNode.find(id)!=varToNode.end()){
			return varToNode[id];
		}
		else {
			DagNode* tmp=new DagNode(id,isImmediate);
			varToNode[id]=tmp;
			nodes.push_back(tmp);
			return tmp;
		}
	}
}

DagNode* DagMap::getNodeBySon(MidCodeOp op,DagNode* left,DagNode* right){
	for(DagNode* i:nodes){
		if(i->op==op&&i->left==left&&i->right==right){
			return i;
		}
		else if((op == MIDADD || op == MIDMULT || op == MIDEQL || op == MIDNEQ)
			&&i->op==op&&i->left==right&&i->right==left){
			return i;
		}
	}
	DagNode *tmp=new DagNode(op,left,right);
	nodes.push_back(tmp);
	return tmp;
}

void DagMap::handleMidCode(MidCode c){
	if(c.labelNo!=MIDNOLABEL){
		label=c.labelNo;//标签只能出现在一个基本块的第一个语句上；
	}
		switch (c.op) {
		case MIDFUNC:
		case MIDPARA:
		case MIDNOP:
			//只能出现在最前部，不会影响DAG
			beginning.push_back(c);
			break;
		//只能出现在最后，不会影响DAG
		case MIDGOTO:
		case MIDCALL:
		case MIDREADCHAR:
		case MIDREADINTEGER:
			ending.push_back(c);
			break;
		case MIDPUSH:
		case MIDRET:
		case MIDBNZ:
		case MIDBZ:
			mustOut.insert(c.operand1);
			ending.push_back(c);
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
		case MIDREM:
		case MIDARRAYGET:
		{
			DagNode* node1 = getNodeByVar(c.operand1, c.isImmediate1);
			DagNode* node2 = getNodeByVar(c.operand2, c.isImmediate2);
			DagNode* node = getNodeBySon(c.op, node1, node2);
			if (varToNode.find(c.target) != varToNode.end() && varToNode[c.target] != node) {
				//出现了先引用再赋值的情况
				varToNode[c.target]->possibleNames.erase(c.target);//使旧变量无法再被命名为c.target；
				if (assigned.find(c.target)==assigned.end()) {
					//对于第一次重新赋值的变量，记录他的旧名字。
					//这里要防范一个问题就是一个变量上来就直接被赋值了，用另一个变量或者立即数
					//寻找的标准是，未被赋值但是使用过的变量，是否被使用过由varToNode判断，是否被赋值过由assigned判断
					varToNode[c.target]->shouldReAssign = true;
					varToNode[c.target]->formerName = c.target;
				}
				
			}
			varToNode[c.target] = node;
			node->possibleNames.insert(c.target);
			assigned.insert(c.target);
			if (c.target > 0) {
				SymbolEntry* entry = MidCode::table->getSymbolById(c.target);
				if (entry->scope == "") {
					mustOut.insert(c.target);
				}
			}
			break;
		}
		case MIDNEGATE:
		{
			DagNode* node1 = getNodeByVar(c.operand1, c.isImmediate1);
			DagNode* node = getNodeBySon(c.op, node1, nullptr);
			if (varToNode.find(c.target) != varToNode.end() && varToNode[c.target] != node) {
				//出现了先引用再赋值的情况
				varToNode[c.target]->possibleNames.erase(c.target);//使旧变量无法再被命名为c.target；
				if (assigned.find(c.target) == assigned.end()) {
					//对于第一次重新赋值的变量，记录他的旧名字。
					varToNode[c.target]->shouldReAssign = true;
					varToNode[c.target]->formerName = c.target;
				}
			}
			varToNode[c.target] = node;
			node->possibleNames.insert(c.target);
			assigned.insert(c.target);
			if (c.target > 0) {
				SymbolEntry* entry = MidCode::table->getSymbolById(c.target);
				if (entry->scope == "") {
					mustOut.insert(c.target);
				}
			}
			break;
		}
		case MIDASSIGN:
		{
			//这里有一个-1:RET的问题：拿返回值赋值只能是第一句，我们必须保证这是第一句
			if (c.operand1 == -1) {
				beginning.push_back(c);
				shouldAssign.insert(c.operand1);
				break;
			}
			DagNode* node1 = getNodeByVar(c.operand1, c.isImmediate1);
			if (varToNode.find(c.target) != varToNode.end() && varToNode[c.target] != node1) {
				//出现了先引用再赋值的情况
				varToNode[c.target]->possibleNames.erase(c.target);//使旧变量无法再被命名为c.target；
				if (assigned.find(c.target) == assigned.end()) {
					//对于第一次重新赋值的变量，记录他的旧名字。
					varToNode[c.target]->shouldReAssign = true;
					varToNode[c.target]->formerName = c.target;
				}
			
			}
			varToNode[c.target] = node1;
			node1->possibleNames.insert(c.target);
			assigned.insert(c.target);
			if (c.target > 0) {
				SymbolEntry* entry = MidCode::table->getSymbolById(c.target);
				if (entry->scope == "") {
					mustOut.insert(c.target);
				}
			}
			break;
		}
		case MIDPRINTCHAR:
		case MIDPRINTINT:
		{
			DagNode* node1 = getNodeByVar(c.operand1, c.isImmediate1);
			DagNode* node2 = getNodeByVar(fakeOutputStream, false);
			//必须直接新建，输出一样的东西两次不能当一次
			DagNode* node = new DagNode(c.op, node1, node2);
			nodes.push_back(node);
			//肯定重新赋值,
			if (varToNode.find(fakeOutputStream) != varToNode.end()) {
				varToNode[fakeOutputStream]->possibleNames.erase(fakeOutputStream);
			}
			node->possibleNames.insert(fakeOutputStream);
			varToNode[fakeOutputStream] = node;
			break;
		}
		case MIDPRINTSTRING: 
		{
			//把字符串编号偷偷藏在leafOperand里面
			DagNode* node2 = getNodeByVar(fakeOutputStream, false);
			DagNode* node = new DagNode(c.op, nullptr, node2);
			node->leafOperand = c.operand1;//字符串编号
			nodes.push_back(node);
			
			//若有，肯定重新赋值
			if (varToNode.find(fakeOutputStream) != varToNode.end()) {
				varToNode[fakeOutputStream]->possibleNames.erase(fakeOutputStream);
			}
			node->possibleNames.insert(fakeOutputStream);
			varToNode[fakeOutputStream] = node;
			break;
		}
		case MIDARRAYWRITE:
		{
			DagNode* node1 = getNodeByVar(c.operand1, c.isImmediate1);
			DagNode* node2 = getNodeByVar(c.operand2, c.isImmediate2);
			DagNode* node = new DagNode(c.op, node1, node2);
			nodes.push_back(node);
			//bfs
			if (varToNode.find(c.target) != varToNode.end()) {
				deque<DagNode*>queue;
				queue.push_back(varToNode[c.target]);
				//没有环，不用检查重复
				while (queue.size() > 0) {
					DagNode* mark = queue.front();
					for (DagNode* i : mark->father) {
						if (i != node) {
							queue.push_back(i);
						}
					}
					queue.pop_front();
					mark->father.insert(node);
					node->son.insert(mark);
				}	
			}
			node->possibleNames.insert(c.target);
			node->leafOperand = c.target;
			varToNode[c.target] = node;
			break;
		}
		default:
			cout << "miss";

	}
}

vector<MidCode> DagMap::dumpMidCode() {
	//分配名字
	for (DagNode* i : nodes) {
		if (i->isLeaf&&i->isImmediate) {
			//立即数
			i->name = i->leafOperand;
		}
		else if (i->isLeaf && !i->isImmediate && i->possibleNames.find(i->leafOperand) != i->possibleNames.end()) {
			//叶子节点要有初值，如果不发生先使用再赋值的事，必须考虑用初始节点
			i->name = i->leafOperand;
			mustOut.erase(i->name);
		}
		else {
			
			//靠possibleNames的
			if (i->possibleNames.size() == 0) {
				i->name = MidCode::tmpVarAlloc();
			}
			else {
				bool given = false;
				for (int j : i->possibleNames) {
					if (mustOut.find(j) != mustOut.end()) {
						i->name = j;
						given = true;
						mustOut.erase(j);
						break;
					}
				}
				if (!given) {
					i->name = *(i->possibleNames.begin());
				}
			}
			if (i->shouldReAssign && shouldAssign.find(i->formerName) != shouldAssign.end()) {
				//该变量shouldassign被标记为true且在开头处活跃,生成赋值语句
				MidCode code;
				int tmp1 = MidCode::tmpVarAlloc();
				code = MidCode::generateMidCode(MIDASSIGN, tmp1,
					i->formerName, false, MIDUNUSED, false, MIDNOLABEL);
				/*.op = MIDASSIGN; code.target = i->name;
				code.operand1 = i->formerName; code.isImmediate1 = false;
				code.operand2 = MIDUNUSED; code.isImmediate2 = false, code.labelNo = MIDNOLABEL;
				*/
				beginning.push_back(code);
				code = MidCode::generateMidCode(MIDASSIGN, i->name,
					tmp1, false, MIDUNUSED, false, MIDNOLABEL);
				middle.push_back(code);
			}
		}
	}
	//开始确定导出顺序
	vector<DagNode*>q;
	int i = 0;
	DagNode* tmp = nullptr;
	while (i < nodes.size() ) {
		tmp = nullptr;
		for (int j = 0; j < nodes.size(); j++) {
			if (nodes[j]->dumped) { continue; }
			else if (nodes[j]->isLeaf) {
				i++;
				nodes[j]->dumped = true;
			}
			else if (nodes[j]->father.size() == 0) {
				tmp = nodes[j];
				break;
			}
		}
		while (tmp != nullptr && !tmp->dumped) {
			i++;
			tmp->dumped = true;
			q.push_back(tmp);
			for (DagNode* son : tmp->son) {
				son->father.erase(tmp);
			}
			if (tmp->left != NULL&&tmp->left->father.size()==0) {
				tmp = tmp->left;
				continue;
			}
			if (tmp->right != NULL && tmp->right->father.size() == 0) {
				tmp = tmp->right;
				continue;
			}
		}
	}
	//开始按顺序生成中间代码
	for (int i = q.size() - 1; i >= 0; i--) {
		MidCode code = nodeToMidCode(q[i]);
		middle.push_back(code);
	}
	//开始根据mustout生成赋值语句
	for (int i : mustOut) {
		if (varToNode.find(i) != varToNode.end()) {
			DagNode* node = varToNode[i];
			MidCode code;
			code.op = MIDASSIGN; code.target = i;
			code.operand1 = node->name; code.isImmediate1 = node->isImmediate;
			code.operand2 = MIDUNUSED; code.isImmediate2 = false, code.labelNo = MIDNOLABEL;
			middle.push_back(code);
		}
	}
	//开始合并
	beginning.insert(beginning.end(), middle.begin(), middle.end());
	beginning.insert(beginning.end(), ending.begin(), ending.end());
	if (label != MIDNOLABEL) {
		if (beginning.size() == 0) {
			MidCode code;
			code.op = MIDNOP; code.target = i;
			code.operand1 = -1; code.isImmediate1 =false;
			code.operand2 = -1; code.isImmediate2 = false, code.labelNo = MIDNOLABEL;
			beginning.push_back(code);
		}
		beginning[0].labelNo = label;
	}
	return beginning;
}

MidCode DagMap::nodeToMidCode(DagNode* node) {
	switch (node->op) {
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
		case MIDARRAYGET:
		case MIDARRAYWRITE:
		case MIDREM:
		{
			MidCode res;
			res.op = node->op;
			res.target = node->name;
			res.operand1 = node->left->name;
			res.isImmediate1 = node->left->isImmediate;
			res.operand2 = node->right->name;
			res.isImmediate2 = node->right->isImmediate;
			res.labelNo = MIDNOLABEL;
			return res;
		}
		case MIDNEGATE:
		{
			MidCode res;
			res.op = node->op;
			res.target = node->name;
			res.operand1 = node->left->name;
			res.isImmediate1 = node->left->isImmediate;
			res.operand2 = -1; res.isImmediate2 = false; res.labelNo = MIDNOLABEL;
			return res;
		}
		case MIDPRINTCHAR:
		case MIDPRINTINT:
		{
			MidCode res;
			res.op = node->op;
			res.target = -1;
			res.operand1 = node->left->name;
			res.isImmediate1 = node->left->isImmediate;
			res.operand2 = -1; res.isImmediate2 = false; res.labelNo = MIDNOLABEL;
			return res;
		}
		case MIDPRINTSTRING:
		{
			MidCode res;
			res.op = node->op;
			res.target = -1;
			res.operand1 = node->leafOperand;
			res.isImmediate1 = false;
			res.operand2 = -1; res.isImmediate2 = false; res.labelNo = MIDNOLABEL;
			return res;
		}

	}
}
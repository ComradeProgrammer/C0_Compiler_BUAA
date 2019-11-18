#include"DagMap.h"
DagNode::DagNode(bool _isLeaf, int _varid, bool _isImmediate) {
	isLeaf = _isLeaf;
	isImmediate = _isImmediate;
	varId = _varid;
	left = NULL;
	right = NULL;
	if (!_isImmediate) {
		nameVarIdPool.insert(_varid);
	}
}

DagNode::DagNode(MidCodeOp _op, DagNode* _left, DagNode* _right) {
	isLeaf = false;
	op = _op;
	left = _left;
	right = _right;
	if (left != NULL) {
		left->fathers.insert(this);
	}
	if (right != NULL) {
		right->fathers.insert(this);
	}
}

DagNode* DagMap::getNodeByVar(int varid, bool isImmediate) {
	if (isImmediate) {
		map<int, DagNode*>::iterator itr = nodeImmTable.find(varid);
		if (itr == nodeImmTable.end()) {
			return NULL;
		}
		else {
			return itr->second;
		}
	}
	else {
		map<int, DagNode*>::iterator itr = nodeVarTable.find(varid);
		if (itr == nodeVarTable.end()) {
			return NULL;
		}
		else {
			return itr->second;
		}
	}
}

DagNode* DagMap::getNodeBySon(MidCodeOp op, DagNode* left, DagNode* right) {
	for (DagNode* i : dagTree) {
		if (i->op == op && i->left == left && i->right == right) {
			return  i;
		}
		else if ((op == MIDADD || op == MIDMULT || op == MIDEQL || op == MIDNEQ) 
			&& i->op == op && i->right == left && i->left == right) {
			return i;
		}
	}
	return NULL;
}

MidCode DagMap::parseToMidCode(DagNode* node) {
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
		{
			MidCode res;
			res.op = node->op;
			res.target = node->nameVarId;

			res.operand1 = node->left->nameVarId;
			res.isImmediate1 = node->left->isImmediate;

			res.operand2 = node->right->nameVarId;
			res.isImmediate2 = node->right->isImmediate;
			res.labelNo = -1;
			return res;
		}
		case MIDNEGATE:
		{
			MidCode res;
			res.op = node->op;
			res.target = node->nameVarId;
			res.operand1 = node->left->nameVarId;
			res.isImmediate1 = node->left->isImmediate;
			res.operand2 = MIDUNUSED;
			res.isImmediate2 = false;
			res.labelNo = -1;
			return res;
		}
		case MIDPRINTCHAR:
		case MIDPRINTINT:
		{
			MidCode res;
			res.op = node->op;
			res.target = MIDUNUSED;
			res.operand1 = node->left->nameVarId;
			res.isImmediate1 = node->left->isImmediate;
			res.operand2 = MIDUNUSED;
			res.isImmediate2 = false;
			res.labelNo = -1;
			return res;
		}
		case MIDPRINTSTRING:
		{
			MidCode res;
			res.op = node->op;
			res.target = MIDUNUSED;
			res.operand1 = node->varId;
			res.isImmediate1 = false;
			res.operand2 = MIDUNUSED;
			res.isImmediate2 = false;
			res.labelNo = -1;
			return res;
		}
		case MIDREADCHAR:
		case MIDREADINTEGER:
		{
			MidCode res;
			res.op = node->op;
			res.target = node->nameVarId;
			res.operand1 = MIDUNUSED;
			res.isImmediate1 = false;
			res.labelNo = -1;
			res.operand2 = MIDUNUSED;
			res.isImmediate2 = false;
			res.labelNo = -1;
			return res;
		}
	}
}

void DagMap::dumpCurrentCode() {
	for (DagNode* i : currentNodes) {
		//赋节点名字
		if (i->isLeaf&&initialValueMap.find(i->varId)!=initialValueMap.end()
			&& initialValueMap[i->varId]==i) {
			i->nameVarId = i->varId;
			continue;
		}
		else {
			if (i->nameVarIdPool.size() == 0) {
				i->nameVarId = MidCode::tmpVarAlloc();
				continue;
			}
			bool given = false;
			for (int j : i->nameVarIdPool) {
				if (mustOut.find(j) != mustOut.end()) {
					i->nameVarId = j;
					given = true;
					break;
				}
			}
			if (!given) {
				i->nameVarId = *(i->nameVarIdPool.begin());
			}
		}
		mustOut.erase(i->nameVarId);
	}
	vector<DagNode*>queue;
	int i = 0;
	while (i < currentNodes.size()) {
		DagNode* j = NULL;
		for (DagNode* j2 : currentNodes) {
			if (j2->dumped) {
				continue;
			}
			j = j2;
			if (j2->fathers.size() == 0) {
				break;
			}
		}
		while (!j->dumped ) {
			j->dumped = true;
			queue.push_back(j);
			i++;
			if (j->left != NULL) {
				j->left->fathers.erase(j);
			}
			if (j->right != NULL) {
				j->right->fathers.erase(j);
			}
			if (j->left != NULL && j->left->fathers.size() == 0&&!j->left->dumped) {
				j = j->left;
			}
			else if (j->right != NULL && j->right->fathers.size() == 0&&!j->right->dumped) {
				j = j->right;
			}
		}
	}
	for (int i = queue.size() - 1; i >= 0;i--) {
		if (queue[i]->isLeaf) { continue; }
		MidCode code = parseToMidCode(queue[i]);
		middle.push_back(code);
	}
	currentNodes.clear();
}

void DagMap::handleMidCode(MidCode c) {
	if (c.labelNo != MIDNOLABEL) { label = c.labelNo; }
	switch (c.op) {
		case MIDFUNC:
		case MIDPARA:
			beginning.push_back(c);
			break;
		case MIDPUSH:
		case MIDRET:
		case MIDBNZ:
		case MIDBZ:
			mustOut.insert(c.operand1);
			ending.push_back(c);
			break;
		case MIDGOTO:
		case MIDCALL:
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
		case MIDARRAYGET:
		{
			//这都是三个操作数全使用上的
			DagNode* node1 = getNodeByVar( c.operand1, c.isImmediate1);
			if (node1 == NULL) {
				node1 = new DagNode(true, c.operand1, c.isImmediate1);
				dagTree.push_back(node1);
				currentNodes.push_back(node1);
				if (c.isImmediate1) { nodeImmTable[c.operand1] = node1; }
				else { nodeVarTable[c.operand1] = node1; }
			}
			DagNode* node2 = getNodeByVar( c.operand2, c.isImmediate2);
			if (node2 == NULL) {
				node2 = new DagNode(true, c.operand2, c.isImmediate2);
				dagTree.push_back(node2);
				currentNodes.push_back(node2);
				if (c.isImmediate2) { nodeImmTable[c.operand2] = node2; }
				else { nodeVarTable[c.operand2] = node2; }
			}
			DagNode* node = getNodeBySon( c.op, node1, node2);
			if (node == NULL) {
				node = new DagNode(c.op, node1, node2);
				dagTree.push_back(node);
				currentNodes.push_back(node);
			}
			if (nodeVarTable.find(c.target) != nodeVarTable.end() && nodeVarTable[c.target] != node) {
				nodeVarTable[c.target]->nameVarIdPool.erase(c.target);//重新赋值了
				if (shouldAssign.find(c.target) != shouldAssign.end() &&//这个变量是活跃的，有使用没赋值那种
					initialValueMap.find(c.target) == initialValueMap.end()//头一次重新赋值那种
					) {
					initialValueMap[c.target] = nodeVarTable[c.target];
				}
			}
			nodeVarTable[c.target] = node;
			node->nameVarIdPool.insert(c.target);
			break;
		}
		case MIDARRAYWRITE:
		{
			DagNode* node1 = getNodeByVar( c.operand1, c.isImmediate1);
			if (node1 == NULL) {
				node1 = new DagNode(true, c.operand1, c.isImmediate1);
				dagTree.push_back(node1);
				currentNodes.push_back(node1);
				if (c.isImmediate1) { nodeImmTable[c.operand1] = node1; }
				else { nodeVarTable[c.operand1] = node1; }
			}
			DagNode* node2 = getNodeByVar( c.operand2, c.isImmediate2);
			if (node2 == NULL) {
				node2 = new DagNode(true, c.operand2, c.isImmediate2);
				dagTree.push_back(node2);
				currentNodes.push_back(node2);
				if (c.isImmediate2) { nodeImmTable[c.operand2] = node2; }
				else { nodeVarTable[c.operand2] = node2; }
			}
			DagNode* node = getNodeBySon( c.op, node1, node2);
			bool inject = false;
			if (node == NULL) {
				node = new DagNode(c.op, node1, node2);
				inject = true;
				
			}
			if (nodeVarTable.find(c.target) != nodeVarTable.end() && nodeVarTable[c.target] != node) {
				//重新赋值了，这次不删名字，直接导出之前的中间代码重新开始
				dumpCurrentCode();
			}
			if (inject) {
				dagTree.push_back(node);
				currentNodes.push_back(node);
			}
			nodeVarTable[c.target] = node;
			node->nameVarIdPool.insert(c.target);
			break;
		}
		case MIDNEGATE:
			//case MIDASSIGN:没有assign的事，assign应该直接套标签，写在后面了
		{
			//这是个只有target和操作数1的
			DagNode* node1 = getNodeByVar( c.operand1, c.isImmediate1);
			if (node1 == NULL) {
				node1 = new DagNode(true, c.operand1, c.isImmediate1);
				dagTree.push_back(node1);
				currentNodes.push_back(node1);
				if (c.isImmediate1) { nodeImmTable[c.operand1] = node1; }
				else { nodeVarTable[c.operand1] = node1; }
			}
			DagNode* node = getNodeBySon( c.op, node1, NULL);
			if (node == NULL) {
				node = new DagNode(c.op, node1, NULL);
				dagTree.push_back(node);
				currentNodes.push_back(node);
			}
			if (nodeVarTable.find(c.target) != nodeVarTable.end() && nodeVarTable[c.target] != node) {
				nodeVarTable[c.target]->nameVarIdPool.erase(c.target);//重新赋值了
				if (shouldAssign.find(c.target) != shouldAssign.end() &&//这个变量是活跃的，有使用没赋值那种
					initialValueMap.find(c.target) == initialValueMap.end()//头一次重新赋值那种
					) {
					initialValueMap[c.target] = nodeVarTable[c.target];
					
				}
			}
			nodeVarTable[c.target] = node;
			node->nameVarIdPool.insert(c.target);
			break;
		}
		case MIDASSIGN:
		{
			DagNode* node1 =getNodeByVar( c.operand1, c.isImmediate1);
			if (node1 == NULL) {
				node1 = new DagNode(true, c.operand1, c.isImmediate1);
				dagTree.push_back(node1);
				currentNodes.push_back(node1);
				if (c.isImmediate1) { nodeImmTable[c.operand1] = node1; }
				else { nodeVarTable[c.operand1] = node1; }
			}
			DagNode* node = node1;
			if (nodeVarTable.find(c.target) != nodeVarTable.end() && nodeVarTable[c.target] != node) {
				nodeVarTable[c.target]->nameVarIdPool.erase(c.target);//重新赋值了
				if (shouldAssign.find(c.target) != shouldAssign.end() &&//这个变量是活跃的，有使用没赋值那种
					initialValueMap.find(c.target) == initialValueMap.end()//头一次重新赋值那种
					) {
					initialValueMap[c.target] = nodeVarTable[c.target];
					
				}
			}
			nodeVarTable[c.target] = node;
			node->nameVarIdPool.insert(c.target);
			break;

		}
		case MIDPRINTCHAR:
		case MIDPRINTINT:
		{
			DagNode* node1 = getNodeByVar( c.operand1, c.isImmediate1);
			if (node1 == NULL) {
				node1 = new DagNode(true, c.operand1, c.isImmediate1);
				dagTree.push_back(node1);
				currentNodes.push_back(node1);
				if (c.isImmediate1) { nodeImmTable[c.operand1] = node1; }
				else { nodeVarTable[c.operand1] = node1; }
			}
			DagNode* node2 =getNodeByVar( output, false);
			if (node2 == NULL) {
				node2 = new DagNode(true, output, false);
				dagTree.push_back(node2);
				currentNodes.push_back(node2);
				if (c.isImmediate2) { nodeImmTable[c.operand2] = node2; }
				else { nodeVarTable[c.operand2] = node2; }
			}
			DagNode* node = getNodeBySon( c.op, node1, node2);
			if (node == NULL) {
				node = new DagNode(c.op, node1, node2);
				dagTree.push_back(node);
				currentNodes.push_back(node);
			}
			if (nodeVarTable.find(output) != nodeVarTable.end() && nodeVarTable[output] != node) {
				nodeVarTable[output]->nameVarIdPool.erase(output);//重新赋值了,输出流肯定不需要开头assign
			}
			nodeVarTable[output] = node;
			node->nameVarIdPool.insert(output);
			break;
		}
		case MIDPRINTSTRING:
		{
			DagNode* node2 = getNodeByVar(output, false);
			if (node2 == NULL) {
				node2 = new DagNode(true, output, false);
				dagTree.push_back(node2);
				currentNodes.push_back(node2);
				if (c.isImmediate2) { nodeImmTable[c.operand2] = node2; }
				else { nodeVarTable[c.operand2] = node2; }
			}
			DagNode* node = getNodeBySon( c.op, NULL, node2);
			if (node == NULL) {
				node = new DagNode(c.op, (DagNode*)NULL, node2);
				dagTree.push_back(node);
				currentNodes.push_back(node);
			}
			if (nodeVarTable.find(output) != nodeVarTable.end() && nodeVarTable[output] != node) {
				nodeVarTable[output]->nameVarIdPool.erase(output);//重新赋值了
			}
			nodeVarTable[output] = node;
			node->nameVarIdPool.insert(output);
			node->varId = c.operand1;
			break;
		}
		case MIDREADCHAR:
		case MIDREADINTEGER:
		{
			DagNode* node =new DagNode(c.op, (DagNode*)NULL, NULL);
			dagTree.push_back(node);
			currentNodes.push_back(node);
			//这个节点不能被复用
			if (nodeVarTable.find(c.target) != nodeVarTable.end() && nodeVarTable[c.target] != node) {
				nodeVarTable[c.target]->nameVarIdPool.erase(c.target);//重新赋值了,赋的肯定不是数组
				if (shouldAssign.find(c.target) != shouldAssign.end() &&//这个变量是活跃的，有使用没赋值那种
					initialValueMap.find(c.target) == initialValueMap.end()//头一次重新赋值那种
					) {
					initialValueMap[c.target] = nodeVarTable[c.target];
					
				}
			}
			nodeVarTable[c.target] = node;
			node->nameVarIdPool.insert(c.target);
			break;
		}
		case MIDNOP:
			break;
		default:
			cout << "bug at dag" << endl;
	}
}

void DagMap::init(set<int>activeIn, set<int>activeOut) {
	mustOut = activeOut;
	shouldAssign = activeIn;
	label = -1;
	output = MidCode::tmpVarAlloc();
	currentNodes.clear();
	dagTree.clear();
	nodeVarTable.clear();
	nodeImmTable.clear();
	initialValueMap.clear();
	beginning.clear();
	middle.clear();
	ending.clear();
}

vector<MidCode> DagMap::result() {
	dumpCurrentCode();
	for (auto& i : initialValueMap) {
		MidCode c;
		c.op = MIDASSIGN;
		c.target = i.second->nameVarId;
		c.operand1 = i.first;
		c.isImmediate1 = false;
		c.operand2 = -1;
		c.isImmediate2 = false;
		c.labelNo = -1;
		beginning.push_back(c);
	}
	beginning.insert(beginning.end(), middle.begin(), middle.end());
	if (beginning.size() != 0 && label != -1) {
		beginning[0].labelNo = label;
	}
	for (int i : mustOut) {
		if (nodeVarTable.find(i) == nodeVarTable.end()) { continue; }
		MidCode c;
		c.op = MIDASSIGN;
		c.target = i;
		c.operand1 = nodeVarTable[i]->nameVarId;
		c.isImmediate1 = nodeVarTable[i]->isImmediate;
		c.operand2 = -1;
		c.isImmediate2 = false;
		c.labelNo = -1;
		beginning.push_back(c);
	}
	beginning.insert(beginning.end(), ending.begin(), ending.end());
	return beginning;
}
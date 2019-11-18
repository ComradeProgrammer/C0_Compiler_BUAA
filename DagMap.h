#pragma once
#include"MidCode.h"
struct DagNode {
	bool isLeaf = false;
	bool isImmediate = false;//only works when isLeaf is true
	int varId;//only works when isLeaf is true
	set<int>nameVarIdPool;
	int nameVarId;
	MidCodeOp op;
	DagNode* left = NULL;
	DagNode* right = NULL;
	set<DagNode*> fathers;
	bool dumped = false;//是否已经导出过
	DagNode(bool isLeaf, int varid, bool isImmediate);
	DagNode(MidCodeOp op, DagNode* left, DagNode* right);
};

class DagMap {
private:
	int output;
	int label;
	set<int>mustOut;
	set<int>shouldAssign;
	vector<DagNode*>currentNodes;
	vector<DagNode*>dagTree;
	map<int, DagNode*>nodeVarTable;
	map<int, DagNode*>nodeImmTable;
	map<int, DagNode*>initialValueMap;
	vector<MidCode> beginning;
	vector<MidCode>middle;
	vector<MidCode>ending;

	DagNode* getNodeByVar(int varid, bool isImmediate);
	DagNode* getNodeBySon(MidCodeOp op, DagNode* left, DagNode* right);
	void dumpCurrentCode();
	MidCode parseToMidCode(DagNode* node);
public:
	void init(set<int>activeIn, set<int>activeOut);
	void handleMidCode(MidCode c);
	vector<MidCode> result();

};
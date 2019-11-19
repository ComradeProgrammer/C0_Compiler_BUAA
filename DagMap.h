#pragma once
#include<queue>
#include"MidCode.h"
struct DagNode {
	bool isLeaf=false;//是否是叶子节点
	//若为叶子节点
	bool isImmediate=false;//若为叶子节点是否是立即数
	int leafOperand;//若为叶子节点，存储立即数或变量编号
	bool shouldReAssign=false;//是否是被先使用后赋值的节点
	int formerName;//如果上一个答案是true，那么他本来代表的变量是
	//若不为叶子节点
	MidCodeOp op;
	DagNode* left=nullptr;
	DagNode* right=nullptr;
	set<DagNode*> father;
	set<DagNode*>son;//供继承儿子使用
	//导出时使用的命名问题
	set<int>possibleNames;
	int name;
	DagNode(int leafInformation, bool isImmediate);//叶子节点构造函数
	DagNode(MidCodeOp op, DagNode* left, DagNode* right);//非叶子节点的构造函数
	//导出记录
	bool dumped = false;
	/*读语句算一个控制流转移吧不然太要命了
	数组这里需要一套。。。继承父亲机制*/
};

class DagMap {
public:
	DagMap(set<int>_mustOut,set<int>_shouldAssign);
	~DagMap();
	DagNode* getNodeByVar(int id,bool isImmediate);
	DagNode* getNodeBySon(MidCodeOp op,DagNode* left,DagNode* right);
	void handleMidCode(MidCode c);
	vector<MidCode>dumpMidCode();
private:
	int label;
	int fakeOutputStream;
	vector<DagNode*>nodes;

	map<int, DagNode*>immToNode;
	map<int, DagNode*>varToNode;

	set<int>mustOut;
	set<int>shouldAssign;
	set<int>assigned;
		
	vector<MidCode>beginning;
	vector<MidCode>ending;
	vector<MidCode>middle;

	
	MidCode nodeToMidCode(DagNode* node);
};
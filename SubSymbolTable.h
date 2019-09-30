#pragma once
#include<vector>
#include<map>
#include<string>
using namespace std;
/*������Ƿ�������*/
enum SymbolType {
	TYPEINT,
	TYPECHAR,
	TYPEINTARRAY,
	TYPECHARARRAY,
	TYPEINTCONST,
	TYPECHARCONST,
	TYPEFUNCTION
};
/*�������������Ϊ����ʱ��������*/
enum ReturnType {
	RETINT,
	RETCHAR,
	RETVOID
};

/*�ڱ���Ϊ����ʱ�ĸ�����*/
struct FunctionLink {
	ReturnType returnType;
	int paraNum;
	vector<SymbolType>paras;
	int tmpVarSize;
	friend ostream& operator<<(ostream& stream,FunctionLink f);
};

/*���ű���*/
struct SymbolEntry {
	string name;//���Զ�����
	int id;//���Զ�����
	SymbolType type;
	string scope;//���Զ�����
	int initValue;
	unsigned int addr;
	int dimension=0;
	FunctionLink* link=NULL;//���Զ�����
	friend ostream& operator<<(ostream& stream,SymbolEntry s);
};

/*�ӷ��ű���Ϊÿ���������������ķ��ű����еķ��ű���������̬����*/
class SubSymbolTable {
public:
	SubSymbolTable(string _nameScope);
	~SubSymbolTable();
	SymbolEntry* addSymbol(string name,bool isFuntion);
	SymbolEntry* getSymbolByName(string name);

	void selfTest();
private:
	string nameScope;
	map<string, SymbolEntry*>symbolMap;
};
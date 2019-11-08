#pragma once
#define MIDINT 0
#define MIDCHAR 1
#define MIDVOID 2
#define MIDNOLABEL -1
#define MIDUNUSED -1
#include<iostream>
#include<vector>
#include<set>
#include"SymbolTable.h"
using namespace std;
/*�Լ��Ĺ涨��printf��scanf��ֱ��չ���ں���������*/
/*�ڸ�ֵ�����-1�ű�������RET�������������������-1������õĲ�����*/
/*��ǩ��-1�ű�����־��û�б�ǩ*/
enum MidCodeOp {
	MIDFUNC = 0,//�������� 1-�����ķ��ű�id 2-�����ķ���ֵ����
	MIDPARA,//�����β����� 1-��ʽ�����ı��id 2-����������
	MIDPUSH,//����ʵ��ȷ�� 1-ʵ�ε�id
	MIDCALL,//�������� 1-������id
	MIDRET,//�������� 1-���ر�� ��Ϊ-1���޷���ֵ
	MIDADD,
	MIDSUB,
	MIDMULT,
	MIDDIV,
	MIDLSS,
	MIDLEQ,
	MIDGRE,
	MIDGEQ,
	MIDEQL,
	MIDNEQ,
	MIDNEGATE,//ȡ�෴����1-��ȡ������
	MIDARRAYGET,// x=a[i],x��idΪtarget��a��idΪ1,i��idΪ2
	MIDARRAYWRITE,//a[i]=j a��idΪtarget i��iDΪ1 j��idΪ2
	MIDASSIGN,//1-��ֵ�Ľ��
	MIDGOTO,//��������ת��1-��תĿ���ţ��˴���������ʶʹ��false
	MIDBNZ,//����Ϊ����ת 1-��������� 2-Ŀ����
	MIDBZ,//����Ϊ����ת 1-��������� 2-Ŀ����
	MIDPRINTINT,//SYSCALL1 1-��ӡֵ��id
	MIDPRINTCHAR,//SYSCALL11 1-��ӡ�ַ���id
	MIDPRINTSTRING,//SYSCALL4 1-��ӡ�ַ�����id
	MIDREADINTEGER,//SYSCALL5 target-����������id
	MIDREADCHAR,//SYSCALL12  target-�����ַ���id
	MIDNOP,//�Ը���תʹ�õģ��Ż�ʱ��Ͷ��ɵ��ˣ�ʲô����Ҫʲô������
};
/*�������¼һ��*/
class MidCode {
public:
	static SymbolTable* table;
	static int tmpVarNo;
	static int tmpVarAlloc();
	static int labelCount ;
	static int labelAlloc();

	MidCodeOp op;
	int target;
	//��һ������
	int operand1;
	bool isImmediate1 = false;
	//�ڶ�������
	int operand2;
	bool isImmediate2 = false;
	int labelNo = -1;

	friend ostream& operator<<(ostream& out, MidCode c);
	static string getOperandName(int n,bool isImmediate);
	static string getLabelName(int n);
	
	
};
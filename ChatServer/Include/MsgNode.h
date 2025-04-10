#ifndef MSGNODE_H
#define MSGNODE_H
#include <iostream>

using namespace std;
class LogicSystem;

class IMsgNode
{
public:
	IMsgNode(unsigned short max_len);

	virtual ~IMsgNode();

	void Clear();

	unsigned short CurLen{0};
	unsigned short TotalLen;
	char* Data;
};

class CRecvNode : public IMsgNode
{
	friend class SChatLogic;

public:
	CRecvNode(unsigned short max_len, unsigned short msg_id);

private:
	unsigned short MsgID;
};

class CSendNode : public IMsgNode
{
	friend class SChatLogic;

public:
	CSendNode(const char* msg, unsigned short max_len, unsigned short msg_id);

private:
	unsigned short MsgID;
};
#endif // MSGNODE_H

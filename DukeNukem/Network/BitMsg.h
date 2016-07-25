// BitMsg.h
//

#pragma once

#include <windows.h>

//
// BitMsg
//
class BitMsg
{
public:
	BitMsg();

	void SetData(byte *buffer, int length);

	template<typename T>
	T Read();

	byte *ReadData(int len);
	void ReadData(byte *data, int len);

	template<typename T>
	void Write(T var);

	void WriteData(byte *data, int length);

	byte *GetBuffer() { return buffer; }
	int GetLength() { return msglen; }
	int GetWrittenLength() { return currentposition; }
private:
	byte *buffer;
	int currentposition;
	int msglen;
};

__forceinline BitMsg::BitMsg()
{
	this->currentposition = 0;
}

__forceinline void BitMsg::SetData(byte *buffer, int length)
{
	this->buffer = buffer;
	this->msglen = length;
	this->currentposition = 0;
}

template<typename T>
__forceinline T BitMsg::Read()
{
	T var;
	memcpy(&var, &buffer[currentposition], sizeof(T));
	currentposition += sizeof(T);
	return var;
}

__forceinline byte *BitMsg::ReadData(int len)
{
	byte *position = &buffer[currentposition];
	currentposition += len;
	return position;
}

__forceinline void BitMsg::ReadData(byte *data, int len)
{
	byte *position = &buffer[currentposition];
	currentposition += len;
	memcpy(data, position, len);
}

template<typename T>
__forceinline void BitMsg::Write(T var)
{
	WriteData((byte *)&var, sizeof(T));
}

__forceinline void BitMsg::WriteData(byte *data, int length)
{
	memcpy(&buffer[currentposition], data, length);
	currentposition += length;
}
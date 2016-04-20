// cache1d_wrapper.cpp
//

#include "compat.h"
#ifdef _WIN32
// for FILENAME_CASE_CHECK
# include <shellapi.h>
#endif
#include "cache1d.h"

FILE* fopen_mkdir(const char* name, const char* mode) 
{
	char* mname = strdup(name);
	int i;
	for (i = 0; mname[i] != '\0'; i++) {
		if (i > 0 && (mname[i] == '\\' || mname[i] == '/')) {
			char slash = mname[i];
			mname[i] = '\0';
			mkdir(mname);
			mname[i] = slash;
		}
	}
	free(mname);
	return fopen(name, mode);
}

//
// BuildFile::OpenFile
//
BuildFile *BuildFile::OpenFile(const char *fmt, BuildFileOpenType openType)
{
	if (openType == BuildFile_Read)
	{
		int32_t fp = kopen4load(fmt, 0);

		if (fp < 0)
			return NULL;

		return new BuildFile(fp, openType);
	}

	FILE *file = NULL;
	
	if (openType == BuildFile_Write)
	{
		file = fopen_mkdir(fmt, "wb");
	}
	else if (openType == BuildFile_Append)
	{
		file = fopen(fmt, "a+");
	}

	if (!file)
	{
		return NULL;
	}

	return new BuildFile(file, openType);
}

//
// BuildFile::BuildFile
//
BuildFile::BuildFile(int32_t fp, BuildFileOpenType openType)
{
	this->openType = openType;
	this->fp = fp;
	this->file = NULL;
}
//
// BuildFile::BuildFile
//
BuildFile::BuildFile(FILE *file, BuildFileOpenType openType)
{
	this->fp = -1;
	this->file = file;
	this->openType = openType;
}

BuildFile::~BuildFile()
{
	if (openType == BuildFile_Read)
	{
		kclose(fp);
	}
	else if (file != NULL)
	{
		fclose(file);
		file = NULL;
	}
}

//
// BuildFile::WriteFile
//
bool BuildFile::WriteFile(const char *fileName, const char *buffer, int length)
{
	BuildFile *file = OpenFile(fileName, BuildFile_Write);
	file->Write(buffer, length);
	delete file;
	return true;
}

//
// BuildFile::Tell
//
int32_t BuildFile::Tell()
{
	if(openType == BuildFile_Read)
		return ktell(fp);

	return ftell(file);
}

//
// Seek
//
void BuildFile::Seek(int32_t offset, int32_t whence)
{
	if (openType == BuildFile_Read)
	{
		klseek(fp, offset, whence);
		return;
	}

	fseek(file, offset, whence);
}

//
// Length
//
int32_t BuildFile::Length()
{
	int32_t position = Tell();
	Seek(0, SEEK_END);
	int32_t size = Tell();
	Seek(position, SEEK_SET);
	return size;
}

//
// BuildFile::Read
//
int32_t BuildFile::Read(void *buffer, int32_t leng)
{
	return kread(fp, buffer, leng);
}


/*
=================
BuildFile::ReadInt
=================
*/
void BuildFile::ReadInt(int &value) {
	Read(&value, sizeof(value));
}

/*
=================
BuildFile::ReadUnsignedInt
=================
*/
void BuildFile::ReadUnsignedInt(unsigned int &value) {
	Read(&value, sizeof(value));
}

/*
=================
BuildFile::ReadShort
=================
*/
void BuildFile::ReadShort(short &value) {
	Read(&value, sizeof(value));
}

/*
=================
idFile::ReadUnsignedShort
=================
*/
void BuildFile::ReadUnsignedShort(unsigned short &value) {
	Read(&value, sizeof(value));
}

/*
=================
BuildFile::ReadChar
=================
*/
void BuildFile::ReadChar(char &value) {
	Read(&value, sizeof(value));
}

/*
=================
BuildFile::ReadUnsignedChar
=================
*/
void BuildFile::ReadUnsignedChar(unsigned char &value) {
	Read(&value, sizeof(value));
}

/*
=================
BuildFile::ReadFloat
=================
*/
void BuildFile::ReadFloat(float &value) {
	Read(&value, sizeof(value));
}

/*
=================
BuildFile::ReadBool
=================
*/
void BuildFile::ReadBool(bool &value) {
	unsigned char c;
	ReadUnsignedChar(c);
	value = c ? true : false;
}
/*
=================
BuildFile::ReadBool
=================
*/
void BuildFile::ReadString(std::string &str)
{
	int length;
	ReadInt(length);
	str.resize(length);
	Read((void *)str.c_str(), length);
}

void BuildFile::Write(const char *buffer, int length)
{
	fwrite(buffer, 1, length, file);
}

void BuildFile::WriteInt(const int value)
{
	fwrite(&value, 1, sizeof(int), file);
}

void BuildFile::WriteUnsignedInt(const unsigned int value)
{
	fwrite(&value, 1, sizeof(unsigned int), file);
}

void BuildFile::WriteShort(const short value)
{
	fwrite(&value, 1, sizeof(short), file);
}

void BuildFile::WriteUnsignedShort(unsigned short value)
{
	fwrite(&value, 1, sizeof(unsigned short), file);
}

void BuildFile::WriteChar(const char value)
{
	fwrite(&value, 1, sizeof(unsigned char), file);
}

void BuildFile::WriteUnsignedChar(const unsigned char value)
{
	fwrite(&value, 1, sizeof(unsigned char), file);
}

void BuildFile::WriteFloat(const float value)
{
	fwrite(&value, 1, sizeof(float), file);
}

void BuildFile::WriteBool(const bool value)
{
	fwrite(&value, 1, sizeof(bool), file);
}

void BuildFile::WriteString(const char *string)
{
	int len = strlen(string);
	WriteInt(len);
	fwrite(string, 1, len, file);
}
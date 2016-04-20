
#ifndef BUILD_SCRIPTFILE_H_
#define BUILD_SCRIPTFILE_H_

#include <Vector>

typedef struct {
	char *textbuf;
	uint32_t textlength;
	char *ltextptr;		// pointer to start of the last token fetched (use this for line numbers)
	char *textptr;
	char *eof;
	char *filename;
	int32_t linenum;
	int32_t *lineoffs;
} scriptfile;

typedef std::string ScriptToken;

//
// TokenBlock
//
class TokenBlock
{
friend class Parser;
public:
	TokenBlock();
	const ScriptToken &GetNextToken();

	bool IsEOF();
private:

	int currentToken;
	std::vector<std::string> tokens;
};

//
// Parser
//
class Parser
{
public:
	Parser(const char *fn, bool loadFromFileSystem);
	~Parser();

	const ScriptToken &GetString();

	void ParseBracketSection(TokenBlock &block);

	bool IsEOF();
private:
	TokenBlock tokenBlock;
};

char *scriptfile_gettoken(scriptfile *sf);
int32_t scriptfile_getnumber(scriptfile *sf, int32_t *num);
int32_t scriptfile_getdouble(scriptfile *sf, double *num);
int32_t scriptfile_getstring(scriptfile *sf, char **st);
int32_t scriptfile_getsymbol(scriptfile *sf, int32_t *num);
int32_t scriptfile_getlinum(const scriptfile *sf, const char *ptr);
int32_t scriptfile_getbraces(scriptfile *sf, char **braceend);

scriptfile *scriptfile_fromfile(const char *fn);
scriptfile *scriptfile_fromstring(const char *string);
void scriptfile_close(scriptfile *sf);
int32_t scriptfile_eof(scriptfile *sf);

int32_t scriptfile_getsymbolvalue(char *name, int32_t *val);
int32_t scriptfile_addsymbolvalue(char *name, int32_t val);
void scriptfile_clearsymbols(void);

#endif

// Scriptfile_wrapper.cpp
//

#include "compat.h"
#include "scriptfile.h"
#include "baselayer.h"
#include "compat.h"
#include "cache1d.h"

TokenBlock::TokenBlock()
{
	currentToken = 0;
}

const ScriptToken &TokenBlock::GetNextToken()
{
	if (IsEOF())
	{
		return "";
	}

	return tokens[currentToken++];
}

bool TokenBlock::IsEOF()
{
	if (currentToken >= tokens.size())
		return true;

	return false;
}

Parser::Parser(const char *fn, bool loadFromFileSystem)
{
	scriptfile *handle;
	if (loadFromFileSystem)
	{
		handle = scriptfile_fromfile(fn);
	}
	else
	{
		handle = scriptfile_fromstring(fn);
	}

	while (scriptfile_eof(handle) == 0)
	{
		char *token;

		if (scriptfile_getstring(handle, &token) != 0)
			break;

		tokenBlock.tokens.push_back(token);
	}

	scriptfile_close(handle);
}

Parser::~Parser()
{
	
}

bool Parser::IsEOF()
{
	return tokenBlock.IsEOF();
}

const ScriptToken &Parser::GetString()
{
	return tokenBlock.GetNextToken();
}

void Parser::ParseBracketSection(TokenBlock &block)
{
	int currentBracketLayer = 0;
	if (GetString() != "{")
		return;

	while (!IsEOF())
	{
		const ScriptToken &token = GetString();
		if (token[0] == '}')
		{
			if (currentBracketLayer == 0)
			{
				break;
			}
			else
			{
				currentBracketLayer--;
			}
		}

		if (token == "{")
		{
			currentBracketLayer++;
		}

		block.tokens.push_back(token);
	}
}
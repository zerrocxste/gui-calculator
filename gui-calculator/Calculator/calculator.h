#include "../includes.h"

class CCalculator;
class CCalculator
{
public:
	void setup(const std::string pszExpression)
	{
		_szExpression = pszExpression;
	}
	void compute();
	float getResult()
	{
		return _flResult;
	}
private:
	#define DEBUG 0
	std::string _szExpression;
	float _flResult;
	enum OPERATORS
	{
		NONE,
		ADDITION,
		SUBSTRACTION,
		MULTIPLICATION,
		DIVISION
	};
	enum STEPS
	{
		WAIT_FOR_FIRST_NUMBER,
		FOUND_SYMBOL,
		WAIT_FOR_SECOND_NUMBER,
		CALC_SECOND_EXPRESSION
	};
	bool getBrackets();
	void solveBrackets();
	float computeExpression(std::string pszExpression);
	OPERATORS reintepretSymbol(char s)
	{
		switch (s)
		{
		case '+':
			return OPERATORS::ADDITION;
			break;
		case '-':
			return OPERATORS::SUBSTRACTION;
			break;
		case '*':
			return OPERATORS::MULTIPLICATION;
			break;
		case '/':
			return OPERATORS::DIVISION;
			break;
		}
	}
	struct utils_s
	{
		void StringReplace(char* buf, const char* search, const char* replace)
		{
			char* p = buf;

			size_t l1 = strlen(search);
			size_t l2 = strlen(replace);

			while (p = strstr(p, search))
			{
				memmove(p + l2, p + l1, strlen(p + l1) + 1U);
				memcpy(p, replace, l2);
				p += l2;
			}
		}
	}utils;
};
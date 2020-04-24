/*!
	\file
	\author kovbas
*/
#include "pre.h"
#include "dcmtkUtils.h"

XRAD_BEGIN

// обработка ошибок выполнения функций
void	checkOFResult(const OFCondition &result, const std::string &what_are_we_doing)
{
	if (result.bad())
	{
		ForceDebugBreak();
		throw runtime_error(what_are_we_doing + "\n" + result.text());
	}
}

XRAD_END

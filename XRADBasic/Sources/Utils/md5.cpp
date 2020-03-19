/*!
	\file
	\date 2019/09/23 14:20
	\author kulberg
*/
#include "pre.h"
#include "md5.h"
#include <XRADBasic/ThirdParty/md5/md5_core.h>

XRAD_BEGIN

using namespace md5_core;

md5_digest_t md5(const void *bytes, size_t n_bytes)
{
	md5_state_t state;
	md5_byte_t digest[16];

	md5_init(&state);
	md5_append(&state, (const md5_byte_t *)bytes, int(n_bytes));
	md5_finish(&state, digest);
	md5_digest_t result;

	for(int i = 0; i < 16; ++i) result[i] = digest[i];

	return result;
}

md5_digest_t md5(const string &s)
{
	return md5(s.data(), s.size());
}

md5_digest_t md5(const wstring &s)
{
	auto	s8 = convert_to_string8(s);
	return md5(s8.data(), s8.size());
}


string	md5_digest_string(const md5_digest_t &digest, bool caps = false)
{
	string	result;
	string	format = caps ? "%02X":"%02x";

	for(int i = 0; i < 16; ++i)
	{
		result += ssprintf(format, digest[i]);
	}

	return result;
}

wstring	md5_digest_wstring(const md5_digest_t &digest, bool caps = false)
{
	return convert_to_wstring(md5_digest_string(digest,caps));
}

XRAD_END

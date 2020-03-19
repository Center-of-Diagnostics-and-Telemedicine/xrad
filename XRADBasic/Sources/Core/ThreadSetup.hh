// file ThreadSetup.hh
//--------------------------------------------------------------

XRAD_BEGIN

//--------------------------------------------------------------

template <class... Args>
ThreadErrorCollectorEx<Args...>::ThreadErrorCollectorEx(const string &name):
	name(name)
{
}

//--------------------------------------------------------------

template <class... Args>
void ThreadErrorCollectorEx<Args...>::Reset(const string &i_name)
{
	name = i_name;
	has_special_errors = false;
	error_count = 0;
	cancel_ep = nullptr;
	quit_ep = nullptr;
	errors.clear();
}

//--------------------------------------------------------------

template <class... Args>
bool ThreadErrorCollectorEx<Args...>::HasSpecialErrors() const
{
	return has_special_errors;
}

//--------------------------------------------------------------

template <class... Args>
bool ThreadErrorCollectorEx<Args...>::HasErrors() const
{
	return error_count.load() != 0;
}

//--------------------------------------------------------------

template <class... Args>
size_t ThreadErrorCollectorEx<Args...>::ErrorCount() const
{
	return error_count.load();
}

//--------------------------------------------------------------

template <class... Args>
void ThreadErrorCollectorEx<Args...>::AddErrorMessage(const string &message, const Args&... args)
{
	++error_count;
	try
	{
		std::lock_guard<std::mutex> lock(mt);
		errors.push_back(std::make_tuple(message, args...));
	}
	catch (...)
	{
	}
}

//--------------------------------------------------------------

template <class... Args>
void ThreadErrorCollectorEx<Args...>::CatchException(const Args&... args)
{
	++error_count;
	try
	{
		try
		{
			throw;
		}
		catch (quit_application &)
		{
			has_special_errors = true;
			std::lock_guard<std::mutex> lock(mt);
			if (!quit_ep)
			{
				quit_ep = std::current_exception();
			}
		}
		catch (canceled_operation &)
		{
			has_special_errors = true;
			std::lock_guard<std::mutex> lock(mt);
			if (!cancel_ep)
			{
				cancel_ep = std::current_exception();
			}
		}
		catch (exception &e)
		{
			std::lock_guard<std::mutex> lock(mt);
			if (!quit_ep && !cancel_ep)
			{
				errors.push_back(std::make_tuple(string(e.what()), args...));
			}
		}
		catch (...)
		{
			std::lock_guard<std::mutex> lock(mt);
			if (!quit_ep && !cancel_ep)
			{
				errors.push_back(std::make_tuple(GetExceptionString(), args...));
			}
		}
	}
	catch (...)
	{
	}
}

//--------------------------------------------------------------

namespace
{
string merge(const list<std::tuple<string>> &strings, const string &delimiter)
{
	if (!strings.size())
		return string();
	auto it = strings.begin();
	string result = std::get<0>(*it);
	for (++it; it != strings.end(); ++it)
	{
		result += delimiter + std::get<0>(*it);
	}
	return result;
}
} // namespace

template <class... Args>
template <class Void>
auto ThreadErrorCollectorEx<Args...>::ThrowIfErrors() -> std::enable_if_t<sizeof...(Args) == 0 && std::is_void<Void>::value, void>
{
	if (!error_count)
		return;
	if (quit_ep)
	{
		std::rethrow_exception(quit_ep);
	}
	if (cancel_ep)
	{
		std::rethrow_exception(cancel_ep);
	}
	auto message = ssprintf("%s:\n%s",
			EnsureType<const char*>(name.c_str()),
			EnsureType<const char*>(merge(errors, "\n").c_str()));
	if (error_count > errors.size())
	{
		message += ssprintf("\nAn unknown error occurred %zu times.",
				EnsureType<size_t>(error_count-errors.size()));
	}
	throw runtime_error(message);
}

//--------------------------------------------------------------

template <class... Args>
void ThreadErrorCollectorEx<Args...>::ThrowIfSpecialExceptions()
{
	if (quit_ep)
	{
		std::rethrow_exception(quit_ep);
	}
	if (cancel_ep)
	{
		std::rethrow_exception(cancel_ep);
	}
}

//--------------------------------------------------------------

template <class... Args>
template <class Functor>
void ThreadErrorCollectorEx<Args...>::ProcessErrors(Functor f)
{
	for (auto &v: errors)
	{
		caller<tuple_size<tuple_type>::value>::call(f, v);
	}
}

//--------------------------------------------------------------

XRAD_END

//--------------------------------------------------------------

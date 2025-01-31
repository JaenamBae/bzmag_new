//-----------------------------------------------------------------------------
inline CurrentSnapShot::CurrentSnapShot():
line_(0)
{
    // empty
}


//-----------------------------------------------------------------------------
inline CurrentSnapShot::CurrentSnapShot
(const char* type, const char* date,
 const char* time, const char* file_name,
 const char* func_sig, const char* func_name, int line,
 const char* expression)
{
    setSemantic(type, date, time, file_name,
        func_sig, func_name, line, expression);
}


//-----------------------------------------------------------------------------
inline void CurrentSnapShot::setSemantic
(const char* type, const char* date,
 const char* time, const char* file_name,
 const char* func_sig, const char* func_name, int line,
 const char* expression)
{
    type_ = type;
    date_ = date;
    time_ = time;
    fileName_ = file_name;
    functionSignature_ = func_sig;
    functionName_ = func_name;
    line_ = line;
    expression_ = expression;
}

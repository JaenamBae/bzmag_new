#ifndef BZMAG_CORE_BASE_DEFINE_H
#define BZMAG_CORE_BASE_DEFINE_H

#define ___FILE___ __FILE__
#define ___FUNCSIG___ __FUNCSIG__
#define ___FUNCTION___ __FUNCTION__
#define ___LINE___  __LINE__
#define ___DATE___  __DATE__
#define ___TIME___  __TIME__

//-----------------------------------------------------------------------------
template <typename DERIVED, typename BASE>
DERIVED DOWN_CAST(BASE* p)
{
    return dynamic_cast<DERIVED>(p);
}

#endif // BZMAG_CORE_BASE_DEFINE_H

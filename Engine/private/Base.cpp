#include "Base.h"

Base::Base()
{
}

_uint Base::AddRef()
{
	return ++referenceCount;
}

_uint Base::Release()
{
    if (referenceCount == 0)
    {
        Free();
        delete this;
        return 0;
    }
    else
    {
        return  referenceCount--;
    }
}

void Base::Free()
{
}

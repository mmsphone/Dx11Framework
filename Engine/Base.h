#pragma once

#include "Engine_Defines.h"

NS_(Engine)

/* 스마트 포인터의 레퍼런스 카운트 기능을 대체하고 직접 관리하기 위함 */
class ENGINE_DLL Base abstract
{
protected:
	Base();
	virtual  ~Base() = default;

public:
	//레퍼런스 카운트 증가, 증가한 값 반환
	_uint AddRef();
	//레퍼런스 카운트 감소 밑 삭제, 감소 전 값 반환
	_uint Release();

protected:
	_uint referenceCount = 0;

public:
	virtual void Free();
};

_NS
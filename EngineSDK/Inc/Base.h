#pragma once

#include "Engine_Defines.h"

NS_BEGIN(Engine)

/* 스마트 포인터의 레퍼런스 카운트 기능을 대체, 직접 관리 */
class ENGINE_DLL Base abstract
{
protected:
	Base();
	virtual  ~Base() = default;

public:
	_uint AddRef();//레퍼런스 카운트 증가, 증가한 값 반환
	_uint Release();//레퍼런스 카운트 감소와 삭제, 감소 전 값 반환

	virtual void Free();

protected:
	_uint referenceCount = 0;
};

NS_END
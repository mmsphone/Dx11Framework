#ifndef Engine_Function_h__
#define Engine_Function_h__


namespace Engine
{
#ifdef _DEBUG
	inline void _DbgPrintA(const char* fmt, ...)
	{
		char buf[1024];
		va_list ap; va_start(ap, fmt);
		_vsnprintf_s(buf, _TRUNCATE, fmt, ap);
		va_end(ap);
		OutputDebugStringA(buf);
	}

	// __FUNCSIG__에서 "SafeAddRef<...>" 또는 "SafeRelease<...>" 사이의 타입만 추출하고 정리
	inline std::string _ShortenFuncSig(const char* fs)
	{
		std::string s(fs);
		// 어떤 함수인지
		const char* call = (s.find("SafeAddRef<") != std::string::npos) ? "SafeAddRef" : "SafeRelease";

		// 템플릿 인자 타입 추출
		auto lb = s.find('<');
		auto rb = s.find('>');
		std::string t = (lb != std::string::npos && rb != std::string::npos && rb > lb)
			? s.substr(lb + 1, rb - lb - 1)
			: "unknown";

		// 지저분한 토큰 정리
		auto cleanup = [&](const char* from) {
			for (size_t pos = 0; (pos = t.find(from, pos)) != std::string::npos; )
				t.erase(pos, std::strlen(from));
			};
		cleanup("class ");
		cleanup("struct ");
		cleanup("enum ");
		cleanup("const ");
		cleanup("volatile ");

		// 공백 정리: 포인터 기호 앞뒤 정도만 남기고 대부분 제거
		t.erase(std::remove_if(t.begin(), t.end(),
			[](unsigned char c) { return std::isspace(c); }), t.end());

		// 참고: MSVC는 템플릿 인자에 T*& 같은 표기가 들어올 수 있음 → 그대로 둬도 충분히 읽힘

		return std::string(call) + " " + t; // 예: "SafeAddRef Engine::EngineUtility*"
	}
#endif
	// 템플릿은 기능의 정해져있으나 자료형은 정해져있지 않은 것
	// 기능을 인스턴스화 하기 위하여 만들어두는 틀

	template<typename T>
	void	SafeDelete(T& Pointer)
	{
		if (nullptr != Pointer)
		{
			delete Pointer;
			Pointer = nullptr;
		}
	}

	template<typename T>
	void	SafeDeleteArray(T& Pointer)
	{
		if (nullptr != Pointer)
		{
			delete [] Pointer;
			Pointer = nullptr;
		}
	}

	template<typename T>
	unsigned int SafeAddRef(T& pInstance)
	{
		unsigned int		iRefCnt = 0;
		if (nullptr != pInstance)		
			iRefCnt = pInstance->AddRef();	
		return iRefCnt;
	}

	template<typename T>
	unsigned int SafeRelease(T& pInstance)
	{
		unsigned int		iRefCnt = 0;
		if (nullptr != pInstance)
		{
			iRefCnt = pInstance->Release();
			if (0 == iRefCnt)
				pInstance = nullptr;
		}
		return iRefCnt;
	}
}

#endif // Engine_Function_h__

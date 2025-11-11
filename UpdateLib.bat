xcopy	/y/s		.\Engine\public\*.*				.\EngineSDK\Inc\
xcopy	/y		.\Engine\bin\Debug\Engine.lib	.\EngineSDK\Lib\Debug\
xcopy	/y		.\Engine\bin\Release\Engine.lib	.\EngineSDK\Lib\Release\
xcopy	/y		.\Engine\ThirdPartyLib\*.lib		.\EngineSDK\Lib\

xcopy	/y		.\Engine\bin\Debug\Engine.dll	.\Client\bin\Debug\
xcopy	/y		.\Engine\bin\Release\Engine.dll	.\Client\bin\Release\
xcopy	/y		.\Engine\bin\Shader\*.*			.\Client\bin\Shader\

xcopy	/y		.\Engine\bin\Debug\Engine.dll	.\Tool\bin\Debug\
xcopy	/y		.\Engine\bin\Release\Engine.dll	.\Tool\bin\Release\
xcopy	/y		.\Engine\bin\Shader\*.*			.\Tool\bin\Shader\

xcopy	/y		.\Tool\bin\data\*.*				.\Client\bin\data\
xcopy	/y /s		.\Tool\bin\Resources\*.*		.\Client\bin\Resources\

exit		/b		0
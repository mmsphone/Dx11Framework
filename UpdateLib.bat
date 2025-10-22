xcopy	/y/s		.\Engine\public\*.*				.\EngineSDK\Inc\
xcopy	/y		.\Engine\bin\Engine.lib			.\EngineSDK\Lib\
xcopy	/y		.\Engine\ThirdPartyLib\*.lib		.\EngineSDK\Lib\

xcopy	/y		.\Engine\bin\Engine.dll			.\Client\bin\
xcopy	/y		.\Engine\bin\Shader\*.*			.\Client\bin\Shader\

xcopy	/y		.\Engine\bin\Engine.dll			.\Tool\bin\
xcopy	/y		.\Engine\bin\Shader\*.*			.\Tool\bin\Shader\

exit		/b		0
// 명령어				옵션			원본 파일의 위치				사본 파일을 저장할 위치

xcopy				/y/s		.\Engine\public\*.*			.\EngineSDK\Inc\

xcopy				/y			.\Engine\bin\Engine.dll		 .\Client\bin\
xcopy				/y			.\Engine\bin\Engine.lib		 .\EngineSDK\Lib\
xcopy				/y			.\Engine\ThirdPartyLib\*.lib .\EngineSDK\Lib\
xcopy				/y			.\Engine\bin\Shader\*.* .\Client\bin\Shader\
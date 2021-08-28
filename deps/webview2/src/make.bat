@ECHO OFF

@REM download https://go.microsoft.com/fwlink/p/?LinkId=2124703
@REM download https://www.nuget.org/api/v2/package/Microsoft.Web.WebView2

PUSHD webview2-1.0.902.49
IF NOT EXIST ..\..\include (XCOPY /E/Y/I/F build\native\include ..\..\include)
IF NOT EXIST ..\..\lib (XCOPY /E/Y/I/F build\native\x64\WebView2LoaderStatic.lib ..\..\lib\)

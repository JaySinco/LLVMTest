@ECHO OFF

PUSHD boost_1_76_0
IF NOT EXIST .\b2.exe (.\bootstrap.bat)

.\b2.exe --with-system --with-thread --with-filesystem --with-chrono --with-date_time --with-regex ^
    --with-serialization --with-program_options --with-test --with-timer --with-atomic --with-context ^
    --build-dir=%~dp0\out\ ^
    --prefix=%~dp0..\ ^
    toolset=msvc variant=release ^
    link=static runtime-link=shared threading=multi ^
    architecture=x86 address-model=64 ^
    install

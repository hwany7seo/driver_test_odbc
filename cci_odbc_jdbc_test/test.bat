@echo off
setlocal


rem set "values=100 1000 10000 50000"
set "values=50000"
set "testCount=1"

for %%c in (%values%) do (
    echo Running tests with -c %%c

    echo Running JDBC NO BATCH tests with -c %%c
    for /L %%i in (1,1,%testCount%) do (
        java -cp d:\JDBC-11.3.0.0049-cubrid.jar JDBCTestNoBatch.java
    )

    echo Running JDBC BATCH tests with -c %%c
    for /L %%i in (1,1,%testCount%) do (
        java -cp d:\JDBC-11.3.0.0049-cubrid.jar JDBCTestNoBind.java
    )

    echo Running ODBC tests with -c %%c
    for /L %%i in (1,1,%testCount%) do (
        call odbc_test.exe
    )

    echo Running ODBC PREPARE tests with -c %%c
    for /L %%i in (1,1,%testCount%) do (
        call odbc_test_prepare.exe
    )

    echo Running CCI tests with -c %%c
    for /L %%i in (1,1,%testCount%) do (
        call odbc_test.exe
    )

    echo Running CCI PREPARE tests with -c %%c
    for /L %%i in (1,1,%testCount%) do (
        call cci_test_prepare.exe
    )
)

echo All tests completed.
endlocal
pause

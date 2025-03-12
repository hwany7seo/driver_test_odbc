@echo off
setlocal


rem set "values=100 1000 10000 50000"
set "values=50000"

for %%c in (%values%) do (
    echo Running tests with -c %%c

    echo Running ORI PHP PREPARE tests with -c %%c
    for /L %%i in (1,1,10) do (
        call C:\php_pdo\php\php-7.4.2-nts-Win32-vc15-x64\php.exe .\ori_php_test.php >> results_ori_php_test_%%c.txt
    )

    echo Running ORI PDO tests with -c %%c
    for /L %%i in (1,1,10) do (
        call C:\php_pdo\pdo\php-7.4.2-nts-Win32-vc15-x64\php.exe .\ori_pdo_test.php >> results_ori_pdo_test_%%c.txt
    )

    echo Running ORI PDO PREPARE tests with -c %%c
    for /L %%i in (1,1,10) do (
        call C:\php_pdo\pdo\php-7.4.2-nts-Win32-vc15-x64\php.exe .\ori_pdo_test_prepare.php >> results_ori_pdo_test_prepare_%%c.txt
    )

    echo Running PHP LIB PREPARE tests with -c %%c
    for /L %%i in (1,1,10) do (
        call C:\php_pdo\pdo\php-7.4.2-nts-Win32-vc15-x64\php.exe .\php_pdo_odbc_prepare.php >> results_php_pdo_odbc_prepare_%%c.txt
    )

    echo Running PHP LIB tests with -c %%c
    for /L %%i in (1,1,10) do (
        call C:\php_pdo\pdo\php-7.4.2-nts-Win32-vc15-x64\php.exe .\php_pdo_odbc_test.php >> results_php_pdo_odbc_test_%%c.txt
    )

    echo Running PHP ODBC LIB PREPARE tests with -c %%c
    for /L %%i in (1,1,10) do (
        call C:\php_pdo\pdo\php-7.4.2-Win32-vc15-x64\php.exe .\php_odbc_test_prepare.php >> results_php_odbc_test_prepare_%%c.txt
    )

    echo Running PHP ODBC LIB tests with -c %%c
    for /L %%i in (1,1,10) do (
        call C:\php_pdo\pdo\php-7.4.2-Win32-vc15-x64\php.exe .\php_odbc_test.php >> results_php_odbc_test_%%c.txt
    )
)

echo All tests completed.
endlocal
pause

@ECHO OFF
setlocal enabledelayedexpansion

SET /A all_count=0
SET /A ok_count=0

FOR /D %%I IN ("expected/*") DO (
    FOR %%J IN ("expected/"%%I"/*") DO (
        SET /A all_count+=1
        CALL "../Debug/Pascal.exe" expected/%%I/%%J > output.txt
        FC output.txt expected/%%I/output/%%J >nul 2>&1
        IF errorlevel 1 (
            ECHO Error in test %%I/%%J
        ) ELSE (
            SET /A ok_count+=1
            ECHO .
        )
    )
)
::DEL output.txt
ECHO %all_count% test(s) ran.
ECHO %ok_count% test(s) passed.
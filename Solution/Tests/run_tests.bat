@ECHO OFF
setlocal enabledelayedexpansion

SET /A all_count=0
SET /A ok_count=0
SET file=0

FOR /D %%I IN ("expected/sem*") DO (
    FOR %%J IN ("expected/"%%I"/*.pas") DO (
        SET /A all_count+=1
        CALL "../Debug/Pascal.exe" expected/%%I/%%J > output.txt 2>&1

        FC output.txt expected/%%I/%%~nJ.out >nul 2>&1
        IF errorlevel 1 (
            ECHO Error in test %%I/%%J
        ) ELSE (
            SET /A ok_count+=1
            ECHO Passed %%I/%%J
        )
    )
    ECHO[
)

ECHO %all_count% test(s) ran.
ECHO %ok_count% test(s) passed.
@echo off
title Qt Project Builder - By Nhu Thai
chcp 65001 >nul
color 0A

:: ============================================
:: 🧩 THIET LAP MOI TRUONG QT & MINGW
:: ============================================
echo ============================================
echo   DANG THIET LAP MOI TRUONG CHO QT 6.9.2
echo ============================================

set PATH=C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.9.2\mingw_64\bin;%PATH%

where mingw32-make >nul 2>nul
if errorlevel 1 (
    echo ❌ Loi: Khong tim thay mingw32-make.exe
    pause
    exit /b
)

where qmake >nul 2>nul
if errorlevel 1 (
    echo ❌ Loi: Khong tim thay qmake.exe
    pause
    exit /b
)

set PROJECT_PATH=D:\university\HKI_25_26\trr\project
cd /d "%PROJECT_PATH%"

:: ============================================
:: 🧭 MENU LUA CHON
:: ============================================
:MENU
cls
echo ============================================
echo         🧰 QT PROJECT BUILD MENU
echo ============================================
echo  [1] Clean project
echo  [2] Build project
echo  [3] Clean + Build project
echo  [4] Mo thu muc Release
echo  [5] Chay chuong trinh (neu co)
echo  [6] Thoat
echo  [7] Xoa hoan toan build cu (Release + Debug)
echo --------------------------------------------
set /p choice=Nhap lua chon cua ban (1-7): 

if "%choice%"=="1" goto CLEAN
if "%choice%"=="2" goto BUILD
if "%choice%"=="3" goto CLEANBUILD
if "%choice%"=="4" goto OPENRELEASE
if "%choice%"=="5" goto RUNPROGRAM
if "%choice%"=="6" goto EXIT
if "%choice%"=="7" goto FULLCLEAN
goto MENU

:: ============================================
:: 🧹 CLEAN PROJECT
:: ============================================
:CLEAN
cls
echo 🧹 Dang don dep cac file cu...
if exist Makefile (
    mingw32-make clean
    echo ✅ Da xoa cac file build cu.
) else (
    echo ⚠️  Chua co Makefile de xoa.
)
timeout /t 2 /nobreak >nul
goto MENU

:: ============================================
:: ⚒️ BUILD PROJECT
:: ============================================
:BUILD
cls
echo 🏗️ Dang tao Makefile moi...
qmake
echo ⚒️ Dang tien hanh build...
mingw32-make

if errorlevel 1 (
    echo ❌ BUILD THAT BAI!
    pause
    goto MENU
)
echo ✅ BUILD HOAN TAT THANH CONG!
timeout /t 2 /nobreak >nul
goto MENU

:: ============================================
:: 🧩 CLEAN + BUILD
:: ============================================
:CLEANBUILD
cls
echo 🧹 Don dep truoc khi build...
if exist Makefile mingw32-make clean
echo 🏗️ Tao Makefile va build lai...
qmake
mingw32-make
if errorlevel 1 (
    echo ❌ BUILD THAT BAI!
    pause
    goto MENU
)
echo ✅ BUILD HOAN TAT THANH CONG!
timeout /t 2 /nobreak >nul
goto MENU

:: ============================================
:: 📁 MO THU MUC RELEASE
:: ============================================
:OPENRELEASE
cls
if exist release (
    echo 📂 Dang mo thu muc Release...
    start release
) else (
    echo ⚠️ Khong tim thay thu muc release.
)
timeout /t 2 /nobreak >nul
goto MENU

:: ============================================
:: ▶️ CHAY CHUONG TRINH TRONG RELEASE
:: ============================================
:RUNPROGRAM
cls
if exist release (
    for %%f in (release\*.exe) do (
        echo ▶️ Dang chay %%f ...
        start "" "%%f"
        timeout /t 2 /nobreak >nul
        goto MENU
    )
    echo ⚠️ Khong tim thay file .exe trong release.
) else (
    echo ⚠️ Thu muc release khong ton tai.
)
timeout /t 2 /nobreak >nul
goto MENU

:: ============================================
:: 🧨 XOA HOAN TOAN BUILD CU
:: ============================================
:FULLCLEAN
cls
echo ⚠️ Ban co chac muon xoa tat ca build cu (Release + Debug)? (Y/N)
set /p confirm=Nhap lua chon: 
if /I "%confirm%" NEQ "Y" goto MENU

if exist Makefile (
    echo 🧹 Chay mingw32-make clean...
    mingw32-make clean
)

if exist release (
    echo 🗑️ Dang xoa thu muc release...
    rmdir /s /q release
)

if exist debug (
    echo 🗑️ Dang xoa thu muc debug...
    rmdir /s /q debug
)

echo ✅ Da xoa tat ca build cu thanh cong.
timeout /t 2 /nobreak >nul
goto MENU

:: ============================================
:: 🚪 THOAT
:: ============================================
:EXIT
cls
echo Tam biet! 👋
timeout /t 2 /nobreak >nul
exit /b

#include <iostream>
#include <windows.h>

void color(WORD c) 
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

void clearConsole()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) return;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    DWORD cellCount = csbi.dwSize.X * csbi.dwSize.Y;
    DWORD written;
    FillConsoleOutputCharacter(hConsole, ' ', cellCount, { 0, 0 }, &written);
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    FillConsoleOutputAttribute(hConsole, csbi.wAttributes, cellCount, { 0, 0 }, &written);
    SetConsoleCursorPosition(hConsole, { 0, 0 });
}

void titleMenu()
{
    color(5);
    std::cout << "BN Calculator test build v0.0.0.1b win x64 (c)2024.";
    color(10);
    std::cout << " Backend of MPFR <";
    color(4);
    std::cout << "999";
    color(10);
    std::cout << "> bits.\nAt any time enter 'set' to adjust output, 'cls' to clear screen, 'list' to list functions and 'exit' to exit.\n";
}

void funcList()
{
    std::cout << "\nBasic operators: + - * / % ^ ( )\nFuncions: sin() cos() tan() sinh() cosh() tanh() ln() log() exp() sqrt()\n"
                << "Customized functions: sind() cosd() tand() fact() rms() peak()\nConstants: pi e phi\n";
}
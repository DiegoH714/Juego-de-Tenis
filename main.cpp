#include "cocos2d.h"
#include "AppDelegate.h"
#include <Windows.h>

int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nShowCmd
)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    EpicGame::AppDelegate app;

    return cocos2d::Application::getInstance()->run();
}

int main(int argc, char* argv[])
{
  
    EpicGame::AppDelegate app;
    return cocos2d::Application::getInstance()->run();
}

#include "AppDelegate.h"
#include "MenuScene.h"

USING_NS_CC;

namespace EpicGame {

    static cocos2d::Size designResolutionSize = cocos2d::Size(1280, 720);
    static cocos2d::Size smallResolutionSize = cocos2d::Size(1280, 720);
    static cocos2d::Size mediumResolutionSize = cocos2d::Size(1920, 1080);
    static cocos2d::Size largeResolutionSize = cocos2d::Size(2560, 1440);

    AppDelegate::AppDelegate()
    {
    }

    AppDelegate::~AppDelegate()
    {
    }

    void AppDelegate::initGLContextAttrs()
    {
        GLContextAttrs glContextAttrs = { 8, 8, 8, 8, 24, 8, 0 };
        GLView::setGLContextAttrs(glContextAttrs);
    }

    bool AppDelegate::applicationDidFinishLaunching()
    {
        auto director = Director::getInstance();
        auto glview = director->getOpenGLView();

        if (!glview) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
            glview = GLViewImpl::createWithRect("Tennis Game", cocos2d::Rect(0, 0, designResolutionSize.width, designResolutionSize.height));
#else
            glview = GLViewImpl::create("Tennis Game");
#endif
            director->setOpenGLView(glview);
        }

        director->setDisplayStats(true);
        director->setAnimationInterval(1.0f / 60);

        glview->setDesignResolutionSize(
            designResolutionSize.width,
            designResolutionSize.height,
            ResolutionPolicy::NO_BORDER
        );

        auto frameSize = glview->getFrameSize();
        if (frameSize.height < mediumResolutionSize.height) {
            director->setContentScaleFactor(MIN(smallResolutionSize.height / designResolutionSize.height,
                smallResolutionSize.width / designResolutionSize.width));
        }
        else if (frameSize.height < largeResolutionSize.height) {
            director->setContentScaleFactor(MIN(mediumResolutionSize.height / designResolutionSize.height,
                mediumResolutionSize.width / designResolutionSize.width));
        }
        else {
            director->setContentScaleFactor(MIN(largeResolutionSize.height / designResolutionSize.height,
                largeResolutionSize.width / designResolutionSize.width));
        }

        FileUtils::getInstance()->addSearchPath("Resources");

        auto scene = MenuScene::createScene();
        director->runWithScene(scene);

        return true;
    }

    void AppDelegate::applicationDidEnterBackground()
    {
        Director::getInstance()->stopAnimation();
    }

    void AppDelegate::applicationWillEnterForeground()
    {
        Director::getInstance()->startAnimation();
    }

} 
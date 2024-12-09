#ifndef __APP_DELEGATE_H__
#define __APP_DELEGATE_H__

#include "cocos2d.h"

namespace EpicGame {

    class AppDelegate : public cocos2d::Application
    {
    public:
        AppDelegate();
        virtual ~AppDelegate();

        virtual void initGLContextAttrs();

        virtual bool applicationDidFinishLaunching();
        virtual void applicationDidEnterBackground();
        virtual void applicationWillEnterForeground();
    };

} 

#endif 
#ifndef __MENU_SCENE_H__
#define __MENU_SCENE_H__

#include "cocos2d.h"

namespace EpicGame {

    class MenuScene : public cocos2d::Scene {
    public:
        static cocos2d::Scene* createScene();
        virtual bool init();

        CREATE_FUNC(MenuScene);

    private:
        void menuPlayCallback(Ref* pSender);
        void menuExitCallback(Ref* pSender);
    };

} 

#endif 
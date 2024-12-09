#include "MenuScene.h"
#include "TennisScene.h"

USING_NS_CC;

namespace EpicGame {

    Scene* MenuScene::createScene() {
        auto scene = MenuScene::create();
        if (scene == nullptr) {
            CCLOG("Error: No se pudo crear MenuScene");
            return nullptr;
        }
        return scene;
    }

    bool MenuScene::init() {
        if (!Scene::init()) {
            CCLOG("Error: Fallo en Scene::init()");
            return false;
        }

        auto visibleSize = Director::getInstance()->getVisibleSize();
        Vec2 origin = Director::getInstance()->getVisibleOrigin();

        auto background = Sprite::create("Menu.png");
        if (background == nullptr) {
            CCLOG("Error: No se pudo cargar Menu.png");
            return false;
        }

        background->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
        float scaleX = visibleSize.width / background->getContentSize().width;
        float scaleY = visibleSize.height / background->getContentSize().height;
        float scale = std::max(scaleX, scaleY);
        background->setScale(scale);
        this->addChild(background, 0);

        auto label = Label::createWithSystemFont("Tennis Game", "Arial", 60);
        if (label == nullptr) {
            CCLOG("Error: No se pudo crear el título");
            return false;
        }
        label->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height - 100));
        this->addChild(label, 1);

        // Crear botones del menú
        auto playItem = MenuItemLabel::create(
            Label::createWithSystemFont("Start Game", "Arial", 45),
            CC_CALLBACK_1(MenuScene::menuPlayCallback, this));
        if (playItem == nullptr) {
            CCLOG("Error: No se pudo crear el botón Start Game");
            return false;
        }
        playItem->setPosition(Vec2(0, 50));

        auto exitItem = MenuItemLabel::create(
            Label::createWithSystemFont("Exit", "Arial", 45),
            CC_CALLBACK_1(MenuScene::menuExitCallback, this));
        if (exitItem == nullptr) {
            CCLOG("Error: No se pudo crear el botón Exit");
            return false;
        }
        exitItem->setPosition(Vec2(0, -150));

        auto menu = Menu::create(playItem, exitItem, nullptr);
        if (menu == nullptr) {
            CCLOG("Error: No se pudo crear el menú");
            return false;
        }
        menu->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2));
        this->addChild(menu, 1);

        return true;
    }

    void MenuScene::menuPlayCallback(Ref* pSender) {
        auto scene = TennisScene::createScene();
        if (scene == nullptr) {
            CCLOG("Error: No se pudo crear la escena del juego");
            return;
        }
        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
    }

    void MenuScene::menuExitCallback(Ref* pSender) {
        Director::getInstance()->end();
    }

} 
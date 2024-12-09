#ifndef __TENNIS_SCENE_H__
#define __TENNIS_SCENE_H__

#include "cocos2d.h"
#include <vector>
#include <random>

namespace EpicGame {

    class TennisScene : public cocos2d::Scene {
    public:
        static cocos2d::Scene* createScene();
        virtual bool init();
        CREATE_FUNC(TennisScene);

    private:
        const float SERVE_HEIGHT = 100.0f;
        const float SERVE_START_HEIGHT = 30.0f;
        const float SERVE_DURATION = 0.8f;
        const float SERVE_SPEED = 700.0f;
        const float HIT_BASE_SPEED = 500.0f;
        const float HIT_SPEED = 750.0f;
        const float SMASH_SPEED = 750.0f;
        const float LOB_SPEED = 450.0f;
        const float GRAVITY = -900.0f;
        const float BALL_BOUNCE_DAMPING = 0.7f;
        const float AIR_RESISTANCE = 0.997f;
        const float PLAYER_SPEED = 400.0f;
        const float AI_SPEED = 500.0f;
        const float BALL_SPIN = 0.2f;
        const float FRONT_PLAYER_SCALE = 0.4f;
        const float BACK_PLAYER_SCALE = 0.18f;
        const float BALL_BASE_SCALE = 0.05f;
        const float BALL_MIN_SCALE = 0.04f;
        const float SERVE_ANGLE = 75.0f;
        const float HIT_ANGLE = 45.0f;
        const float HIT_DISTANCE = 50.0f;
        const float HIT_UPWARD_ANGLE = 60.0f;
        bool hasBounced = false;
        bool hasBouncedInOpponentCourt = false;
        const float NET_Y = 0.5f;
        const float COURT_TOP = 0.85f;

        struct CourtDimensions {
            float width;
            float height;
            float serviceLineY;
            float serviceBoxWidth;
            float netHeight;
            float baselineOffset;
            float sideLineOffset;
        };

        enum class ServeState {
            READY,
            TOSS,
            READY_TO_HIT,
            FALLING
        };

        enum class GameState {
            SERVE,
            PLAY,
            POINT_END,
            GAME_END
        };

        enum class ShotType {
            NORMAL,
            TOPSPIN,
            SLICE,
            LOB,
            SMASH
        };

        cocos2d::Sprite* court = nullptr;
        cocos2d::Sprite* player1 = nullptr;
        cocos2d::Sprite* player2 = nullptr;
        cocos2d::Sprite* ball = nullptr;
        cocos2d::Sprite* ballShadow = nullptr;
        cocos2d::Label* scoreLabel = nullptr;
        cocos2d::Label* gameScoreLabel = nullptr;
        cocos2d::Label* serviceIndicator = nullptr;

        CourtDimensions courtDims;
        GameState gameState = GameState::SERVE;
        ServeState serveState = ServeState::READY;
        ShotType currentShot = ShotType::NORMAL;

        bool isDeuce = false;
        bool isMatchPoint = false;
        int currentSet = 1;
        bool isDeuceSide = true;
        int pointsToWinGame = 4;
        int serverPosition = 1;
        int player1Points = 0;
        int player2Points = 0;
        int player1Games = 0;
        int player2Games = 0;
        int player1Sets = 0;
        int player2Sets = 0;

        bool isServing = true;
        bool ballInPlay = false;
        int servingPlayer = 1;
        float serveTimer = 0.0f;
        bool isPowerCharging = false;
        float powerCharge = 0.0f;
        float shotAngle = 0.0f;
        int faultCount = 0;
        bool lastPointWinner = true;
        int playerScore = 0;
        int aiScore = 0;
        float aiServeTimer = 0.0f;

        bool leftPressed = false;
        bool rightPressed = false;
        bool upPressed = false;
        bool downPressed = false;
        bool spacePressed = false;
        bool canHit = false;
        bool canServe = false;

        cocos2d::Vec2 ballVelocity;
        cocos2d::Vec2 lastHitDirection;
        float ballSpin = 0.0f;
        cocos2d::Vec2 player1LeftPos;
        cocos2d::Vec2 player1RightPos;
        cocos2d::Vec2 player2LeftPos;
        cocos2d::Vec2 player2RightPos;

        const std::vector<std::string> TENNIS_POINTS{ "0", "15", "30", "40", "Ad" };

        void initCourt();
        void initPlayers();
        void initBall();
        void initUI();
        void initShadows();

        void update(float delta) override;
        void updatePlayerPosition(float delta);
        void updateBallPhysics(float delta);
        void updateServe(float delta);
        void updateAI(float delta);
        void updatePowerCharge(float delta);
        void updateBallShadow();
        void updateScoreDisplay();
        void updatePlayerPositions();

        void hitBall();
        void hitServe();
        void executeShot(ShotType type);
        void returnBallAI();
        void checkBallCollisions();
        void checkCourtBoundaries();
        bool isInServiceBox();

        void handlePointEnd(bool player1Won);
        void startNewPoint();
        void switchServer();
        void resetBall();
        void resetServe();
        void switchSides();
        void positionPlayersForServe();
        void setupNextServe();

        float getPerspectiveScale(float yPos);
        cocos2d::Vec2 getAdjustedVelocity(const cocos2d::Vec2& velocity, float yPos);
        float calculateShotPower();
        cocos2d::Vec2 calculateShotDirection();
        void updatePowerDisplay();
        bool isPositionOutOfCourt(const cocos2d::Vec2& position);
        bool isInServiceBox(const cocos2d::Vec2& position);

        void onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);
        void onKeyReleased(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);
    };

}

#endif
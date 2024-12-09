#include "TennisScene.h"

USING_NS_CC;

namespace EpicGame {

    Scene* TennisScene::createScene() {
        return TennisScene::create();
    }

    bool TennisScene::init() {
        if (!Scene::init()) {
            return false;
        }

        auto visibleSize = Director::getInstance()->getVisibleSize();

        courtDims.width = visibleSize.width * 0.8f;
        courtDims.height = visibleSize.height * 0.75f;
        courtDims.serviceLineY = visibleSize.height * 0.4f;
        courtDims.serviceBoxWidth = courtDims.width * 0.3f;
        courtDims.netHeight = visibleSize.height * 0.02f;
        courtDims.baselineOffset = visibleSize.height * 0.1f;
        courtDims.sideLineOffset = visibleSize.width * 0.1f;

        float courtCenter = visibleSize.width / 2;
        float frontY = visibleSize.height * 0.15f;
        float backY = visibleSize.height * 0.85f;
        float sideOffset = courtDims.width * 0.25f;

        player1LeftPos = cocos2d::Vec2(courtCenter - sideOffset, frontY);
        player1RightPos = cocos2d::Vec2(courtCenter + sideOffset, frontY);
        player2LeftPos = cocos2d::Vec2(courtCenter - sideOffset, backY);
        player2RightPos = cocos2d::Vec2(courtCenter + sideOffset, backY);

        initCourt();
        initPlayers();
        initBall();
        initShadows();
        initUI();

        positionPlayersForServe();

        auto keyListener = EventListenerKeyboard::create();
        keyListener->onKeyPressed = CC_CALLBACK_2(TennisScene::onKeyPressed, this);
        keyListener->onKeyReleased = CC_CALLBACK_2(TennisScene::onKeyReleased, this);
        _eventDispatcher->addEventListenerWithSceneGraphPriority(keyListener, this);

        scheduleUpdate();
        return true;
    }
    void TennisScene::initCourt() {
        auto visibleSize = Director::getInstance()->getVisibleSize();

        court = Sprite::create("court.png");
        if (court) {
            court->setPosition(visibleSize.width / 2, visibleSize.height / 2);
            court->setScale(visibleSize.width / court->getContentSize().width,
                visibleSize.height / court->getContentSize().height);
            this->addChild(court, 0);
        }
    }

    void TennisScene::initPlayers() {
        player1 = Sprite::create("player1.png");
        player2 = Sprite::create("player2.png");

        if (player1 && player2) {
            player1->setScale(FRONT_PLAYER_SCALE);
            player2->setScale(BACK_PLAYER_SCALE);

            this->addChild(player1, 2);
            this->addChild(player2, 2);
        }
    }

    void TennisScene::initBall() {
        ball = Sprite::create("ball.png");
        if (ball) {
            ball->setScale(BALL_BASE_SCALE);
            resetBall();
            this->addChild(ball, 2);
        }
    }

    void TennisScene::initShadows() {
        ballShadow = Sprite::create("ball_shadow.png");
        if (ballShadow) {
            ballShadow->setScale(BALL_BASE_SCALE * 0.7f);
            ballShadow->setOpacity(150);
            ballShadow->setColor(Color3B(0, 0, 0));
            this->addChild(ballShadow, 1);
        }
    }

    void TennisScene::initUI() {
        auto visibleSize = Director::getInstance()->getVisibleSize();

        scoreLabel = Label::createWithSystemFont("0 - 0", "Arial", 32);
        if (scoreLabel) {
            scoreLabel->setPosition(Vec2(visibleSize.width - 100, visibleSize.height - 30));
            scoreLabel->setAlignment(TextHAlignment::RIGHT);
            this->addChild(scoreLabel, 3);
        }

        gameScoreLabel = Label::createWithSystemFont("0-0 (0-0)", "Arial", 24);
        if (gameScoreLabel) {
            gameScoreLabel->setPosition(Vec2(visibleSize.width - 100, visibleSize.height - 60));
            gameScoreLabel->setAlignment(TextHAlignment::RIGHT);
            this->addChild(gameScoreLabel, 3);
        }

        serviceIndicator = nullptr;

        updateScoreDisplay();
    }
    void EpicGame::TennisScene::update(float delta) {
        switch (gameState) {
        case GameState::SERVE:
            if (servingPlayer == 2) {
                aiServeTimer += delta;

                switch (serveState) {
                case ServeState::READY:
                    if (aiServeTimer > 1.0f) {
                        serveState = ServeState::TOSS;
                        aiServeTimer = 0;
                        ballVelocity = cocos2d::Vec2::ZERO;
                    }
                    break;

                case ServeState::TOSS:
                {
                    float progress = aiServeTimer / SERVE_DURATION;
                    if (progress <= 1.0f) {
                        float height = SERVE_HEIGHT * (1 - (2 * progress - 1) * (2 * progress - 1));
                        float xOffset = 10.0f * sin(progress * M_PI);
                        cocos2d::Vec2 ballPos(player2->getPositionX() + xOffset,
                            player2->getPositionY() - SERVE_START_HEIGHT - height);

                        ball->setPosition(ballPos);
                        updateBallShadow();

                        if (progress > 0.5f && progress < 0.8f) {
                            serveState = ServeState::READY_TO_HIT;
                            canServe = true;
                            hitServe();
                        }
                    }
                    else {
                        serveState = ServeState::FALLING;
                        ballVelocity = cocos2d::Vec2::ZERO;
                        canServe = false;
                    }
                }
                break;

                case ServeState::FALLING:
                    ballVelocity.y -= GRAVITY * delta;
                    cocos2d::Vec2 pos = ball->getPosition();
                    pos += ballVelocity * delta;
                    ball->setPosition(pos);

                    if (pos.y >= player2->getPositionY() - SERVE_START_HEIGHT) {
                        faultCount++;
                        if (faultCount >= 2) {
                            handlePointEnd(true);
                        }
                        else {
                            resetServe();
                        }
                    }
                    break;
                }
            }
            else {
                updateServe(delta);
            }
            break;

        case GameState::PLAY:
            updatePlayerPosition(delta);
            updateBallPhysics(delta);
            updateAI(delta);
            updateBallShadow();
            checkCourtBoundaries();
            break;

        case GameState::POINT_END:
            startNewPoint();
            gameState = GameState::SERVE;
            break;
        }
    }

    void TennisScene::updatePlayerPosition(float delta) {
        if (!isServing || serveState != ServeState::READY) {
            Vec2 pos = player1->getPosition();
            float moveSpeed = PLAYER_SPEED * delta;

            if (leftPressed) pos.x -= moveSpeed;
            if (rightPressed) pos.x += moveSpeed;

            auto visibleSize = Director::getInstance()->getVisibleSize();
            float minX = (visibleSize.width - courtDims.width) / 2 +
                player1->getContentSize().width * player1->getScale() / 2;
            float maxX = (visibleSize.width + courtDims.width) / 2 -
                player1->getContentSize().width * player1->getScale() / 2;

            pos.x = std::min(std::max(pos.x, minX), maxX);
            player1->setPosition(pos);
        }
    }
    void TennisScene::updateBallPhysics(float delta) {
        if (!ballInPlay) return;

        auto visibleSize = Director::getInstance()->getVisibleSize();
        ballVelocity.y -= GRAVITY * delta * 0.15f;
        ballVelocity.x *= 0.997f;

        cocos2d::Vec2 newPos = ball->getPosition() + ballVelocity * delta;

        float hitDistance = 150.0f;
        float verticalHitDistance = 180.0f;

        bool nearPlayer1 = (std::abs(newPos.x - player1->getPositionX()) < hitDistance &&
            std::abs(newPos.y - player1->getPositionY()) < verticalHitDistance);

        canHit = nearPlayer1;

        float netY = visibleSize.height * 0.5f;
        float netThickness = visibleSize.height * 0.02f;

        if (ball->getPositionY() >= netY - netThickness &&
            ball->getPositionY() <= netY + netThickness &&
            std::abs(ball->getPositionX() - visibleSize.width * 0.5f) < 10.0f) {

            bool lastHitByPlayer1 = ballVelocity.y > 0;
            handlePointEnd(lastHitByPlayer1);
            return;
        }

        if (newPos.y < visibleSize.height * 0.05f || newPos.y > visibleSize.height * 0.95f) {
            handlePointEnd(newPos.y < visibleSize.height * 0.5f);
            return;
        }

        if (newPos.x < visibleSize.width * 0.1f || newPos.x > visibleSize.width * 0.9f) {
            handlePointEnd(newPos.y > visibleSize.height * 0.5f);
            return;
        }

        float minSpeed = 100.0f;
        float currentSpeed = sqrt(ballVelocity.x * ballVelocity.x + ballVelocity.y * ballVelocity.y);
        if (currentSpeed < minSpeed && ballInPlay) {
            handlePointEnd(ball->getPositionY() < visibleSize.height * 0.5f);
            return;
        }

        ball->setPosition(newPos);
        updateBallShadow();
    }

    void EpicGame::TennisScene::updateServe(float delta) {
        switch (serveState) {
        case ServeState::READY: {
            ball->setPosition(player1->getPositionX(),
                player1->getPositionY() + SERVE_START_HEIGHT);
            ballShadow->setVisible(false);
            break;
        }
        case ServeState::TOSS: {
            serveTimer += delta;
            float progress = serveTimer / SERVE_DURATION;

            if (progress <= 1.0f) {
                float height = SERVE_HEIGHT * (1 - (2 * progress - 1) * (2 * progress - 1));
                float xOffset = 10.0f * sin(progress * M_PI);
                cocos2d::Vec2 ballPos(player1->getPositionX() + xOffset,
                    player1->getPositionY() + SERVE_START_HEIGHT + height);

                ball->setPosition(ballPos);
                updateBallShadow();

                if (progress > 0.5f && progress < 0.8f) {
                    serveState = ServeState::READY_TO_HIT;
                    canServe = true;
                }
            }
            else {
                serveState = ServeState::FALLING;
                ballVelocity = cocos2d::Vec2::ZERO;
                canServe = false;
            }
            break;
        }
        case ServeState::FALLING: {
            ballVelocity.y += GRAVITY * delta;
            cocos2d::Vec2 pos = ball->getPosition() + ballVelocity * delta;
            ball->setPosition(pos);
            updateBallShadow();

            if (pos.y <= player1->getPositionY() + SERVE_START_HEIGHT) {
                faultCount++;
                if (faultCount >= 2) {
                    handlePointEnd(false);
                }
                else {
                    resetServe();
                }
            }
            break;
        }
        default:
            break;
        }
    }
    void TennisScene::updateAI(float delta) {
        if (!ballInPlay) return;

        auto visibleSize = Director::getInstance()->getVisibleSize();
        cocos2d::Vec2 ballPos = ball->getPosition();
        cocos2d::Vec2 aiPos = player2->getPosition();

        if (gameState == GameState::SERVE && servingPlayer == 2) {
            aiServeTimer += delta;

            switch (serveState) {
            case ServeState::READY:
                ball->setPosition(aiPos.x, aiPos.y + SERVE_START_HEIGHT);
                ballInPlay = true;
                if (aiServeTimer > 1.0f) {
                    serveState = ServeState::TOSS;
                    aiServeTimer = 0;
                }
                break;

            case ServeState::TOSS:
                float progress = aiServeTimer / SERVE_DURATION;
                if (progress <= 1.0f) {
                    float height = SERVE_HEIGHT * (1 - (2 * progress - 1) * (2 * progress - 1));
                    cocos2d::Vec2 ballPos(aiPos.x, aiPos.y + SERVE_START_HEIGHT + height);
                    ball->setPosition(ballPos);
                    updateBallShadow();

                    if (progress > 0.5f && progress < 0.8f && gameState == GameState::SERVE) {
                        float targetX = visibleSize.width * 0.75f;
                        cocos2d::Vec2 direction(-2.0f, -2.0f);
                        direction.normalize();

                        ballVelocity = direction * SERVE_SPEED;
                        gameState = GameState::PLAY;
                    }
                }
                break;
            }
            return;
        }

        float baselineY = visibleSize.height * 0.85f;
        float minHitY = visibleSize.height * 0.7f;
        float hitRangeX = 60.0f;

        if (ballPos.y > visibleSize.height * 0.5f) {
            float targetX = ballPos.x;
            float currentX = aiPos.x;

            if (std::abs(targetX - currentX) > 10.0f) {
                float direction = (targetX > currentX) ? 1.0f : -1.0f;
                cocos2d::Vec2 newPos = aiPos;
                float aiSpeed = AI_SPEED * 0.85f;
                newPos.x += direction * aiSpeed * delta;

                float minX = visibleSize.width * 0.1f;
                float maxX = visibleSize.width * 0.9f;
                newPos.x = std::min(std::max(newPos.x, minX), maxX);
                newPos.y = baselineY;

                if (rand() % 100 < 25) {
                    player2->setPosition(newPos);
                }
                else {
                    player2->setPosition(newPos);
                }
            }

            if (ballPos.y >= minHitY && std::abs(ballPos.x - aiPos.x) < hitRangeX) {
                if (rand() % 100 < 75) {
                    cocos2d::Vec2 direction;
                    direction.y = -2.0f;

                    float randomOffset = (rand() % 300 - 150) / 100.0f;
                    float targetX = player1->getPositionX() + randomOffset * visibleSize.width * 0.15f;
                    direction.x = (targetX - ballPos.x) / (visibleSize.width * 0.5f);

                    direction.normalize();
                    float hitSpeed = 550.0f;
                    ballVelocity = direction * hitSpeed;

                    hasBounced = false;
                    hasBouncedInOpponentCourt = false;
                }
            }
        }
    }
    void TennisScene::executeShot(ShotType type) {
        if (!canHit) return;

        auto visibleSize = Director::getInstance()->getVisibleSize();
        cocos2d::Vec2 ballPos = ball->getPosition();
        cocos2d::Vec2 playerPos = player1->getPosition();

        if (std::abs(ballPos.x - playerPos.x) < 50.0f) {
            cocos2d::Vec2 direction;

            if (ballPos.y < visibleSize.height * 0.5f) {
                direction.y = 1.0f;
            }
            else {
                direction.y = -1.0f;
            }

            if (leftPressed) direction.x -= 0.5f;
            if (rightPressed) direction.x += 0.5f;

            direction.normalize();

            float speed = 600.0f;


            ballVelocity = direction * speed;
            hasBounced = false;
            canHit = false;
        }
    }

    void TennisScene::updateScoreDisplay() {
        std::string p1Score, p2Score;

        if (isDeuce) {
            p1Score = p2Score = "40";
        }
        else if (player1Points >= 3 && player2Points >= 3) {
            if (player1Points > player2Points) {
                p1Score = "Ad";
                p2Score = "40";
            }
            else if (player2Points > player1Points) {
                p1Score = "40";
                p2Score = "Ad";
            }
            else {
                p1Score = p2Score = "40";
            }
        }
        else {
            p1Score = TENNIS_POINTS[std::min(player1Points, 4)];
            p2Score = TENNIS_POINTS[std::min(player2Points, 4)];
        }

        std::string score = p1Score + " - " + p2Score;
        std::string gameScore = std::to_string(player1Games) + "-" + std::to_string(player2Games) +
            " (" + std::to_string(player1Sets) + "-" + std::to_string(player2Sets) + ")";

        scoreLabel->setString(score);
        gameScoreLabel->setString(gameScore);
    }

    void TennisScene::handlePointEnd(bool player1Won) {
        if (gameState == GameState::POINT_END) {
            return;
        }

        ballInPlay = false;
        canHit = false;
        canServe = false;
        serveState = ServeState::READY;

        if (player1Won) {
            player1Points++;
        }
        else {
            player2Points++;
        }

        bool gameWon = false;

        if (player1Points >= 3 && player2Points >= 3) {
            if (player1Points == player2Points) {
                isDeuce = true;
            }
            else if (player1Points > player2Points) {
                if (player1Points - player2Points >= 2) {
                    player1Games++;
                    gameWon = true;
                }
            }
            else {
                if (player2Points - player1Points >= 2) {
                    player2Games++;
                    gameWon = true;
                }
            }
        }
        else {
            if (player1Points >= 4 && player1Points - player2Points >= 2) {
                player1Games++;
                gameWon = true;
            }
            else if (player2Points >= 4 && player2Points - player1Points >= 2) {
                player2Games++;
                gameWon = true;
            }
        }

        if (gameWon) {
            if (player1Games >= 6 && player1Games - player2Games >= 2) {
                player1Sets++;
                player1Games = player2Games = 0;
            }
            else if (player2Games >= 6 && player2Games - player2Games >= 2) {
                player2Sets++;
                player1Games = player2Games = 0;
            }

            player1Points = player2Points = 0;
            isDeuce = false;
            faultCount = 0;
            switchServer();
        }
        else {
            setupNextServe();
        }

        ballVelocity = cocos2d::Vec2::ZERO;
        hasBounced = false;
        hasBouncedInOpponentCourt = false;

        leftPressed = false;
        rightPressed = false;
        upPressed = false;
        downPressed = false;
        spacePressed = false;

        updateScoreDisplay();
        gameState = GameState::POINT_END;
    }
    void TennisScene::checkCourtBoundaries() {
        auto visibleSize = Director::getInstance()->getVisibleSize();
        Vec2 pos = ball->getPosition();
        float perspectiveScale = getPerspectiveScale(pos.y);
        float leftBoundary = (visibleSize.width - courtDims.width) / 2 +
            (1.0f - perspectiveScale) * 20.0f;
        float rightBoundary = (visibleSize.width + courtDims.width) / 2 -
            (1.0f - perspectiveScale) * 20.0f;

        bool isOut = pos.x < leftBoundary ||
            pos.x > rightBoundary ||
            pos.y < courtDims.baselineOffset ||
            pos.y > visibleSize.height - courtDims.baselineOffset;

        if (isOut) {
            handlePointEnd(ballVelocity.y < 0);
        }
    }


    bool TennisScene::isPositionOutOfCourt(const Vec2& position) {
        auto visibleSize = Director::getInstance()->getVisibleSize();
        float courtLeftLimit = visibleSize.width * 0.15f; 
        float courtRightLimit = visibleSize.width * 0.85f; 

        if (position.x < courtLeftLimit || position.x > courtRightLimit) {
            return true;
        }

        if (position.y < visibleSize.height * 0.1f || position.y > visibleSize.height * 0.9f) {
            return true;
        }

        return false;
    }
    void TennisScene::onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event) {
        switch (keyCode) {
        case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
            leftPressed = true;
            break;
        case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
            rightPressed = true;
            break;
        case EventKeyboard::KeyCode::KEY_UP_ARROW:
            upPressed = true;
            break;
        case EventKeyboard::KeyCode::KEY_DOWN_ARROW:
            downPressed = true;
            break;
        case EventKeyboard::KeyCode::KEY_SPACE:
            spacePressed = true;
            
            if (isServing && servingPlayer == 1) {
                if (serveState == ServeState::READY) {
                    serveState = ServeState::TOSS;
                    serveTimer = 0;
                    ballVelocity = cocos2d::Vec2::ZERO;
                }
                else if (serveState == ServeState::READY_TO_HIT && canServe) {
                    hitServe();
                }
            }
           
            else if (servingPlayer == 2 || gameState == GameState::PLAY) {
                cocos2d::Vec2 ballPos = ball->getPosition();
                cocos2d::Vec2 playerPos = player1->getPosition();
                auto visibleSize = Director::getInstance()->getVisibleSize();

                if (std::abs(ballPos.x - playerPos.x) < 100.0f &&
                    ballPos.y < visibleSize.height * 0.4f) {

                    cocos2d::Vec2 direction;
                    if (upPressed)
                        direction = cocos2d::Vec2(0, 1.2f);
                    else if (downPressed)
                        direction = cocos2d::Vec2(0, 0.8f);
                    else
                        direction = cocos2d::Vec2(0, 1.0f);

                    if (leftPressed)
                        direction.x -= 0.5f;
                    if (rightPressed)
                        direction.x += 0.5f;

                    direction.normalize();
                    float speed = HIT_BASE_SPEED;


                    ballVelocity = direction * speed;
                    hasBounced = false;
                    hasBouncedInOpponentCourt = false;
                    ballInPlay = true;
                    canHit = false;

                    if (gameState == GameState::SERVE) {
                        gameState = GameState::PLAY;
                    }
                }
            }
            break;
        }
    }

    void TennisScene::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event) {
        switch (keyCode) {
        case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
            leftPressed = false;
            break;
        case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
            rightPressed = false;
            break;
        case EventKeyboard::KeyCode::KEY_UP_ARROW:
            upPressed = false;
            break;
        case EventKeyboard::KeyCode::KEY_DOWN_ARROW:
            downPressed = false;
            break;
        case EventKeyboard::KeyCode::KEY_SPACE:
            spacePressed = false;


            break;
        default:
            break;
        }
    }

    void TennisScene::switchSides() {
        isDeuceSide = true;
        auto temp = player1LeftPos;
        player1LeftPos = player2LeftPos;
        player2LeftPos = temp;

        temp = player1RightPos;
        player1RightPos = player2RightPos;
        player2RightPos = temp;

        updatePlayerPositions();
    }

    void TennisScene::updateBallShadow() {
        if (!ballShadow || !ball || !ballInPlay) {
            if (ballShadow) {
                ballShadow->setVisible(false);
            }
            return;
        }

        auto visibleSize = Director::getInstance()->getVisibleSize();
        Vec2 ballPos = ball->getPosition();
        float courtBottom = visibleSize.height * 0.15f;
        float courtTop = visibleSize.height * 0.85f;
        Vec2 shadowPos;
        shadowPos.x = ballPos.x;
        shadowPos.y = ballPos.y;  

        float scale = BALL_BASE_SCALE;
        if (shadowPos.y > visibleSize.height * 0.5f) {
            
            scale *= 0.6f;
        }
        else {
            scale *= 0.8f;
        }
        shadowPos.y = std::min(std::max(shadowPos.y, courtBottom), courtTop);

        ballShadow->setPosition(shadowPos);
        ballShadow->setScale(scale);
        ballShadow->setOpacity(120);
        ballShadow->setVisible(true);
        ballShadow->setLocalZOrder(1);
    }

    void TennisScene::resetBall() {
        if (!ball || !player1) return;

        ball->setPosition(player1->getPositionX(),
            player1->getPositionY() + SERVE_START_HEIGHT);
        ball->setScale(BALL_BASE_SCALE);

        if (ballShadow) {
            ballShadow->setVisible(false);
        }

        ballVelocity = Vec2::ZERO;
        hasBounced = false; 
        hasBouncedInOpponentCourt = false;
        isServing = true;
        ballInPlay = false;
        serveTimer = 0;
        canHit = false;
        canServe = false;
        serveState = ServeState::READY;
    }

    float TennisScene::getPerspectiveScale(float yPos) {
        float scale = BACK_PLAYER_SCALE + (FRONT_PLAYER_SCALE - BACK_PLAYER_SCALE) * (yPos / courtDims.height);
        return scale;
    }
    bool TennisScene::isInServiceBox(const cocos2d::Vec2& position) {
        auto visibleSize = Director::getInstance()->getVisibleSize();
        float centerX = visibleSize.width * 0.5f;
        float serviceBoxWidth = visibleSize.width * 0.2f;

        if (servingPlayer == 1) {
            float minY = visibleSize.height * 0.6f;  
            float maxY = visibleSize.height * 0.85f; 

            if (isDeuceSide) {
                return (position.x >= centerX - serviceBoxWidth &&
                    position.x <= centerX &&
                    position.y >= minY &&
                    position.y <= maxY);
            }
            else {
                return (position.x >= centerX &&
                    position.x <= centerX + serviceBoxWidth &&
                    position.y >= minY &&
                    position.y <= maxY);
            }
        }
        else {
            float minY = visibleSize.height * 0.15f; 
            float maxY = visibleSize.height * 0.4f;  

            if (isDeuceSide) {
                return (position.x >= centerX &&
                    position.x <= centerX + serviceBoxWidth &&
                    position.y >= minY &&
                    position.y <= maxY);
            }
            else {
                return (position.x >= centerX - serviceBoxWidth &&
                    position.x <= centerX &&
                    position.y >= minY &&
                    position.y <= maxY);
            }
        }
    }
    void TennisScene::hitBall() {
        if (!canHit) return;

        auto visibleSize = Director::getInstance()->getVisibleSize();
        float netY = visibleSize.height * 0.5f;

        cocos2d::Vec2 direction;
        if (servingPlayer == 1) {
            float minHeight = (netY - ball->getPositionY()) / visibleSize.height;
            direction.y = std::max(2.0f, minHeight * 4.0f); 
        }
        else {
            float minHeight = (ball->getPositionY() - netY) / visibleSize.height;
            direction.y = std::min(-2.0f, -minHeight * 4.0f); 
        }

        direction.x = (leftPressed ? -0.3f : (rightPressed ? 0.3f : 0.0f));
        direction.normalize();

        float speed = HIT_SPEED * 1.3f;
        ballVelocity = direction * speed;

        ballInPlay = true;
        hasBounced = false;
        canHit = false;
    }

    void TennisScene::startNewPoint() {
        faultCount = 0;
        gameState = GameState::SERVE;
        isServing = true;
        ballInPlay = false;
        canHit = false;
        canServe = false;
        currentShot = ShotType::NORMAL;
        aiServeTimer = 0;
        serveState = ServeState::READY;

        leftPressed = false;
        rightPressed = false;
        upPressed = false;
        downPressed = false;
        spacePressed = false;

        hasBounced = false;
        hasBouncedInOpponentCourt = false;
        ballVelocity = cocos2d::Vec2::ZERO;
        positionPlayersForServe();
        updateScoreDisplay();
    }

    void TennisScene::switchServer() {
        servingPlayer = (servingPlayer == 1) ? 2 : 1;
        if (servingPlayer == 2) {
            isDeuceSide = false;  
        }
        else {
            isDeuceSide = true;   
        }

        faultCount = 0;
        serveState = ServeState::READY;
        isServing = true;
        aiServeTimer = 0;

        positionPlayersForServe();
    }


    void TennisScene::hitServe() {
        auto visibleSize = Director::getInstance()->getVisibleSize();
        float centerX = visibleSize.width * 0.5f;
        float targetX;

        if (servingPlayer == 2) {
            if (isDeuceSide) {
                targetX = centerX - (visibleSize.width * 0.25f);
            }
            else {
                targetX = centerX + (visibleSize.width * 0.25f);
            }

            cocos2d::Vec2 direction;
            direction.y = -2.0f;
            direction.x = (targetX - ball->getPositionX()) / (visibleSize.width * 0.3f);
            direction.normalize();
            ballVelocity = direction * 600.0f;
            canHit = true;
        }
        else {
            if (isDeuceSide) {
                targetX = centerX - (visibleSize.width * 0.15f);
            }
            else {
                targetX = centerX + (visibleSize.width * 0.15f);
            }

            cocos2d::Vec2 direction;
            direction.y = 2.0f;
            direction.x = (targetX - ball->getPositionX()) / (visibleSize.width * 0.3f);
            direction.normalize();
            ballVelocity = direction * 700.0f;
        }

        ball->setVisible(true);
        ballShadow->setVisible(true);
        isServing = false;
        ballInPlay = true;
        serveState = ServeState::READY;
        gameState = GameState::PLAY;
        hasBounced = false;
    }


    void TennisScene::setupNextServe() {
        isDeuceSide = !isDeuceSide;
        startNewPoint();
    }

    void TennisScene::positionPlayersForServe() {
        auto visibleSize = Director::getInstance()->getVisibleSize();
        float centerX = visibleSize.width * 0.5f;
        float frontBaselineY = visibleSize.height * 0.2f;
        float backBaselineY = visibleSize.height * 0.92f;
        float centerOffset = visibleSize.width * 0.05f;

        if (servingPlayer == 2) { 
            if (isDeuceSide) {
                player2->setPosition(centerX + centerOffset, backBaselineY);
                player1->setPosition(centerX - centerOffset, frontBaselineY);
            }
            else {
                player2->setPosition(centerX - centerOffset, backBaselineY);
                player1->setPosition(centerX + centerOffset, frontBaselineY);
            }
            ball->setPosition(player2->getPositionX(), player2->getPositionY());
        }
        else { 
            if (isDeuceSide) {
                player1->setPosition(centerX + centerOffset, frontBaselineY);
                player2->setPosition(centerX - (visibleSize.width * 0.2f), backBaselineY);
            }
            else {
                player1->setPosition(centerX - centerOffset, frontBaselineY);
                player2->setPosition(centerX + (visibleSize.width * 0.2f), backBaselineY);
            }
            ball->setPosition(player1->getPositionX(), player1->getPositionY() + SERVE_START_HEIGHT);
        }
    }

    void TennisScene::updatePlayerPositions() {
        positionPlayersForServe(); 
    }

    void TennisScene::resetServe() {
        serveState = ServeState::READY;
        serveTimer = 0;
        ball->setPosition(player1->getPositionX(), player1->getPositionY() + SERVE_START_HEIGHT);
        ball->setScale(BALL_BASE_SCALE);
        ballShadow->setVisible(false);
        canServe = false;

    }

} 
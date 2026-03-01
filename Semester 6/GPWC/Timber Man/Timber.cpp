#include <SFML/Graphics.hpp>
#include <sstream>
#include <ctime>

using namespace sf;
using namespace std;

const int NUM_BRANCHES = 6;
enum class Side { LEFT, RIGHT, NONE };

Sprite* branches[NUM_BRANCHES];
Side branchPositions[NUM_BRANCHES];

void updateBranches(int seed)
{
    for (int j = NUM_BRANCHES - 1; j > 0; j--)
        branchPositions[j] = branchPositions[j - 1];

    srand((int)time(0) + seed);
    int r = rand() % 5;
    switch (r)
    {
        case 0:  branchPositions[0] = Side::LEFT;  break;
        case 1:  branchPositions[0] = Side::RIGHT; break;
        default: branchPositions[0] = Side::NONE;  break;
    }
}

int main()
{
    srand((int)time(0));

    RenderWindow window(
        VideoMode(Vector2u(1920, 1080)),
        "Timber Game!!!"
    );

    View view(FloatRect(Vector2f(0.f, 0.f), Vector2f(1920.f, 1080.f)));
    window.setView(view);

    // Background
    Texture textureBackground;
    if (!textureBackground.loadFromFile("graphics/background.png")) return -1;
    Sprite spriteBackground(textureBackground);
    spriteBackground.setPosition(Vector2f(0.f, 0.f));

    // Tree
    Texture textureTree;
    if (!textureTree.loadFromFile("graphics/tree.png")) return -1;
    Sprite spriteTree(textureTree);
    spriteTree.setPosition(Vector2f(810.f, 0.f));

    // Player
    Texture texturePlayer;
    if (!texturePlayer.loadFromFile("graphics/player.png")) return -1;
    Sprite spritePlayer(texturePlayer);
    spritePlayer.setPosition(Vector2f(580.f, 720.f));

    // Axe
    Texture textureAxe;
    if (!textureAxe.loadFromFile("graphics/axe.png")) return -1;
    Sprite spriteAxe(textureAxe);
    spriteAxe.setPosition(Vector2f(700.f, 830.f));
    const float AXE_POSITION_LEFT  = 700.f;
    const float AXE_POSITION_RIGHT = 1075.f;

    // Bee
    Texture textureBee;
    if (!textureBee.loadFromFile("graphics/bee.png")) return -1;
    Sprite spriteBee(textureBee);
    spriteBee.setPosition(Vector2f(2000.f, 500.f));
    bool beeActive = false;
    float beeSpeed = 0.f;

    // Branches
    Texture textureBranch;
    if (!textureBranch.loadFromFile("graphics/branch.png")) return -1;
    for (int i = 0; i < NUM_BRANCHES; i++)
    {
        branches[i] = new Sprite(textureBranch);
        branches[i]->setPosition(Vector2f(-2000.f, -2000.f));
        branches[i]->setOrigin(Vector2f(220.f, 20.f));
        branchPositions[i] = Side::NONE;
    }

    updateBranches(1);
    updateBranches(2);
    updateBranches(3);
    updateBranches(4);
    updateBranches(5);
    branchPositions[5] = Side::NONE;

    // Font & Text
    Font font;
    if (!font.openFromFile("font/KOMIKAP_.ttf")) return -1;

    Text scoreText(font, "Score = 0", 100);
    scoreText.setFillColor(Color::Red);
    scoreText.setPosition(Vector2f(20.f, 20.f));

    int score = 0;

    // Message text
    Text messageText(font, "Press Enter or Space to Start!", 75);
    messageText.setFillColor(Color::Yellow);
    FloatRect textRect = messageText.getLocalBounds();
    messageText.setOrigin(Vector2f(
        textRect.position.x + textRect.size.x / 2.f,
        textRect.position.y + textRect.size.y / 2.f
    ));
    messageText.setPosition(Vector2f(1920.f / 2.f, 1080.f / 2.f));

    // Time bar
    RectangleShape timeBar;
    float timeBarStartWidth = 400.f;
    float timeBarHeight = 80.f;
    timeBar.setSize(Vector2f(timeBarStartWidth, timeBarHeight));
    timeBar.setFillColor(Color::Green);
    timeBar.setPosition(Vector2f(1920.f / 2.f - timeBarStartWidth / 2.f, 980.f));
    float timeRemaining = 6.f;
    float timeBarWidthPerSecond = timeBarStartWidth / timeRemaining;

    bool paused = true;
    bool spaceWasPressed = false;

    Clock clock;

    while (window.isOpen())
    {
        // --- Events ---
        while (auto event = window.pollEvent())
        {
            if (event->is<Event::Closed>())
                window.close();

            if (const auto* keyEvent = event->getIf<Event::KeyPressed>())
            {
                if (keyEvent->code == Keyboard::Key::Enter ||
                    keyEvent->code == Keyboard::Key::Space)
                {
                    if (paused)
                    {
                        paused = false;
                        score = 0;
                        timeRemaining = 6.f;
                        clock.restart();
                        messageText.setString("");

                        stringstream ss;
                        ss << "Score = " << score;
                        scoreText.setString(ss.str());
                    }
                }
            }
        }

        if (Keyboard::isKeyPressed(Keyboard::Key::Escape))
            window.close();

        // --- Spacebar: one chop per press ---
        if (!paused)
        {
            bool spaceNow = Keyboard::isKeyPressed(Keyboard::Key::Space);

            if (spaceNow && !spaceWasPressed)
            {
                score++;

                stringstream ss;
                ss << "Score = " << score;
                scoreText.setString(ss.str());

                timeRemaining = 6.f;

                updateBranches(score);
            }

            spaceWasPressed = spaceNow;
        }

        // --- Update ---
        if (!paused)
        {
            Time dt = clock.restart();

            // Timer counts down
            timeRemaining -= dt.asSeconds();
            timeBar.setSize(Vector2f(timeBarWidthPerSecond * timeRemaining, timeBarHeight));

            // Green → Yellow → Red
            if (timeRemaining > 4.f)
                timeBar.setFillColor(Color::Green);
            else if (timeRemaining > 2.f)
                timeBar.setFillColor(Color::Yellow);
            else
                timeBar.setFillColor(Color::Red);

            if (timeRemaining <= 0.f)
            {
                paused = true;
                messageText.setString("OUT OF TIME!");
                FloatRect tr = messageText.getLocalBounds();
                messageText.setOrigin(Vector2f(
                    tr.position.x + tr.size.x / 2.f,
                    tr.position.y + tr.size.y / 2.f
                ));
                messageText.setPosition(Vector2f(1920.f / 2.f, 1080.f / 2.f));
            }

            // Bee
            if (!beeActive)
            {
                beeSpeed = (float)(rand() % 200) + 200.f;
                float height = (float)(rand() % 500) + 500.f;
                spriteBee.setPosition(Vector2f(2000.f, height));
                beeActive = true;
            }
            else
            {
                spriteBee.setPosition(Vector2f(
                    spriteBee.getPosition().x - beeSpeed * dt.asSeconds(),
                    spriteBee.getPosition().y
                ));
                if (spriteBee.getPosition().x < -100.f)
                    beeActive = false;
            }
        }

        // --- Branch Rendering ---
        for (int i = 0; i < NUM_BRANCHES; i++)
        {
            float height = (float)(i * 150);
            if (branchPositions[i] == Side::LEFT)
            {
                branches[i]->setPosition(Vector2f(610.f, height));
                branches[i]->setRotation(degrees(180.f));
            }
            else if (branchPositions[i] == Side::RIGHT)
            {
                branches[i]->setPosition(Vector2f(1330.f, height));
                branches[i]->setRotation(degrees(0.f));
            }
            else
            {
                branches[i]->setPosition(Vector2f(3000.f, height));
            }
        }

        // --- Draw ---
        window.clear();
        window.draw(spriteBackground);
        window.draw(spriteTree);
        for (int i = 0; i < NUM_BRANCHES; i++)
            window.draw(*branches[i]);
        window.draw(spriteBee);
        window.draw(spritePlayer);
        window.draw(spriteAxe);
        window.draw(scoreText);
        window.draw(timeBar);
        if (paused)
            window.draw(messageText);
        window.display();
    }

    for (int i = 0; i < NUM_BRANCHES; i++)
        delete branches[i];

    return 0;
}
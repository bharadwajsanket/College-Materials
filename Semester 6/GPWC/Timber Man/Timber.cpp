#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <ctime>
#include <sstream>

using namespace std;
using namespace sf;

const int NUM_BRANCHES = 6;

enum class Side { LEFT, RIGHT, NONE };

Side branchPositions[NUM_BRANCHES];

void updateBranches(int seed) {
  for (int j = NUM_BRANCHES - 1; j > 0; j--)
    branchPositions[j] = branchPositions[j - 1];

  int r = rand() % 5;

  if (r == 0)
    branchPositions[0] = Side::LEFT;
  else if (r == 1)
    branchPositions[0] = Side::RIGHT;
  else
    branchPositions[0] = Side::NONE;
}

int main() {
  srand((int)time(0));

  RenderWindow window(VideoMode(Vector2u(1920, 1080)), "Timber Game!!!");

  View view(FloatRect(Vector2f(0, 0), Vector2f(1920, 1080)));
  window.setView(view);

  // Background
  Texture textureBackground;
  if (!textureBackground.loadFromFile("graphics/background.png"))
    return -1;
  Sprite spriteBackground(textureBackground);

  // Tree
  Texture textureTree;
  if (!textureTree.loadFromFile("graphics/tree.png"))
    return -1;
  Sprite spriteTree(textureTree);
  spriteTree.setPosition(Vector2f(810, 0));

  // Clouds
  Texture textureCloud;
  if (!textureCloud.loadFromFile("graphics/cloud.png"))
    return -1;

  Sprite spriteCloud1(textureCloud);
  Sprite spriteCloud2(textureCloud);
  Sprite spriteCloud3(textureCloud);

  spriteCloud1.setScale(Vector2f(1.0f, 1.0f));
  spriteCloud2.setScale(Vector2f(0.8f, 0.8f));
  spriteCloud3.setScale(Vector2f(0.6f, 0.6f));

  bool cloudActive1 = false;
  bool cloudActive2 = false;
  bool cloudActive3 = false;

  float cloudSpeed1 = 0;
  float cloudSpeed2 = 0;
  float cloudSpeed3 = 0;

  // log
  Texture textureLog;
  if (!textureLog.loadFromFile("graphics/log.png"))
    return -1;
  Sprite spriteLog(textureLog);
  spriteLog.setPosition(Vector2f(810, 720));

  bool logActive = false;
  float logSpeedX = 1000;
  float logSpeedY = -1500;

  // Player
  Texture texturePlayer;
  if (!texturePlayer.loadFromFile("graphics/player.png"))
    return -1;
  Sprite spritePlayer(texturePlayer);
  spritePlayer.setPosition(Vector2f(580, 720));
  spritePlayer.setScale(Vector2f(-1, 1));

  // Axe
  Texture textureAxe;
  if (!textureAxe.loadFromFile("graphics/axe.png"))
    return -1;

  Sprite spriteAxe(textureAxe);
  spriteAxe.setPosition(Vector2f(700, 830));

  const float AXE_POSITION_LEFT = 700;
  const float AXE_POSITION_RIGHT = 1075;

  Side playerSide = Side::LEFT;
  bool acceptInput = false;

  // RIP
  Texture textureRip;
  if (!textureRip.loadFromFile("graphics/rip.png"))
    return -1;

  Sprite spriteRip(textureRip);
  spriteRip.setPosition(Vector2f(3000, 720));

  // Branch texture
  Texture textureBranch;
  if (!textureBranch.loadFromFile("graphics/branch.png"))
    return -1;

  Sprite branches[NUM_BRANCHES] = {
      Sprite(textureBranch), Sprite(textureBranch), Sprite(textureBranch),
      Sprite(textureBranch), Sprite(textureBranch), Sprite(textureBranch)};

  for (int i = 0; i < NUM_BRANCHES; i++) {
    branches[i].setOrigin(Vector2f(220, 20));
    branchPositions[i] = Side::NONE;
  }

  updateBranches(1);
  updateBranches(2);
  updateBranches(3);
  updateBranches(4);
  updateBranches(5);

  // Bee
  Texture textureBee;
  if (!textureBee.loadFromFile("graphics/bee.png"))
    return -1;

  Sprite spriteBee(textureBee);

  bool beeActive = false;
  float beeSpeed = 0;

  // Sound effects
  SoundBuffer chopBuffer;
  if (!chopBuffer.loadFromFile("sound/chop.wav"))
    return -1;
  Sound chopSound(chopBuffer);

  SoundBuffer deathBuffer;
  if (!deathBuffer.loadFromFile("sound/death.wav"))
    return -1;
  Sound deathSound(deathBuffer);

  SoundBuffer outOfTimeBuffer;
  if (!outOfTimeBuffer.loadFromFile("sound/out_of_time.wav"))
    return -1;
  Sound outOfTimeSound(outOfTimeBuffer);

  // UI
  Font font;
  if (!font.openFromFile("font/KOMIKAP_.ttf"))
    return -1;

  Text scoreText(font, "Score = 0", 100);
  scoreText.setFillColor(Color::Red);
  scoreText.setPosition(Vector2f(20, 20));

  int score = 0;
  int highScore = 0;

  Text highScoreText(font, "High Score = 0", 50);
  highScoreText.setFillColor(Color::Yellow);
  highScoreText.setPosition(Vector2f(20, 120));

  Text messageText(font, "Press Enter to Start", 75);
  messageText.setFillColor(Color::Green);

  FloatRect textRect = messageText.getLocalBounds();

  messageText.setOrigin(Vector2f(textRect.position.x + textRect.size.x / 2,
                                 textRect.position.y + textRect.size.y / 2));

  messageText.setPosition(Vector2f(960, 540));

  // Time bar
  RectangleShape timeBar;

  float timeBarStartWidth = 400;
  float timeBarHeight = 80;

  timeBar.setSize(Vector2f(timeBarStartWidth, timeBarHeight));
  timeBar.setFillColor(Color::Red);
  timeBar.setPosition(Vector2f(960 - timeBarStartWidth / 2, 980));

  float timeRemaining = 10;
  float timeBarWidthPerSecond = timeBarStartWidth / timeRemaining;

  bool paused = true;

  Clock clock;
  Time dt;

  // Game loop
  while (window.isOpen()) {
    dt = clock.restart();

    while (auto event = window.pollEvent()) {
      if (event->is<Event::Closed>())
        window.close();

      if (const auto *key = event->getIf<Event::KeyPressed>()) {
        if (key->code == Keyboard::Key::Enter && paused) {
          paused = false;
          timeRemaining = 10;
          score = 0;
          acceptInput = true;

          messageText.setString("");

          spritePlayer.setPosition(Vector2f(580, 720));
          spritePlayer.setScale(Vector2f(-1, 1));
          spriteRip.setPosition(Vector2f(3000, 720));

          for (int i = 0; i < NUM_BRANCHES; i++)
            branchPositions[i] = Side::NONE;

          updateBranches(1);
          updateBranches(2);
          updateBranches(3);
          updateBranches(4);
          updateBranches(5);
        }
      }

      if (event->is<Event::KeyReleased>() && !paused) {
        acceptInput = true;
        spriteAxe.setPosition(Vector2f(2000, 830));
      }
    }

    if (!paused) {
      timeRemaining -= dt.asSeconds();

      timeBar.setSize(
          Vector2f(timeBarWidthPerSecond * timeRemaining, timeBarHeight));

      if (timeRemaining <= 0) {
        paused = true;
        if (score > highScore)
          highScore = score;
        messageText.setString("Out of Time!");
        outOfTimeSound.play();
      }

      // Clouds
      if (!cloudActive1) {
        cloudSpeed1 = rand() % 200;
        spriteCloud1.setPosition(Vector2f(-200, rand() % 150));
        cloudActive1 = true;
      } else {
        spriteCloud1.move(Vector2f(cloudSpeed1 * dt.asSeconds(), 0));
        if (spriteCloud1.getPosition().x > 1920)
          cloudActive1 = false;
      }

      if (!cloudActive2) {
        cloudSpeed2 = rand() % 200;
        spriteCloud2.setPosition(Vector2f(-200, (rand() % 300) - 150));
        cloudActive2 = true;
      } else {
        spriteCloud2.move(Vector2f(cloudSpeed2 * dt.asSeconds(), 0));
        if (spriteCloud2.getPosition().x > 1920)
          cloudActive2 = false;
      }

      if (!cloudActive3) {
        cloudSpeed3 = rand() % 200;
        spriteCloud3.setPosition(Vector2f(-200, (rand() % 450) - 150));
        cloudActive3 = true;
      } else {
        spriteCloud3.move(Vector2f(cloudSpeed3 * dt.asSeconds(), 0));
        if (spriteCloud3.getPosition().x > 1920)
          cloudActive3 = false;
      }

      // Chop
      if (acceptInput) {
        if (Keyboard::isKeyPressed(Keyboard::Key::Left)) {
          playerSide = Side::LEFT;

          spritePlayer.setPosition(Vector2f(580, 720));
          spritePlayer.setScale(Vector2f(-1, 1));
          spriteAxe.setPosition(Vector2f(AXE_POSITION_LEFT, 830));

          score++;
          timeRemaining += (3 / (float)score) + .25;

          // Log flies to the right
          logActive = true;
          spriteLog.setPosition(Vector2f(810, 720));
          logSpeedX = 1000;
          logSpeedY = -1500;

          updateBranches(score);
          chopSound.play();
          acceptInput = false;
        }

        if (Keyboard::isKeyPressed(Keyboard::Key::Right)) {
          playerSide = Side::RIGHT;

          spritePlayer.setPosition(Vector2f(1200, 720));
          spritePlayer.setScale(Vector2f(1, 1));
          spriteAxe.setPosition(Vector2f(AXE_POSITION_RIGHT, 830));

          score++;
          timeRemaining += (3 / (float)score) + .25;

          // Log flies to the left
          logActive = true;
          spriteLog.setPosition(Vector2f(810, 720));
          logSpeedX = -1000;
          logSpeedY = -1500;

          updateBranches(score);
          chopSound.play();
          acceptInput = false;
        }
      }
      // log movement
      if (logActive) {
        spriteLog.move(
            Vector2f(logSpeedX * dt.asSeconds(), logSpeedY * dt.asSeconds()));
        if (spriteLog.getPosition().x < -100 ||
            spriteLog.getPosition().x > 2000) {
          logActive = false;
          spriteLog.setPosition(Vector2f(810, 720));
        }
      }

      // Death
      if (branchPositions[5] == playerSide) {
        paused = true;
        acceptInput = false;

        spritePlayer.setPosition(Vector2f(2000, 720));
        spritePlayer.setScale(Vector2f(1, 1));

        if (playerSide == Side::LEFT)
          spriteRip.setPosition(Vector2f(600, 720));
        else
          spriteRip.setPosition(Vector2f(1150, 720));

        if (score > highScore)
          highScore = score;
        messageText.setString("SQUISHED!!");
        deathSound.play();
      }

      // Bee
      if (!beeActive) {
        beeSpeed = (rand() % 200) + 200;
        spriteBee.setPosition(Vector2f(2000, (rand() % 500) + 500));
        beeActive = true;
      } else {
        spriteBee.move(Vector2f(-beeSpeed * dt.asSeconds(), 0));

        if (spriteBee.getPosition().x < -100)
          beeActive = false;
      }

      stringstream ss;
      ss << "Score = " << score;
      scoreText.setString(ss.str());

      stringstream hs;
      hs << "High Score = " << highScore;
      highScoreText.setString(hs.str());
    }

    // Branch render
    for (int i = 0; i < NUM_BRANCHES; i++) {
      float height = i * 150;

      if (branchPositions[i] == Side::LEFT) {
        branches[i].setPosition(Vector2f(610, height));
        branches[i].setRotation(degrees(180));
      } else if (branchPositions[i] == Side::RIGHT) {
        branches[i].setPosition(Vector2f(1330, height));
        branches[i].setRotation(degrees(0));
      } else {
        branches[i].setPosition(Vector2f(3000, height));
      }
    }

    window.clear();

    window.draw(spriteBackground);
    window.draw(spriteCloud1);
    window.draw(spriteCloud2);
    window.draw(spriteCloud3);
    window.draw(spriteTree);

    for (int i = 0; i < NUM_BRANCHES; i++)
      window.draw(branches[i]);

    window.draw(spritePlayer);
    window.draw(spriteRip);
    window.draw(spriteAxe);
    window.draw(spriteLog);
    window.draw(spriteBee);
    window.draw(highScoreText);
    window.draw(scoreText);
    window.draw(timeBar);

    if (paused)
      window.draw(messageText);

    window.display();
  }

  return 0;
}

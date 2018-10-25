#include "GameState_Play.h"
#include "Common.h"
#include "Physics.h"
#include "Assets.h"
#include "GameEngine.h"
#include "Components.h"

GameState_Play::GameState_Play(GameEngine & game, const std::string & levelPath)
	: GameState(game)
	, m_levelPath(levelPath)
{
	init(m_levelPath);
}

void GameState_Play::init(const std::string & levelPath)
{
	loadLevel(levelPath);

	spawnPlayer();


}

void GameState_Play::loadLevel(const std::string & filename)
{
	// reset the entity manager every time we load a level
	m_entityManager = EntityManager();

	std::ifstream fin(filename);
	std::string token, type;
	int x, y;

	while (fin.good()) {
		fin >> token;
		if (token == "Tile") {
			fin >> type >> x >> y;
			Vec2 pos(x, y);
			auto block = m_entityManager.addEntity("tile");
			block->addComponent<CTransform>(pos);
			block->addComponent<CAnimation>(m_game.getAssets().getAnimation(type), true);
			// add a bounding box, this will now show up if we press the 'F' key
			block->addComponent<CBoundingBox>(m_game.getAssets().getAnimation(type).getSize());
		}
		else if (token == "Dec") {
			fin >> type >> x >> y;
			Vec2 pos(x, y);
			auto block = m_entityManager.addEntity("dec");
			block->addComponent<CTransform>(pos);
			block->addComponent<CAnimation>(m_game.getAssets().getAnimation(type), true);
		}
		else if (token == "Player") {
			fin >> m_playerConfig.X >> m_playerConfig.Y >> m_playerConfig.CX >> m_playerConfig.CY >> m_playerConfig.SPEED >> m_playerConfig.JUMP >> m_playerConfig.MAXSPEED >> m_playerConfig.GRAVITY >> m_playerConfig.WEAPON;
		}
		else { std::cout << "Bad\n"; }
	}
	// TODO: read in the level file and add the appropriate entities
	//       use the PlayerConfig struct m_playerConfig to store player properties
	//       this struct is defined at the top of GameState_Play.h
}

void GameState_Play::spawnPlayer()
{
	// here is a sample player entity which you can use to construct other entities
	m_player = m_entityManager.addEntity("player");
	Vec2 pos(m_playerConfig.X, m_playerConfig.Y);
	m_player->addComponent<CTransform>(pos);
	m_player->addComponent<CBoundingBox>(Vec2(m_playerConfig.CX,m_playerConfig.CY));
	m_player->addComponent<CAnimation>(m_game.getAssets().getAnimation("Stand"), true);
	m_player->addComponent<CInput>();
	m_player->addComponent<CGravity>(m_playerConfig.GRAVITY);
	m_player->addComponent<CState>("air");
	// TODO: be sure to add the remaining components to the player
}

void GameState_Play::spawnBullet(std::shared_ptr<Entity> entity)
{
	auto bullet = m_entityManager.addEntity("bullet");
	Vec2 pos(entity->getComponent<CTransform>()->pos);
	bullet->addComponent<CTransform>(pos);
	bullet->addComponent<CBoundingBox>(m_game.getAssets().getAnimation("Buster").getSize());
	bullet->addComponent<CAnimation>(m_game.getAssets().getAnimation("Buster"), true);
	bullet->getComponent<CTransform>()->speed.x = m_playerConfig.SPEED * 3 * entity->getComponent<CTransform>()->scale.x;
	bullet->addComponent<CLifeSpan>(1000);

	// TODO: this should spawn a bullet at the given entity, going in the direction the entity is facing
}

void GameState_Play::update()
{
	m_entityManager.update();

	// TODO: implement pause functionality
	if (!m_paused)
	{
		sMovement();
		sLifespan();
		sCollision();
		sAnimation();
	}
	sUserInput();
	sRender();
}

void GameState_Play::sMovement()

{
	m_player->getComponent<CTransform>()->speed.x = 0;
	//    // TODO: Implement player movement / jumping based on its CInput component
	//

	if (m_player->getComponent<CInput>()->up)
	{
		if (m_player->getComponent<CState>()->state == "standing")
		{
			m_player->getComponent<CTransform>()->speed.y += m_playerConfig.JUMP;
		}
	}


	if (m_player->getComponent<CInput>()->down)
	{
		m_player->getComponent<CTransform>()->speed.y -= m_playerConfig.SPEED;
	}


	if (m_player->getComponent<CInput>()->right)
	{
		m_player->getComponent<CTransform>()->scale.x = 1;
		m_player->getComponent<CTransform>()->speed.x += m_playerConfig.SPEED;
	}


	if (m_player->getComponent<CInput>()->left)
	{
		m_player->getComponent<CTransform>()->scale.x = -1;
		if (m_player->getComponent<CTransform>()->pos.x > m_playerConfig.CX / 2)
		{
			m_player->getComponent<CTransform>()->speed.x -= m_playerConfig.SPEED;
		}
	}
	m_player->getComponent<CTransform>()->speed.y += m_player->getComponent<CGravity>()->gravity;

	if (m_player->getComponent<CTransform>()->speed.y > m_playerConfig.MAXSPEED) { m_player->getComponent<CTransform>()->speed.y = m_playerConfig.MAXSPEED; }
	if (m_player->getComponent<CTransform>()->speed.y < -(m_playerConfig.MAXSPEED)) { m_player->getComponent<CTransform>()->speed.y = -(m_playerConfig.MAXSPEED); }
	if (m_player->getComponent<CTransform>()->speed.x > m_playerConfig.MAXSPEED) { m_player->getComponent<CTransform>()->speed.x = m_playerConfig.MAXSPEED; }
	if (m_player->getComponent<CTransform>()->speed.x < -(m_playerConfig.MAXSPEED)) { m_player->getComponent<CTransform>()->speed.x = -(m_playerConfig.MAXSPEED); }

	for (auto e : m_entityManager.getEntities())
	{
		e->getComponent<CTransform>()->pos.y += e->getComponent<CTransform>()->speed.y;
		e->getComponent<CTransform>()->pos.x += e->getComponent<CTransform>()->speed.x;
	}


	if (m_player->getComponent<CInput>()->shoot && m_player->getComponent<CInput>()->canShoot)
	{
		spawnBullet(m_player);
		m_player->getComponent<CInput>()->canShoot = false;
	}








	// TODO: Implement gravity's effect on the player
	//m_player->getComponent<CTransform>()->speed.y =  -10;

	

	// TODO: Implement the maxmimum player speed in both X and Y directions
	
	if (m_player->getComponent<CTransform>()->pos.y <= 0)
	{
		m_player->destroy();
		spawnPlayer();
	}
}

void GameState_Play::sLifespan()
{
	for (auto e : m_entityManager.getEntities()) 
	{
		if (e->hasComponent<CLifeSpan>())
		{
			
			sf::Time time = e->getComponent<CLifeSpan>()->clock.getElapsedTime();
			if (time.asMilliseconds() >= e->getComponent<CLifeSpan>()->lifespan)
			{
				e->destroy();
			}
		}
	}
	// TODO: Check lifespawn of entities that have them, and destroy them if they go over
}

void GameState_Play::sCollision()
{
	// TODO: Implement Physics::GetOverlap() function, use it inside this function

	// TODO: Implement bullet / tile collisions
	//       Destroy the tile if it has a Brick animation
	// TODO: Implement player / tile collisions and resolutions
	//       Update the CState component of the player to store whether
	//       it is currently on the ground or in the air. This will be
	//       used by the Animation system
	m_player->getComponent<CState>()->state = "air";
	auto pTransform = m_player->getComponent<CTransform>();
	for (auto tile : m_entityManager.getEntities("tile"))
	{
		Vec2 overlap = Physics::GetOverlap(tile, m_player);
		if (overlap.x > 0 && overlap.y > 0)
		{
			Vec2 prevPos = pTransform->pos - pTransform->speed;
			Vec2 prevOv = Physics::GetOverlap(tile, m_player, prevPos);
			if (prevOv.y > 0)
			{
				pTransform->pos.x -= (overlap.x * pTransform->scale.x);
			}
			else
			{
				if (pTransform->pos.y > tile->getComponent<CTransform>()->pos.y)
				{
					//std::cout << m_player->getComponent<CTransform>()->pos.y;
					pTransform->pos.y += overlap.y;
					
					m_player->getComponent<CState>()->state = "standing";
				}
				else
				{
					pTransform->pos.y -= overlap.y;
					if (tile->getComponent<CAnimation>()->animation.getName() == "Question") 
					{
						tile->addComponent<CAnimation>(m_game.getAssets().getAnimation("Question2"), true);
						auto tempDec = m_entityManager.addEntity("dec");
						Vec2 tilePos(tile->getComponent<CTransform>()->pos);	
						tilePos.y += 64;
						tempDec->addComponent<CTransform>(tilePos);
						tempDec->addComponent<CAnimation>(m_game.getAssets().getAnimation("Coin"), true);
						tempDec->addComponent<CLifeSpan>(500);
						
					}
					if (tile->getComponent<CAnimation>()->animation.getName() == "Brick")
					{
						auto tempDec = m_entityManager.addEntity("dec");
						Vec2 pos(tile->getComponent<CTransform>()->pos);
						tile->destroy();
						tempDec->addComponent<CTransform>(pos);
						tempDec->addComponent<CAnimation>(m_game.getAssets().getAnimation("Explosion"), false);
					}
					
				}
				pTransform->speed.y = 0;
			}
			if (tile->getComponent<CAnimation>()->animation.getName() == "Pole" || tile->getComponent<CAnimation>()->animation.getName() == "PoleTop")
			{
				m_player->destroy();
				spawnPlayer();
			}
		}
	}
	for (auto bullet : m_entityManager.getEntities("bullet"))
	{
		for (auto tile : m_entityManager.getEntities("tile"))
		{
			Vec2 overlap = Physics::GetOverlap(tile, bullet);
			if (overlap.x > 0 && overlap.y > 0)
			{
				bullet->destroy();
				if (tile->getComponent<CAnimation>()->animation.getName() == "Brick")
				{
					auto tempDec = m_entityManager.addEntity("Dec");
					Vec2 pos(tile->getComponent<CTransform>()->pos);
					tile->destroy();
					tempDec->addComponent<CTransform>(pos);
					tempDec->addComponent<CAnimation>(m_game.getAssets().getAnimation("Explosion"), false);
				}
			}
		}
	}


	// TODO: Check to see if the player has fallen down a hole (y < 0)
	// TODO: Don't let the player walk off the left side of the map
}

void GameState_Play::sUserInput()
{
	// TODO: implement the rest of the player input

	sf::Event event;
	while (m_game.window().pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
		{
			m_game.quit();
		}

		if (event.type == sf::Event::KeyPressed)
		{
			switch (event.key.code)
			{
			case sf::Keyboard::Escape: { m_game.popState(); break; }
			case sf::Keyboard::Z: { init(m_levelPath); break; }
			case sf::Keyboard::W: { m_player->getComponent<CInput>()->up = true; break; }
			case sf::Keyboard::S: { m_player->getComponent<CInput>()->down = true; break; }
			case sf::Keyboard::A: { m_player->getComponent<CInput>()->left = true; break; }
			case sf::Keyboard::D: { m_player->getComponent<CInput>()->right = true; break; }
			case sf::Keyboard::R: { m_drawTextures = !m_drawTextures; break; }
			case sf::Keyboard::F: { m_drawCollision = !m_drawCollision; break; }
			case sf::Keyboard::P: { setPaused(!m_paused); break; }
			case sf::Keyboard::Space: {m_player->getComponent<CInput>()->shoot = true; break;}
			}
		}

		if (event.type == sf::Event::KeyReleased)
		{
			switch (event.key.code)
			{
			case sf::Keyboard::Escape: { break; }
			case sf::Keyboard::W: {m_player->getComponent<CInput>()->up = false; break; }
			case sf::Keyboard::S: {m_player->getComponent<CInput>()->down = false; break; }
			case sf::Keyboard::A: {m_player->getComponent<CInput>()->left = false; break; }
			case sf::Keyboard::D: {m_player->getComponent<CInput>()->right = false; break; }
			case sf::Keyboard::Space:
				{
				m_player->getComponent<CInput>()->shoot = false;
				m_player->getComponent<CInput>()->canShoot = true;
				break;
				}
			}
		}
	}

}

void GameState_Play::sAnimation()
{
	if (m_player->getComponent<CState>()->state != "air") 
	{
		if (m_player->getComponent<CTransform>()->speed.x == 0)
		{
			m_player->addComponent<CAnimation>(m_game.getAssets().getAnimation("Stand"), true);
		}
		else
		{
			if (m_player->getComponent<CAnimation>()->animation.getName() != "Run") 
			{
				m_player->addComponent<CAnimation>(m_game.getAssets().getAnimation("Run"), true);
			}
		}
		
	}
	else
	{
		m_player->addComponent<CAnimation>(m_game.getAssets().getAnimation("Air"), true);
	}

	for (auto e : m_entityManager.getEntities())
	{
		e->getComponent<CAnimation>()->animation.update();

		if (e->getComponent<CAnimation>()->animation.hasEnded() && !(e->getComponent<CAnimation>()->repeat))
		{
			
			std::cout << e->getComponent<CAnimation>()->animation.getName();
			e->destroy();
		}
	}
	
}

// this function has been completed for you
void GameState_Play::sRender()
{
	// color the background darker so you know that the game is paused
	if (!m_paused) { m_game.window().clear(sf::Color(100, 100, 255)); }
	else { m_game.window().clear(sf::Color(50, 50, 150)); }

	// set the viewport of the window to be centered on the player if it's far enough right
	auto pPos = m_player->getComponent<CTransform>()->pos;
	float windowCenterX = fmax(m_game.window().getSize().x / 2.0f, pPos.x);
	sf::View view = m_game.window().getView();
	view.setCenter(windowCenterX, m_game.window().getSize().y - view.getCenter().y);
	m_game.window().setView(view);

	// draw all Entity textures / animations
	if (m_drawTextures)
	{
		for (auto e : m_entityManager.getEntities())
		{
			auto transform = e->getComponent<CTransform>();

			if (e->hasComponent<CAnimation>())
			{
				auto animation = e->getComponent<CAnimation>()->animation;
				animation.getSprite().setRotation(transform->angle);
				animation.getSprite().setPosition(transform->pos.x, m_game.window().getSize().y - transform->pos.y);
				animation.getSprite().setScale(transform->scale.x, transform->scale.y);
				m_game.window().draw(animation.getSprite());
			}
		}
	}

	// draw all Entity collision bounding boxes with a rectangleshape
	if (m_drawCollision)
	{
		for (auto e : m_entityManager.getEntities())
		{
			if (e->hasComponent<CBoundingBox>())
			{
				auto box = e->getComponent<CBoundingBox>();
				auto transform = e->getComponent<CTransform>();
				sf::RectangleShape rect;
				rect.setSize(sf::Vector2f(box->size.x - 1, box->size.y - 1));
				rect.setOrigin(sf::Vector2f(box->halfSize.x, box->halfSize.y));
				rect.setPosition(transform->pos.x, m_game.window().getSize().y - transform->pos.y);
				rect.setFillColor(sf::Color(0, 0, 0, 0));
				rect.setOutlineColor(sf::Color(255, 255, 255, 255));
				rect.setOutlineThickness(1);
				m_game.window().draw(rect);
			}
		}
	}

	m_game.window().display();
}

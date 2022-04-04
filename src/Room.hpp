#pragma once
#include "Tako.hpp"
#include "World.hpp"
#include "Grid.hpp"
#include "Player.hpp"
#include "Renderer.hpp"
#include "parson.h"
#include "Entities.hpp"
#include "DialogSystem.hpp"
#include "Diary.hpp"
#include "SceneManager.hpp"

using GetTextureCallback = std::function<tako::Texture*(std::string)>;

class Room
{
public:

	void Init(std::string levelName, GetTextureCallback getTexture)
	{
		m_levelName = std::move(levelName);
		PaletteManager::Reset();
		m_getTexture = getTexture;
		std::string levelFolder = "/Map/" + m_levelName + "/";
		std::string file = levelFolder + "data.json";
		constexpr size_t bufferSize = 1024 * 1024;
		std::array <tako::U8, bufferSize> buffer;
		size_t bytesRead = 0;
		if (!tako::FileSystem::ReadFile(file.c_str(), buffer.data(), bufferSize, bytesRead))
		{
			LOG_ERR("Could not read level {}", file);
		}
		buffer[bytesRead] = '/0';

		auto jsonRoot = json_parse_string((char*)buffer.data());
		auto levelJson = json_value_get_object(jsonRoot);

		auto layerArray = json_object_get_array(levelJson, "layers");
		auto layerCount = json_array_get_count(layerArray);

		for (int i = 0; i < layerCount; i++)
		{
			auto layer = json_array_get_string(layerArray, i);
			m_tilesPNG.push_back(getTexture(levelFolder + layer));
		}

		auto intGrid = tako::Bitmap::FromFile((levelFolder + "IntGrid-int.png").c_str());
		for (int x = 0; x < intGrid.Width(); x++)
		{
			for (int y = 0; y < intGrid.Height(); y++)
			{
				auto p = intGrid.GetPixel(x, y);
				if (p == tako::Color(0, 0, 0, 255))
				{
					m_world.Create(
						Tile{x, -y}
					);
				}
			}
		}

		auto entitiesJSON = json_object_get_object(levelJson, "entities");
		auto loadEntityType = [&](const char* name, std::function<void(int, int, JSON_Object*)> callback)
		{
			auto array = json_object_get_array(entitiesJSON, name);
			auto count = json_array_get_count(array);
			for (int i = 0; i < count; i++)
			{
				auto jsonObj = json_array_get_object(array, i);
				int x = json_object_get_number(jsonObj, "x") / 8;
				int y = -json_object_get_number(jsonObj, "y") / 8;
				callback(x, y, json_object_get_object(jsonObj, "customFields"));
			}
		};

		loadEntityType("PlayerSpawn", [&](int x, int y, JSON_Object* custom)
		{
			m_world.Create
			(
				Player({true}),
				Camera(),
				GridObject(x, y),
				MovingObject{x, y, 7},
				SpriteRenderer{{8,8}, getTexture("/Player.png"), {0, 0}}
			);
		});

		loadEntityType("Bed", [&](int x, int y, JSON_Object* custom) mutable
		{
			m_world.Create
			(
				GridObject(x, y),
				SpriteRenderer{{8,12}, getTexture("/Bed.png"), {0, 4}},
				Interactable{new InteractionCallback([=](tako::World& world, tako::Entity self, tako::Entity other) mutable
				{
					if (!world.HasComponent<Player>(other))
					{
						return false;
					}
					if (!world.GetComponent<Player>(other).human)
					{
						return false;
					}
					std::vector<DialogPart> dialog;
					dialog.push_back("I could really get some sleep.");
					if (Diary::IsDailyTasksDone())
					{

						dialog.push_back("I have done everything\nnecessary for today.");
						dialog.push_back("I could go to sleep or\nspend more precious time");
						dialog.push_back("with Peanut.\nWhile I still can.");
						//TODO: ask
						dialog.push_back([=]() mutable
						{
							AudioClips::Play("/Sleep.wav");
							Diary::StartNextDayRoutine([=]() mutable
							{
								newDay = true;
							});
						});
					}
					else
					{
						auto status = Diary::GetDayStatus();
						dialog.push_back("But I have still things to do:");
						if (!status.gaveFood)
						{
							dialog.push_back("Peanut needs something\nto munch on");
						}
						if (!status.gaveMedicine)
						{
							dialog.push_back("Peanut needs his medicine.");
						}
					}
					DialogSystem::StartDialog({std::move(dialog)});
					return true;
				})}
			);
		});

		loadEntityType("Dog", [&](int x, int y, JSON_Object* custom)
		{
			m_world.Create
			(
				GridObject(x, y),
				MovingObject{x, y, 2},
				Dog(),
				SpriteRenderer{{8,8}, getTexture("/Dog.png"), {0, 0}},
				Interactable{new InteractionCallback([&](tako::World& world, tako::Entity self, tako::Entity other)
				{
					DialogSystem::StartDialog(
					{
						"Oh, who is a good boy?!\nYes you are!",
						"Wufff, Wuf!",
						"No, you're the best"
					});
					return true;
				})},
				Appliable{[=](tako::World& world, tako::Entity self, tako::Entity other, InventoryItem item)
				{
					if (!world.HasComponent<Player>(other))
					{
						return false;
					}
					switch (item)
					{
						case InventoryItem::Bone:
						{
							DialogSystem::StartDialog(
							{
								"Awoooooooooooo!\n(A Bone!)",
								"(You're the best)",
								[=]()
								{
									auto& player = m_world.GetComponent<Player>(other);
									player.RemoveHeldItem(item);
									Diary::GaveFood();
								}
							});
							break;
						}
						case InventoryItem::DogFood:
						{
							DialogSystem::StartDialog(
							{
								"*munch* *munch* *slurp*",
								"(Yummy!)",
								[=]()
								{
									auto& player = m_world.GetComponent<Player>(other);
									player.RemoveHeldItem(item);
									Diary::GaveFood();
								}
							});
							break;
						}
						case InventoryItem::Stick:
						{
							DialogSystem::StartDialog(
							{
								"Wuff Grr!\n(A Stick!)",
								"(Let's play the next time\n we get to the park!)"
							});
							break;
						}
						case InventoryItem::Medicine:
						{
							DialogSystem::StartDialog(
							{
								"Slllurp",
								"Like a champ",
								[=]()
								{
									auto& player = m_world.GetComponent<Player>(other);
									player.RemoveHeldItem(item);
									Diary::GaveMedicine();
								}
							});
							break;
						}
					}
					return false;
				}}
			);
		});

		loadEntityType("Pickup", [&](int x, int y, JSON_Object* custom)
		{
			auto itemStr = json_object_get_string(custom, "Item");
			auto item = GetItemFromString(itemStr);
			SpawnItem(x, y, item);
		});

		loadEntityType("ItemSpawner", [&](int x, int y, JSON_Object* custom)
		{
			auto itemStr = json_object_get_string(custom, "Item");
			auto item = GetItemFromString(itemStr);
			int day = json_object_get_number(custom, "Day");
			m_world.Create
			(
				ItemSpawner{x, y, item, day}
			);
		});

		loadEntityType("ParkWarp", [&](int x, int y, JSON_Object* custom)
		{
			m_world.Create
			(
				GridObject(x, y),
				Interactable{new InteractionCallback([=](tako::World& world, tako::Entity self, tako::Entity other)
				{
					DialogSystem::NightTimeReading
					(
						{"Let's go to the Park"},
						[]() {SceneManager::LoadRoom("Park");}
					);
					return true;
				})}
			);
		});

		json_value_free(jsonRoot);
	}

	void ReplenishSpawns()
	{
		std::vector<ItemSpawner> spawner;
		m_world.IterateComps<ItemSpawner>([&](ItemSpawner& spawn)
		{
			if (spawn.day < 0 || spawn.day == Diary::GetDay())
			{
				spawner.push_back(spawn);
			}
		});
		for (auto s : spawner)
		{
			SafeSpawn(s.x, s.y, s.item);
		}
	}

	bool SafeSpawn(int x, int y, InventoryItem item)
	{
		std::optional<tako::Entity> col;
		m_world.IterateComps<tako::Entity, GridObject, Pickup>([&](tako::Entity ent, GridObject& grid, Pickup& pick)
		{
			if (grid.x == x && grid.y == y)
			{
				col = ent;
			}
		});
		if (!col)
		{
			SpawnItem(x, y, item);
			return true;
		}
		return false;
	}

	void SpawnItem(int x, int y, InventoryItem item)
	{
		m_world.Create
		(
			GridObject(x, y),
			Pickup{item},
			SpriteRenderer{{8,8}, m_getTexture(GetItemPic(item)), {0, 0}},
			Interactable{new InteractionCallback([=](tako::World& world, tako::Entity self, tako::Entity other) mutable
			{
				if (!m_world.HasComponent<Player>(other))
				{
					return false;
				}
				tako::Audio::Play(*AudioClips::Load("/Pickup.wav"));
				DialogSystem::StartDialog(
				{
					"Found " + GetItemName(item) + "!",
					[=]() mutable
					{
						auto& pickup = m_world.GetComponent<Pickup>(self);
						auto& inter = m_world.GetComponent<Interactable>(self);
						auto& player = m_world.GetComponent<Player>(other);
						player.items.push_back(pickup.item);
						auto ptr = inter.callback;
						m_world.Delete(self);
						delete ptr;
					}
				});
				return true;
			})}
		);
	}

	void Update(tako::Input* input, float dt)
	{
		static float lastBump = 0;
		lastBump += dt;
		if (newDay)
		{
			if (Diary::GetDay() != 7 || m_levelName == "House")
			{
				ReplenishSpawns();
				std::vector<DialogPart> dialog;
				for (auto s : Diary::GetDailyAgenda())
				{
					dialog.emplace_back(s);
				}
				DialogSystem::StartDialog(std::move(dialog));
			}
			else
			{
				LOG("Park");
			}

			newDay = false;
		}
		Tile playerPos;
		m_world.IterateComps<tako::Entity, GridObject, MovingObject, Player>([&](tako::Entity ent, GridObject& grid, MovingObject& move, Player& player)
		{
			playerPos = grid.GetTile();
			if (!DialogSystem::IsOpen())
			{
				if (!move.IsMoving(grid))
				{
					auto inventoryKey = input->GetKeyDown(tako::Key::Space) || input->GetKeyDown(tako::Key::Enter) || input->GetKeyDown(tako::Key::Gamepad_Start);
					auto interactionKey = input->GetKeyDown(tako::Key::L) || input->GetKeyDown(tako::Key::Gamepad_A);
					auto useKey = input->GetKeyDown(tako::Key::K) || input->GetKeyDown(tako::Key::Gamepad_B);
					if (player.inventoryOpen && inventoryKey)
					{
						player.inventoryOpen = false;
					}
					else if (!player.inventoryOpen && inventoryKey)
					{
						if (player.items.size() <= 0)
						{
							DialogSystem::StartDialog({"No items in inventory"});
						}
						else
						{
							player.inventoryOpen = true;
							player.inventorySlotSelection = 0;
						}
					}
					else if (player.inventoryOpen)
					{
						if (interactionKey)
						{
							player.inventoryOpen = false;
							std::swap(player.items[0], player.items[player.inventorySlotSelection]);
						}
						else
						{
							if (input->GetKeyDown(tako::Key::Left) || input->GetKeyDown(tako::Key::A) || input->GetKeyDown(tako::Key::Gamepad_Dpad_Left))
							{
								player.inventorySlotSelection = std::max(0, player.inventorySlotSelection - 1);
							}
							if (input->GetKeyDown(tako::Key::Right) || input->GetKeyDown(tako::Key::D) || input->GetKeyDown(tako::Key::Gamepad_Dpad_Right))
							{
								player.inventorySlotSelection = std::min((int)player.items.size() - 1, player.inventorySlotSelection + 1);
							}
						}
					}
					else if (interactionKey)
					{
						auto targetTile = grid.GetTile() + GetFaceDelta(player.facing);
						bool interacted = false;
						m_world.IterateComps<tako::Entity, GridObject, Interactable>([&](tako::Entity e, GridObject& g, Interactable& inter)
						{
							if (!interacted && inter.callback && g.GetTile() == targetTile)
							{
								interacted = inter.callback->operator()(m_world, e, ent);
							}
						});
					}
					else if (useKey)
					{
						if (player.items.size() <= 0)
						{
							auto targetTile = grid.GetTile() + GetFaceDelta(player.facing);
							bool interacted = false;
							m_world.IterateComps<tako::Entity, GridObject, Interactable, Pickup>([&](tako::Entity e, GridObject& g, Interactable& inter, Pickup& up)
							{
								if (!interacted && inter.callback && g.GetTile() == targetTile)
								{
									interacted = inter.callback->operator()(m_world, e, ent);
								}
							});
							if (!interacted)
							{
								DialogSystem::StartDialog({"This isn't the time to\nuse that !", "Nevermind, you have no bike,\nor like anything"});
							}
						}
						else
						{
							auto targetTile = grid.GetTile() + GetFaceDelta(player.facing);
							bool interacted = false;
							m_world.IterateComps<tako::Entity, GridObject, Appliable>([&](tako::Entity e, GridObject& g, Appliable& app)
							{
								if (!interacted && g.GetTile() == targetTile)
								{
									interacted = app.callback(m_world, e, ent, player.items[0]);
								}
							});
						}
					}
					else
					{
						//TODO: limit movement to one direction at a time
						int xDelta = 0;
						int yDelta = 0;
						if (input->GetKey(tako::Key::Left) || input->GetKey(tako::Key::A) || input->GetKey(tako::Key::Gamepad_Dpad_Left))
						{
							xDelta -= 1;
						}
						if (input->GetKey(tako::Key::Right) || input->GetKey(tako::Key::D) || input->GetKey(tako::Key::Gamepad_Dpad_Right))
						{
							xDelta += 1;
						}
						if (input->GetKey(tako::Key::Up) || input->GetKey(tako::Key::W) || input->GetKey(tako::Key::Gamepad_Dpad_Up))
						{
							yDelta += 1;
						}
						if (input->GetKey(tako::Key::Down) || input->GetKey(tako::Key::S) || input->GetKey(tako::Key::Gamepad_Dpad_Down))
						{
							yDelta -= 1;
						}
						if (xDelta != 0 && yDelta != 0)
						{
							if (player.facing == FaceDirection::Down || player.facing == FaceDirection::Up)
							{
								yDelta = 0;
							}
							else
							{
								xDelta = 0;
							}
						}
						if (xDelta != 0 || yDelta != 0)
						{
							if (xDelta == 1)
							{
								player.facing = FaceDirection::Right;
							}
							if (xDelta == -1)
							{
								player.facing = FaceDirection::Left;
							}
							if (yDelta == 1)
							{
								player.facing = FaceDirection::Up;
							}
							if (yDelta == -1)
							{
								player.facing = FaceDirection::Down;
							}
							auto col = Grid::Move(ent, xDelta, yDelta, m_world);
							if (!col)
							{
								AudioClips::Play("/Step.wav");
							}
							else if (lastBump >= 0.5f)
							{
								AudioClips::Play("/Bump.wav");
								lastBump = 0;
							}
						}
					}
				}
			}
		});

		m_world.IterateComps<tako::Entity, GridObject, MovingObject, Dog>([&](tako::Entity ent, GridObject& grid, MovingObject& move, Dog& dog)
		{
			/*
			if (!DialogSystem::IsOpen() && !move.IsMoving(grid) && rand() % 250 == 1)
			{
				bool mvX = rand() % 2;
				int sign = rand() % 2 ? 1 : -1;
				Grid::Move(ent, mvX ? sign : 0, mvX ? 0 : sign, m_world);
			}
			 */
			if (!move.IsMoving(grid))
			{
				auto target = Grid::GetNextTileToTarget(ent, playerPos, m_world);
				if (target)
				{
					auto t = target.value();
					Grid::Move(ent, t.x - move.targetX, t.y - move.targetY, m_world);
				}
			}
		});
		m_world.IterateComps<GridObject, MovingObject>([&](GridObject& grid, MovingObject& move)
		{
			if (move.targetX != grid.x)
			{
				grid.x += (move.targetX - grid.x) * move.gridSpeed * dt;
				if (tako::mathf::abs(grid.x - move.targetX) < EPSILON)
				{
					grid.x = move.targetX;
				}
			}
			if (move.targetY != grid.y)
			{
				grid.y += (move.targetY - grid.y) * move.gridSpeed * dt;
				if (tako::mathf::abs(grid.y - move.targetY) < EPSILON)
				{
					grid.y = move.targetY;
				}
			}
		});
	}

	void Draw(tako::OpenGLPixelArtDrawer* drawer, const Palette& palette)
	{
		drawer->SetTargetSize(240/2, 135/2);
		drawer->SetCameraPosition(GetCamera() - tako::Vector2(-4,4));

		for (auto layer : m_tilesPNG)
		{
			drawer->DrawImage(0, 0, layer->width, layer->height, layer->handle);
		}

		m_world.IterateComps<GridObject, RectangleRenderer>([&](GridObject& grid, RectangleRenderer& rect)
		{
			drawer->DrawRectangle(grid.x * 8, grid.y * 8, rect.size.x, rect.size.y, rect.color);
		});

		m_world.IterateComps<GridObject, SpriteRenderer>([&](GridObject& grid, SpriteRenderer& sp)
		{
			drawer->DrawImage(grid.x * 8 + sp.offset.x, grid.y * 8 + sp.offset.y, sp.size.x, sp.size.y, sp.sprite->handle);
		});

		m_world.IterateComps<Player>([&](Player& player)
		{
			if (player.items.size() <= 0)
			{
				return;
			}
			drawer->SetCameraPosition({0,0});
			if (!player.inventoryOpen)
			{
				tako::Vector2 topLeft{-drawer->GetCameraViewSize().x/2 + 1, drawer->GetCameraViewSize().y/2 - 1};
				drawer->DrawRectangle(topLeft.x, topLeft.y, 12, 12, palette[0]);
				topLeft += { 1, -1 };
				drawer->DrawRectangle(topLeft.x, topLeft.y, 10, 10, palette[3]);
				topLeft += { 1, -1 };
				drawer->DrawImage(topLeft.x, topLeft.y, 8, 8, m_getTexture(GetItemPic(player.items[0]))->handle);
				return;
			}

			auto totalWidth = player.items.size() * 14 + 2;
			tako::Vector2 topLeft{-(float)totalWidth/2, 6};
			for (int i = 0; i < player.items.size(); i++)
			{
				auto item = player.items[i];
				drawer->DrawRectangle(topLeft.x + 1, topLeft.y, 12, 12, i == player.inventorySlotSelection ? palette[1] : palette[0]);
				drawer->DrawRectangle(topLeft.x + 2, topLeft.y - 1, 10, 10, palette[3]);
				drawer->DrawImage(topLeft.x + 3, topLeft.y - 2, 8, 8, m_getTexture(GetItemPic(item))->handle);
				topLeft.x += 13;
			}
		});

	}


private:
	bool newDay = true;
	std::string m_levelName;
	tako::World m_world;
	std::vector<tako::Texture*> m_tilesPNG;
	GetTextureCallback m_getTexture;

	tako::Vector2 GetCamera()
	{
		tako::Vector2 pos;
		m_world.IterateComps<GridObject, Camera>([&](GridObject& grid, Camera& cam)
		{
			pos = { grid.x * 8, grid.y * 8};
		});

		return pos;
	}
};
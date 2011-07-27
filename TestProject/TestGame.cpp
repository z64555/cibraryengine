#include "../CibraryEngine/CibraryEngine.h"

#include "TestGame.h"
#include "TestScreen.h"
#include "HUD.h"
#include "Dood.h"
#include "DSNMaterial.h"
#include "GlowyModelMaterial.h"
#include "Sun.h"
#include "TerrainChunk.h"
#include "Weapon.h"
#include "AIController.h"

#include "../CibraryEngine/UnrealImport.h"

#include "ConverterWhiz.h"
#include "SoldierConverter.h"
#include "CrabBugConverter.h"

#include "AlienGun.h"
#include "CrabWeapon.h"
#include "DefaultWeapon.h"
#include "WorldBoundary.h"

#include "LevelLoad.h"

#include "MaterialLoader.h"

#include "Brambles.h"
#include "Corpse.h"
#include "StaticLevelGeometry.h"

#include "../CibraryEngine/DebugRenderer.h"

using namespace std;

namespace Test
{
	/*
	 * TestGame::Loader methods
	 */
	void TestGame::Loader::operator ()()
	{
		game->Load();
	}




	/*
	 * Struct to get all of the bots on a level
	 */
	struct BotGetter : public EntityQualifier
	{
		bool Accept(Entity* ent)
		{
			Dood* dood = dynamic_cast<Dood*>(ent);
			if(dood != NULL && dynamic_cast<AIController*>(dood->controller) != NULL)
				return true;
			else
				return false;
		}
	};




	DebugRenderer debug_renderer = DebugRenderer();
	/*
	 * TestGame methods
	 */
	TestGame::TestGame(TestScreen* screen, SoundSystem* sound_system) :
		screen(screen),
		mouse_motion_handler(this),
		bot_death_handler(this),
		player_death_handler(this),
		player_damage_handler(this),
		total_game_time(0),
		god_mode(false),
		debug_draw(false),
		alive(true),
		player_controller(NULL),
		player_pawn(NULL),
		debug_text(""),
		load_status(this)
	{
		this->sound_system = sound_system;
		content = screen->window->content;

		physics_world->dynamics_world->setDebugDrawer(&debug_renderer);
		debug_renderer.setDebugMode(btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_DrawConstraints);
	}

	void TestGame::Load()
	{
		sound_system->TryToEnable();

		font = content->Load<BitmapFont>("../Font");

		if(load_status.abort)
		{
			load_status.stopped = true;
			return;
		}

		if(load_status.abort)
		{
			load_status.stopped = true;
			return;
		}
		load_status.task = "dsn shader";

		// setting the content loader for materials...
		// TODO: delete this somewhere?
		content->SetHandler<Material>(new MaterialLoader(content));

		ScriptSystem::SetGS(this);

		ContentReqList content_req_list(content);

		ScriptSystem::SetContentReqList(&content_req_list);
		ScriptSystem::GetGlobalState().DoFile("Files/Scripts/load.lua");
		ScriptSystem::SetContentReqList(NULL);
		content_req_list.LoadContent(&load_status.task);

		if(content->Load<UberModel>("nbridge") == NULL)
		{
			load_status.task = "terrain mesh";

			// create terrain zzz file
			vector<MaterialModelPair> pairs;
			vector<string> material_names;
			MaterialModelPair pair;

			pair.material_index = 0;
			pair.vbo = new SkinVInfoVertexBuffer(*content->Load<VTNModel>("nbridge")->GetVBO());
			pairs.push_back(pair);
			material_names.push_back("stone");

			SkinnedModel* heightfield_skinny = new SkinnedModel(pairs, material_names, new Skeleton());
			UberModel* heightfield_model = UberModelLoader::CopySkinnedModel(heightfield_skinny);
			heightfield_model->bone_physics.push_back(UberModel::BonePhysics());
			heightfield_model->bone_physics[0].shape = ShapeFromSkinnedModel(heightfield_skinny);

			UberModelLoader::SaveZZZ(heightfield_model, "Files/Models/nbridge.zzz");

			content->GetMetadata<UberModel>(content->GetHandle<UberModel>("nbridge")).fail = false;
		}

		load_status.task = "sky";

		// creating sky
		Shader* sky_vertex_shader = content->Load<Shader>("sky-v");
		Shader* sky_fragment_shader = content->Load<Shader>("sky-f");
		sky_shader = new ShaderProgram(sky_vertex_shader, sky_fragment_shader);
		sky_shader->AddUniform<TextureCube>(new UniformTextureCube("sky_texture", 0));
		sky_shader->AddUniform<Mat4>(new UniformMatrix4("inv_view", true));
		sky_texture = content->Load<TextureCube>("sky_cubemap");
		sky_sphere = content->Load<VTNModel>("sky_sphere");

		sun_billboard = content->Load<VTNModel>("sun_billboard");
		sun_texture = content->Load<Texture2D>("sun");

		if(load_status.abort)
		{
			load_status.stopped = true;
			return;
		}
		load_status.task = "soldier";

		// Dood's model
		model = content->Load<UberModel>("soldier");

		if(model == NULL)
		{
			ConvertSoldier(content);

			UberModel* uber_model = UberModelLoader::CopySkinnedModel(content->Load<SkinnedModel>("soldier"));

			UberModel::Bone eye_bone;
			eye_bone.name = "eye";
			eye_bone.parent = 5;
			eye_bone.pos = Vec3(0, 1.88, -0.1);
			uber_model->bones.push_back(eye_bone);

			UberModel::Bone lgrip_bone;
			lgrip_bone.name = "l grip";
			lgrip_bone.parent = 9;
			lgrip_bone.pos = Vec3(0.546, 1.059, 0.016);
			lgrip_bone.ori = Quaternion::FromPYR(0, 0, 0.5) * Quaternion::FromPYR(M_PI * 0.5, 0, 0) * Quaternion::FromPYR(0, 0.1, 0);
			uber_model->bones.push_back(lgrip_bone);

			UberModel::Bone rgrip_bone;
			rgrip_bone.name = "r grip";
			rgrip_bone.parent = 16;
			rgrip_bone.pos = Vec3(-0.546, 1.059, 0.016);
			rgrip_bone.ori = Quaternion::FromPYR(0, 0, -0.5) * Quaternion::FromPYR(M_PI * 0.5, 0, 0) * Quaternion::FromPYR(0, -0.1, 0);
			uber_model->bones.push_back(rgrip_bone);

			for(unsigned int i = 0; i < uber_model->bones.size(); i++)
			{
				UberModel::BonePhysics phys;
				phys.pos = uber_model->bones[i].pos;
				phys.mass = 1.0;
				float radii[] = {0.2f};
				btVector3 centers[] = {btVector3(phys.pos.x, phys.pos.y, phys.pos.z)};
				phys.shape = new btMultiSphereShape(centers, radii, 1);
				uber_model->bone_physics.push_back(phys);
			}

			UberModelLoader::SaveZZZ(uber_model, "Files/Models/soldier.zzz");

			content->GetMetadata<UberModel>(content->GetHandle<UberModel>("soldier")).fail = false;
			model = content->Load<UberModel>("soldier");
		}

		Texture2D* mflash = content->Load<Texture2D>("mflash-d");
		Texture2D* shot = content->Load<Texture2D>("tracer-d");
		mflash_material = new GlowyModelMaterial(mflash);
		shot_material = new GlowyModelMaterial(shot);
		gun_model = content->Load<UberModel>("gun");
		mflash_model = content->Load<VTNModel>("mflash");
		shot_model = content->Load<VTNModel>("shot");

		if(load_status.abort)
		{
			load_status.stopped = true;
			return;
		}
		load_status.task = "crab bug";

		enemy = content->Load<UberModel>("crab_bug");
		if(enemy == NULL)
		{
			// if you want to re-convert the crab bug, simply delete the .zzz file to force conversion
			ConvertCrabBug(content);

			content->GetMetadata<UberModel>(content->GetHandle<UberModel>("crab_bug")).fail = false;
			enemy = content->Load<UberModel>("crab_bug");
		}

		if(load_status.abort)
		{
			load_status.stopped = true;
			return;
		}
		load_status.task = "misc";

		// loading weapon sounds
		//fire_sound = content->Load<SoundBuffer>("SFX_mk2_fire");
		fire_sound = content->Load<SoundBuffer>("shot");
		chamber_click_sound = content->Load<SoundBuffer>("SFX_gun_empty");
		reload_sound = content->Load<SoundBuffer>("SFX_mk2_reload");

		// loading particle materials...
		blood_particle = new ParticleMaterial(Texture3D::FromSpriteSheetAnimation(content->Load<Texture2D>("blood_splatter"), 32, 32, 4, 2, 7), Alpha);
		dirt_particle = new ParticleMaterial(Texture3D::FromSpriteSheetAnimation(content->Load<Texture2D>("dirt_impact"), 32, 32, 2, 2, 4), Alpha);

		if(load_status.abort)
		{
			load_status.stopped = true;
			return;
		}
		load_status.task = "level";

		LoadLevel(this, "TestLevel");

		if(load_status.abort)
		{
			load_status.stopped = true;
			return;
		}
		load_status.task = "starting game";

		// creating player
		SpawnPlayer(Vec3(0, GetTerrainHeight(0, 0) + 1, 0));

		screen->input_state->MouseMoved += &mouse_motion_handler;

		sun = new Sun(Vec3(0, 4, -2), Vec3(1, 1, 1), sun_billboard, sun_texture);

		hud = new HUD(this, screen->content);

		ScriptSystem::GetGlobalState().DoFile("Files/Scripts/game_start.lua");
		hud->SetPlayer(player_pawn);

		load_status.stopped = true;
	}

	Dood* TestGame::SpawnDood(Vec3 pos, UberModel* model)
	{
		Dood* dood = new Dood(this, model, pos);
		dood->yaw = Random3D::Rand(2.0 * M_PI);

		Spawn(dood);

		return dood;
	}

	Dood* TestGame::SpawnPlayer(Vec3 pos)
	{
		if(player_pawn != NULL)
			player_pawn->is_valid = false;
		if(player_controller != NULL)
			player_controller->is_valid = false;

		player_pawn = SpawnDood(pos, model);
		Spawn(player_pawn->equipped_weapon = new DefaultWeapon(this, player_pawn, gun_model, mflash_model, shot_model, mflash_material, shot_material, fire_sound, chamber_click_sound, reload_sound));

		player_pawn->OnDeath += &player_death_handler;
		player_pawn->OnDamageTaken += &player_damage_handler;

		player_controller = new PlayerController(this);
		player_controller->Possess(player_pawn);
		Spawn(player_controller);

		return player_pawn;
	}

	Dood* TestGame::SpawnBot(Vec3 pos)
	{
		Dood* dood = SpawnDood(pos, enemy);
		Spawn(dood->intrinsic_weapon = new CrabWeapon(this, dood));
		dood->hp *= 0.3;

		dood->OnDeath += &bot_death_handler;

		AIController* ai = new AIController(this);
		ai->Possess(dood);
		Spawn(ai);

		return dood;
	}

	unsigned int TestGame::GetNumberOfBots()
	{
		BotGetter getter = BotGetter();
		EntityList bots = GetQualifyingEntities(getter);

		return bots.Count();
	}

	TestGame::~TestGame() { Dispose(); ScriptSystem::SetGS(NULL); }

	void TestGame::Update(TimingInfo time)
	{
		float elapsed = min((float)time.elapsed, 0.1f);
		total_game_time += elapsed;
		elapsed_game_time = elapsed;

		TimingInfo clamped_time = TimingInfo(elapsed, total_game_time);

		hud->UpdateHUDGauges(clamped_time);

		if(elapsed > 0)
		{
			stringstream fps_counter_ss;
			fps_counter_ss << "FPS = " << (int)(1.0 / time.elapsed);
			debug_text = fps_counter_ss.str();
		}

		ScriptSystem::GetGlobalState().DoFile("Files/Scripts/update.lua");

		GameState::Update(clamped_time);
	}

	void TestGame::Draw(int width_, int height_)
	{
		GLErrorDebug(__LINE__, __FILE__);

		width = width_;
		height = height_;

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glViewport(0, 0, width, height);

		static CameraView camera(Mat4::Identity(), 1.0f, 1.0f);
		if(alive)
			camera = CameraView(((Dood*)player_controller->GetControlledPawn())->GetViewMatrix(), 2.0f, (float)width / height);
		Mat4 proj_t = camera.GetProjectionMatrix().Transpose();
		Mat4 view_t = camera.GetViewMatrix().Transpose();

		// TODO: find a better place for this sound-related code?
		sound_system->SetListenerPos(-camera.GetViewMatrix().TransformVec3(0, 0, 0, 1));
		sound_system->SetListenerUp(camera.GetViewMatrix().TransformVec3(0, 1, 0, 0));
		sound_system->SetListenerForward(camera.GetViewMatrix().TransformVec3(0, 0, 1, 0));

		GLErrorDebug(__LINE__, __FILE__);

		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(&proj_t.values[0]);

		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(&view_t.values[0]);

		if(debug_draw)
		{
			SceneRenderer renderer(&camera);
			DrawPhysicsDebuggingInfo(&renderer);
		}
		else
		{
			sun->view_matrix = camera.GetViewMatrix();
			DrawBackground(camera.GetViewMatrix().Transpose());

			GLErrorDebug(__LINE__, __FILE__);

			SceneRenderer renderer(&camera);

			for(list<Entity*>::iterator iter = entities.begin(); iter != entities.end(); iter++)
				(*iter)->Vis(&renderer);

			renderer.lights.push_back(sun);

			GLErrorDebug(__LINE__, __FILE__);

			renderer.Render();
			renderer.Cleanup();

			GLErrorDebug(__LINE__, __FILE__);

			for(list<Entity*>::iterator iter = entities.begin(); iter != entities.end(); iter++)
				(*iter)->VisCleanup();
		}

		hud->Draw(width, height);

		GLErrorDebug(__LINE__, __FILE__);
	}

	void TestGame::DrawPhysicsDebuggingInfo(SceneRenderer* renderer)
	{
		physics_world->dynamics_world->debugDrawWorld();
	}

	void TestGame::DrawBackground(Mat4 view_matrix)
	{
		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glDepthMask(false);

		GLErrorDebug(__LINE__, __FILE__);

		sky_shader->SetUniform<TextureCube>("sky_texture", sky_texture);
		GLErrorDebug(__LINE__, __FILE__);
		sky_shader->SetUniform<Mat4>("inv_view", &view_matrix);
		GLErrorDebug(__LINE__, __FILE__);
		ShaderProgram::SetActiveProgram(sky_shader);

		GLErrorDebug(__LINE__, __FILE__);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		sky_sphere->GetVBO()->Draw();

		glPopMatrix();

		ShaderProgram::SetActiveProgram(NULL);

		sun->Draw();

		glDepthMask(true);			// otherwise depth testing breaks
	}

	float TestGame::GetTerrainHeight(float x, float z)
	{
		// define a callback for when a ray intersects an object
		struct : btCollisionWorld::RayResultCallback
		{
			float result;

			btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace)
			{
				void* void_pointer = rayResult.m_collisionObject->getUserPointer();
				if(void_pointer != NULL)
				{
					StaticLevelGeometry* slg = dynamic_cast<StaticLevelGeometry*>((Entity*)void_pointer);
					if(slg != NULL)
					{
						float frac = rayResult.m_hitFraction;
						if(frac > result)
							result = frac;
					}
				}
				return 1;
			}
		} ray_callback;

		ray_callback.result = 0;

		// run that function for anything on this ray...
		float top = 1000;
		physics_world->dynamics_world->rayTest(btVector3(x, 0, z), btVector3(x, top, z), ray_callback);

		if(ray_callback.result >= 0)
			return ray_callback.result * top;
		else
			return 0;
	}

	void TestGame::InnerDispose()
	{
		delete hud; 

		screen->input_state->MouseMoved -= &mouse_motion_handler;

		GameState::InnerDispose();
	}

	void TestGame::VisUberModel(SceneRenderer* renderer, UberModel* model, int lod, Mat4 xform, SkinnedCharacter* character, vector<Material*>* materials)
	{
		if(model == NULL)
			return;

		vector<Material*> use_materials;
		if(materials != NULL)
			use_materials = *materials;
		else
		{
			for(unsigned int i = 0; i < model->materials.size(); i++)
				use_materials.push_back(content->Load<Material>(model->materials[i]));
		}

		UberModel::LOD* use_lod = model->lods[lod];

		Sphere bs = model->GetBoundingSphere();
		bs.center = xform.TransformVec3(bs.center, 1.0);

		vector<MaterialModelPair>* mmps = use_lod->GetVBOs();

		for(vector<MaterialModelPair>::iterator iter = mmps->begin(); iter != mmps->end(); iter++)
		{
			MaterialModelPair& mmp = *iter;

			DSNMaterial* material = (DSNMaterial*)use_materials[mmp.material_index];
			SkinVInfoVertexBuffer* vbo = (SkinVInfoVertexBuffer*)mmp.vbo;

			if(character != NULL)
			{
				DSNMaterialNodeData* node_data = new DSNMaterialNodeData(vbo, xform, bs, character->GetBoneMatrices(), character->skeleton->bones.size());
				renderer->objects.push_back(RenderNode(material, node_data, Vec3::Dot(renderer->camera->GetForward(), bs.center)));
			}
			else
			{
				DSNMaterialNodeData* node_data = new DSNMaterialNodeData(vbo, xform, bs);
				renderer->objects.push_back(RenderNode(material, node_data, Vec3::Dot(renderer->camera->GetForward(), bs.center)));
			}
		}
	}

	int gs_spawnBot(lua_State* L);
	int gs_spawnPlayer(lua_State* L);
	int gs_getTerrainHeight(lua_State* L);
	int gs_getNumberOfBots(lua_State* L);
	int gs_getDoodsList(lua_State* L);

	void TestGame::SetupScripting(ScriptingState& state)
	{
		GameState::SetupScripting(state);

		lua_State* L = state.GetLuaState();

		lua_pushlightuserdata(L, (void*)this);
		lua_pushcclosure(L, gs_spawnBot, 1);
		lua_setfield(L, 1, "spawnBot");

		lua_pushlightuserdata(L, (void*)this);
		lua_pushcclosure(L, gs_spawnPlayer, 1);
		lua_setfield(L, 1, "spawnPlayer");

		lua_pushlightuserdata(L, (void*)this);
		lua_pushcclosure(L, gs_getTerrainHeight, 1);
		lua_setfield(L, 1, "getTerrainHeight");

		lua_pushlightuserdata(L, (void*)this);
		lua_pushcclosure(L, gs_getNumberOfBots, 1);
		lua_setfield(L, 1, "getNumberOfBots");

		lua_pushlightuserdata(L, (void*)this);
		lua_pushcclosure(L, gs_getDoodsList, 1);
		lua_setfield(L, 1, "getDoodsList");
	}




	/*
	 * TestGame::MouseMotionHandler methods
	 */
	TestGame::MouseMotionHandler::MouseMotionHandler(TestGame* game) : game(game) { }

	void TestGame::MouseMotionHandler::HandleEvent(Event* evt)
	{
		/*
		MouseMotionEvent* mm_evt = (MouseMotionEvent*)evt;
		if(game->player_controller !=  NULL)
		{
			ControlState* control_state = game->player_controller->GetControlState();
			if(control_state != NULL)
			{
				(*control_state)[Dood::Yaw] = (*control_state)[Dood::Yaw] + mm_evt->dx * 0.005f;
				(*control_state)[Dood::Pitch] = (*control_state)[Dood::Pitch] + mm_evt->dy * 0.005f;
			}
		}
		*/
	}




	/*
	 * TestGame::DoodDeathHandler methods
	 */
	TestGame::DoodDeathHandler::DoodDeathHandler(TestGame* game) : game(game) { }

	void TestGame::DoodDeathHandler::HandleEvent(Event* evt) { Dood::DeathEvent* de = (Dood::DeathEvent*)evt; if(de->dood == game->player_pawn) { game->alive = false; } }




	/*
	 * TestGame::PlayerDamageHandler methods
	 */
	TestGame::PlayerDamageHandler::PlayerDamageHandler(TestGame* game) : game(game) { }

	void TestGame::PlayerDamageHandler::HandleEvent(Event* evt)
	{
		Dood::DamageTakenEvent* d_evt = (Dood::DamageTakenEvent*)evt;
		if(game->god_mode)
			d_evt->cancel = true;
	}




	/*
	 * TestGame scripting stuff
	 */
	int gs_spawnBot(lua_State* L)
	{
		int n = lua_gettop(L);
		if(n == 1 && lua_isuserdata(L, 1))
		{
			void* ptr = lua_touserdata(L, 1);
			Vec3* vec = dynamic_cast<Vec3*>((Vec3*)ptr);

			lua_settop(L, 0);

			if(vec != NULL)
			{
				lua_pushvalue(L, lua_upvalueindex(1));
				TestGame* gs = (TestGame*)lua_touserdata(L, 1);
				Dood* dood = gs->SpawnBot(*vec);
				lua_pop(L, 1);

				PushDoodHandle(L, dood);

				return 1;
			}
		}

		Debug("gs.spawnBot takes exactly 1 argument, a position vector; returning nil\n");
		return 0;
	}

	int gs_spawnPlayer(lua_State* L)
	{
		int n = lua_gettop(L);
		if(n == 1 && lua_isuserdata(L, 1))
		{
			void* ptr = lua_touserdata(L, 1);
			Vec3* vec = dynamic_cast<Vec3*>((Vec3*)ptr);

			lua_settop(L, 0);

			if(vec != NULL)
			{
				lua_pushvalue(L, lua_upvalueindex(1));
				TestGame* gs = (TestGame*)lua_touserdata(L, 1);
				lua_pop(L, 1);

				PushDoodHandle(L, gs->SpawnPlayer(*vec));

				return 1;
			}
		}

		Debug("gs.spawnPlayer takes exactly 1 argument, a position vector; returning nil\n");
		return 0;
	}

	int gs_getTerrainHeight(lua_State* L)
	{
		int n = lua_gettop(L);
		if(n == 2 && lua_isnumber(L, 1) && lua_isnumber(L, 2))
		{
			float x = lua_tonumber(L, 1);
			float z = lua_tonumber(L, 2);

			lua_settop(L, 0);
			lua_pushvalue(L, lua_upvalueindex(1));
			TestGame* gs = (TestGame*)lua_touserdata(L, 1);
			lua_pop(L, 1);

			lua_pushnumber(L, gs->GetTerrainHeight(x, z));

			return 1;
		}

		Debug("gs.getTerrainHeight takes exactly 2 arguments, an x and z coordinate; returning nil\n");
		return 0;
	}

	int gs_getNumberOfBots(lua_State* L)
	{
		int n = lua_gettop(L);
		if(n == 0)
		{
			lua_settop(L, 0);
			lua_pushvalue(L, lua_upvalueindex(1));
			TestGame* gs = (TestGame*)lua_touserdata(L, 1);
			lua_pop(L, 1);

			lua_pushnumber(L, gs->GetNumberOfBots());

			return 1;
		}

		Debug("gs.getTerrainHeight doesn't take any arguments; returning nil\n");
		return 0;
	}

	int gs_getDoodsList(lua_State* L)
	{
		int n = lua_gettop(L);
		if(n == 0)
		{
			lua_settop(L, 0);
			lua_pushvalue(L, lua_upvalueindex(1));
			TestGame* gs = (TestGame*)lua_touserdata(L, 1);
			lua_pop(L, 1);

			struct : EntityQualifier { bool Accept(Entity* ent) { return dynamic_cast<Dood*>(ent) != NULL; } } doods_only;
			EntityList e = gs->GetQualifyingEntities(doods_only);

			// begin table
			lua_newtable(L);							// push; top = 1

			for(unsigned int i = 0; i < e.Count(); i++)
			{
				lua_pushnumber(L, i + 1);
				PushDoodHandle(L, (Dood*)e[i]);
				lua_settable(L, 1);
			}

			return 1;
		}

		Debug("gs.getDoodsList doesn't take any arguments; returning nil\n");
		return 0;
	}
}
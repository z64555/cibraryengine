#pragma once

#include "StdAfx.h"

#include "Team.h"

namespace Test
{
	using namespace CibraryEngine;

	class TestScreen;
	class HUD;
	class Dood;
	class Soldier;
	class Sun;
	class Rubbish;

	class TestGame : public GameState
	{
		private:

			struct Imp;
			Imp* imp;

			TestScreen* screen;

		protected:

			void InnerDispose();

		public:

			static Team human_team;
			static Team bug_team;

			bool nav_editor;
			bool god_mode;
			bool debug_draw;

			bool quit;

			BitmapFont* font;

			int width, height;

			HUD* hud;

			ScriptedController* player_controller;
			//Soldier* player_pawn;
			Dood* player_pawn;

			string debug_text;

			float chapter_text_start, chapter_text_end;
			string chapter_text, chapter_sub_text;

			unsigned int nav_graph;

			Cache<Texture2D>* tex2d_cache;
			Cache<VertexBuffer>* vtn_cache;
			Cache<UberModel>* ubermodel_cache;
			Cache<Material>* mat_cache;
			Cache<ModelPhysics>* mphys_cache;

			struct Loader : public Disposable
			{
				private:

					struct Imp;
					Imp* imp;

				protected:

					void InnerDispose();

				public:

					TestGame* game;

					string task;

					Loader();

					void operator ()();

					bool HasStopped();
					bool HasAborted();
					void Stop();
					void Abort();
			} load_status;

			TestGame(TestScreen* screen, SoundSystem* sound_system);
			~TestGame();

			void Load();

			Dood* SpawnPlayer(Vec3 pos);
			Dood* SpawnBot(Vec3 pos);
			Dood* SpawnArtilleryBug(Vec3 pos);
			Dood* SpawnRobotArm(Vec3 pos);
			Dood* SpawnRobotTripod(Vec3 pos);
			unsigned int GetNumberOfBugs();

			Rubbish* SpawnRubbish(Vec3 pos);

			void Update(TimingInfo time);

			// Drawing-related functions...
			void Draw(int width, int height);

			void ShowChapterText(string title, string subtitle, float duration);

			float GetTerrainHeight(float x, float z);

			void SetupScripting(ScriptingState& state);
	};
}
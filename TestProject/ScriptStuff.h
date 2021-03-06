#pragma once
#include "StdAfx.h"

namespace Test
{
	using namespace CibraryEngine;

	class TestGame;
	class ContentReqList;

	struct ScriptStuff
	{
		static void Init();
		static void Shutdown();

		static ScriptingState GetGlobalState();
		static void SetGS(TestGame* gs);
		static void SetContentReqList(ContentReqList* req);
	};
}

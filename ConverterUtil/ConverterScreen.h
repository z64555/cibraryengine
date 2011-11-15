#pragma once

#include "StdAfx.h"

using namespace CibraryEngine;

namespace ConverterUtil
{
	/**
	 * Screen for converting models
	 *
	 * Convert UnrealEngine's psk/psa models to UberModel (zzz) format
	 * Convert obj models
	 *
	 * Edit collision shapes
	 */
	class ConverterScreen : public ProgramScreen
	{
		private:

			struct Imp;
			Imp* imp;

		public:

			ConverterScreen(ProgramWindow* win);
			~ConverterScreen();

			void Activate();
			void Deactivate();

			void Draw(int width, int height);
			ProgramScreen* Update(TimingInfo time);	
	};
}

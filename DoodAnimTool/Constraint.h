#pragma once

#include "StdAfx.h"

namespace DoodAnimTool
{
	class DATKeyframe;

	class Constraint
	{
		public:

			virtual ~Constraint() { }

			virtual float GetErrorAmount(const DATKeyframe& pose) = 0;
			virtual bool  SetLockedBones(DATKeyframe& pose, bool* locked_bones) { return false; }
	};
}

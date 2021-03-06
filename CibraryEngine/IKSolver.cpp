#include "StdAfx.h"

#include "IKSolver.h"
#include "SkeletalAnimation.h"

namespace CibraryEngine
{
	/*
	 * IKSolver::IKObject methods
	 */
	IKSolver::IKObject::IKObject(Skeleton* skeleton, Vec3 pos, Quaternion ori) :
		result_valid(false),
		skeleton(skeleton),
		desired_pos(pos),
		result_pos(pos),
		desired_ori(ori),
		result_ori(ori)
	{
	}

	void IKSolver::IKObject::ComputeNextState(PhysicsWorld* physics, TimingInfo time)
	{
		result = Skeleton(skeleton);
		result_valid = true;

		// TODO: implement this
	}

	void IKSolver::IKObject::ApplyComputedState()
	{
		// for now, result pos will just be the same as desired pos
		result_pos = desired_pos;
		result_ori = desired_ori;
		
		if(result_valid)
		{
			// copy result to skeleton
			unordered_map<unsigned int, Bone*> result_bones;
			for(vector<Bone*>::iterator iter = result.bones.begin(); iter != result.bones.end(); ++iter)
				result_bones[(*iter)->name] = *iter;

			vector<Bone*>& skeleton_bones =  skeleton->bones;
			for(vector<Bone*>::iterator iter = skeleton_bones.begin(); iter != skeleton_bones.end(); ++iter)
			{
				Bone* bone_i = *iter;
				
				unordered_map<unsigned int, Bone*>::iterator found = result_bones.find(bone_i->name);
				if(found != result_bones.end())
				{
					Bone* bone_f = found->second;

					bone_i->pos = bone_f->pos;
					bone_i->ori = bone_f->ori;
				}
			}

			result_valid = false;
		}
	}




	/*
	 * IKSolver methods
	 */
	IKSolver::IKSolver(PhysicsWorld* physics) : ik_objects(), physics(physics) { }

	void IKSolver::InnerDispose() { ClearObjects(); }

	void IKSolver::ClearObjects()
	{
		for(unordered_map<void*, IKObject*>::iterator iter = ik_objects.begin(); iter != ik_objects.end(); ++iter)
		{
			IKObject* obj = iter->second;
			if(obj->skeleton != NULL)
				delete obj->skeleton;
			delete obj;
		}
		ik_objects.clear();
	}

	void IKSolver::AddObject(void* user_ptr, Skeleton* skeleton, Vec3 pos, Quaternion ori)
	{
		ik_objects[user_ptr] = new IKObject(skeleton, pos, ori);
	}

	void IKSolver::DeleteObject(void* user_ptr)
	{
		unordered_map<void*, IKObject*>::iterator found = ik_objects.find(user_ptr);
		if(found != ik_objects.end())
			ik_objects.erase(found);
	}

	Skeleton* IKSolver::GetObjectSkeleton(void* user_ptr)
	{
		unordered_map<void*, IKObject*>::iterator found = ik_objects.find(user_ptr);
		if(found != ik_objects.end())
			return found->second->skeleton;
		else
			return NULL;
	}

	void IKSolver::SetDesiredState(void* user_ptr, Vec3 pos, Quaternion ori)
	{
		unordered_map<void*, IKObject*>::iterator found = ik_objects.find(user_ptr);
		if(found != ik_objects.end())
		{
			found->second->desired_pos = pos;
			found->second->desired_ori = ori;
		}
	}

	void IKSolver::GetResultState(void* user_ptr, Vec3& pos, Quaternion& ori)
	{
		unordered_map<void*, IKObject*>::iterator found = ik_objects.find(user_ptr);
		if(found != ik_objects.end())
		{
			pos = found->second->result_pos;
			ori = found->second->result_ori;
		}
	}

	void IKSolver::Update(TimingInfo time)
	{
		unordered_map<void*, IKObject*>::iterator iter;

		for(iter = ik_objects.begin(); iter != ik_objects.end(); ++iter)
			iter->second->ComputeNextState(physics, time);

		for(iter = ik_objects.begin(); iter != ik_objects.end(); ++iter)
			iter->second->ApplyComputedState();
	}
}

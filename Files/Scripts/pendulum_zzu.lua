--Common routines
local Vec3      = ba.createVector

--Common constants
  --Pendulums are a single chain (leg) of one ore more links (bones)
local n_bones = 1               -- number of bones within the leg
local bone_length = 1.00        -- How long each bone is
local carapace_radius = 0.15    -- Radius of the carapace/body
local Carapace_offset = Vec3(0, carapace_radius, 0)

local min_extents = Vec3(-3, 0, 0)
local max_extents = Vec3( 3, 0, 0)



--Model stuff
local U = dofile("Files/Scripts/ubermodel_util.lua")

local models = { { model = "nu_crab", material = "nu_crab" } }

local bones = { }

-- Add carapace
U.add_single_bone(bones, "carapace", nil, ba.createVector(0.0, n_bones * bone_length, 0.0))

-- Procedurally add bones, starting from the carapace
for i = 1, n_bones do
	local bname = "leg a " .. i  -- This bone's name
	local bpname = nil           -- Parent bone's name
	
	if (i == 1) then
		bpname = "carapace"
	else
		bpname = "leg a " .. (i-1)
	end

	U.add_single_bone(bones, bname, bpname, ba.createVector( 0.0, (n_bones - i) * bone_length, 0.0))
end

-- Add eye point
U.add_single_bone(bones, "eye", ("leg a " .. n_bones), ba.createVector(0, (n_bones * bone_length) + 0.2, 0))

ba.saveUberModel(models, bones, "pendulum")



--Physics Stuff
local P = dofile("Files/Scripts/modelphysics_util.lua")
local Sphere    = P.sphere
local Bone      = P.add_single_bone
local SymBones  = P.add_symmetric_bones
local Joint     = P.add_single_joint
local SymJoints = P.add_symmetric_joints

local b = { }
local j = { }


-- Add carapace collision sphere (cs)
Bone(b, "carapace", 64.0, {Sphere(0.0, n_bones * bone_length, 0.0, carapace_radius)})

-- Procedurally add cs's for each bone
for i = 1, n_bones do
  --local bname = "leg a " .. i
  local bname = "leg a " .. i -- This bone's name
  local bpname = nil          -- Parent bone's name

  if ( i == 1 ) then
	bpname = "carapace"
  else
	bpname = "leg a " .. (i - 1)
  end

  local axes = {Vec3(0, 0, bone_length), Vec3(0, 0, bone_legth)}
  local spheres = {
    { center = Vec3(0, (n_bones - i + 1) * bone_length, 0), radius = 0.05  },
	{ center = Vec3(0, (n_bones - i) * bone_length, 0), radius = 0.01 }
  }

  Bone(b, bname, 12.0, spheres)
  -- P.add_single_joint = function(add_to, bones_list, parent, child, pos, name, axes, min_extents, max_extents)
  Joint(j, b, bpname, bname, spheres[1].center, nil, axes, min_extents, max_extents )
end

ba.saveModelPhysics(b, j, "pendulum")
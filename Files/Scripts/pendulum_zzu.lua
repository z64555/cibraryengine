--Common routines
local Vec3      = ba.createVector

--Common constants
  --Pendulums are a single chain (leg) of one ore more links (bones)
local n_bones = 3               -- number of bones within the leg
local bone_length = 1.00        -- How long each bone is
local carapace_radius = 0.15    -- Radius of the carapace/body
local Carapace_offset = Vec3(0, carapace_radius, 0)

local min_extents = Vec3(-3, 0, 0)
local max_extents = Vec3( 3, 0, 0)



--Model stuff
local U = dofile("Files/Scripts/ubermodel_util.lua")
local uBone = U.add_single_bone
local uSymBones = U.add_symmetric_bones

local models = { { model = "nu_crab", material = "nu_crab" } }

local bones = { }

-- Add carapace
--add_single_bone(add_to, name, parent_name, pos, ori)
uBone(bones, "carapace", nil, ba.createVector(0.0, n_bones * bone_length, 0.0))

-- Procedurally add bones, starting from the carapace
for i = 1, n_bones do
	local bname = "leg a " .. i  -- This bone's name
	local bpname = nil           -- Parent bone's name
	
	if (i == 1) then
		bpname = "carapace"
	else
		bpname = "leg a " .. (i-1)
	end

	uBone(bones, bname, bpname, ba.createVector( 0.0, (n_bones - i) * bone_length, 0.0))
end

-- Add eye point
uBone(bones, "eye", ("leg a " .. n_bones), ba.createVector(0, (n_bones * bone_length) + 0.2, 0))

ba.saveUberModel(models, bones, "pendulum")



--Physics Stuff
local P = dofile("Files/Scripts/modelphysics_util.lua")
local pSphere    = P.sphere
local pBone      = P.add_single_bone
local pSymBones  = P.add_symmetric_bones
local pJoint     = P.add_single_joint
local pSymJoints = P.add_symmetric_joints

local b = { }
local j = { }


-- Add carapace collision sphere (cs)
pBone(b, "carapace", 64.0, {pSphere(0.0, n_bones * bone_length, 0.0, carapace_radius)})

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

  pBone(b, bname, 12.0, spheres)
  -- P.add_single_joint = function(add_to, bones_list, parent, child, pos, name, axes, min_extents, max_extents)
  pJoint(j, b, bpname, bname, spheres[1].center, nil, axes, min_extents, max_extents )
end

ba.saveModelPhysics(b, j, "pendulum")
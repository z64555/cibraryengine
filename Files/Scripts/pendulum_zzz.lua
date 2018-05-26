
local U = dofile("Files/Scripts/ubermodel_util.lua")

local models = { { model = "nu_crab", material = "nu_crab" } }

local bones = { }

--Form a single chain/leg of one or more links/bones
local n_bones = 1        -- number of bones within the leg
local bone_length = 1.00 -- How long each bone is

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
U.add_single_bone(bones, "eye", "carapace", ba.createVector(0, (n_bones * bone_length) + 0.2, 0))

ba.saveUberModel(models, bones, "pendulum")

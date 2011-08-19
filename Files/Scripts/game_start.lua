
-- function to be called when the player dies
function player_death(dood)
	local retry_text = "Press Esc to return to the main menu"
	local you_died_pre = "You died on wave "
	local you_died_post = "!"
	gs.showChapterText("GAME OVER!", retry_text .. "\n\n" .. you_died_pre .. level .. you_died_post, -1.0)
end

-- figure out which dood is the player, and remember that
player = gs.spawnPlayer(ba.createVector(8.3, 149.2 + 1, 69.5))
player.setAI(player_ai)
player.setDeath(player_death)

dood_properties = {}

poll_mouse_motion()

level = 0
kills = 0
kills_this_level = 0

bot_spawn_timer = 0

bots_spawned = 0

god_toggle = false
nav_edit_toggle = false
debug_draw_toggle = false

game_over = false

gs.setGodMode(god_mode)
gs.setNavEditMode(nav_edit_mode)
gs.setDebugDrawMode(debug_draw_mode)

dofile("Files/Scripts/goals.lua")

-- the crab bugs' ai
function crab_bug_ai(dood)

	local props = dood_properties[dood]

	-- select a target
	local my_pos = dood.getPosition()

	local dood_list = gs.getDoodsList()

	local target = nil
	for i, ent in ipairs(dood_list) do
		if ent.isPlayer() and ent ~= dood then
			target = ent
		end
	end

	-- reset certain parts of the control state...
	local control_state = dood.getControlState()
	control_state.primary_fire = false
	control_state.forward = 0
	control_state.leap = false

	local yaw = dood.getYaw()
	local forward = ba.createVector(-math.sin(yaw), 0, math.cos(yaw))
	local rightward = ba.createVector(-forward.z, 0, forward.x)

	local nav = gs.getNearestNav(my_pos)
	props.nav = nav

	-- handle moving into a neighboring nav point's area
	local path_search = props.path_search
	local path = props.path

	-- engage target, if applicable
	if target then

		local target_nav = gs.getNearestNav(target.getPosition())
		local target_thing = { pos = target.getPosition(), nav = nil }

		if not props.goal or (target_thing.pos - my_pos).length > 10 then
			if not path_search or path_search.target ~= target_nav then
				path = nil
				path_search = gs.newPathSearch(nav, target_nav)
			end

			if not path_search.finished then
				path_search.think(5)
				if path_search.finished then
					path = path_search.solution
					props.goal = goal_follow_path(dood, path)
					props.goal.activate()
				end
			end

			props.path_search = path_search
			props.path = path

		end
	end

	if props.goal and props.goal.status == GoalStatus.ACTIVE then
		props.goal.process()
	end

	do_steering_behavior(dood, control_state)
end

-- function called every time a bug dies
function crab_bug_death(dood)
	if not game_over then
		kills_this_level = kills_this_level + 1
		kills = kills + 1
	end
end


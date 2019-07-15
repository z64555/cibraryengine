#./TestProject/CMakeLists.txt

set(TEST_SOURCES
	ArtilleryBug.cpp
	ArtilleryBug.h
	ArtilleryWeapon.cpp
	ArtilleryWeapon.h
	BetterBrain.cpp
	BetterBrain.h
	Brambles.cpp
	Brambles.h
	CBone.cpp
	CBone.h
	CJoint.cpp
	CJoint.h
	CMakeLists.txt
	ConverterWhiz.cpp
	ConverterWhiz.h
	CrabBug.cpp
	CrabBug.h
	CrabWeapon.cpp
	CrabWeapon.h
	Credits.cpp
	Credits.h
	Damage.cpp
	Damage.h
	DefaultWeapon.cpp
	DefaultWeapon.h
	Dood.cpp
	Dood.h
	DynamicBrain.h
	ExperimentalScreen.cpp
	ExperimentalScreen.h
	GAExperiment.cpp
	GAExperiment.h
	GhostCamera.cpp
	GhostCamera.h
	Gun.cpp
	Gun.h
	HUD.cpp
	HUD.h
	InstructionsScreen.cpp
	InstructionsScreen.h
	JetpackNozzle.cpp
	JetpackNozzle.h
	LevelLoad.cpp
	LevelLoad.h
	LoadingScreen.cpp
	LoadingScreen.h
	Main.cpp
	MainMenu.cpp
	MainMenu.h
	MultiLayerBrain.cpp
	MultiLayerBrain.h
	NeuralNet.cpp
	NeuralNet.h
	Particle.cpp
	Particle.h
	Pendulum.cpp
	Pendulum.h
	PoseAimingGun.cpp
	PoseAimingGun.h
	RocketBug.cpp
	RocketBug.h
	Rubbish.cpp
	Rubbish.h
	ScaledIOBrain.cpp
	ScaledIOBrain.h
	Shootable.h
	Shot.cpp
	Shot.h
	Soldier.cpp
	Soldier.h
	Spark.cpp
	Spark.h
	StaticLevelGeometry.cpp
	StaticLevelGeometry.h
	StdAfx.cpp
	StdAfx.h
	Sun.cpp
	Sun.h
	Team.cpp
	Team.h
	TestGame.cpp
	TestGame.h
	TestScreen.cpp
	TestScreen.h
	Tripod.cpp
	Tripod.h
	WalkPose.cpp
	WalkPose.h
	Weapon.cpp
	Weapon.h
	WeaponEquip.cpp
	WeaponEquip.h
	WeaponIntrinsic.cpp
	WeaponIntrinsic.h
	WorldBoundary.cpp
	WorldBoundary.h
)
## Resources (scripts, artwork, etc) directories
include_directories("${PROJECT_SOURCE_DIR}/Files")

## Third party libs an utils directores
include_directories("${PROJECT_SOURCE_DIR}/build/include")          #For OpenAL and OpenGL include files

#Boost
find_package(Boost REQUIRED COMPONENTS date_time)
include_directories(${Boost_INCLUDE_DIRS})

#OpenGL
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

#SOIL
include_directories(${SOIL_ROOT}/src)

#LUA
add_library(lua SHARED IMPORTED GLOBAL)
set_property(TARGET lua PROPERTY IMPORTED_LOCATION ${LUA_LIB})

include_directories(${LUA_INCLUDE})
## Add target

add_executable(TestProject ${TEST_SOURCES})

target_include_directories(TestProject PUBLIC CibraryEngine)
target_link_libraries(TestProject PUBLIC CibraryEngine)

target_link_directories(TestProject PUBLIC ${Boost_LIBRARY_DIRS})
target_link_libraries(TestProject PUBLIC ${Boost_LIBRARIES})

target_link_directories(TestProject PUBLIC LUA_LIB)
target_link_libraries(TestProject PUBLIC lua51)
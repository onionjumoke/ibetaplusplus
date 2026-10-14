#pragma once

#include "PlayerController.h"

// net.minecraft.src.PlayerControllerTest
class PlayerControllerTest : public PlayerController
{
public:
	PlayerControllerTest(Minecraft *minecraft);

	void func_6473_b(EntityPlayer *entityplayer) override;
	bool shouldDrawHUD() override;
	void func_717_a(World *world) override;
	void updateController() override;
};


// NOTCH PEDAZO DE PELOTUDOP
// PARA QUE MIERDA PONES CONTROLLER SUPPORT SI NI EN JAVA EN 2026 ANDA
// ??? CONTROLLER TEST TODO STUB ESTA
// HACETE COGER CULIADO ARRE CORDOBES
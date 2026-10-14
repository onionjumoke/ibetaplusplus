#pragma once

// net.minecraft.src.MouseFilter
class MouseFilter
{
public:
	MouseFilter();

	float func_22386_a(float f, float f1);
	float smooth(float f, float f1) { return func_22386_a(f, f1); }

private:
	float field_22388_a;
	float field_22387_b;
	float field_22389_c;
};

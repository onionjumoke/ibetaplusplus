#pragma once

class ModelRenderer;

// net.minecraft.src.SignModel
class SignModel
{
public:
	SignModel();

	void func_887_a();
	void renderSign() { func_887_a(); }

	ModelRenderer *signBoard;
	ModelRenderer *signStick;
};

#ifndef SHADERLIB_H
#define SHADERLIB_H

class ShaderLib
{
public:

	ShaderLib() {}

	static const char* GetVertexShaderPNT();
	static const char* GetFragmentShaderPNT();

	static const char* GetVertexShaderPNC();
	static const char* GetFragmentShaderPNC();

	static const char* GetVertexShaderPC();
	static const char* GetFragmentShaderPC();

	static const char* GetVertexShaderPCT();
	static const char* GetFragmentShaderPCT();

	static const char* GetVertexShaderSS();
	static const char* GetFragmentShaderSS();

	static const char* GetVertexShaderSSCT();
	static const char* GetFragmentShaderSSCT();

	static const char* GetVertexShaderImage();
	static const char* GetFragmentShaderImage();

	static const char* GetVertexShaderSkyBox();
	static const char* GetFragmentShaderSkyBox();

	static const char* GetVertexShaderTerrain();
	static const char* GetFragmentShaderTerrain();

	static const char* GetVertexShaderWater();
	static const char* GetFragmentShaderWater();

	static const char* GetVertexShaderWaterJC();
	static const char* GetFragmentShaderWaterJC();
};

#endif // #ifndef SHADERLIB_H


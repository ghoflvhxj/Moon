#pragma once
#ifndef __CONSTANT_BUFFER_STRUCT_H__

#ifdef __cplusplus
	#define NameSpaceBegin(x) namespace x {
	#define NameSpaceEnd }
#else
	#define NameSpace(x)
	#define ConstantBuffer : register(b0)
#endif


NameSpaceBegin(ConstantBufferStruct)
	NameSpaceBegin(VertexStruct)
		struct WorldViewMatrix
		{
		public:
			WorldViewMatrix(const Mat4 &worldMatrix, const Mat4 &worldViewMatrix, const Mat4 &projectionMatrix);
			~WorldViewMatrix() = default;

		public:
			void update();

		public:
			Mat4 _worldMatrix;
			Mat4 _worldViewMatrix;
			Mat4 _projectionMatrix;
		};

		// 애니메이션에 사용됨
		struct JointMatrices
		{
			Mat4 jointMatrixList[200];
		};
	NameSpaceEnd

	NameSpaceBegin(PixelStruct)
		//__declspec(align(16)) struct TextureFlag
		//{
		//	//bool checkTextureFlag(TextureFlag flag);

		//	//uint32 _textureFlag;
		//};

		__declspec(align(16)) struct TextureInfo
		{
			TextureInfo()
				: usingNormalTexture(false), usingSepcuarTexture(false)
			{

			}

			bool usingNormalTexture;
			bool usingSepcuarTexture;
		};
	NameSpaceEnd
NameSpaceEnd

#define __CONSTANT_BUFFER_STRUCT_H__
#endif
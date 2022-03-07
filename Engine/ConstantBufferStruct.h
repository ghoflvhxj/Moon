#pragma once
#ifndef __CONSTANT_BUFFER_STRUCT_H__

#ifdef __cplusplus
	#define NameSpaceBegin(x) namespace x {
	#define NameSpaceEnd }
	#define ConstantBufferSlot(x)
#else
	#define NameSpaceBegin(x)
	#define NameSpaceEnd
	#define ConstantBufferSlot(x) : register(bx)
#endif


NameSpaceBegin(ConstantBufferStruct)
	NameSpaceBegin(Constant)
		__declspec(align(16)) struct Constant ConstantBufferSlot(0)
		{
			
		};
	NameSpaceEnd

	NameSpaceBegin(Frame)
		__declspec(align(16)) struct Frame ConstantBufferSlot(1)
		{
		public:
			uint32 width;
			uint32 height;
		};
	NameSpaceEnd

	
	NameSpaceBegin(VertexStruct)
		// Per Frame
		__declspec(align(16)) struct ViewProjectionMatrix ConstantBufferSlot(2)
		{
		public:
			ViewProjectionMatrix(const Mat4 &worldViewMatrix, const Mat4 &projectionMatrix);
			~ViewProjectionMatrix() = default;

		public:
			void update();

		public:
			Mat4 _viewMatrix;
			Mat4 _projectionMatrix;
		};

		// Per Object
		// 애니메이션에 사용됨
		__declspec(align(16)) struct WolrdMatrix ConstantBufferSlot(3)
		{
		public:
			WolrdMatrix(const Mat4 &worldMatrix);
			~WolrdMatrix();

		public:
			void update();

		public:
			Mat4 Mat4 _worldMatrix;
		};

		// 애니메이션에 사용됨
		__declspec(align(16)) struct JointMatrices ConstantBufferSlot(4)
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
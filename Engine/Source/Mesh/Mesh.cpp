#include "Mesh.h"
#include "FBXSDK/fbxsdk.h"

using namespace fbxsdk;

void AnimationClip::SetFrameInfo(FbxTime& InStart, FbxTime& InEnd)
{
    StartFrame = CastValue<uint32>(InStart.GetFrameCount(FbxTime::eFrames24));
    EndFrame = CastValue<uint32>(InEnd.GetFrameCount(FbxTime::eFrames24));
    Duration = (InEnd - InStart).GetSecondDouble();
    TotalFrame = EndFrame - StartFrame;
    KeyFrames.resize(TotalFrame);
}

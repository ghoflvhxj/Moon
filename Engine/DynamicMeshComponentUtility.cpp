#include "DynamicMeshComponentUtility.h"
#include "FBXSDK/fbxsdk.h"

void AnimationClip::SetFrameInfo(fbxsdk::FbxTime& InStart, fbxsdk::FbxTime& InEnd)
{
    StartFrame = CastValue<uint32>(InStart.GetFrameCount(FbxTime::eFrames30));
    EndFrame = CastValue<uint32>(InEnd.GetFrameCount(FbxTime::eFrames30));
    Duration = (InEnd - InStart).GetSecondDouble();
    TotalFrame = EndFrame - StartFrame;
    KeyFrames.resize(TotalFrame);
}

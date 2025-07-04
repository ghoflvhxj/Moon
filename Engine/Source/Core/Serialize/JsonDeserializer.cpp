#include "JsonDeserializer.h"

using namespace rapidjson;

MJsonDeserializer::MJsonDeserializer()
    : Allocator(Doc.GetAllocator())
{
}

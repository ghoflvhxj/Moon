#pragma once
#ifndef __UNORDEREDMAP_UTILITY_H__

namespace MapUtility
{
	template <class Map, class Key>
	const bool Find(Map &refMap, Key &refKey)
	{
		if (refMap.find(refKey) == refMap.end())
			return false;

		return true;
	}

	template <class Map, class Key, class Value>
	const bool Insert(Map &refMap, Key &refKey, Value &refValue)
	{
		refMap.emplace(refKey, refValue);
		return true;
	}

	template <class Map, class Key, class Value>
	const bool FindInsert(Map &refMap, Key &refKey, Value &refValue)
	{
		if (true == Find(refMap, refKey))
			return false;

		refMap.emplace(refKey, refValue);
		return true;
	}

	template<class Map, class Key = Map::key_type, class Value = Map::mapped_type>
	const bool FindGet(Map &refMap, Key &refKey, Value &outRefValue)
	{
		if (false == Find(refMap, refKey))
			return false;

		outRefValue = refMap[refKey];

		return true;
	}

	//template<class Map, class Key, class Value>
	//const bool &FindGet(Map &refMap, Key &refKey, Value *pValue)
	//{
	//	if (Find(refMap, refKey))
	//		return false;;

	//	return refMap[refKey];
	//}
}

//namespace MapUtility
//{
//	// 문자열 특수화 버전들 입니다.
//	template <class Map>
//	const bool Find(Map &refMap, const wchar_t *strKey)
//	{
//		auto iter = refMap.find(strKey);
//		if (iter == refMap.end())
//			return false;
//
//		return true;
//	}
//
//	template <class Map, class Value>
//	const bool Insert(Map &refMap, const wchar_t *strKey, const Value &refValue)
//	{
//		refMap.emplace(strKey, refValue);
//		return true;1
//	}
//
//	template <class Map, class Value>
//	const bool FindInsert(Map &refMap, const wchar_t *strKey, const Value &refValue)
//	{
//		if (Find(refMap, strKey))
//			return false;
//
//		refMap.emplace(strKey, refValue);
//		return true;
//	}
//
//	template<class Value>
//	Value &FindGet(std::map<const wchar_t*, Value> &refMap, const wchar_t *strKey)
//	{
//		if (Find(refMap, refKey))
//			nullptr;
//
//		return refMap[refKey];
//	}
//
//	template<class Value>
//	Value &FindGet(std::unordered_map<const wchar_t*, Value> &refMap, const wchar_t *strKey)
//	{
//		if (Find(refMap, refKey))
//			nullptr;
//
//		return refMap[refKey];
//	}
//}

#define __UNORDEREDMAP_UTILITY_H__
#endif
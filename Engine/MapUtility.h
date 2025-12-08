#pragma once

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
	const bool FindInsert(Map &refMap, Key &refKey, Value &refValue, bool bReplace = false)
	{
		if (Find(refMap, refKey) && bReplace == false)
			return false;

		refMap[refKey] = refValue;
		return true;
	}

	template <class Map, class Key, class Value>
	const bool FindInsert(Map &refMap, Key &refKey, Value &refValue, std::function<void()> func)
	{
		if (true == Find(refMap, refKey))
			return false;

		refMap.emplace(refKey, refValue);
		func();

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
}
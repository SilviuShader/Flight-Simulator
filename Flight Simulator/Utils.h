#pragma once

#include <utility>

typedef std::pair<int, int> Vec2Int;

struct HashHelper
{
public:

	struct HashPair
	{
		template <class T1, class T2>
		size_t operator()(const std::pair<T1, T2>& p) const
		{
			auto hash1 = std::hash<T1>{}(p.first);
			auto hash2 = std::hash<T2>{}(p.second);
			return hash1 ^ hash2;
		}
	};

public:

	template<class T>
	static size_t HashCombine(size_t currentHash, T newValue)
	{
		currentHash += currentHash * 31 + std::hash<T>()(newValue);
		return currentHash;
	}
};
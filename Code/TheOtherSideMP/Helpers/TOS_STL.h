/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
**************************************************************************/

#pragma once

#include "Cry_Math.h"

#include <random>
#include <iterator>
#include <type_traits>

namespace TOS_STL
{
	//template <class Container, typename DataType>
	//DataType GetRandomFromSTL(const Container& containter)
	//{
	//	const float itemsCount = containter.size();

	//	if (itemsCount > 1)
	//	{
	//		//const int minItemIndex = 0;
	//		const int maxItemIndex = containter.size() - 1;

	//		//const float ranf = Random(minItemIndex, maxItemIndex);
	//		//const int ran = round(ranf);
	//		const float ran = Random(maxItemIndex);

	//		for (int i = 0; i < containter.size(); i++)
	//		{
	//			if (i != ran)
	//				continue;

	//			return containter[i];
	//		}

	//	}
	//	else if (itemsCount == 1)
	//	{
	//		for (DataType item : containter)
	//		{
	//			return item;
	//		}
	//	}

	//	return containter[0];
	//}

	template <typename Container>
	typename Container::value_type GetRandomFromSTL(const Container& container)
	{
		typedef typename Container::const_iterator ConstIterator;

		size_t itemsCount = std::distance(container.begin(), container.end());

		if (itemsCount == 0)
		{
			// Возвращаем значение по умолчанию для типа элементов контейнера.
			return typename Container::value_type();
		}

		size_t randomIndex = cry_rand() % itemsCount; // Генерация случайных чисел

		ConstIterator it = container.begin();
		std::advance(it, randomIndex);

		return *it;
	}

	template <class Container, typename DataType>
	int GetIndexFromMapKey(const Container& containter, DataType keyValue)
	{
		return std::distance(containter.begin(), containter.find(keyValue));
	}

	//! Find element in container.
	// @return true if item found.
	template <class Container, class Value>
	bool Find(const Container& container, const Value& value)
	{
		return std::find(container.begin(), container.end(), value) != container.end();
	}

	//! Find element in container.
	// @return true if item found.
	template <class Container, class Value>
	bool Find(const Container* container, const Value& value)
	{
		return std::find(container->begin(), container->end(), value) != container->end();
	}
}
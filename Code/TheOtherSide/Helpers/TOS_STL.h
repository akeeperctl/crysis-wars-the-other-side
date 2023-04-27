#pragma once

#include "Cry_Math.h"

namespace TOS_STL
{
	template <class Container, typename DataType>
	DataType GetRandomFromSTL(const Container& containter)
	{
		const float itemsCount = containter.size();

		if (itemsCount > 1)
		{
			const int minItemIndex = 0;
			const int maxItemIndex = containter.size() - 1;

			const float ranf = Random(minItemIndex, maxItemIndex);
			const int ran = round(ranf);

			for (int i = 0; i < containter.size(); i++)
			{
				if (i != ran)
					continue;

				return containter[i];
			}

		}
		else if (itemsCount == 1)
		{
			for (DataType itemIndex : containter)
			{
				return itemIndex;
			}
		}
	}

	template <class Container, typename DataType>
	int GetIndexFromMapKey(const Container& containter, DataType keyValue)
	{
		return std::distance(containter.begin(), containter.find(keyValue));
	}
}
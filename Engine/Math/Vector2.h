#pragma once

namespace SmoothieMath {
	template<class type> struct Vector2
	{
		type x, y;

		Vector2(type x = 0, type y = 0);
		
		const type* dataPointer();
	};

	template<class type> 
	inline Vector2<type>::Vector2(type x, type y)
	{
		this->x = x;
		this->y = y;
	}

	template<class type>
	inline const type* Vector2<type>::dataPointer()
	{
		return &x;
	}
}

#pragma once

namespace Utility
{

	template<size_t N, auto value, auto ...values>
	class _DefaultValues;


	template<size_t N, const  auto value, const  auto ...values>
	class _DefaultValues : public _DefaultValues<N + 1, values...>
	{
		using Base = _DefaultValues<N + 1, values...>;
	public:
		template<size_t Index>
		static inline auto Get()
		{
			if constexpr (N == Index)
				return value;
			return Base::Get<Index>();
		}
	};

	template<const auto ...values>
	class DefaultValues : public _DefaultValues<0, values...>
	{
		using Base = _DefaultValues<0, values...>;
	public:
		template<size_t Index>
		static inline auto Get()
		{
			return Base::Get<Index>();
		}
	};
}

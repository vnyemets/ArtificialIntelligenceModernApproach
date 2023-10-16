#pragma once

#include <array>
#include <bit>
#include <bitset>
#include <cassert>
#include <concepts>
#include <cstdint>
#include <cstdlib>
#include <experimental/generator>
#include <initializer_list>
#include <iomanip>
#include <iosfwd>
#include <ranges>
#include <stack>
#include <unordered_set>

template <std::uint8_t side_size>
struct board_t
{
	constexpr static std::uint8_t board_size = side_size * side_size;
	constexpr static std::uint8_t cell_width = std::bit_width(board_size - 1U);
	using cell_t = std::bitset<cell_width>;
	using cells_t = std::bitset<board_size* cell_width>;
	std::uint8_t m_empty_cell_index;
	cells_t m_cells;
	template<std::ranges::bidirectional_range Rng>
	constexpr board_t(bool, Rng&& r) noexcept
		: m_empty_cell_index{}, m_cells{}
	{
		assert(r.size() == board_size);
		std::unordered_set<cell_t> unique_set;
		std::uint8_t i = board_size;
		for (auto cell : std::ranges::reverse_view(r))
		{
			assert(unique_set.insert(cell).second);
			m_cells <<= cell_width;
			m_cells |= cells_t{ cell.to_ulong() };
			if (cell_t{} == cell)
			{
				m_empty_cell_index = i;
			}
			i--;
		}
	}
	constexpr board_t(std::initializer_list<cell_t> cells) noexcept
		: board_t(true, cells)
	{
	}
	constexpr board_t(std::array<cell_t, board_size> const& cells) noexcept
		: board_t(true, cells)
	{
	}
	constexpr board_t swap_with_empty(std::uint8_t j) const noexcept
	{
		auto ans = *this;
		ans.m_empty_cell_index = j;
		auto i = m_empty_cell_index;
		--i;
		--j;
		assert(i < board_size);
		assert(j < board_size);
		for (std::uint8_t k = 0; k < cell_width; k++)
		{
			ans.m_cells.set(i * cell_width + k, m_cells[j * cell_width + k]);
			ans.m_cells.set(j * cell_width + k, m_cells[i * cell_width + k]);
		}
		assert(this->parity() == ans.parity());
		return ans;
	}
	constexpr bool operator == (board_t const& other) const noexcept
	{
		return m_cells == other.m_cells;
	}
	constexpr std::uint8_t heuristic() const noexcept
	{
		std::uint8_t ans = 0;
		for (std::uint8_t current_pos = 0ui8; current_pos < board_size; current_pos++)
		{
			if (current_pos == (m_empty_cell_index - 1ui8)) continue;
			auto const shift = cell_width * current_pos;
			std::uint8_t const right_pos = ((m_cells >> shift).to_ullong() - 1U) & 0x0f;
			if (current_pos != right_pos)
			{
				auto const current_dv = std::div(current_pos, side_size);
				auto const right_dv = std::div(right_pos, side_size);
				ans += std::abs(current_dv.quot - right_dv.quot);
				ans += std::abs(current_dv.rem - right_dv.rem);
			}
		}
		return ans;
	}
	constexpr bool parity() noexcept
	{
		int ans = 0;
		for (std::uint8_t i = 0; i < board_size; i++)
		{
			if (i == (m_empty_cell_index-1ui8)) continue;
			auto const shift = cell_width * (i * side_size);
			auto const lhs = ((m_cells >> shift).to_ullong() & 0x0f);
			for (std::uint8_t j = i + 1ui8; j < board_size; j++)
			{
				if (j == (m_empty_cell_index-1ui8)) continue;
				auto const shift = cell_width * (j * side_size);
				auto const rhs = ((m_cells >> shift).to_ullong() & 0x0f);
				if (lhs > rhs)
				{
					ans++;
				}
			}
		}
		return ans & 1;
	}
	friend std::ostream& operator<<(std::ostream& os, board_t<side_size> arg)
	{
		for (std::uint8_t row = 0; row < side_size; row++)
		{
			for (std::uint8_t col = 0; col < side_size; col++)
			{
				auto const shift = cell_width * (row * side_size + col);
				os << ((arg.m_cells >> shift).to_ullong() & 0x0f) << ' ';
			}
			os << '\n';
		}
		return os;
	}
	std::experimental::generator<board_t<side_size>> neighbors() const
	{
		if (!is_left_border()) co_yield swap_with_empty(m_empty_cell_index - 1U);
		if (!is_right_border()) co_yield swap_with_empty(m_empty_cell_index + 1U);
		if (!is_top_border()) co_yield swap_with_empty(m_empty_cell_index - side_size);
		if (!is_bottom_border()) co_yield swap_with_empty(m_empty_cell_index + side_size);
	}
private:
	//1   2  3  4
	//5   6  7  8
	//9  10 11 12
	//13 14 15 16
	constexpr bool is_right_border() const noexcept
	{
		return 0 == m_empty_cell_index % side_size;
	}
	constexpr bool is_left_border() const noexcept
	{
		return 1U == m_empty_cell_index % side_size;
	}
	constexpr bool is_top_border() const noexcept
	{
		return m_empty_cell_index <= side_size;
	}
	constexpr bool is_bottom_border() const noexcept
	{
		return (m_empty_cell_index + side_size) > (side_size * side_size);
	}
	friend void test();
};

template<std::uint8_t side_size>
struct std::hash<board_t<side_size>>
{
	constexpr std::size_t operator()(const board_t<side_size>& arg) const noexcept
	{
		return std::hash<typename board_t<side_size>::cells_t>{}(arg.m_cells);
	}
};
// ArtificialIntelligenceModernApproach.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <algorithm>
#include <array>
#include <bit>
#include <bitset>
#include <cassert>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <experimental/generator>
#include <initializer_list>
#include <iomanip>
#include <iostream>
#include <queue>
#include <random>
#include <ranges>
#include <unordered_set>

//#include <boost/heap/fibonacci_heap.hpp>

template <std::uint8_t side_size>
struct board_t
{
	constexpr static std::uint8_t board_size = side_size * side_size;
	constexpr static std::uint8_t cell_width = std::bit_width(board_size - 1U);
	using cell_t = std::bitset<cell_width>;
	using cells_t = std::bitset<board_size* cell_width>;
	cells_t m_cells;
	board_t const* parent;
	std::uint16_t depth, cost;

	template<std::ranges::bidirectional_range Rng>
	constexpr board_t(std::from_range_t, Rng&& r) noexcept
		: m_cells{}, parent{ nullptr }, depth{}, cost{}
	{
		assert(r.size() == board_size);
		std::unordered_set<cell_t> unique_set;
		for (auto cell : std::ranges::reverse_view(r))
		{
			assert(unique_set.insert(cell).second);
			m_cells <<= cell_width;
			m_cells |= cells_t{ cell.to_ulong() };
		}
	}
	constexpr board_t(std::initializer_list<cell_t> cells) noexcept
		: board_t(std::from_range_t{}, cells)
	{
	}
	constexpr board_t(std::array<cell_t, board_size> cells) noexcept
		: board_t(std::from_range_t{}, cells)
	{
	}
	constexpr board_t swap_empty(std::uint8_t i, std::uint8_t j) const noexcept
	{
		auto ans = *this;
		--i;
		--j;
		assert(i < board_size);
		assert(j < board_size);
		for (std::uint8_t k = 0; k < cell_width; k++)
		{
			ans.m_cells.set(i * cell_width + k, m_cells[j * cell_width + k]);
			ans.m_cells.set(j * cell_width + k, m_cells[i * cell_width + k]);
		}
		if (!(this->parity() == ans.parity()))
		{
			std::cout << *this << '\n';
			std::cout << ans << '\n';
		}
		assert(this->parity() == ans.parity());
		ans.parent = this;
		ans.depth = depth + 1ui16;
		ans.cost = depth + 1ui16 + heuristic();
		return ans;
	}
	constexpr bool operator == (board_t const& other) const noexcept
	{
		return m_cells == other.m_cells;
	}
	constexpr bool operator < (board_t const& other) const noexcept
	{
		return cost < other.cost;
	}
	friend std::ostream& operator<<(std::ostream& os, board_t<side_size> arg)
	{
		for (std::uint8_t row = 0; row < side_size; row++)
		{
			for (std::uint8_t col = 0; col < side_size; col++)
			{
				auto const shift = cell_width * (row * side_size + col);
				os << std::setw(2)<< ((arg.m_cells >> shift).to_ullong() & 0x0f) << ' ';
			}
			os << '\n';
		}
		return os;
	}
	std::experimental::generator<board_t<side_size>> neighbors() const
	{
		auto const empty_idx = empty_cell_index();
		if (!is_left_border(empty_idx)) co_yield swap_empty(empty_idx, empty_idx - 1ui8);
		if (!is_right_border(empty_idx)) co_yield swap_empty(empty_idx, empty_idx + 1ui8);
		if (!is_top_border(empty_idx)) co_yield swap_empty(empty_idx, empty_idx - side_size);
		if (!is_bottom_border(empty_idx)) co_yield swap_empty(empty_idx, empty_idx + side_size);
	}
	constexpr std::uint8_t heuristic() const noexcept
	{
		std::uint8_t ans = 0;
		for (std::uint8_t current_pos = 0ui8; current_pos < board_size; current_pos++)
		{
			auto const shift = cell_width * current_pos;
			std::uint8_t const desired_pos = (m_cells >> shift).to_ullong() & 0x0f;
			//if (0ui8 == desired_pos) continue;
			if (current_pos != desired_pos)
			{
				auto const current_dv = std::div(current_pos, side_size);
				auto const right_dv = std::div(desired_pos, side_size);
				ans += std::abs(current_dv.quot - right_dv.quot);
				ans += std::abs(current_dv.rem - right_dv.rem);
			}
		}
		return ans;
	}
	constexpr bool parity() const noexcept
	{
		int ans = 0;
		std::uint8_t const empty_idx = empty_cell_index() - 1ui8;
		for (std::uint8_t i = 0; i < board_size; i++)
		{
			if (i == empty_idx) continue;
			auto const shift = cell_width * i;
			auto const lhs = ((m_cells >> shift).to_ullong() & 0x0f);
			for (std::uint8_t j = i + 1ui8; j < board_size; j++)
			{
				if (j == empty_idx) continue;
				auto const shift = cell_width * j;
				auto const rhs = ((m_cells >> shift).to_ullong() & 0x0f);
				if (lhs > rhs)
				{
					ans++;
				}
			}
		}
		return ans & 1;
	}
private:
	//1   2  3  4
	//5   6  7  8
	//9  10 11 12
	//13 14 15 16
	constexpr std::uint8_t empty_cell_index() const noexcept
	{
		std::uint8_t ans = 1ui8;
		cells_t mask{ 0x0f };
		for (; ans <= board_size; ++ans)
		{
			if (mask == ((m_cells ^ mask) & mask))break;
			mask <<= cell_width;
		}
		assert(ans <= board_size);
		return ans;
	}
	constexpr bool is_right_border(std::uint8_t const empty_idx) const noexcept
	{
		return 0 == empty_idx % side_size;
	}
	constexpr bool is_left_border(std::uint8_t const empty_idx) const noexcept
	{
		return 1ui8 == empty_idx % side_size;
	}
	constexpr bool is_top_border(std::uint8_t const empty_idx) const noexcept
	{
		return empty_idx <= side_size;
	}
	constexpr bool is_bottom_border(std::uint8_t const empty_idx) const noexcept
	{
		return (empty_idx + side_size) > (side_size * side_size);
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

template <std::uint8_t side_size>
bool astar(board_t<side_size> const start, board_t<side_size> const goal)
{
	using history_t = std::unordered_set<board_t<side_size>>;
	history_t explored_nodes;
	std::uint16_t max_depth = 0ui16;
	auto cmp = [](auto lhs, auto rhs) noexcept -> bool
		{
			return lhs->cost > rhs->cost;
		};
	std::priority_queue<typename history_t::const_iterator,
		std::deque<typename history_t::const_iterator>, decltype(cmp)> frontier{cmp};
	//boost::heap::fibonacci_heap<typename history_t::const_iterator, boost::heap::compare<decltype(cmp)>> frontier(cmp);
	frontier.push(explored_nodes.emplace(start).first);
	while (!frontier.empty())
	{
		auto const parent = frontier.top();
		frontier.pop();
		if (*parent == goal)
		{
			std::cout << "explored_nodes   " << explored_nodes.size() << '\n';
			std::cout << "nodes_expanded   " << explored_nodes.size() - frontier.size() << '\n';
			std::cout << "depth            " << parent->depth << '\n';
			std::cout << "max depth        " << max_depth << '\n';
			std::cout << "cost             " << parent->cost << '\n';
			return true;
		}
		for (auto const& neighbor : parent->neighbors())
		{
			if (auto [it, inserted] = explored_nodes.emplace(neighbor); inserted)
			{
				frontier.push(it);
				max_depth = std::max(max_depth, neighbor.depth);
			}
		}
	}
	return false;
}

template <std::uint8_t side_size>
bool bfs(board_t<side_size> const start, board_t<side_size> const goal)
{
	using history_t = std::unordered_set<board_t<side_size>>;
	history_t explored_nodes;
	std::uint16_t max_depth = 0ui16;
	std::deque<typename history_t::const_iterator> frontier;
	frontier.push_back(explored_nodes.emplace(start).first);
	while (!frontier.empty())
	{
		auto const parent = frontier.front();
		frontier.pop_front();
		explored_nodes.emplace(*parent);
		if (*parent == goal)
		{
			std::cout << "explored_nodes   " << explored_nodes.size() << '\n';
			std::cout << "nodes_expanded   " << explored_nodes.size() - frontier.size() << '\n';
			std::cout << "depth            " << parent->depth << '\n';
			std::cout << "max depth        " << max_depth << '\n';
			std::cout << "cost             " << parent->cost << '\n';
			return true;
		}
		for (auto const& neighbor : parent->neighbors())
		{
			if (auto [it, inserted] = explored_nodes.emplace(neighbor); inserted)
			{
				frontier.push_back(it);
				max_depth = std::max(max_depth, neighbor.depth);
			}
		}
	}
	return false;
}

template <std::uint8_t side_size>
bool dfs(board_t<side_size> const start, board_t<side_size> const goal)
{
	using history_t = std::unordered_set<board_t<side_size>>;
	history_t explored_nodes;
	std::uint16_t max_depth = 0ui16;
	std::deque<typename history_t::const_iterator> frontier;
	frontier.push_back(explored_nodes.emplace(start).first);
	while (!frontier.empty())
	{
		auto const parent = frontier.back();
		frontier.pop_back();
		explored_nodes.emplace(*parent);
		if (*parent == goal)
		{
			std::cout << "explored_nodes   " << explored_nodes.size() << '\n';
			std::cout << "nodes_expanded   " << explored_nodes.size() - frontier.size() << '\n';
			std::cout << "depth            " << parent->depth << '\n';
			std::cout << "max depth        " << max_depth << '\n';
			std::cout << "cost             " << parent->cost << '\n';
			return true;
		}
		for (auto const& neighbor : parent->neighbors())
		{
			if (auto [it, inserted] = explored_nodes.emplace(neighbor); inserted)
			{
				frontier.push_back(it);
				max_depth = std::max(max_depth, neighbor.depth);
			}
		}
	}
	return false;
}

template <std::uint8_t side_size>
bool dls(board_t<side_size> const start, board_t<side_size> const goal, std::uint16_t limit)
{
	using history_t = std::unordered_set<board_t<side_size>>;
	history_t explored_nodes;
	std::uint16_t max_depth = 0ui16;
	std::deque<typename history_t::const_iterator> frontier;
	frontier.push_back(explored_nodes.emplace(start).first);
	while (!frontier.empty())
	{
		auto const parent = frontier.back();
		frontier.pop_back();
		explored_nodes.emplace(*parent);
		if (*parent == goal)
		{
			std::cout << "explored_nodes   " << explored_nodes.size() << '\n';
			std::cout << "nodes_expanded   " << explored_nodes.size() - frontier.size() << '\n';
			std::cout << "depth            " << parent->depth << '\n';
			std::cout << "max depth        " << max_depth << '\n';
			std::cout << "cost             " << parent->cost << '\n';
			std::cout << "limit            " << limit << '\n';
			return true;
		}
		if (parent->depth >= limit) continue;
		for (auto const& neighbor : parent->neighbors())
		{
			if (auto [it, inserted] = explored_nodes.emplace(neighbor); inserted)
			{
				frontier.push_back(it);
				max_depth = std::max(max_depth, neighbor.depth);
			}
		}
	}
	return false;
}

template <std::uint8_t side_size>
bool iddfs(board_t<side_size> const start, board_t<side_size> const goal)
{
	for (std::uint16_t i = 0; ;i++)
	{
		if (dls<side_size>(start, goal, i))
		{
			return true;
		}
	}
	return false;
}

int main()
{
	std::random_device rd;
	std::mt19937 gen{ rd() };
	std::array<std::bitset<4>, 9> init{ 0,1,2,3,4,5,6,7,8 };
	//std::array<std::uint32_t, 90> stats{0};
	for (size_t i = 0; i < 10; i++)
	{
		do
		{
			std::ranges::shuffle(init, gen);
		} while (board_t<3>{init}.parity());
		board_t<3> const start{ std::from_range, init };
		std::cout << start << std::boolalpha;
		auto ans = astar<3>(
			start,
			{ 0,1,2,3,4,5,6,7,8});
		std::cout << ans << '\n';
		ans = bfs<3>(
			start,
			{ 0,1,2,3,4,5,6,7,8 });
		std::cout << ans << '\n';
		ans = dfs<3>(
			start,
			{ 0,1,2,3,4,5,6,7,8 });
		std::cout << ans << '\n';
		ans = iddfs<3>(
			start,
			{ 0,1,2,3,4,5,6,7,8 });
		std::cout << ans << '\n';
		char c;
		std::cin >> c;
		//stats[ans]++;
	}
	//for (auto acc : stats)
	//{
	//	std::cout << acc << '\n';
	//}
}
// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

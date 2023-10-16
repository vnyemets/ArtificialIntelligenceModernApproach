#pragma once

#include "npuzzle.hpp"

#include <deque>
#include <queue>
//#include <stack>
#include <unordered_map>

template <std::uint8_t side_size>
int solve(board_t<side_size> const start, board_t<side_size> const target)
{
	using history_t = std::unordered_map<board_t<side_size>, std::uint8_t>;
	history_t history;
	auto cmp = [](auto const lhs, auto const rhs) noexcept
	{
		return (lhs->second + lhs->first.heuristic()) > (rhs->second + rhs->first.heuristic());
	};
	std::priority_queue<typename history_t::const_iterator,
		std::deque<typename history_t::const_iterator>,
		decltype(cmp)> q(cmp);
	q.push(history.emplace(start, 0ui16).first);
	while (!q.empty())
	{
		auto const parent = q.top();
		q.pop();
		for (auto const& child : parent->first.available_states())
		{
			if (child == target)
			{
				return 1;
			}
			if (parent->second + 1U == std::numeric_limits<std::uint8_t>::max()) continue;
			if (auto [it, inserted] = history.emplace(child, parent->second + 1U); inserted)
			{
				q.push(it);
			}
		}
	}
	return 0;
}
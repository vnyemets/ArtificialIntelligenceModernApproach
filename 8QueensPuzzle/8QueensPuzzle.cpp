// 8QuennsPuzzle.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <atomic>
#include <experimental/generator>
#include <iostream>
#include <limits>
#include <locale>
#include <mutex>
#include <thread>
#include <vector>

struct board_t
{
	unsigned int storage;
	constexpr int queen_pos(int i) const noexcept
	{
		return (storage >> (3 * i)) & 7;
	}
	constexpr void set_queen_pos(int i, int v) noexcept
	{
		int mask = queen_pos(i) << (3 * i);
		storage ^= mask;
		storage |= (v << (3 * i));
	}
	constexpr int cost() const noexcept
	{
		int ans = 0;
		for (int i = 0; i != 7; i++)
		{
			auto const qi = queen_pos(i);
			for (int j = i + 1; j != 8; j++)
			{
				auto const qj = queen_pos(j);
				ans += (qi == qj);
				auto const inc = j - i;
				if (qi <= (7 - inc)) ans += ((qi + inc) == qj);
				if (qi >= inc) ans += ((qi - inc) == qj);
			}
		}
		return ans;
	}
	std::experimental::generator<board_t> neighbors() const
	{
		for (int i = 0; i != 8; i++)
		{
			auto const q = queen_pos(i);
			for (int j = 0; j != 8; j++)
			{
				if (j == q) continue;
				auto ans = *this;
				ans.set_queen_pos(i, j);
				co_yield ans;
			}
		}
	}
	void print() const
	{
		for (int row = 0; row != 8; row++)
		{
			for (int i = 0; i != 8; i++)
			{
				auto const q = queen_pos(i);
				std::cout << ((row == q) ? "♕" : "◻");
			}
			std::cout << '\n';
		}
		std::cout << cost() << '\n';
	}
};

void hill_climbing(
	board_t const start,
	std::vector<std::atomic_flag>& processed,
	std::atomic<int>& succeed,
	std::atomic<int>& solutions)
{
	int min_cost = std::numeric_limits<int>::max();
	for (auto current = start; /*!processed[current.storage].test_and_set()*/true;)
	{
		board_t next = current;
		for (auto const neighbor : current.neighbors())
		{
			//if (processed[neighbor.storage].test()) continue;
			if (auto const cost = neighbor.cost(); cost < min_cost)
			{
				min_cost = cost;
				next = neighbor;
				if (0 == min_cost)
				{
					succeed.fetch_add(1);
					if (!processed[neighbor.storage].test_and_set())
					{
						solutions.fetch_add(1);
						static std::mutex m;
						std::lock_guard g{ m };
						start.print();
						neighbor.print();
					}
					return;
				}
			}
		}
		if (next.storage == current.storage)
		{
			return;
		}
		current = next;
	}
}

int main()
{
	std::vector<std::atomic_flag> processed(8 * 8 * 8 * 8 * 8 * 8 * 8 * 8);
	std::atomic<int> succeed = 0, solutions = 0;
	std::vector<std::jthread> tasks;
	tasks.reserve(std::thread::hardware_concurrency());
	auto task = [&processed, &succeed, &solutions](unsigned int first, unsigned int last)
		{
			for (unsigned int i = first; i < last; i++)
			{
				hill_climbing(board_t{ i }, processed, succeed, solutions);
			}
		};
	int const chunk = processed.size() / std::thread::hardware_concurrency();
	for (size_t i = 0; i != std::thread::hardware_concurrency(); i++)
	{
		tasks.emplace_back(task, chunk * i, chunk * (i + 1));
	}
	for (auto& task : tasks)
	{
		task.join();
	}
	std::cout << "solutions: " << solutions << '\n';
	std::cout << "successed: " << succeed << '\n';
	std::cout << "successed %: " << float(100*succeed) / processed.size() << '\n';
	return 0;
}
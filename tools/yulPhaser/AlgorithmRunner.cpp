/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <tools/yulPhaser/AlgorithmRunner.h>

#include <tools/yulPhaser/Exceptions.h>

#include <libsolutil/Assertions.h>

#include <cerrno>
#include <cstring>
#include <fstream>

using namespace std;
using namespace solidity::phaser;

void AlgorithmRunner::run(GeneticAlgorithm& _algorithm)
{
	populationAutosave();

	for (size_t round = 0; !m_options.maxRounds.has_value() || round < m_options.maxRounds.value(); ++round)
	{
		m_population = _algorithm.runNextRound(m_population);
		randomiseDuplicates();

		m_outputStream << "---------- ROUND " << round + 1 << " ----------" << endl;
		m_outputStream << m_population;

		populationAutosave();
	}
}

void AlgorithmRunner::populationAutosave() const
{
	if (!m_options.populationAutosaveFile.has_value())
		return;

	ofstream outputStream(m_options.populationAutosaveFile.value(), ios::out | ios::trunc);
	assertThrow(
		outputStream.is_open(),
		FileOpenError,
		"Could not open file '" + m_options.populationAutosaveFile.value() + "': " + strerror(errno)
	);

	for (auto& individual: m_population.individuals())
		outputStream << individual.chromosome << endl;

	assertThrow(
		!outputStream.bad(),
		FileWriteError,
		"Error while writing to file '" + m_options.populationAutosaveFile.value() + "': " + strerror(errno)
	);
}

void AlgorithmRunner::randomiseDuplicates()
{
	if (m_options.randomiseDuplicates)
	{
		assert(m_options.minChromosomeLength.has_value());
		assert(m_options.maxChromosomeLength.has_value());

		m_population = randomiseDuplicates(
			m_population,
			m_options.minChromosomeLength.value(),
			m_options.maxChromosomeLength.value()
		);
	}
}

Population AlgorithmRunner::randomiseDuplicates(
	Population _population,
	size_t _minChromosomeLength,
	size_t _maxChromosomeLength
)
{
	if (_population.individuals().size() == 0)
		return _population;

	vector<Chromosome> chromosomes{_population.individuals()[0].chromosome};
	size_t duplicateCount = 0;
	for (size_t i = 1; i < _population.individuals().size(); ++i)
		if (_population.individuals()[i].chromosome == _population.individuals()[i - 1].chromosome)
			++duplicateCount;
		else
			chromosomes.push_back(_population.individuals()[i].chromosome);

	return (
		Population(_population.fitnessMetric(), chromosomes) +
		Population::makeRandom(_population.fitnessMetric(), duplicateCount, _minChromosomeLength, _maxChromosomeLength)
	);
}

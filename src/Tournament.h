#ifndef TOURNAMENT_H
#define TOURNAMENT_H

//https://github.com/alenic/realGA/blob/master/src/selection_tournament.cpp
class TournamentSelection
{
public:

    TournamentSelection(size_t tournamentSize, float selectionProbability = 0.85f) : mTournamentSize(tournamentSize), mSelectionProbability(selectionProbability)
    {
        //mTournamentSize = std::max<size_t>(2, static_cast<size_t>(static_cast<float>(populationSize) *0.1f ));
    }

    void select(const Population& population, size_t &indexA, size_t &indexB) const
    {
        std::vector<Score> tournamentFitness(mTournamentSize);
        std::vector<size_t> tournamentIndex(mTournamentSize);

        indexA = tournament(population, tournamentFitness, tournamentIndex);
        indexB = tournament(population, tournamentFitness, tournamentIndex);

        std::uniform_int_distribution<> dst(0, mTournamentSize - 1);

        while(indexA == indexB)
        {
            indexA = tournamentIndex[dst(gen)];
            indexB = tournamentIndex[dst(gen)];
        }
    }

    size_t tournament(const Population& population, std::vector<Score>& tournamentFitness, std::vector<size_t>& tournamentIndex) const
    {
        std::uniform_real_distribution dist(0.0f, 1.0f);
        std::uniform_int_distribution<> iDistElite(0, populationSizeAndElite - 1);

        // return the index of winner
        size_t i = 0;
        bool match = false;

        // sample random mTournamentSize indices from mPopulationSize indices, without replacement
        while(i < mTournamentSize)
        {
            size_t index = iDistElite(gen);
            // check if the index was choosen
            match = false;
            for(size_t j = 0; j < i; j++)
            {
                if(tournamentIndex[j] == index)
                {
                    match = true;
                    break;
                }
            }
            if(!match)
            {
                tournamentIndex[i] = index;
                tournamentFitness[i] = population[index].fitness;
                i++;
            }
        }

        size_t kth;
        size_t kthMinIndex;
        size_t localKthIndex;

        if(mSelectionProbability < 1.0f)
        {
            float chooseP = mSelectionProbability;
            // choose the winner with a probability p, 2nd with p*(1-p), 3rd with p*(1-p)*2,...
            for(kth = 1; kth<mTournamentSize; kth++)
            {
                if(dist(gen) < chooseP)
                {
                    break;
                }
                chooseP *= (1.0 - mSelectionProbability);
            }
            // Get k-th min value
            localKthIndex = argSelection(tournamentFitness, 0, mTournamentSize - 1, kth);
            kthMinIndex = tournamentIndex[localKthIndex];
        }
        else
        {
            localKthIndex = argMax(tournamentFitness, 0, mTournamentSize - 1);
            kthMinIndex = tournamentIndex[localKthIndex];
        }


        return kthMinIndex;
    }

private:

    size_t argMax(const std::vector<Score>& arr, int l, int r) const
    {
        Score maxValue = arr[0];
        size_t maxIndex = 0;
        for(size_t i = l + 1; i <= r; i++)
        {
            if(arr[i] > maxValue)
            {
                maxValue = arr[i];
                maxIndex = i;
            }
        }
        return maxIndex;
    }

    size_t argSelection(std::vector<Score>& arr, int l, int r, int kth) const
    {
        // k * O(n)
        Score maxValue;
        size_t maxIndex;
        // find min
        for(size_t k = 0; k<kth; k++)
        {
            maxValue = arr[k];
            maxIndex = k;
            for(int i = l + k + 1; i <= r; i++)
            {
                if(arr[i] > maxValue)
                {
                    maxValue = arr[i];
                    maxIndex = i;
                }
            }
            if(k < kth - 1) Swap(arr[k], arr[maxIndex]);
        }
        return maxIndex;
    }

    void Swap(Score &a, Score &b) const
    {
        Score c = a;
        a = b;
        b = c;
    }

    size_t mTournamentSize;

    float mSelectionProbability;
};

#endif
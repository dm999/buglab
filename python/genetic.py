import numpy as np
import run_cython
import pygad
import os.path

#https://pygad.readthedocs.io/en/latest/pygad.html#examples
#https://github.com/ahmedfgad/GARI/blob/master/example.py

def fitness_func(ga_instance, solution, solution_idx):
    solution = solution > 0.5
    #~ print(solution[0])
    solution = np.concatenate((np.zeros(shape=(solution.shape[0], 1)), solution), axis=1)
    solution = np.concatenate((solution, np.zeros(shape=(solution.shape[0], 1))), axis=1)
    #~ print("!!!!!!")
    #~ print(solution.shape)
    #~ print(solution[0])
    #~ print(solution[0].astype(np.bool))
    threads = 10
    val = solution.astype(np.bool)
    res = run_cython.py_compute(val, threads)
    #~ print(res.shape)
    #~ print(res[:, 0])
    return res[:, 0]
    
    
num_generations = 1000000000000 # Number of generations.
num_parents_mating = 10 # Number of solutions to be selected as parents in the mating pool.

sol_per_pop = 1000 # Number of solutions in the population.
num_genes = 549

last_fitness = 0
def on_generation(ga_instance):
    global last_fitness
    print(f"{ga_instance.generations_completed:10d} F = {ga_instance.best_solution(pop_fitness=ga_instance.last_generation_fitness)[1]}")
    last_fitness = ga_instance.best_solution(pop_fitness=ga_instance.last_generation_fitness)[1]
    
    if ga_instance.generations_completed % 50 == 0:
        ga_instance.save("pygad")
    
    
if(os.path.isfile("pygad.pkl")):
    ga_instance = pygad.load("pygad")
    ga_instance.on_generation=on_generation
    ga_instance.fitness_func=fitness_func
else:
    ga_instance = pygad.GA(num_generations=num_generations,
                       num_parents_mating=num_parents_mating,
                       #~ gene_type = int,
                       fitness_batch_size = 50,
                       sol_per_pop=sol_per_pop,
                       num_genes=num_genes,
                       fitness_func=fitness_func,
                       init_range_low=0.0,
                       init_range_high=1.0,
                       #~ mutation_percent_genes=10.0,
                       #~ mutation_type="random",
                       mutation_percent_genes=[0.1, 1],
                       mutation_type="adaptive",
                       #~ mutation_type="inversion",
                       mutation_by_replacement=True,
                       random_mutation_min_val=0.0,
                       random_mutation_max_val=1.0,
                       crossover_type="single_point",
                       on_generation=on_generation)
                       

# Running the GA to optimize the parameters of the function.
ga_instance.run()

ga_instance.plot_fitness()
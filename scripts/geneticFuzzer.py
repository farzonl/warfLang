import argparse
import random
import os
import subprocess

# Number of individuals in each generation
POPULATION_SIZE = 100
  
# Valid genes
GENES = '''abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOP
QRSTUVWXYZ1234567890=+-*/&|!^()'''
  
# Target string to be generated
StartingSeed = 'a = (3*(2+1)/4 == 5*(10*(3+4)-10)) && true'
TargetLen = len(StartingSeed)

# coverage Name
os.environ["LLVM_PROFILE_FILE"] ='WarfGenetic'
covergeName = os.environ.get('LLVM_PROFILE_FILE')
LLVM_PATH = ''
WARF_CLI_PATH = ''
best_score = 40.0
best_chromosome = []
runs = 0
parser = argparse.ArgumentParser(
                    prog='geneticFuzzer',
                    description='This Program generates lines to input into Warf')

parser.add_argument('llvm_path', type=str)
parser.add_argument('warf_cli_path', type=str)  
parser.add_argument('-t', '--target_length', dest='targetLen', type=int, default=len(StartingSeed), help='Determines the length of the target string.')
parser.add_argument('-s', '--seed', dest='startingSeed', type=str, default=StartingSeed, help='Infulence the starting generation')

def run_llvm_cov(chromosome: str) -> float:
    global covergeName
    global runs
    os.environ["LLVM_PROFILE_FILE"] = "{}{}.profraw".format(covergeName, runs)
    runs = runs + 1
    
    #print(os.environ["LLVM_PROFILE_FILE"])

    # ./build/src/cli/Warf -e 1+2
    cli_command = [WARF_CLI_PATH, "-e", chromosome]
    cli_process = subprocess.Popen(cli_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    _,_ =  cli_process.communicate()
    

    #$(brew --prefix llvm)/bin/llvm-profdata merge -sparse WarfGenetic.profraw -o WarfGenetic.profdata
    profdata_command =  [LLVM_PATH+"/llvm-profdata", "merge", "-sparse", os.environ["LLVM_PROFILE_FILE"], "-o", "WarfGenetic.profdata"]
    profdata_process = subprocess.Popen(profdata_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    _,_ =  profdata_process.communicate()

    #$(brew --prefix llvm)/bin/llvm-cov report -instr-profile=WarfGenetic.profdata -object=build/test/WarfLang_TEST
    llvm_cov_command = [LLVM_PATH+"/llvm-cov", "report", "-instr-profile=WarfGenetic.profdata", "-object={}".format(WARF_CLI_PATH)]
    profdata_process = subprocess.Popen(llvm_cov_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = profdata_process.communicate()

    lines = stdout.splitlines()
    last_line = lines[-1].split()
    cov_line_percentage = float(last_line[9].decode('utf-8').rstrip('%'))
    #print(cov_line_percentage)
    return cov_line_percentage

class Individual(object):
    '''
    Class representing individual in population
    '''
    def __init__(self, chromosome):
        self.chromosome = chromosome 
        self.fitness = self.cal_fitness()
  
    @classmethod
    def mutated_genes(self):
        '''
        create random genes for mutation
        '''
        global GENES
        gene = random.choice(GENES)
        return gene
  
    @classmethod
    def create_gnome(self):
        '''
        create chromosome or string of genes
        '''
        global TargetLen
        return [self.mutated_genes() for _ in range(TargetLen)]
  
    def mate(self, par2):
        '''
        Perform mating and produce new offspring
        '''
  
        # chromosome for offspring
        child_chromosome = []
        for gp1, gp2 in zip(self.chromosome, par2.chromosome):    
  
            # random probability  
            prob = random.random()
  
            # if prob is less than 0.45, insert gene
            # from parent 1 
            if prob < 0.45:
                child_chromosome.append(gp1)
  
            # if prob is between 0.45 and 0.90, insert
            # gene from parent 2
            elif prob < 0.90:
                child_chromosome.append(gp2)
  
            # otherwise insert random gene(mutate), 
            # for maintaining diversity
            else:
                child_chromosome.append(self.mutated_genes())
  
        # create new Individual(offspring) using 
        # generated chromosome for offspring
        return Individual(child_chromosome)
  
    def cal_fitness(self):
        '''
        Calculate fitness score, it is the number of
        characters in string which differ from target
        string.
        '''
        #global best_score
        #global Target
        fitness = 100 - run_llvm_cov(''.join(self.chromosome))
        #for gs, gt in zip(self.chromosome, Target):
        #    new_score = run_llvm_cov(''.join(self.chromosome))
        #    if gs != gt and new_score < best_score: fitness+= 1
        #    else: best_score = new_score

        #print("score: {} fitness: {} gene {}".format(new_score,fitness,''.join(self.chromosome)))
        return fitness
  
# Driver code
def main():
    global POPULATION_SIZE
  
    #current generation
    generation = 1
  
    found = False
    population = []
  
    # create initial population
    for _ in range(POPULATION_SIZE):
                gnome = Individual.create_gnome()
                population.append(Individual(gnome))
  
    while not found:
  
        # sort the population in increasing order of fitness score
        population = sorted(population, key = lambda x:x.fitness)
  
        # if the individual having lowest fitness score ie. 
        # 0 then we know that we have reached to the target
        # and break the loop
        if population[0].fitness <= 0:
            found = True
            break
  
        # Otherwise generate new offsprings for new generation
        new_generation = []
  
        # Perform Elitism, that mean 10% of fittest population
        # goes to the next generation
        s = int((10*POPULATION_SIZE)/100)
        new_generation.extend(population[:s])
  
        # From 50% of fittest population, Individuals 
        # will mate to produce offspring
        s = int((90*POPULATION_SIZE)/100)
        for _ in range(s):
            parent1 = random.choice(population[:50])
            parent2 = random.choice(population[:50])
            child = parent1.mate(parent2)
            new_generation.append(child)
  
        population = new_generation
  
        print("Generation: {}\tString: {}\tFitness: {}".\
              format(generation,
              "".join(population[0].chromosome),
              population[0].fitness))
  
        generation += 1
  
      
    print("Generation: {}\tString: {}\tFitness: {}".\
          format(generation,
          "".join(population[0].chromosome),
          population[0].fitness))
    print(''.join(best_chromosome))
  
if __name__ == '__main__':
    args = parser.parse_args()
    LLVM_PATH = args.llvm_path
    WARF_CLI_PATH  = args.warf_cli_path
    TargetLen = args.targetLen
    main()
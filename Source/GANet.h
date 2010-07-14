#pragma once

#include "NeuralNet.h"

class Chromosome : public Element
{
  int genecount;
  double *genes, *newgenes;

  double fitness;

public:

  void SetGenes(WEIGHTFUNCTION weight);

  inline int GetGeneCount() { return genecount; }
  inline double* GetGenes() { return genes; }
  inline double* GetNewGenes() { return genes; }

  inline double& Fitness() { return fitness; }

  // mutates this Chromosome
  void Mutate(double rate, double maxoffset);

  // "copies" newgenes to genes by swapping the pointers
  void Update();

  // uses two parent chromosome to produce offspring
  // 2 chromosomes are produced, with the new genes stored
  // in newgenes of outa and outb
  // rate is the crossover rate, that is the chance that the genes
  // of the parent chromosomes are mixed
  static void CrossOver(Chromosome *ina, Chromosome *inb, 
                        Chromosome *outa, Chromosome *outb, double rate);

  // creates a Chromosomes with the specified number of genes
  Chromosome(int length);

  // frees the gene arrays
  ~Chromosome();
};

class GATrainer : public NetTrainer
{
protected:

  CleanContainer chromosomes;
  Container nets;

  double mutationrate;
  double mutationoffset;
  double crossoverrate;

  // get the number of weights in a net
  static int GetWeightCount(LayeredNet *net);

  // transfers the data from a chromosome to a net
  static void SetWeights(LayeredNet *net, Chromosome *chromosome);

  // transfers the data from the net to a chromosome
  static void GetWeights(LayeredNet *net, Chromosome *chromosome);

  // returns the sum of fitness values of all the Chromosomes
  double GetFitnessTotal();

  // returns a Chromosome by roulette wheel selection
  // value must be between 0 and FitnessTotal
  Chromosome* GetChromosome(double value);

  // creates a new population one from the existing one
  // the fitness values of the Chromosomes are used
  void CreatePopulation();

public:

  // the rate at which the Chromosomes mutate
  // a value between 0.05 and 0.2 is recommended
  inline double& MutationRate() { return mutationrate; }
  
  // the amound by which the genes mutate
  // 0.3 is a good starting point
  inline double& MutationOffset() { return mutationoffset; }

  // the chances that two Chromosomes cross over
  // 0.7 is a good starting value
  inline double& CrossOverRate() { return crossoverrate; }

  // adds a new net to the population
  void AddNet(LayeredNet *net);
    
  // removes (deletes) all the nets the trainer contains
  inline void RemoveNets() { nets.Empty(); }

  // returns the net population
  inline Container* GetNets() { return &nets; }

  // trains (= evolves) a new generation
  double Train(int state);

  // creates a trainer
  GATrainer(int inputcount, int outputcount);
};


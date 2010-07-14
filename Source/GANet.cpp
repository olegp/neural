#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "GANet.h"

Chromosome::Chromosome(int length) : genecount(length)
{
  genes = new double[genecount];
  newgenes = new double[genecount];
}

Chromosome::~Chromosome()
{
  safe_delete_array(genes);
  safe_delete_array(newgenes);
  // genecount = 0;
}

void Chromosome::SetGenes(WEIGHTFUNCTION weight)
{
  for(int i = 0; i < genecount; i ++)
    genes[i] = weight();
}

void Chromosome::Mutate(double rate, double maxoffset)
{
  for (int i = 0; i < genecount; i ++) {
    if (Random::GetDouble() < rate) {
			//add or subtract a small value to the weight
      genes[i] += (Random::GetDouble(-1, 1) * maxoffset);
		}
	}
}

void Chromosome::Update()
{
  double *temp = genes;
  genes = newgenes;
  newgenes = temp;
}

void Chromosome::CrossOver(Chromosome *ina, Chromosome *inb, 
                           Chromosome *outa, Chromosome *outb, double rate)
{
  if(ina->genecount != inb->genecount) return;

  if(Random::GetDouble() > rate || ina == inb) {
    for(int i = 0; i < ina->genecount; i ++) {
      outa->GetNewGenes()[i] = ina->GetGenes()[i];
      outb->GetNewGenes()[i] = inb->GetGenes()[i];
    }
    return;
  }

  int flippoint = Random::GetInt(ina->genecount + 1);

  for(int i = 0; i < flippoint; i ++) {
    outa->GetNewGenes()[i] = ina->GetGenes()[i];
    outb->GetNewGenes()[i] = inb->GetGenes()[i];
  }

  for(int i = flippoint; i < ina->genecount; i ++) {
    outa->GetNewGenes()[i] = inb->GetGenes()[i];
    outb->GetNewGenes()[i] = ina->GetGenes()[i];
  }
}

int GATrainer::GetWeightCount(LayeredNet *net)
{
  int i = 0;
  forEach(Layer, (*net->GetGroups()), layer) {
    forEach(Neuron, layer->neurons, neuron) {
      forEach(InputSynapse, neuron->inputs, input)
        i ++;
    }
  }
  return i;
}

void GATrainer::SetWeights(LayeredNet *net, Chromosome *chromosome)
{
  int i = 0;
  forEach(Layer, (*net->GetGroups()), layer) {
    forEach(Neuron, layer->neurons, neuron) {
      forEach(InputSynapse, neuron->inputs, input)
        input->weight = chromosome->GetGenes()[i ++];
    }
  }
}

void GATrainer::GetWeights(LayeredNet *net, Chromosome *chromosome)
{
  int i = 0;
  forEach(Layer, (*net->GetGroups()), layer) {
    forEach(Neuron, layer->neurons, neuron) {
      forEach(InputSynapse, neuron->inputs, input)
        chromosome->GetGenes()[i ++] = input->weight;
    }
  }
}


void GATrainer::AddNet(LayeredNet *net)
{
  int weightcount;
  if(nets.GetSize() == 0)
    weightcount = GetWeightCount(net);
  else
    weightcount = ((Chromosome *)chromosomes.Elements())->GetGeneCount();

  nets.Attach(net);

  if(chromosomes.GetSize() < nets.GetSize()) {

    Chromosome *chromosome = new Chromosome(weightcount);
    chromosomes.Attach(chromosome);
  }

  int index = nets.GetIndex(net);
  Chromosome *chromosome = (Chromosome *)chromosomes.Get(index);
  GetWeights(net, chromosome);
}

double GATrainer::Train(int state)
{
  if(nets.GetSize() <= 1) return NetTrainer::TrainError;

  input(inputbuffer, state); // generate the given state (the state parameter may be ignored by the network)
  train(inputbuffer, outputbuffer1);

  Chromosome *chromosome = (Chromosome *)chromosomes.Elements();

  double netmeanerror = 0.0;
  int j = 0;
  forEach(LayeredNet, nets, net) {
    net->Update(inputbuffer, outputbuffer2);

    double meanerror = 0.0;
    int i = 0;
    forEach(Neuron, (*net->GetOutputs()), neuron) {
      double desiredoutput = outputbuffer1[i];
      double actualoutput = outputbuffer2[i]; // == neuron->GetData();

      meanerror += fabs(desiredoutput - actualoutput);
      i ++;
    }

    meanerror /= (double)i;
    netmeanerror += meanerror;

    // translate the error into a fitness score
    // the smaller the error, the greater the score, 
    // the more likely this net will "reproduce"

    //TODO make sure this works properly
    chromosome->Fitness() = exp(1.0 - meanerror);

    chromosome = (Chromosome *)chromosome->Next();
    j ++;
  }

  CreatePopulation();

  netmeanerror /= (double)j;
  return netmeanerror;
}

double GATrainer::GetFitnessTotal()
{
  double total = 0.0;
  forEach(Chromosome, chromosomes, chromosome)
    total += chromosome->Fitness();
  return total;
}

Chromosome* GATrainer::GetChromosome(double value)
{
  double total = 0.0;
  forEach(Chromosome, chromosomes, chromosome) {
    total += chromosome->Fitness();
    if(value <= total) return chromosome;
  }

  return null; 
}

void GATrainer::CreatePopulation()
{
  double total = GetFitnessTotal();

  forEach(Chromosome, chromosomes, chromosome) {
    Chromosome *ina = GetChromosome(Random::GetDouble(0, total));
    Chromosome *inb = GetChromosome(Random::GetDouble(0, total));

    Chromosome *outa = chromosome;
    Chromosome *outb = (Chromosome *)chromosome->Next();
    if(outb == null) outb = outa;

    Chromosome::CrossOver(ina, inb, outa, outb, crossoverrate);

    chromosome = outb;
  }

  Chromosome *newchromosome = (Chromosome *)chromosomes.Elements();
  forEach(LayeredNet, nets, net) {
    newchromosome->Update();
    newchromosome->Mutate(mutationrate, mutationoffset);
    SetWeights(net, newchromosome);

    newchromosome = (Chromosome *)newchromosome->Next();
  }

}

GATrainer::GATrainer(int inputcount, int outputcount) : NetTrainer(inputcount, outputcount),
  mutationrate(0), mutationoffset(0), crossoverrate(0)
{
}


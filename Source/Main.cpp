#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "GANet.h"

//*******************************************
// xor

void addinput(double *buffer, int state)
{
  buffer[0] = Random::GetDouble();
  buffer[1] = Random::GetDouble();
}


void add(double *input, double *output)
{
  output[0] = input[0] + input[1];
}

// creates a new net
// the nets used by GATrainer must have an identical structure
LayeredNet* CreateNet()
{
  LayeredNet *net = new LayeredNet(2, 1);
  net->AddLayer(new Layer(3));
  // net.AddLayer(new Layer(2)); // can add another layer if we want to
  net->ConnectGroups(); // connect all the layers together

  // create additional connections between the input and output layers
  // this might speed up the learning process
  net->GetInputLayer()->Connect(net->GetOutputLayer()); // comment this line to see the difference

  net->SetTransferFunctions(Neuron::SigmoidTransfer);
  net->SetWeights(Neuron::RandomWeights);

  return net;
}


// create a trainer, add Nets to it
// and make them reproduce
GATrainer* trainadd()
{
  GATrainer *trainer = new GATrainer(2, 1);

  trainer->SetInputFunction(addinput);
  trainer->SetTrainFunction(add);

  // set training parameters
  trainer->MutationRate() = 0.2;
  trainer->MutationOffset() = 0.2;
  trainer->CrossOverRate() = 0.7;

  // we will have a population of 100
  for(int i = 0; i < 100; i ++)
    trainer->AddNet(CreateNet());

  // 1000 generations
  int iterationcount = 100;
  double meanerror = 0;

  printf("errors in output:\n");
  for(int i = 0; i < iterationcount; i ++) {
	// train the net, in the case of GATrainer
	// the function spawns a new generation
    meanerror = trainer->Train(0);
    if(i%(iterationcount/20) == 0) printf("%g\n", meanerror);
  }

  return trainer;
}

const int INPUT_COUNT = 3, OUTPUT_COUNT = 1;

void test() {
  int count = 50;

  // create an array of pointers to the nets
  LayeredNet** nets = new LayeredNet*[count];

  // create the nets themselves
  for(int i = 0; i < count; i ++) {
    nets[i] = new LayeredNet(INPUT_COUNT, OUTPUT_COUNT);
    
    nets[i]->AddLayer(new Layer(3));
    // net.AddLayer(new Layer(2)); // can add another layer if we want to
    nets[i]->ConnectGroups(); // connect all the layers together

    // create additional connections between the input and output layers
    // this might speed up the learning process
    nets[i]->GetInputLayer()->Connect(nets[i]->GetOutputLayer()); // comment this line to see the difference

    nets[i]->SetTransferFunctions(Neuron::SigmoidTransfer);
    nets[i]->SetWeights(Neuron::RandomWeights);
  }

  // to access a layered net number 10
  // nets[10]->AddLayer()


  // to free up memory

  // delete the nets
  for(int i = 0; i < count; i ++)
  delete nets[i];

  // delete the array of pointers to the nets
  delete[] nets;
}

void testserialize(LayeredNet *net) 
{
  FILE *out = fopen("test.net", "wb");
  net->Save(out);
  fclose(out);

  FILE *in = fopen("test.net", "rb");
  net->Load(in);
  fclose(in);
}

void main()
{
  // get a trainer
  GATrainer *trainer = trainadd();
  printf("Training completed!\n");

  // get the first (random) Net from the trainer
  // we could try getting the average result from the all the nets instead
  LayeredNet *net = (LayeredNet *)trainer->GetNets()->Elements();


  while(true) {
    double in[2], out[1];
    float f;
    scanf("%f", &f);
    in[0] = f;
    printf("x\n");
    scanf("%f", &f);
    in[1] = f;

    testserialize(net);

    if(net->Update(in, out)) {
      printf("=%g\n", out[0]);
    } else
      printf( "Update failed: there's something wrong with the structure of the neural net\n"
              "               check for loops and neurons without inputs\n");
  }

  // remove all the nets weve created
  trainer->RemoveNets();
  delete trainer;
}

#include <stdio.h>
#include <stdlib.h>
#include "BPNet.h"

// additional examples will be included here


double inputs[][6] = {
{0.1, 0.2, 0.4, 0.22, 0.78, 0.9},    
{0.3, 0.6, 0.4, 0.32, 0.18, 0.1},    
{0.2, 0.7, 0.4, 0.72, 0.28, 0.2},    
{0.4, 0.8, 0.4, 0.72, 0.38, 0.4},    
{0.1, 0.8, 0.4, 0.12, 0.48, 0.7},    
{0.7, 0.2, 0.4, 0.62, 0.78, 0.6},    
{0.9, 0.1, 0.4, 0.27, 0.88, 0.0},    
{0.7, 0.2, 0.4, 0.27, 0.72, 0.3},    
{0.1, 0.2, 0.4, 0.29, 0.71, 0.9} };
  
// expected outputs
double outputs[] = { 
  1,
  0,
  1,
  1,
  0,
  1,
  0,
  1,
  1 };

void stockinput(double *input, int state)
{
  // select a random input, this will most likely come
  // from some other source than memory
  int r = Random::GetInt(0, 9); 
  
  // copy input from one buffer to another
  for(int i = 0; i < 6; i ++)
    input[i] = inputs[r][i];
}

void stocktrain(double *input, double *output)
{
  // match the input with the output
  // in a real application there might be some other
  // function which provides us with an expected output
  int i = 0;
  for(; i < 9; i ++) {
    int j = 0;
    for(; j < 6; j ++) {
      if(inputs[i][j] != input[j]) break;
    }

    // if we've found a match
    if(j == 6) break;
  }

  if(i == 9) {
    // error couldn't find a matching input
  } else
    output[0] = outputs[i];
}


void train(LayeredNet *net)
{
  BPTrainer trainer(6, 1); // 6 inputs, 1 output, the net must have the right size as well
  net->SetWeights(Neuron::RandomWeights);
  trainer.SetInputFunction(stockinput);
  trainer.SetTrainFunction(stocktrain);

  int iterationcount = 100000;
  double precision = 0;

  printf("errors in output:\n");
  trainer.Step() = 10; // we set the step here
  for(int i = 0; i < iterationcount; i ++) {
    precision = trainer.Train(net, 0); // ignore state, set step to 10.0
    if((i+1)%(iterationcount/20) == 0) printf("%g\n", precision);
  }
}


//*** LayeredNet test

//*******************************************
// xor
/*
void xorinput(double *input, int state)
{
  switch(state) {
    case 0:
      input[0] = 0;
      input[1] = 0;
      break;
    case 1:
      input[0] = 1;
      input[1] = 0;
      break;
    case 2:
      input[0] = 0;
      input[1] = 1;
      break;
    case 3:
      input[0] = 1;
      input[1] = 1;
      break;
  }
}

void xor(double *input, double *output)
{
  if(input[0] == 0) {
    if(input[1] == 0) output[0] = 0;
    else output[0] = 1;
  } else {
    if(input[1] == 0) output[0] = 1;
    else output[0] = 0;
  }
}

void trainxor(LayeredNet *net)
{
  BPTrainer trainer(2, 1);
  net->SetWeights(Neuron::RandomWeights);
  trainer.SetInputFunction(xorinput);
  trainer.SetTrainFunction(xor);

  int iterationcount = 100000;
  double precision = 0;
  
  trainer.Step() = 2;

  printf("errors in output:\n");
  for(int i = 0; i < iterationcount; i ++) {
    for(int j = 0; j < 4; j ++) {
      // the learning rate is greatly affected by the learning step, which is the 2nd parameter
      precision = trainer.Train(net, j);
      if(i%(iterationcount/20) == 0) printf("%g\t", precision);
    }   
    if(i%(iterationcount/20) == 0) printf("\n");
  }
}

//*******************************************

// addition (within the range 0-1)

// generate random input
void addinput(double *buffer, int state)
{
  const double lower = 0, upper = 1;
  double s = upper - lower;
  for(int i = 0; i < 2; i ++) {
    double r = (double)Random::GetInt()/(double)RAND_MAX;
    buffer[i] = s * r + lower;
  }
}

// generate categorized input with increments of 0.1
// this results in much faster learning
// we assume state goes from 0 to 99
void addinput2(double *buffer, int state)
{
  int a = state / 10, b = state % 10;
  buffer[0] = (double)a * 0.1;
  buffer[1] = (double)b * 0.1;
}


void add(double *input, double *output)
{
  output[0] = input[0] + input[1];
}

void trainadd(LayeredNet *net)
{
  BPTrainer trainer(2, 1);
  net->SetWeights(Neuron::RandomWeights);
  trainer.SetInputFunction(addinput2); // try replacing addinput2 with addinput or vice versa to see the speed/quality difference
  trainer.SetTrainFunction(add);

  int iterationcount = 100000;
  double precision = 0;

  printf("errors in output:\n");
  trainer.Step() = 10;
  for(int i = 0; i < iterationcount; i ++) {
    // the learning rate is greatly affected by the learning step, which is the 2nd parameter
    precision = trainer.Train(net, i%100); 
    if(i%(iterationcount/20) == 0) 
      printf("%g\n", precision);
  }
}

//*******************************************

// prints the number of outputs at each neuron starting with the input layer
void draw(NeuralNet *net)
{
  forEach(Group, (*net->GetGroups()), group) {
    forEach(Neuron, (*group->GetOutputs()), neuron)
      printf("%d ", neuron->outputs.GetSize());
    printf("\n");
  }

}


void testBP()
{
  LayeredNet net(2, 1);

  net.AddLayer(new Layer(3));
  // net.AddLayer(new Layer(2)); // can add another layer if we want to
  net.ConnectGroups(); // connect all the layers together

  // create additional connections between the input and output layers
  // this might speed up the learning process
  net.GetInputLayer()->Connect(net.GetOutputLayer()); // comment this line to see the difference

  net.SetTransferFunctions(Neuron::SigmoidTransfer);
  
  trainxor(&net);
  printf("Training completed!\n");

  while(true) {
    double in[2], out[1];
    float f;
    scanf("%f", &f);
    in[0] = f;
    printf("x\n");
    scanf("%f", &f);
    in[1] = f;

    if(net.Update(in, out)) {
      printf("=%g\n", out[0]);
    } else
      printf( "Update failed: there's something wrong with the structure of the neural net\n"
              "               check for loops and neurons without inputs\n");
  }
}

*/
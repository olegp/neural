#include <stdio.h>
#include <math.h>
#include "BPNet.h"

double BPTrainer::Train(LayeredNet *net, int state)
{
  Container *outputs = net->GetOutputs();
  input(inputbuffer, state); // generate the given state (the state parameter may be ignored by the network)

  train(inputbuffer, outputbuffer1);
  net->Update(inputbuffer, outputbuffer2);

  double meanerror = 0.0;
  // set the output layer errors and input weights
  int i = 0;
  forEach(Neuron, (*outputs), neuron) {
    double desiredoutput = outputbuffer1[i];
    double actualoutput = outputbuffer2[i]; // == neuron->GetData();

    double errorfactor = actualoutput * (1 - actualoutput) * (desiredoutput - actualoutput);
    neuron->TempDouble() = errorfactor; // store error factor in temp

    forEach(InputSynapse, (neuron->inputs), inputsynapse) {
      double data = inputsynapse->GetConnectedData();
      double weightdelta = step * errorfactor * data;
      inputsynapse->temp = inputsynapse->weight + weightdelta;
    }

    meanerror += fabs(desiredoutput - actualoutput);
    i ++;
  }

  meanerror /= (double)i;

  // process the other layers
  for(Layer *layer = (Layer *)net->GetOutputLayer()->Prev(); layer != net->GetInputLayer(); layer = (Layer *)layer->Prev()) {
    
    //printf("%d", layer->neurons.GetSize());

    forEach(Neuron, layer->neurons, neuron) {
      double errorfactor = 0;

      // GetConnectedNeuron()->GetTemp() returns the error factor of the neuron in the next layer
      forEach(OutputSynapse, neuron->outputs, outputsynapse)
        errorfactor += outputsynapse->GetConnectedNeuron()->TempDouble() * outputsynapse->GetWeight(); 
      
      double data = neuron->Data();
      errorfactor *= data * (1 - data);
      neuron->TempDouble() = errorfactor;

      forEach(InputSynapse, (neuron->inputs), inputsynapse) {
        double data = inputsynapse->GetConnectedData();
        double weightdelta = step * errorfactor * data;
        inputsynapse->temp = inputsynapse->weight + weightdelta;
      }
    }

  }

  // set the new weights
  net->SetWeights(null); //TODO move to NeuralNet::Reset, so we don't have to cycle twice
  return meanerror;
}

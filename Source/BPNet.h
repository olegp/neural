#pragma once

#include "NeuralNet.h"

// a simple Back Propagation Net is made up only of layers
// for a basic introduction see http://www.dontveter.com/bpr/public2.html
class BPTrainer : public NetTrainer
{
protected:
  double step;

public:

  // this is the step parameter
  // good values are in the range 0.1 to 10
  // if the step is too small training will take a very long time
  // if it's too high, the train method might miss the minimum
  inline double& Step() { return step; }

  // train a single net
  double Train(LayeredNet *net, int state);

  // creates a trainer
  BPTrainer(int inputcount, int outputcount) : NetTrainer(inputcount, outputcount) {}
};

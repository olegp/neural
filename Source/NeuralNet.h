#pragma once

#include "Util.h"

// info on neural nets is available at ftp://ftp.sas.com/pub/neural/FAQ.html

// synapses are used to connect neurons together
// a connection is made up of 2 such synapses: output and input
// if you delete either one of them, the other is deleted as well
// and the connection is removed
class Synapse : public Element
{
protected:
  friend class Neuron;

  Neuron *neuron;
  Synapse *synapse;

public:
  inline Synapse* GetConnectedSynapse() { return synapse; }

  virtual double GetConnectedData();
  virtual Neuron* GetConnectedNeuron();
  virtual double GetWeight() = 0;

  Synapse();
  virtual ~Synapse();
};

// the output connector points to the input connector
// that in turn points to the neuron which the output connector
// connects to
class OutputSynapse : public Synapse
{
public:

  double GetWeight();
};

// the input connector is almost identical to the output connector
// it also carries the weight of this connection
class InputSynapse : public Synapse
{
protected:

public:
  double weight;
  double temp;


  double GetWeight();

  InputSynapse();
};

// the bias synapse doesn't actually have a synapse at the other end
// GetConnectedData() returns a constant value 1
class Bias : public InputSynapse
{
public:
  double GetConnectedData();
  Neuron* GetConnectedNeuron();
};


typedef double (*TRANSFERFUNCTION)(double input);
typedef double (*WEIGHTFUNCTION)(void);

// a neuron describes the most basic block of a neural network
// neurons are contained in neuron groups, see class Group
// collections of interconnected groups in turn make up the network
class Neuron : public Element
{
protected:

  double data;  // used to store the actual data
  
  // temporary values, may used in training
  double tempdouble;  
  int tempint;

  TRANSFERFUNCTION TransferFunction;

  Bias *bias;

public:

  const static double NoData, ThisNeuron;
  
  // InputSynapse
  CleanContainer inputs;

  // OutputSynapse
  CleanContainer outputs;
  
  // set the neuron data, this is the value passed
  // on by the neuron to neurons further down the line
  inline double& Data() { return data; }

  inline double& TempDouble() { return tempdouble; }
  inline int& TempInt() { return tempint; }

  void SetBias(double bias);
  
  double GetBias();

  // this is the most simple transfer function
  static double LinearTransfer(double input);

  // this is a very common transfer function
  static double StepTransfer(double intput);

  // sigmoid transfer function
  static double SigmoidTransfer(double input);

  // ...
  // other activation (or transfer) functions can be added here

  // weight functions
  static double ZeroWeights();
  static double OneWeights();
  static double RandomWeights();

  // connects "neuron" to this one
  // weight specifies the weight of the connection
  void AddInput(Neuron *neuron, double weight = 0.0);

  // connects this neuron to "neuron"
  // the data from this neuron will now flow there as well
  // weight specifies the weight of the connection
  void AddOutput(Neuron *neuron, double weight = 0.0);

  // sets the weights
  void SetWeights(WEIGHTFUNCTION weight);

  // sets the transfer function
  inline void SetTransferFunction(TRANSFERFUNCTION transfer) { TransferFunction = transfer; }

  // clears the data
  inline void Reset() { data = tempdouble = NoData; tempint = 0;}

  // updates this neuron's data by updating the neurons it depends on
  // summing the weighed inputs and applying the transfer function
  // the result is stored in data and returned from the function
  double Update();

  Neuron();
  Neuron(double data);
  Neuron(double data, double bias);
};


// class Group describes a group of neurons
// it makes it easier to connect such groups together

// Layer inherits from Group
// NeuralNet does as well, which means that a separate neural network can plugged into another
class Group : public Element
{
public:

  // returns the list of input neurons as a container
  virtual ::Container* GetInputs() = 0;

  // returns the list of output neurons as a container
  virtual ::Container* GetOutputs() = 0;

  // sets the weights for all the neurons in this group
  // weight is a function, see class Neuron
  virtual void SetWeights(WEIGHTFUNCTION weight) = 0;

  // sets the transfer function for all the neurons in this group
  // transfer is a function, see class Neuron
  virtual void SetTransferFunctions(TRANSFERFUNCTION transfer) = 0;

  // updates the neurons starting with the output ones first
  // see Neuron::Update
  virtual bool Update();

  // connects every single output neuron in this group to
  // every input neuron in "group"
  virtual void Connect(Group *group);

  // connects the neurons with a one to one mapping
  virtual void ConnectParallel(Group *group);

  // connects all the neurons in this group to "neuron"
  virtual void Connect(Neuron *neuron);

  // clears the data for every neuron in this group
  virtual void Reset() = 0;
  };


// a Layer is the most basic neuron Group
// where each input neuron is also an output
// there are no connections between neurons in a Layer

// to implement something like a Kohonen network you would need to extend this class
// and set up connections within the layer as well
class Layer : public Group
{
public:
  CleanContainer neurons;

  // returns the list of input neurons as a container
  ::Container* GetInputs();

  // returns the list of output neurons as a container
  ::Container* GetOutputs();

  // sets the weights for all the neurons in this group
  // weight is a function, see class Neuron
  void SetWeights(WEIGHTFUNCTION weight);

  // sets the transfer function for all the neurons in this group
  // transfer is a function, see class Neuron
  void SetTransferFunctions(TRANSFERFUNCTION transfer);

  // resets all the neurons in this group
  void Reset();

  // creates a new layer with the specified number of neurons
  Layer(int size);
};

// a neural net describes a collection of neuron groups
class NeuralNet : public Group
{
protected:
  Layer *input, *output;
  CleanContainer groups;

public:

  // returns the list of input neurons as a container
  ::Container* GetInputs();

  // returns the list of output neurons as a container
  ::Container* GetOutputs();

  inline Layer* GetInputLayer() { return input; }
  inline Layer* GetOutputLayer() { return output; }

  inline ::Container* GetGroups() { return &groups; }

  // adds a Group to the list of groups, treating it as a layer
  // it is inserted between the last hidden layer and the output layer
  void AddGroup(Group *group);

  void ConnectGroups();

  // sets the weights for all the neurons in this net
  // weight is a function, see class Neuron
  void SetWeights(WEIGHTFUNCTION weight);

  // sets the transfer function for all the neurons in this net
  // transfer is a function, see class Neuron
  void SetTransferFunctions(TRANSFERFUNCTION transfer);

  // feeds the contents of the inputbuffer to the input neurons
  // and grabs the output from the output neurons
  // neural nets are updated recursively starting from the output neurons first
  // see Neuron::Update
  bool Update(double *inputbuffer, double *outputbuffer);

  // resets all the neurons in this net by removing any data they contain
  // called internally in Update
  void Reset();

  // bool Verify();
  // bool Clean();

  // creates a new net with the specified number of input and output neurons
  NeuralNet(int inputcount, int outputcount);
};


// a layered net is a simple net which consists only of layers
// remember to call ConnectGroups after the layers have been added
class LayeredNet : public NeuralNet
{
public:
  inline void AddLayer(Layer *layer) { AddGroup(layer); }
  LayeredNet(int inputcount, int outputcount) : NeuralNet(inputcount, outputcount) {}

  Neuron* GetNeuron(int groupindex, int neuronindex);

  virtual bool Save(FILE *file);
  virtual bool Load(FILE *file);
};


typedef void (*INPUTFUNCTION)(double *input, int state);
typedef void (*TRAINFUNCTION)(double *input, double *output);

// the NetTrainer class provides the basic facilities for
// training a network
class NetTrainer
{
protected:
  double *inputbuffer;
  double *outputbuffer1;
  double *outputbuffer2;

  int inputcount, outputcount;

  INPUTFUNCTION input;
  TRAINFUNCTION train;

public:

  const static double TrainError;

  inline void SetInputFunction(INPUTFUNCTION input) { this->input = input; }
  inline void SetTrainFunction(TRAINFUNCTION train) { this->train = train; }

  // virtual double Train(int state) = 0;
  
  NetTrainer(int inputcount, int outputcount);
  ~NetTrainer();
};


class Arena
{

};

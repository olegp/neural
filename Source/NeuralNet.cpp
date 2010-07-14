#include <math.h>
#include <stdlib.h>
#include <float.h>
#include "NeuralNet.h"

double Synapse::GetConnectedData()
{
  return synapse->neuron->Data();
}

Neuron* Synapse::GetConnectedNeuron()
{
  return synapse->neuron;
}

Synapse::Synapse() : neuron(null), synapse(null)
{
}

Synapse::~Synapse() 
{
  if(synapse != null) {
    synapse->synapse = null;
    delete synapse;
  }
}

double OutputSynapse::GetWeight()
{
  return ((InputSynapse *)synapse)->weight;
}

double InputSynapse::GetWeight()
{
  return weight;
}

InputSynapse::InputSynapse() : weight(0.0), temp(0.0)
{
}

double Bias::GetConnectedData()
{
  return 1.0;
}

Neuron* Bias::GetConnectedNeuron()
{
  return null;
}

const double Neuron::NoData = DBL_MIN;
const double Neuron::ThisNeuron = DBL_MAX;

double Neuron::LinearTransfer(double input)
{
  return input;
}

double Neuron::StepTransfer(double input)
{
  return input > 0.0 ? 1.0 : 0.0;
}

double Neuron::SigmoidTransfer(double input)
{
  const double p = 1.0;
  return 1.0/(1.0 + exp(-input / p));
}

double Neuron::ZeroWeights()
{
  return 0;
}

double Neuron::OneWeights()
{
  return 1;
}

double Neuron::RandomWeights()
{
  return Random::GetDouble(-1, 1);
}


void Neuron::SetWeights(WEIGHTFUNCTION weight)
{
  if(weight != null) {
    forEach(InputSynapse, inputs, connector)
      connector->weight = weight();
  } else {
    forEach(InputSynapse, inputs, connector)
      connector->weight = connector->temp;
  }
}

void Neuron::AddInput(Neuron *neuron, double weight)
{
  neuron->AddOutput(this, weight);
}

void Neuron::AddOutput(Neuron *neuron, double weight)
{
  OutputSynapse *output = new OutputSynapse();
  output->neuron = this;
  outputs.Attach(output);

  InputSynapse *input = new InputSynapse();
  input->neuron = neuron;
  neuron->inputs.Attach(input);

  input->weight = weight;

  output->synapse = input;
  input->synapse = output;
}

double Neuron::Update()
{
  double sum = 0.0;

  data = ThisNeuron;

  forEach(InputSynapse, inputs, input) {
    double inputdata = input->GetConnectedData();

    if(inputdata == NoData)
      inputdata = input->GetConnectedNeuron()->Update();
    // check for errors: loops
    else if(inputdata == ThisNeuron) 
      return 0.0;
    
    sum += inputdata * input->weight;
  }

  // printf("Neuron::Update: %d-%d\n", inputs.GetSize(), outputs.GetSize());
  data = TransferFunction(sum);
  return data;
}

void Neuron::SetBias(double b) 
{
  if(bias != null) bias->weight = b;
  else {
    bias = new Bias();
    bias->weight = b;
    inputs.Attach(bias);
  }
}

double Neuron::GetBias() 
{
  if(bias != null) return bias->GetWeight();
  return 0.0;
}

Neuron::Neuron() : TransferFunction(null), bias(null), data(NoData)
{
}

Neuron::Neuron(double d) : TransferFunction(null), bias(null), data(d)
{
}

Neuron::Neuron(double d, double b) : TransferFunction(null), data(d)
{
  bias = new Bias();
  bias->weight = b;
  inputs.Attach(bias);
}

bool Group::Update()
{
  bool ret = true;
  ::Container *outputs = GetOutputs();
  forEach(Neuron, (*outputs), output) {
    double data = output->Update();
    if(data == 0.0) ret = false;
  }
  return ret;
}

void Group::Connect(Group *group)
{
  ::Container *outputs = GetOutputs();
  ::Container *inputs = group->GetInputs();

  forEach(Neuron, (*outputs), thisneuron) {
    forEach(Neuron, (*inputs), thatneuron)
      thisneuron->AddOutput(thatneuron);
  }
}

void Group::ConnectParallel(Group *group)
{
  Neuron *thisneuron = (Neuron *)GetOutputs()->Elements();
  Neuron *thatneuron = (Neuron *)group->GetInputs()->Elements();

  for(; thisneuron != null && thatneuron != null; 
    thisneuron = (Neuron *)thisneuron->Next(), thatneuron = (Neuron *)thatneuron->Next())
    thisneuron->AddOutput(thatneuron);
}

void Group::Connect(Neuron *neuron)
{
  ::Container *outputs = GetOutputs();

  forEach(Neuron, (*outputs), thisneuron)
    thisneuron->AddOutput(neuron);
}



Container* Layer::GetInputs()
{
  return &neurons;
}
  
Container* Layer::GetOutputs()
{
  return &neurons;
}

void Layer::SetWeights(WEIGHTFUNCTION weight)
{
  forEach(Neuron, neurons, neuron)
    neuron->SetWeights(weight);
}

void Layer::SetTransferFunctions(TRANSFERFUNCTION transfer)
{
  forEach(Neuron, neurons, neuron)
    neuron->SetTransferFunction(transfer);
}

Layer::Layer(int size)
{
  for(int i = 0; i < size; i ++) {
    // add a neuron with bias
    Neuron *neuron = new Neuron(0.0, 0.0);
    neurons.Attach(neuron);
  }
}

void Layer::Reset()
{
  forEach(Neuron, neurons, neuron)
    neuron->Reset();
}


Container* NeuralNet::GetInputs()
{
  return &input->neurons;
}

Container* NeuralNet::GetOutputs()
{
  return &output->neurons;
}

void NeuralNet::AddGroup(Group *group)
{
  groups.AttachBefore(group, output);
  Group *inputgroup = (Group *)group->Prev();
}

void NeuralNet::ConnectGroups()
{
  forEach(Group, groups, group) {
    if(group->Next()) {
      group->Connect((Group *)group->Next());
    }
  }
}


void NeuralNet::SetWeights(WEIGHTFUNCTION weight)
{
  forEach(Group, groups, group)
    group->SetWeights(weight);
}

void NeuralNet::SetTransferFunctions(TRANSFERFUNCTION transfer)
{
  forEach(Group, groups, group)
    group->SetTransferFunctions(transfer);
}

bool NeuralNet::Update(double *inputbuffer, double *outputbuffer)
{
  Reset();

  int i = 0;
  forEach(Neuron, (input->neurons), in)
    in->Data() = inputbuffer[i ++];

  if(output->Update()) {
    int j = 0;
    forEach(Neuron, (output->neurons), out)
      outputbuffer[j ++] = out->Data();
    return true;
  }

  return false;
}

void NeuralNet::Reset()
{
  forEach(Group, groups, group)
    group->Reset();
}


NeuralNet::NeuralNet(int inputcount, int outputcount)
{
  input = new Layer(inputcount);
  output = new Layer(outputcount);

  groups.Attach(input);
  groups.AttachLast(output);
}

Neuron* LayeredNet::GetNeuron(int groupindex, int neuronindex)
{
  Layer *layer = (Layer *)groups.Get(groupindex);
  if(layer == null)
    return null;

  return (Neuron *)layer->neurons.Get(neuronindex);
}


bool LayeredNet::Save(FILE *file) 
{
  // set the group number for each neuron
  int i = 0;
  forEach(Layer, groups, layer) {
    forEach(Neuron, layer->neurons, neuron) {
      neuron->TempInt() = i;    
    }
    i ++;
  }

  int groupssize = groups.GetSize();
  fwrite(&groupssize, sizeof(int), 1, file);

  i = 0;
  forEach(Layer, groups, layer) {
    int layersize = layer->neurons.GetSize();
    fwrite(&layersize, sizeof(int), 1, file);

    forEach(Neuron, layer->neurons, neuron) {
      int weightcount = neuron->inputs.GetSize();
      fwrite(&weightcount, sizeof(int), 1, file);

      forEach(InputSynapse, neuron->inputs, input) {
        int groupindex, neuronindex;
        Neuron *neuron = input->GetConnectedNeuron();
        groupindex = neuron == null ? -1 : neuron->TempInt();
        
        Layer *thislayer = (Layer *)groups.Get(groupindex);
        neuronindex = neuron == null ? -1 : thislayer->neurons.GetIndex(neuron);
        
        double weight = input->GetWeight();
        
        fwrite(&groupindex, sizeof(int), 1, file);
        fwrite(&neuronindex, sizeof(int), 1, file);
        fwrite(&weight, sizeof(double), 1, file);
      }
    }
    i ++;
  }

  return true;
}

bool LayeredNet::Load(FILE *file)
{
  groups.Empty();
  input = output = null;

  int groupssize;
  fread(&groupssize, sizeof(int), 1, file);
  for(int i = 0; i < groupssize; i ++) {
    int layersize;
    fread(&layersize, sizeof(int), 1, file);
    Layer *layer = new Layer(layersize);
    groups.AttachLast(layer);
    for(int j = 0; j < layersize; j ++) {
      int weightcount;
      fread(&weightcount, sizeof(int), 1, file);

      for(int k = 0; k < weightcount; k ++) {
        int groupindex, neuronindex;
        double weight;
        fread(&groupindex, sizeof(int), 1, file);
        fread(&neuronindex, sizeof(int), 1, file);
        fread(&weight, sizeof(double), 1, file);
        
        if(groupindex == -1) {
          ((Neuron *)layer->neurons.Get(j))->SetBias(weight);
        } else {
          Neuron *neuron = GetNeuron(groupindex, neuronindex);
          
          // todo: speed up
          // note: needs to be feedforward network for this to work
          // however if it isn't feedforward, it isn't valid, so this isn't an issue
          if(neuron != null) {
            Neuron *thisneuron = (Neuron *)layer->neurons.Get(j);
            thisneuron->AddInput(neuron, weight);        
          }
        }
      }

    }
  }

  if(groups.Elements() != null) {
    output = input = (Layer *)groups.Elements();
    while(output->Next() != null) output = (Layer *)output->Next();
  }

  SetTransferFunctions(Neuron::SigmoidTransfer);
  return true;
}

NetTrainer::NetTrainer(int in, int out) : inputcount(in), outputcount(out), input(null), train(null)
{
  inputbuffer = new double[inputcount];
  outputbuffer1 = new double[outputcount];
  outputbuffer2 = new double[outputcount];
}

NetTrainer::~NetTrainer()
{
  delete[] inputbuffer;
  delete[] outputbuffer1;
  delete[] outputbuffer2;
}

const double NetTrainer::TrainError = DBL_MAX;


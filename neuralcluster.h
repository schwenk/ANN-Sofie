#ifndef NEURALCLUSTER_H
#define NEURALCLUSTER_H


#include <vector>
#include <iostream>
#include <cmath>
#include <QDebug>

//Ideas:
//Probailistic Firering
//Dropout
//Momentum

using namespace std;

class NeuralCluster
{
public:
    NeuralCluster(int inputs, int outputs, int hidden);

    void propergate(vector<float> input, vector<float> output, bool sleep);
    vector<vector<float>> getWeights();
    void train(float learningRate);
    void trainBP(vector<float> target,float learningRate,int iterations);
    vector<float> getTarget();
    float signum(float x);

    vector<float> getActivation();

    void syncronize();


private:

    int                   maxPeriod = 1;

    vector<float>         fireCounter;
    vector<float>         counterActivation;
    vector<float>         lastCounter;
    vector<float>         polarityCounter;
    vector<float>         fireReal;
    vector<float>         realActivation;
    vector<float>         lastReal;
    vector<float>         beforelastReal;
    vector<float>         polarityReal;

    vector<int>           counter;
    vector<int>           period;

    vector<float>         slowness;

    vector<vector<float>> weights;
    int                   numInputs,numOutputs,numHiddens;

    vector<float>         error;

};

#endif // NEURALCLUSTER_H

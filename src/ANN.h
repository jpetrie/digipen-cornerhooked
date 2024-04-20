/*!
	@file		ANN.h
	@author		Scott
	@date		07-05-2004
	@ingroup	ANN
	@todo		
	@brief		Implements a feed forward neural network with backpropogation. 
	
	(c) 2004 DigiPen (USA) Corporation, all rights reserved.
*//*__________________________________________________________________________*/

#pragma once

#include <vector>
#include <stdarg.h>
#include <cmath>
#include <fstream>
#include <windows.h>
#include "nsl_random.h"

namespace AI
{

typedef float(*TransferFn)(float);

static float _sigmoid(float f)
{
	return (1.f/(1.f + exp(-f)));
}

/*!
	@class		NeuralNet
	@author		Scott Smith
	@date		07-05-2004
	@ingroup	ANN
	@brief		Implements a feedforward, backpropagation artificial neural 
				network of arbitrary size.
*//*__________________________________________________________________________*/
class NeuralNet
{
private:
	/*!
		@class	Neuron
		@author Scott
		@date	07-05-2004
		@ingroup ANN
		@brief	Internal data structure representing a node in a layer.
	*//*__________________________________________________________________________*/
	struct Neuron	// for lack of a better term
	{
	public:
		Neuron():mOutput(0.f), mError(0.f) {}

		std::vector< float >	mWeights;
		float					mOutput;
		float					mError;
	};

	/*!
		@struct	NNLayer
		@author Scott
		@date 07-05-2004
		@ingroup ANN
		@brief	Internal data structure representing a layer in the network.
	*//*__________________________________________________________________________*/
	struct NNLayer
	{
		enum LayerType{INPUT = 0, HIDDEN, OUTPUT};
		
		NNLayer(int sz = 1, LayerType type = HIDDEN):Neurons(sz), Kind(type) {}
	
		std::vector< Neuron > Neurons;
		LayerType Kind;
	};

public:
	NeuralNet(int layers,...);
	NeuralNet(std::string file);
	virtual ~NeuralNet() {}
	
	void Init(void);
	void Train(std::vector< float > inputs, std::vector< float > outputs, int iterations = 1);
	std::vector< float > Run(std::vector< float > inputs);
	void Load(std::string input);
	void Save(std::string output);
	
protected:
	TransferFn				Sigmoid;
	std::vector< NNLayer >	NetLayers;
	float					LearningRate;
};

} // namespace AI

#include "ANN.inl"




/*!
	@file		ANN.inl
	@author		Scott
	@date		07-12-2004
	@ingroup	ANN
	@brief		Implements a feed forward neural network with backpropogation. 
*//*__________________________________________________________________________*/

namespace AI
{
/*!
	Constructor
	@param layers The number of layers in the network.
	@param ... 
	@brief	This function takes a number of layers for input, followed by the
			size of each layer as a variable argument list.
*//*__________________________________________________________________________*/
inline NeuralNet::NeuralNet(int layers,...):Sigmoid(_sigmoid), LearningRate(.3f) 
{
	va_list Layers;								// init the var arg mech.

	va_start( Layers, layers );
	int sz = va_arg( Layers, int );				// setup the arg list for traversal
	for(int i = 0; i < layers; ++i)
	{
		NNLayer::LayerType type;

		if(i == 0)								// determine the layer type
			type = NNLayer::INPUT;
		else if(i == layers - 1) 
			type = NNLayer::OUTPUT;
		else
			type = NNLayer::HIDDEN;

		if(type == NNLayer::INPUT)
			NetLayers.push_back(NNLayer(sz+1, type));	// build a layer
		else
			NetLayers.push_back(NNLayer(sz, type));

		sz = va_arg( Layers, int );				// get the next argument
	}
	va_end( Layers );	// clean up the var arg stack

	Init();
}
/*!
	Constructor
	@param file The name of the neural network data file to run from.
*//*__________________________________________________________________________*/
inline NeuralNet::NeuralNet(std::string file):Sigmoid(_sigmoid), LearningRate(.3f) 
{
	Load(file);
}

/*!
	@brief Initialize the weights to small random values
*//*__________________________________________________________________________*/
inline void NeuralNet::Init(void)
{
	random r(GetTickCount());
	// for each layer
	for(unsigned int i = 1; i < NetLayers.size(); ++i)
	{
		// for each node in the layer, setup links between the node and each node
		// in the next layer
		for(unsigned int j = 0; j < NetLayers[i].Neurons.size(); ++j)
		{
			// clear the values of the weights
			NetLayers[i].Neurons[j].mWeights.clear();
			// resize the vector for filling
			NetLayers[i].Neurons[j].mWeights.resize(NetLayers[i-1].Neurons.size());
			// set each edge to a small random value;
			for(unsigned int k = 0; k < NetLayers[i].Neurons[j].mWeights.size(); ++k)
			{
				// this sets between 0 and 1
				float w = r.rand_float();
				NetLayers[i].Neurons[j].mWeights[k] = w;
			}
		}
	}
}
/*!
	@param inputs		A std::vector of input values for the network.
	@param outputs		A std::vector of output values, corresponding to the 
						input values in 'inputs'.
	@param iterations	The number of iterations to perform on the training set.
*//*__________________________________________________________________________*/
inline void NeuralNet::Train(std::vector< float > inputs, std::vector< float > outputs, int iterations)
{
	// check that the size of the input vector matches the input layer
	if(inputs.size() != NetLayers[0].Neurons.size() - 1)
	{
		MessageBox(0, "invalid input size", "invalid input size", MB_OKCANCEL);
		exit(1);
	}
	
	// perform a number of iterations
	for(int its = 0; its < iterations; ++its)
	{
	
		// set the input values
		NetLayers[0].Neurons[0].mOutput = 1.f;
		for(unsigned int i = 1; i < NetLayers[0].Neurons.size(); ++i)
		{
			NetLayers[0].Neurons[i].mOutput = inputs[i-1];
		}

		// do the hard stuff
		// feed the input through the network
		// sum up the products (ouput * weight) for each node
		for(unsigned int i = 1; i < NetLayers.size(); ++i)
		{
			for(unsigned int j = 0;  j < NetLayers[i].Neurons.size(); ++j)
			{
				float out = 0.f, weight = 0.f;
				float sum = 0.f;
				for(unsigned int k = 0; k < NetLayers[i-1].Neurons.size(); ++k)
				{
					out = NetLayers[i-1].Neurons[k].mOutput;
					weight = NetLayers[i].Neurons[j].mWeights[k];
					sum += NetLayers[i-1].Neurons[k].mOutput * NetLayers[i].Neurons[j].mWeights[k];
				}
				NetLayers[i].Neurons[j].mOutput = Sigmoid(sum);
			}
		}
/*-----------------------------------------------------------------------------------------------------------*/
		// backpropogation algorithm
		// calculate the error term for each output
		int layer = (int)NetLayers.size() - 1;
		for(unsigned int i = 0; i < NetLayers[layer].Neurons.size(); ++i)
		{
			NetLayers[layer].Neurons[i].mError = NetLayers[layer].Neurons[i].mOutput * 
										(1.f - NetLayers[layer].Neurons[i].mOutput) * 
										(outputs[i] - NetLayers[layer].Neurons[i].mOutput);
		}

		if(NetLayers.size() > 2) // skip if there are no hidden layers
		{
			// calculate the error term at each hidden unit
			for(unsigned int i = (unsigned int)NetLayers.size() - 1; i > 0; --i)
			{
				for(unsigned int j = 0; j < NetLayers[i-1].Neurons.size(); ++j)
				{
					float sum = 0.f;
					for(unsigned int k = 0; k < NetLayers[i].Neurons.size(); ++k)
					{
						sum += NetLayers[i].Neurons[k].mError * NetLayers[i].Neurons[k].mWeights[j];
					}
					NetLayers[i-1].Neurons[j].mError = NetLayers[i-1].Neurons[j].mOutput * 
														(1.f - NetLayers[i-1].Neurons[j].mOutput) * sum;
				}
			}
		}

		// update the weights
		for(unsigned int i = 1; i < NetLayers.size(); ++i)
		{
			for(unsigned int j = 0; j < NetLayers[i].Neurons.size(); ++j)
			{
				for(unsigned int k = 0; k < NetLayers[i].Neurons[j].mWeights.size(); ++k)
				{
					float delta = LearningRate * NetLayers[i].Neurons[j].mError * NetLayers[i-1].Neurons[k].mOutput;
					NetLayers[i].Neurons[j].mWeights[k] += delta;
				}
			}
		}
/*-----------------------------------------------------------------------------------------------------------*/
	}	
}
/*!
	@param inputs A std::vector of input values, matching the size of the 
					input layer of the network.
	@return			A std::vector which is the size of the output layer, and
					contains the network output values in their respective
					orders.
*//*__________________________________________________________________________*/
inline std::vector< float > NeuralNet::Run(std::vector< float > inputs)
{
	std::vector< float > ret;
	// check that the size of the input vector matches the input layer
	if(inputs.size() != NetLayers[0].Neurons.size() - 1)
	{
		MessageBox(0, "invalid input size", "invalid input size", MB_OKCANCEL);
		exit(1);
	}
	
	// set the input values
	NetLayers[0].Neurons[0].mOutput = 1.f;
	for(unsigned int i = 1; i < NetLayers[0].Neurons.size(); ++i)
	{
		NetLayers[0].Neurons[i].mOutput = inputs[i-1];
	}
	
	// sum up the products (ouput * weight) for each node
	for(unsigned int i = 1; i < NetLayers.size(); ++i)
	{
		for(unsigned int j = 0;  j < NetLayers[i].Neurons.size(); ++j)
		{
			float sum = 0.f;
			for(unsigned int k = 0; k < NetLayers[i-1].Neurons.size(); ++k)
			{
				sum += NetLayers[i-1].Neurons[k].mOutput * NetLayers[i].Neurons[j].mWeights[k];
			}
			NetLayers[i].Neurons[j].mOutput = Sigmoid(sum);
		}
	}
	// get the ouput values into a std::vector for return
	int layer = (int)NetLayers.size() - 1;
	for(unsigned int i = 0; i < NetLayers[layer].Neurons.size(); ++i)
	{
		ret.push_back(NetLayers[layer].Neurons[i].mOutput);
	}
	return ret;
}
/*!
	@param	input The name of the file to be loaded, as a string.
	@note	If the file does not exist, execution is halted.  If the extension
			is not specified, it is appended with .bpn before loading.  If an 
			extension other	than .bpn is specified, it will be preserved.
*//*__________________________________________________________________________*/
inline void NeuralNet::Load(std::string input)
{
	// ensure the correct file type.
	if( (input.find_first_of('.') == std::string::npos) && (input.find(".bpn") == std::string::npos))
	{
		input += ".bpn";
	}
	std::ifstream infile(input.c_str());
	if(infile.fail())
	{
		MessageBox(0, "invalid file", "invalid file", MB_OKCANCEL);
		exit(1);
	}
	// in the case that the network has been created already
	NetLayers.clear();
	
	// the number of layers
	int layers;
	infile >> layers;
	NetLayers.resize(layers);
	// the number of neurons in each layer
	for(int i = 0; i < layers; ++i)
	{
		int sz;
		infile >> sz;
		NetLayers[i].Neurons.resize(sz);
		
		// determine the layer type
		NNLayer::LayerType type;

		if(i == 0)								
			type = NNLayer::INPUT;
		else if(i == layers - 1) 
			type = NNLayer::OUTPUT;
		else
			type = NNLayer::HIDDEN;

		NetLayers[i].Kind = type;
	}
	// the weights at each layer
	for(unsigned int i = 1; i < NetLayers.size(); ++i)
	{
		for(unsigned int j = 0; j < NetLayers[i].Neurons.size(); ++j)
		{
			// set the number of weights at each neuron
			NetLayers[i].Neurons[j].mWeights.resize(NetLayers[i-1].Neurons.size());
			// assign the weights
			for(unsigned int k = 0; k < NetLayers[i].Neurons[j].mWeights.size(); ++k)
			{
				float weight;
				infile >> static_cast<float>(weight);
				NetLayers[i].Neurons[j].mWeights[k] = weight;
			}
		}
	}
}
/*!
    @param	output A string for the filename of the file to save to.
	@note	If the file does not exist, it is created.  If the extension
			is not specified, it is appended with .bpn.  If an extension other
			than .bpn is specified, it will be preserved.
*//*__________________________________________________________________________*/
inline void NeuralNet::Save(std::string output)
{
	// ensure the correct file type.
	if( (output.find_first_of('.') == std::string::npos) && (output.find(".bpn") == std::string::npos))
	{
		output += ".bpn";
	}
	std::ofstream outfile(output.c_str());
	
	// the number of layers
	outfile << static_cast<int>(NetLayers.size()) << "\n";
	
	// the number of neurons in each layer
	for(unsigned int i = 0; i < NetLayers.size(); ++i)
	{
		outfile << static_cast<int>(NetLayers[i].Neurons.size()) << "\n";// << "\n";;
	}
	// the weights at each layer
	for(unsigned int i = 1; i < NetLayers.size(); ++i)
	{
		for(unsigned int j = 0; j < NetLayers[i].Neurons.size(); ++j)
		{
			for(unsigned int k = 0; k < NetLayers[i].Neurons[j].mWeights.size(); ++k)
			{
				outfile << static_cast<float>(NetLayers[i].Neurons[j].mWeights[k]) << "\n";
			}
		}
	}
}

}	// namespace AI

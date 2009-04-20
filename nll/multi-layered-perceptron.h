#ifndef NLL_PMC_H__
# define NLL_PMC_H__

# include <time.h>
# include <math.h>
# include "types.h"
# include "database.h"

# define PMC_LIMIT			99999
# define PMC_DEBUG_NFO
# define PMC_MAGIC_NUMBER	0x12459870

namespace nll
{
namespace algorithm
{
   inline
   f32 sigmoide(f32 val)
   {
	   return 1 / (1 + exp(-val));
   }

   inline
   f32 dsigmoide(f32 val)
   {
	   f32 tmp = 1 / (1 + exp(-val));
	   return tmp * (1 - tmp);
   }

   inline
   f32 sigmoide2(f32 val)
   {
	   return 2 / (1 + exp(-val)) - 1;
   }

   inline
   f32 dsigmoide2(f32 val)
   {
	   f32 tmp = 1 / (1 + exp(-val));
	   return  2 * tmp * (1 - tmp);

   }

   struct _neuron
   {
	   _neuron();
	   ~_neuron();

	   f32*	weight;
	   f32	a;
	   f32	s;
	   f32*	dw;
	   f32	da;
   };

   struct	_pmcFileHeader
   {
	   ui32	magic;
	   ui32	nbLayers;
	   f32	momentum;
	   f32	learningRate;
   };

   /**
    @ingroup algorithm
    @brief Define a multi-layered neural network

    Backpropagation is used for optimizing the parameters
    */
   class NLL_API pmc
   {
   public:
	   typedef f32 (*func)(f32);
      typedef core::ClassificationSample<core::Buffer1D<float>, core::Buffer1D<float> > ClassificationSample;
      typedef core::Database<ClassificationSample> Database;

   public:
      /**
       @brief implement a copy constructor
       */
	   pmc(const pmc& cpy);

      /**
       @brief load a network from a file
       */
	   pmc(const std::string& name);

      /**
       @brief build a neural network with default model parameters
       */
	   pmc(f32 learningRate = 0.1f, f32 momentum = 0.1f, func = sigmoide, func derivedFunction = dsigmoide);

      /**
       @brief release ressources
       */
      ~pmc();

      /**
       @brief set the learning rate

       Too low, it will take ages for learning. Too high, it will never converge.
       */
      void setLearningRate( f32 lr ){ _learningRate = lr; }

      /**
       @brief create the neural network structure.

       @param nbLayers the number of layers needed, typically 3 (1 hidden layer)
       @param layerDescriptor layerDescriptor[n] = number of neurons for layer n
       */
	   void create(ui32 nbLayers, const ui32* layerDescriptor);

      /**
       @brief read from a stream the network
       */
	   bool read(std::istream& i);

      /**
       @brief read from a file the network
       */
	   bool read(const std::string& name);

      /**
       @brief write to a stream the network
       */
	   bool write(std::ostream& i) const;

      /**
       @brief write to a file the network
       */
	   bool write(const std::string& name) const;

      /**
       @brief return the values of the last layer of the network with this input
       */
	   const f32* calculate(const f32* inputLayer) const;

      /**
       @brief learn the neural network using backpropagation.
       */
	   bool learn(const Database& database, f32 learningError, f32 testError = PMC_LIMIT, f32 timeout = PMC_LIMIT);

      /**
       @brief destroy the neural structure
       */
	   void destroy();

      /**
       @brief return true if a network is constructed
       */
	   bool isWorking() const {return _layer || _layerDescriptor;}

      /**
       @brief return the input size
       */
	   ui32 getInputSize() const {return _layerDescriptor[0] - 1;}

      /**
       @brief return the output size
       */
	   ui32 getOutputSize() const {return _layerDescriptor[_nbLayers - 1] - 1;}

      /**
       @brief randomly reinitialise the weight of the network
       */
	   void reset(){_initWeight();}
      
      /**
       @brief copies a neutwork (deep copy)
       */
	   pmc& operator=(const pmc& cpy);

   private:
	   const f32* _getOutput() const;
	   void _configureNetwork();
	   void _initWeight();
	   void _calculateW();
	   void _setInput(const f32* inputLayer) const;
	   void _setOutput(const f32* outputLayer);
	   void _propagate() const;
	   f32 _getCurrentError() const;

   private:
	   ui32*		   _layerDescriptor;
	   mutable _neuron**	_layer;
	   f32*		   _outputLearning;
	   f32*		   _output;
	   ui32		   _nbLayers;
	   f32			_learningRate;
	   f32			_momentum;
	   func		   _function;
	   func		   _dfunction;
   };
}
}

#endif

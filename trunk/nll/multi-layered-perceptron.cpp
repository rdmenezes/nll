# include "stdafx.h"
# include "nll.h"
# include "multi-layered-perceptron.h"
# include <sstream>

namespace nll
{
namespace algorithm
{
   _neuron::_neuron()
   {
	   weight	= 0;
	   a		= 0;
	   s		= 0;
	   dw		= 0;
	   da		= 0;
   }

   _neuron::~_neuron()
   {
	   delete [] weight;
	   delete [] dw;
   }

   pmc::pmc(const std::string& name)
   {
	   _layerDescriptor	= 0;
	   _layer				= 0;
	   _output				= 0;
	   _outputLearning	= 0;
	   _nbLayers			= 0;

	   _function			= sigmoide;
	   _dfunction			= dsigmoide;
	   read(name);
   }

   pmc::pmc(f32 learningRate, f32 momentum , func f, func df)
   {
	   _layerDescriptor	= 0;
	   _layer				= 0;
	   _output				= 0;
	   _outputLearning   = 0;
	   _nbLayers			= 0;

	   _learningRate		= learningRate;
	   _momentum			= momentum;
	   _function			= f;
	   _dfunction			= df;
   }

   pmc::~pmc()
   {
	   destroy();
   }

   void pmc::destroy()
   {
	   for (ui32 n = 0; n < _nbLayers; ++n)
		   delete [] _layer[n];
	   delete [] _layerDescriptor;
	   delete [] _layer;
	   delete [] _output;
	   delete [] _outputLearning;

	   _layerDescriptor = 0;
	   _layer = 0;
	   _output = 0;
	   _outputLearning = 0;
   }

   void pmc::create(ui32 nbLayers, const ui32* layerDescriptor)
   {
      destroy();

	   assert(nbLayers > 2 && layerDescriptor[0] && layerDescriptor[nbLayers - 1]); // "bad network configuration"
	   _nbLayers = nbLayers;
	   _layerDescriptor = new ui32[nbLayers];
	   _layer = new _neuron*[nbLayers];
	   for (ui32 n = 0; n < _nbLayers; ++n)
		   _layerDescriptor[n] = layerDescriptor[n] + 1;

	   // _layerDescriptor init OK
	   _output = new f32[_layerDescriptor[nbLayers - 1]];
	   _outputLearning = new f32[_layerDescriptor[nbLayers - 1]];
	   _output[0] = 0;
	   _outputLearning [0] = 0;
	   for (ui32 n = 0; n < _nbLayers; ++n)
		   _layer[n] = new _neuron[_layerDescriptor[n]];
	   for (ui32 n = 0; n < _nbLayers - 1; ++n)
		   for (ui32 u = 0; u < _layerDescriptor[n]; ++u)
		   {
			   _layer[n][u].weight = new f32[_layerDescriptor[n + 1]];
			   _layer[n][u].dw = new f32[_layerDescriptor[n + 1]];
		   }
	   _configureNetwork();
	   _initWeight();
   }

   void pmc::_configureNetwork()
   {
	   for (ui32 n = 0; n < _nbLayers - 1; ++n)
		   _layer[n][0].s = 1;
   }

   void pmc::_initWeight()
   {
	   for (ui32 n = 0; n < _nbLayers - 1; ++n)
		   for (ui32 u = 0; u < _layerDescriptor[n]; ++u)
			   for (ui32 v = 1; v < _layerDescriptor[n + 1]; ++v)
			   {
				   _layer[n][u].weight[v] = 2 * static_cast<f32>(rand()) / RAND_MAX - 1;
				   _layer[n][u].dw[v] = 0;
			   }
   }

   void pmc::_setInput(const f32* inputLayer) const
   {
	   for (ui32 n = 1; n < _layerDescriptor[0]; ++n)
		   _layer[0][n].s = inputLayer[n - 1];
   }
   	
   void pmc::_setOutput(const f32* outputLayer)
   {
	   for (ui32 n = 1; n < _layerDescriptor[_nbLayers - 1]; ++n)
		   _outputLearning[n]= outputLayer[n - 1];
   }

   void pmc::_propagate() const
   {
	   for (ui32 n = 1; n < _nbLayers; ++n)
		   for (ui32 u = 1; u < _layerDescriptor[n]; ++u)
		   {
			   f32 tmp = 0;
			   for (ui32 v = 0; v < _layerDescriptor[n - 1]; ++v)
				   tmp += _layer[n - 1][v].weight[u] * _layer[n - 1][v].s;
			   _layer[n][u].a = tmp;
			   _layer[n][u].s = _function(tmp); 
		   }
   }

   const f32* pmc::_getOutput() const
   {
	   for (ui32 n = 1; n < _layerDescriptor[_nbLayers - 1]; ++n)
		   _output[n] = _layer[_nbLayers - 1][n].s;
	   return _output + 1;
   }

   void pmc::_calculateW()
   {
	   ui32 last = _layerDescriptor[_nbLayers - 1];
	   for (ui32 n = 1; n < last; ++n)
		   _layer[_nbLayers - 1][n].da = 2 * (_layer[_nbLayers - 1][n].s - _outputLearning[n]) * _dfunction(_layer[_nbLayers - 1][n].a);
	   for (i32 c = _nbLayers - 2; c > 0; --c)
		   for (ui32 i = 1; i < _layerDescriptor[c]; ++i)
		   {
			   f32 tmp = 0;
			   for (ui32 k = 1; k < _layerDescriptor[c + 1]; ++k)
				   tmp += _layer[c + 1][k].da * _layer[c][i].weight[k];
			   _layer[c][i].da = tmp * _dfunction(_layer[c][i].a);
		   }
	   for (ui32 c = 1; c < _nbLayers; ++c)
		   for (ui32 i = 1; i < _layerDescriptor[c]; ++i)
			   for (ui32 j = 0; j < _layerDescriptor[c - 1]; ++j)
				   _layer[c - 1][j].dw[i] = _layer[c][i].da * _layer[c - 1][j].s + _momentum * _layer[c - 1][j].dw[i];
	   for (ui32 n = 0; n < _nbLayers - 1; ++n)
		   for (ui32 u = 0; u < _layerDescriptor[n]; ++u)
			   for (ui32 v = 1; v < _layerDescriptor[n + 1]; ++v)
				   _layer[n][u].weight[v] -= _learningRate * _layer[n][u].dw[v];
   }

   f32 pmc::_getCurrentError() const
   {
	   f32 tmp = 0;
	   ui32 last = _layerDescriptor[_nbLayers - 1];
	   for (ui32 n = 1; n < last; ++n)
		   tmp += (_outputLearning[n] - _layer[_nbLayers - 1][n].s) * (_outputLearning[n] - _layer[_nbLayers - 1][n].s);
	   return tmp;
   }

   const f32* pmc::calculate(const f32* inputLayer) const
   {
	   _setInput(inputLayer);
	   _propagate();
	   return _getOutput();
   }

   bool pmc::learn(const Database& database, f32 learningError, f32 testError, f32 timeout)
   {
	   ui32 cycle = 0;
	   f32 errTest = PMC_LIMIT;
	   f32 errLearn = PMC_LIMIT;
      core::Timer timer;
      timer.start();
      f32 last = 0;
	   while (errTest > testError || errLearn > learningError)
	   {
		   cycle++;
		   errTest = 0;
		   errLearn = 0;
		   for (ui32 n = 0; n < database.size(); ++n)
		   {
			   _setInput(database[n].input.getBuf());
			   _setOutput(database[n].output.getBuf());
			   _propagate();
            if (database[n].type == Database::Sample::LEARNING)
			   {
				   _calculateW();
				   errLearn += _getCurrentError();
			   }
            if (database[n].type == Database::Sample::TESTING)
            {
				   errTest += _getCurrentError();
			   }
		   }
		   if ( timer.getCurrentTime() > timeout )
         {
   # ifdef PMC_DEBUG_NFO
            std::stringstream s;
            s << "timeout, cycle = " << cycle << " errLearning = " << errLearn << " errTest = " << errTest;
            core::LoggerNll::write( core::LoggerNll::INFO, s.str() );
   # endif
			   return false;
         }
   # ifdef PMC_DEBUG_NFO
		   if ( timer.getCurrentTime() - last > 0.25f )
		   {
			   last = timer.getTime();
            std::stringstream s;
			   s << "cycle = " << cycle << " errLearning = " << errLearn << " errTest = " << errTest;
            core::LoggerNll::write( core::LoggerNll::INFO, s.str() );
		   }
   # endif
	   }

   # ifdef PMC_DEBUG_NFO
      std::stringstream s;
	   s << "Learning successfull" << std::endl;
	   s << "  cycles               :" << cycle << std::endl;
	   s << "  learning error       :" << errLearn << std::endl;
	   s << "  testing error        :" << errTest << std::endl;
	   s << "  time                 :" << timer.getCurrentTime();
      core::LoggerNll::write( core::LoggerNll::INFO, s.str() );
   # endif
	   return true;
   }

   bool pmc::write(std::ostream& i) const
   {
	   assert(_layer); // "no network exist"
	   if (!_layer)
		   return 0;
	   _pmcFileHeader hd = {PMC_MAGIC_NUMBER, _nbLayers, _momentum, _learningRate};
	   i.write((i8*)&hd, sizeof ( _pmcFileHeader ) );
	   i.write((i8*)_layerDescriptor, sizeof (f32) * _nbLayers);
	   for(ui32 n = 0; n < _nbLayers - 1; ++n)
		   for (ui32 ii = 0; ii < _layerDescriptor[n]; ++ii)
			   i.write((i8*)_layer[n][ii].weight, sizeof (f32) * _layerDescriptor[n + 1]);
	   return true;
   }

   bool pmc::write(const std::string& name) const
   {
	   std::ofstream f(name.c_str(), std::ios_base::binary);
	   if (!f.is_open())
		   return false;
	   return write(f);
   }

   bool pmc::read(std::istream& i)
   {
	   destroy();
	   _pmcFileHeader hd;
	   i.read((i8*)&hd, sizeof ( _pmcFileHeader ) );
	   assert(hd.magic == PMC_MAGIC_NUMBER); // "not a valid PMC file"
	   if (hd.magic != PMC_MAGIC_NUMBER)
		   return false;
	   ui32* descriptor = new ui32[hd.nbLayers];
	   i.read((i8*)descriptor, sizeof ( ui32 ) * hd.nbLayers);
	   for (ui32 n = 0; n < hd.nbLayers; ++n)
		   descriptor[n]--;
	   create(hd.nbLayers, descriptor);
	   for(ui32 n = 0; n < _nbLayers - 1; ++n)
		   for (ui32 ii = 0; ii < _layerDescriptor[n]; ++ii)
			   i.read((i8*)_layer[n][ii].weight, sizeof (f32) * _layerDescriptor[n + 1]);
	   delete [] descriptor;
	   _learningRate = hd.learningRate;
	   _momentum = hd.momentum;
	   return true;
   }

   bool pmc::read(const std::string& name)
   {
	   std::ifstream f(name.c_str(), std::ios_base::binary);
	   if (!f.is_open())
		   return false;
	   return read(f);
   }

   pmc::pmc(const pmc& cpy)
   {
	   operator=(cpy);
   }

   pmc& pmc::operator=(const pmc& cpy)
   {
	   _dfunction = cpy._dfunction;
	   _function = cpy._function;
	   _momentum = cpy._momentum;
	   _learningRate = cpy._learningRate;
	   _nbLayers = cpy._nbLayers;

      if ( cpy._nbLayers && cpy._layerDescriptor )
      {
	      _layer = new _neuron*[_nbLayers];

	      _layerDescriptor = new ui32[_nbLayers];
	      for (ui32 n = 0; n < _nbLayers; ++n)
		      _layerDescriptor[n] = cpy._layerDescriptor[n];

      	
	      _output = new f32[_layerDescriptor[_nbLayers - 1]];
	      if (cpy._output)
		      memcpy(_output, cpy._output, _layerDescriptor[_nbLayers - 1] * sizeof (f32));

	      _outputLearning = new f32[_layerDescriptor[_nbLayers - 1]];
	      if (cpy._outputLearning)
		      memcpy(_outputLearning, cpy._outputLearning, _layerDescriptor[_nbLayers - 1] * sizeof (f32));
      	
	      for (ui32 n = 0; n < _nbLayers; ++n)
		      _layer[n] = new _neuron[_layerDescriptor[n]];
	      for (ui32 n = 0; n < _nbLayers - 1; ++n)
		      for (ui32 u = 0; u < _layerDescriptor[n]; ++u)
		      {
			      _layer[n][u].weight = new f32[_layerDescriptor[n + 1]];
			      _layer[n][u].dw = new f32[_layerDescriptor[n + 1]];
		      }

	      for (ui32 n = 0; n < _nbLayers - 1; ++n)
		      for (ui32 u = 0; u < _layerDescriptor[n]; ++u)
			      for (ui32 v = 1; v < _layerDescriptor[n + 1]; ++v)
			      {
				      _layer[n][u].weight[v] = cpy._layer[n][u].weight[v];
				      _layer[n][u].dw[v] = cpy._layer[n][u].dw[v];
			      }
      } else {
         _layer = 0;
         _layerDescriptor = 0;
         _output = 0;
         _outputLearning = 0;

      }
	   return *this;
   }
}
}

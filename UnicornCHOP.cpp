/* Shared Use License: This file is owned by Derivative Inc. (Derivative)
* and can only be used, and/or modified for use, in conjunction with
* Derivative's TouchDesigner software, and only if you are a licensee who has
* accepted Derivative's TouchDesigner license or assignment agreement
* (which also govern the use of this file). You may share or redistribute
* a modified version of this file provided the following conditions are met:
*
* 1. The shared file or redistribution must retain the information set out
* above and this list of conditions.
* 2. Derivative's name (Derivative Inc.) or its trademarks may not be used
* to endorse or promote products derived from this file without specific
* prior written permission from Derivative.
*/

#include "UnicornCHOP.h"
#include "UnicornTD.h"
#include <stdio.h>
#include <string.h>
#include <cmath>
#include <assert.h>
#include <iostream>
#include <fstream>
#include "VarDev.h"


#define DATA_FILE				"data.bin"		// The name of the file which is storing acquired data.
#define ACQUISITION_DURATION_S	1.0f			// The acquisition duration in seconds.
#define FRAME_LENGTH			1				// The number of samples acquired per get data call.
#define TESTSIGNAL_ENABLED		FALSE
//#define NC17 17
//#define SR250 60
// These functions are basic C function, which the DLL loader can find
// much easier than finding a C++ Class.
// The DLLEXPORT prefix is needed so the compile exports these functions from the .dll
// you are creating
extern "C"
{

DLLEXPORT
void
FillCHOPPluginInfo(CHOP_PluginInfo *info)
{
	// Always set this to CHOPCPlusPlusAPIVersion.
	info->apiVersion = CHOPCPlusPlusAPIVersion;

	// The opType is the unique name for this CHOP. It must start with a 
	// capital A-Z character, and all the following characters must lower case
	// or numbers (a-z, 0-9)
	info->customOPInfo.opType->setString("Unicorn");

	// The opLabel is the text that will show up in the OP Create Dialog
	info->customOPInfo.opLabel->setString("Unicorn CHOP");

	// Information about the author of this OP
	info->customOPInfo.authorName->setString("Author Name");
	info->customOPInfo.authorEmail->setString("email@email.com");

	// This CHOP can work with 0 inputs
	info->customOPInfo.minInputs = 0;

	// It can accept up to 1 input though, which changes it's behavior
	info->customOPInfo.maxInputs = 1;
}

DLLEXPORT
CHOP_CPlusPlusBase*
CreateCHOPInstance(const OP_NodeInfo* info)
{
	// Return a new instance of your class every time this is called.
	// It will be called once per CHOP that is using the .dll
	return new UnicornCHOP(info);
}

DLLEXPORT
void
DestroyCHOPInstance(CHOP_CPlusPlusBase* instance)
{
	// Delete the instance here, this will be called when
	// Touch is shutting down, when the CHOP using that instance is deleted, or
	// if the CHOP loads a different DLL
	delete (UnicornCHOP*)instance;
}

};


UnicornCHOP::UnicornCHOP(const OP_NodeInfo* info) : myNodeInfo(info)
{
	myExecuteCount = 0;
	myOffset = 0.0;
	STDo; 
	Unicorn = new UnicornConnector();
	lastToggle = false;
	updateUnicorn = false;
	
}
	


UnicornCHOP::~UnicornCHOP()
{

}

void
UnicornCHOP::getGeneralInfo(CHOP_GeneralInfo* ginfo, const OP_Inputs* inputs, void* reserved1)
{
	// This will cause the node to cook every frame
	ginfo->cookEveryFrameIfAsked = true;

	// Note: To disable timeslicing you'll need to turn this off, as well as ensure that
	// getOutputInfo() returns true, and likely also set the info->numSamples to how many
	// samples you want to generate for this CHOP. Otherwise it'll take on length of the
	// input CHOP, which may be timesliced.
	ginfo->timeslice = true;

	ginfo->inputMatchIndex = 0;
}

bool
UnicornCHOP::getOutputInfo(CHOP_OutputInfo* info, const OP_Inputs* inputs, void* reserved1)
{
	// If there is an input connected, we are going to match it's channel names etc
	// otherwise we'll specify our own.
	if (inputs->getNumInputs() > 0)
	{
		return false;
	}
	else
	{
		
		info->numChannels = NC17;
		
		// Since we are outputting a timeslice, the system will dictate
		// the numSamples and startIndex of the CHOP data
		//info->numSamples = 1;
		//info->startIndex = 0
		info->numSamples = 4;
		// For illustration we are going to output 120hz data
		info->sampleRate = 60;
		return true;
	}
}

void
UnicornCHOP::getChannelName(int32_t index, OP_String *name, const OP_Inputs* inputs, void* reserved1)
{
	name->setString("chan1");
}





void UnicornCHOP::execute(CHOP_Output* output, const OP_Inputs* inputs,void* reserved){
	myExecuteCount++;
	bool isUnicornUpdate = inputs->getParInt("Unicornupdate");
	double	 scale = inputs->getParDouble("Scale");
	
	

	int ind = 0;
	const OP_CHOPInput* cinput = inputs->getInputCHOP(0);
	

	
	bool isUnicornOn = inputs->getParInt("Unicorn");

	if (lastToggle) {
		

		for (int i = 0; i < output->numChannels; i++) {
			
			for (int j = 0; j < output->numSamples; j++){

			output->channels[i][j] = Unicorn->sampleBuffer[i][j];
			ind++;

			// Make sure we don't read past the end of the CHOP input
			ind = ind % cinput->numSamples;
		}
		
	 }
		 
	} else {
		if (isUnicornOn) {
			Unicorn->connect();
			
			lastToggle = true;
		}
	}
	//if (lastToggle) {
	//	Unicorn->delbuff();
	//	std::cout << std::endl << "del" << std::endl;
	//	Unicorn->newbuff();
	//	//std::cout << std::endl << "new" << std::endl;
	//}
}

int32_t UnicornCHOP::getNumInfoCHOPChans(void * reserved1){
	// We return the number of channel we want to output to any Info CHOP
	// connected to the CHOP. In this example we are just going to send one channel.
	return 2;
}

void UnicornCHOP::getInfoCHOPChan(int32_t index, OP_InfoCHOPChan* chan,	void* reserved1)
{
	// This function will be called once for each channel we said we'd want to return
	// In this example it'll only be called once.

	if (index == 0)
	{
		chan->name->setString("executeCount");
		chan->value = (float)myExecuteCount;
	}

	if (index == 1)
	{
		chan->name->setString("offset");
		chan->value = (float)myOffset;
	}

	if (index == 2)
	{
		chan->name->setString("unicorn");
		chan->value = (const char)STDo;
	}
}

bool		
UnicornCHOP::getInfoDATSize(OP_InfoDATSize* infoSize, void* reserved1)
{
	infoSize->rows = 3;
	infoSize->cols = 2;
	// Setting this to false means we'll be assigning values to the table
	// one row at a time. True means we'll do it one column at a time.
	infoSize->byColumn = false;
	return true;
}

void
UnicornCHOP::getInfoDATEntries(int32_t index,
										int32_t nEntries,
										OP_InfoDATEntries* entries, 
										void* reserved1)
{
	char tempBuffer[4096];

	if (index == 0)
	{
		// Set the value for the first column
		entries->values[0]->setString("executeCount");

		// Set the value for the second column
#ifdef _WIN32
		sprintf_s(tempBuffer, "%d", myExecuteCount);
#else // macOS
        snprintf(tempBuffer, sizeof(tempBuffer), "%d", myExecuteCount);
#endif
		entries->values[1]->setString(tempBuffer);
	}

	if (index == 1)
	{
		// Set the value for the first column
		entries->values[0]->setString("offset");

		// Set the value for the second column
#ifdef _WIN32
        sprintf_s(tempBuffer, "%g", myOffset);
#else // macOS
        snprintf(tempBuffer, sizeof(tempBuffer), "%g", myOffset);
#endif
		entries->values[1]->setString( tempBuffer);
	}

	if (index == 2)
	{
		// Set the value for the first column
		entries->values[0]->setString("Unicorn Status");

		// Set the value for the second column
#ifdef _WIN32
		std::string currentStatus = Unicorn->getCurrentStatus();
		//sprintf_s(tempBuffer, "%g", STDo);
#else // macOS
		snprintf(tempBuffer, sizeof(tempBuffer), "%g", myOffset);
#endif
		entries->values[1]->setString(currentStatus.c_str());
	}

}

void UnicornCHOP::setupParameters(OP_ParameterManager* manager, void *reserved1)
{
	// speed
	{
		OP_NumericParameter	np;

		np.name = "Speed";
		np.label = "Speed";
		np.defaultValues[0] = 1.0;
		np.minSliders[0] = -10.0;
		np.maxSliders[0] =  10.0;
		
		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// scale
	{
		OP_NumericParameter	np;

		np.name = "Scale";
		np.label = "Scale";
		np.defaultValues[0] = 1.0;
		np.minSliders[0] = -10.0;
		np.maxSliders[0] =  10.0;
		
		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// shape
	{
		OP_StringParameter	sp;

		sp.name = "Shape";
		sp.label = "Shape";

		sp.defaultValue = "Sine";

		const char *names[] = { "Sine", "Square", "Ramp" };
		const char *labels[] = { "Sine", "Square", "Ramp" };

		OP_ParAppendResult res = manager->appendMenu(sp, 3, names, labels);
		assert(res == OP_ParAppendResult::Success);
	}

	// pulse
	{
		OP_NumericParameter	np;

		np.name = "Reset";
		np.label = "Reset";
		
		OP_ParAppendResult res = manager->appendPulse(np);
		assert(res == OP_ParAppendResult::Success);
	}
	
	// unicorn on
	{
		OP_NumericParameter	np;

		np.name = "Unicorn";
		np.label = "Unicorn";

		OP_ParAppendResult res = manager->appendToggle(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// unicorn on
	{
		OP_NumericParameter	np;

		np.name = "Unicornupdate";
		np.label = "Unicornupdate";

		OP_ParAppendResult res = manager->appendToggle(np);
		assert(res == OP_ParAppendResult::Success);
	}
}

void UnicornCHOP::pulsePressed(const char* name, void* reserved1)
{
	if (!strcmp(name, "Reset"))
	{
		myOffset = 0.0;
	}
}

#include <iostream>
#include "UnicornTD.h"
#include <fstream>
#include "VarDev.h"


// Specifications for the data acquisition.
//-------------------------------------------------------------------------------------
#define ACQUISITION_DURATION_S	1000.0f			// The acquisition duration in seconds.
#define FRAME_LENGTH			1				// The number of samples acquired per get data call.
#define TESTSIGNAL_ENABLED		FALSE			// Flag to enable or disable testsignal.
//#define NC17 17
//#define SR250 60

UnicornConnector::UnicornConnector() : _myStatus("Initializing")
{
	initializeUnicorn();
}

UnicornConnector::~UnicornConnector()
{
	
}


void UnicornConnector::pollUnicorn()
{
	while (true) 
	{
		try {


			//if (acquisitionBuffer != NULL)
			//{
			//delete[] acquisitionBuffer;
			//acquisitionBuffer = NULL;
			//}
			//acquisitionBuffer = new float[17*250];

			int errorCode;
			//for (int i = 0; i < 16; i++) {
			
				errorCode = UNICORN_GetData(deviceHandle, 1, acquisitionBuffer, numberOfChannelsToAcquire * sizeof(float));
			//}
			//std::cout << std::endl << sampleBuffer << std::endl;
			HandleError(errorCode);

		}
		catch (int errorCode) {
			PrintErrorMessage(errorCode, acquisitionBufferLength);
		}
		catch (...)
		{
			std::cout << std::endl << "An unknown error occurred." << std::endl;
		}

	}
}
void UnicornConnector::bufferData()
{
	

	// memory allocated for elements of rows. 
	sampleBuffer = new float* [17];

	// memory allocated for  elements of each column.  
	for (int i = 0; i < 17; i++) {
		sampleBuffer[i] = new float [50000];
	}
	int  j = 0;
	while (true)
	{
		for (int i = 0; i < 17; i++) {
			sampleBuffer[i][j] = acquisitionBuffer[i];
			
		}
	}
}


void UnicornConnector::update(){
	
	_unicornThread = std::thread(std::bind(&UnicornConnector::pollUnicorn, this));

}

void UnicornConnector::connect(){
	int errorCode = UNICORN_ERROR_SUCCESS;
	deviceHandle = 0;

	try	{
		unsigned int availableDevicesCount = 0;
		errorCode = UNICORN_GetAvailableDevices(NULL, &availableDevicesCount, TRUE);
		HandleError(errorCode);
		if (availableDevicesCount < 1)		{
			std::cout << "No device available. Please pair with a Unicorn device first.";
			errorCode = UNICORN_ERROR_GENERAL_ERROR;
			HandleError(errorCode);
		}
		UNICORN_DEVICE_SERIAL *availableDevices = new UNICORN_DEVICE_SERIAL[availableDevicesCount];
		errorCode = UNICORN_GetAvailableDevices(availableDevices, &availableDevicesCount, TRUE);
		HandleError(errorCode);

		std::cout << "Available devices:" << std::endl;
		for (unsigned int i = 0; i<availableDevicesCount; i++){
			std::cout << "#" << i << ": " << availableDevices[i] << std::endl;
		}
		std::cout << "\nSelect device by ID #";
		unsigned int deviceId;
		std::cin >> deviceId;
		if (deviceId >= availableDevicesCount || deviceId < 0){
			errorCode = UNICORN_ERROR_GENERAL_ERROR;
		}
		HandleError(errorCode);
		std::cout << "Trying to connect to '" << availableDevices[deviceId] << "'." << std::endl;
		errorCode = UNICORN_OpenDevice(availableDevices[deviceId], &deviceHandle);
		HandleError(errorCode);
		std::cout << "Connected to '" << availableDevices[deviceId] << "'." << std::endl;
		std::cout << "Device Handle: " << deviceHandle << std::endl;
		errorCode = UNICORN_GetNumberOfAcquiredChannels(deviceHandle, &numberOfChannelsToAcquire);
		HandleError(errorCode);

		UNICORN_AMPLIFIER_CONFIGURATION configuration;
		errorCode = UNICORN_GetConfiguration(deviceHandle, &configuration);
		HandleError(errorCode);

		samplingRate = UNICORN_SAMPLING_RATE;
		std::cout << std::endl;
		std::cout << "Acquisition Configuration:" << std::endl;
		std::cout << "Sampling Rate: " << samplingRate << "Hz" << std::endl;
		std::cout << "Frame Length: " << FRAME_LENGTH << std::endl;
		std::cout << "Number Of Acquired Channels: " << numberOfChannelsToAcquire << std::endl;
		std::cout << "Data Acquisition Length: " << ACQUISITION_DURATION_S << "s" << std::endl;
        acquisitionBufferLength = numberOfChannelsToAcquire * FRAME_LENGTH;
		acquisitionBuffer = new float[NC17];

		//for (int i = 0; i < samplingRate; i++) {
			//acquisitionBuffer[i] = new float[NC17];  //acquisitionBuffer[i][j]: i = sampling rate || j = NC17 
		//}

		try	{
			errorCode = UNICORN_StartAcquisition(deviceHandle, TESTSIGNAL_ENABLED);
			HandleError(errorCode);
			std::cout << std::endl << "Data acquisition started." << std::endl;
			numberOfGetDataCalls = (int) (ACQUISITION_DURATION_S * (samplingRate / FRAME_LENGTH));
/*			consoleUpdateRate = (int) ((samplingRate / FRAME_LENGTH) / 25.0f);
			if (consoleUpdateRate == 0){
				consoleUpdateRate = 1;
			}
	*/		
		} catch (int errorCode){
			PrintErrorMessage(errorCode, 22);
		}
		catch (...)
		{
			std::cout << std::endl << "An unknown error occurred." << std::endl;
		}
	} catch (int errorCode)	{
		PrintErrorMessage(errorCode, 33);
	}	catch (...)
	{
		std::cout << std::endl << "An unknown error occurred." << std::endl;
	}
	_unicornThread = std::thread(std::bind(&UnicornConnector::pollUnicorn, this));
	_bufferThread = std::thread(std::bind(&UnicornConnector::bufferData, this));

}

std::string UnicornConnector::getCurrentStatus()
{
	return _myStatus;
}

void UnicornConnector::initializeUnicorn()
{

}

void UnicornConnector::HandleError(int errorCode){
	if (errorCode != UNICORN_ERROR_SUCCESS)
	{
		throw errorCode;
	}
}


void UnicornConnector::PrintErrorMessage(int errorCode, int el){
	std::cout << std::endl << "An error occurred. Error Code: " << errorCode << " -  (" << el << ")";
	std::cout << UNICORN_GetLastErrorText();
	std::cout << std::endl;
}
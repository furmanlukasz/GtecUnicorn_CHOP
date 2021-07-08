#pragma once
#ifndef WIIMOTE_H
#define WIIMOTE_H

#define MAX_WIIMOTES				1

#include <thread>
#include <functional>
#include <string>
#include "unicorn.h"
#include "VarDev.h"
class UnicornConnector
{
public:
	UnicornConnector();
	~UnicornConnector();

	UNICORN_HANDLE deviceHandle;
	unsigned int numberOfChannelsToAcquire;
	int acquisitionBufferLength;
	float *acquisitionBuffer;
	float **sampleBuffer;
	int numberOfGetDataCalls;
	// int consoleUpdateRate;
	int samplingRate;
	
	void update();
	void connect();
	
	void HandleError(int errorCode);
	void PrintErrorMessage(int errorCode, int el);
	std::string getCurrentStatus();
	std::thread _unicornThread;
	std::thread _bufferThread;
	void pollUnicorn();
	void bufferData();
private:
	void initializeUnicorn();
	
	std::string _myStatus;
	
};

#endif
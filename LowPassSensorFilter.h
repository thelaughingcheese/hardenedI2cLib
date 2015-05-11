#ifndef LOWPASSSENSORFILTER
#define LOWPASSSENSORFILTER

#define MAXSAMPLES 100

template<class T> class LowPassSensorFilter{
private:
	T samples[MAXSAMPLES];
	short activeSamples;
	short sampleRange;
	int nextSampleIndex;

	int getNextIndex(int in);
	int getPrevIndex(int in);
public:
	LowPassSensorFilter(short range);

	T update(T in);
};

template<class T>
int LowPassSensorFilter<T>::getNextIndex(int in){
	int rtn = in + 1;
	if(rtn >= MAXSAMPLES){
		return 0;
	}
	return rtn;
}

template<class T>
int LowPassSensorFilter<T>::getPrevIndex(int in){
	int rtn = in - 1;
	if(rtn < 0){
		return MAXSAMPLES-1;
	}
	return rtn;
}

template<class T>
LowPassSensorFilter<T>::LowPassSensorFilter(short range){
	activeSamples = 0;
	sampleRange = range;
	nextSampleIndex = 0;
}

template<class T>
T LowPassSensorFilter<T>::update(T in){
	samples[nextSampleIndex] = in;
	activeSamples = min(activeSamples+1,sampleRange);
	float total = 0;

	int index = nextSampleIndex;
	for(int i=0;i<activeSamples;i++){
		total += samples[index];
		index = getPrevIndex(index);
	}

	nextSampleIndex = getNextIndex(nextSampleIndex);
	return total/sampleRange;
}

#endif

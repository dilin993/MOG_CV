#include <iostream>
#include <cmath>
#include <cfloat>


using namespace std;

#define WIDTH 640
#define HEIGHT 480
#define IMG_SIZE WIDTH*HEIGHT
#define K  2 // no of gaussian mxitures
#define BGM_SIZE IMG_SIZE*K


template<typename VT> struct MixData
{
    float sortKey;
    float weight;
    VT mean;
    VT var;
};

typedef float data_t;

void execute(uint8_t frame_in[IMG_SIZE],
             uint8_t frame_out[IMG_SIZE],
             double learningRate);

void process(uint8_t frame_in[IMG_SIZE],
             uint8_t frame_out[IMG_SIZE],
             double learningRate,
             double backgroundRatio,
             double varThreshold,
             double noiseSigma);


#include "core.h"

MixData<data_t> bgmodel[BGM_SIZE];

static const int defaultNMixtures = 5;
static const int defaultHistory = 200;
static const double defaultBackgroundRatio = 0.7;
static const double defaultVarThreshold = 2.5*2.5;
static const double defaultNoiseSigma = 30*0.5;
static const double defaultInitialWeight = 0.05;

int nframes = 0;


void process(uint8_t frame_in[IMG_SIZE],
             uint8_t frame_out[IMG_SIZE],
             double learningRate,
             double backgroundRatio,
             double varThreshold,
             double noiseSigma)
{
    int x, y, k, k1, rows = HEIGHT, cols = WIDTH;
    float alpha = (float)learningRate, T = (float)backgroundRatio, vT = (float)varThreshold;
    MixData<float>* mptr = bgmodel;

    const float w0 = (float)defaultInitialWeight;
    const float sk0 = (float)(w0/(defaultNoiseSigma*2));
    const float var0 = (float)(defaultNoiseSigma*defaultNoiseSigma*4);
    const float minVar = (float)(noiseSigma*noiseSigma);

    for( y = 0; y < rows; y++ )
    {
        const uint8_t * src = frame_in+y*rows;
        uint8_t * dst = frame_out+y*rows;

        if( alpha > 0 )
        {
            for( x = 0; x < cols; x++, mptr += K)
            {
                float wsum = 0;
                float pix = src[x];
                int kHit = -1, kForeground = -1;

                for( k = 0; k < K; k++ )
                {
                    float w = mptr[k].weight;
                    wsum += w;
                    if( w < FLT_EPSILON )
                        break;
                    float mu = mptr[k].mean;
                    float var = mptr[k].var;
                    float diff = pix - mu;
                    float d2 = diff*diff;
                    if( d2 < vT*var )
                    {
                        wsum -= w;
                        float dw = alpha*(1.f - w);
                        mptr[k].weight = w + dw;
                        mptr[k].mean = mu + alpha*diff;
                        var = std::max(var + alpha*(d2 - var), minVar);
                        mptr[k].var = var;
                        mptr[k].sortKey = w/std::sqrt(var);

                        for( k1 = k-1; k1 >= 0; k1-- )
                        {
                            if( mptr[k1].sortKey >= mptr[k1+1].sortKey )
                                break;
                            std::swap( mptr[k1], mptr[k1+1] );
                        }

                        kHit = k1+1;
                        break;
                    }
                }

                if( kHit < 0 ) // no appropriate gaussian mixture found at all, remove the weakest mixture and create a new one
                {
                    kHit = k = std::min(k, K-1);
                    wsum += w0 - mptr[k].weight;
                    mptr[k].weight = w0;
                    mptr[k].mean = pix;
                    mptr[k].var = var0;
                    mptr[k].sortKey = sk0;
                }
                else
                    for( ; k < K; k++ )
                        wsum += mptr[k].weight;

                float wscale = 1.f/wsum;
                wsum = 0;
                for( k = 0; k < K; k++ )
                {
                    wsum += mptr[k].weight *= wscale;
                    mptr[k].sortKey *= wscale;
                    if( wsum > T && kForeground < 0 )
                        kForeground = k+1;
                }

                dst[x] = (uint8_t)(-(kHit >= kForeground));
            }
        }
        else
        {
            for( x = 0; x < cols; x++, mptr += K )
            {
                float pix = src[x];
                int kHit = -1, kForeground = -1;

                for( k = 0; k < K; k++ )
                {
                    if( mptr[k].weight < FLT_EPSILON )
                        break;
                    float mu = mptr[k].mean;
                    float var = mptr[k].var;
                    float diff = pix - mu;
                    float d2 = diff*diff;
                    if( d2 < vT*var )
                    {
                        kHit = k;
                        break;
                    }
                }

                if( kHit >= 0 )
                {
                    float wsum = 0;
                    for( k = 0; k < K; k++ )
                    {
                        wsum += mptr[k].weight;
                        if( wsum > T )
                        {
                            kForeground = k+1;
                            break;
                        }
                    }
                }

                dst[x] = (uint8_t)(kHit < 0 || kHit >= kForeground ? 255 : 0);
            }
        }
    }
}

void execute(uint8_t frame_in[IMG_SIZE],
             uint8_t frame_out[IMG_SIZE],
             double learningRate)
{

    bool needToInitialize = nframes == 0 || learningRate >= 1;

    if( needToInitialize )
    {
        // initialization
        nframes = 0;
        MixData<data_t> tmp;
        tmp.mean = 0;
        tmp.var = 0;
        tmp.sortKey = 0;
        tmp.weight = 0;
        for(int p=0;p<BGM_SIZE;p++)
            bgmodel[p] = tmp;
    }


    ++nframes;
    learningRate = learningRate >= 0 && nframes > 1 ? learningRate : 1./std::min( nframes, defaultHistory );

    process(frame_in,frame_out,learningRate,defaultBackgroundRatio,defaultVarThreshold,defaultNoiseSigma);
}
#include "daisy_pod.h"
#include "daisysp.h"

using namespace std;
using namespace daisy;
using namespace daisysp;

// constants
const float MAX_Q           = 1.f;
const float MIN_Q           = 0.1f;
const float AMP_LIMIT       = 1.f;
const float MIN_CENTER_FREQ = 30.f;
const float MAX_CENTER_FREQ = 18000.f;

// hw related
DaisyPod   hw;
Parameter  center_freq_param, q_param;
WhiteNoise noise;
float      sig, filtered;
float      sample_rate;

// user-defined params
float center_freq = 500;
float q           = 0.707; // 1 / sqrt(2)
float gain_db     = 0.5;   // only for peaking and shelving

// coefficients

float a0, a1, a2, b0, b1, b2;
float x1, x2, y, yOne, y2; // y1 is used in math.h :/


void GetCoefficients()
{
    // ported from https://arachnoid.com/BiQuadDesigner/source_files/BiQuadraticFilter.java, also cf https://webaudio.github.io/Audio-EQ-Cookbook/audio-eq-cookbook.html
    // using floats because daisy
    float omega = 2 * PI_F * center_freq / sample_rate;
    float sn    = sin(omega);
    float cs    = cos(omega);
    float alpha = sn / (2 * q);

    // only for peaking and shelving
    float gain_abs = pow(10, gain_db / 40);
    float beta     = sqrt(gain_abs * 2);

    b0 = (1 - cs) / 2;
    b1 = 1 - cs;
    b2 = (1 - cs) / 2;
    a0 = 1 + alpha;
    a1 = -2 * cs;
    a2 = 1 - alpha;

    // prescale filter constants
    b0 /= a0;
    b1 /= a0;
    b2 /= a0;
    a1 /= a0;
    a2 /= a0;
}

void Init()
{
    noise.Init();
    noise.SetAmp(0.f);
    GetCoefficients();
    center_freq_param.Init(
        hw.knob1, MIN_CENTER_FREQ, MAX_CENTER_FREQ, center_freq_param.LINEAR);
    q_param.Init(hw.knob2, MIN_Q, MAX_Q, q_param.LINEAR);
}

float Process(float x)
{
    y = b0 * x + b1 * x1 + b2 * x2 - a1 * yOne
        - a2 * y2; // all of the coefficients are prescaled hence there's no division by a0
    x2   = x1;
    x1   = x;
    y2   = yOne;
    yOne = y;
    // I am a coward
    if(y > AMP_LIMIT)
    {
        y = AMP_LIMIT;
    }
    else if(y < -AMP_LIMIT)
    {
        y = -AMP_LIMIT;
    }
    return y;
}

void UpdateKnobs()
{
    center_freq = center_freq_param.Process();
    q           = q_param.Process();
}

void UpdateButtons()
{
    if(hw.button1.RisingEdge())
    {
        noise.SetAmp(0.5f);
    }
    else if(hw.button1.FallingEdge())
    {
        noise.SetAmp(0.f);
    }
}

static void AudioCallback(AudioHandle::InputBuffer  in,
                          AudioHandle::OutputBuffer out,
                          size_t                    size)
{
    hw.ProcessDigitalControls();

    UpdateButtons();
    UpdateKnobs();
    GetCoefficients();

    for(size_t i = 0; i < size; i += 1)
    {
        sig       = noise.Process();
        filtered  = Process(sig);
        out[0][i] = filtered;
        out[1][i] = filtered;
    }
    // hw.seed.PrintLine("%.3f", center_freq);
}

int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(4);
    // hw.seed.StartLog();

    sample_rate = hw.AudioSampleRate();
    Init();

    // Start callbacks
    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    while(1) {}
}

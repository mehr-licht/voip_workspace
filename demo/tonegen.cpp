/******************************************************************************/
/*
 *  Project: Voice-over-IP
 *  Author: Matthias Tessmann <matthias.tessmann@th-nuernberg.de
 *  Date: October, 5th 2015
 *
 *  Contents: Simple tone generator for demonstration purposes.
 *            Intended for educational purposes.
 *
 *  Additoinal notes:
 *         - There is very little error handling.
 *         - The implementation may contain bugs of any kind!
 *
 * The MIT License (MIT)
 *
 *  Copyright (c) 2015 Matthias Teßmann <matthias.tessmann@th-nuernberg.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
 /******************************************************************************/

#include <iostream>
#include <cmath>
#include <random>
#include "RtAudio.h"
#include "tclap/CmdLine.h"

/******************************************************************************/
/* Tone generator classes */
/******************************************************************************/
class ToneGen {
public:
  ToneGen(float a) : sample_ (-1.0), amplitude_(a), sampleCount_(0) {
  }
  virtual ~ToneGen() {}
  virtual float nextSample() = 0;

protected:
  float        sample_;
  float        amplitude_;
  unsigned int sampleCount_;
};

class SineGen : public ToneGen {
public:
  SineGen(float a = 1.0) : ToneGen(a) {}

  float nextSample() {
    sample_ = amplitude_ * ::sin(static_cast<float>(sampleCount_)/16);
    ++sampleCount_;
    return sample_;
  }
};

class SquareGen : public ToneGen {
public:
  SquareGen(float a = 1.0) : ToneGen(a) {}

  float nextSample() {
    sample_ = amplitude_ * sgn(::sin(static_cast<float>(sampleCount_)/16));
    ++sampleCount_;
    return sample_;
  }

private:
  int sgn(float val) {
    return (0.0 < val) - (val < 0.0);
  }
};

class WhiteNoiseGen : public ToneGen {
public:
  WhiteNoiseGen(float a = .25) : ToneGen(a), gen_(), dist_(-a, a) {
  }

  float nextSample() {
    return dist_(gen_);
  }

private:
  std::default_random_engine gen_;
  std::uniform_real_distribution<float> dist_;
};

/******************************************************************************/
/* SimplePlayback class */
/******************************************************************************/
class SimplePlayback {
public:
  static int rtaudio_callback(void *outBuf, void *inBuf, unsigned int nFrames,
                              double streamTime, RtAudioStreamStatus status, void *userData);

public:
  SimplePlayback();
  ~SimplePlayback();

  bool init(unsigned int sr, unsigned int nch, unsigned int toneGen);
  bool start();
  bool stop();

private:
  int callback(void *outBuf, void *inBuf, unsigned int nFrames, double streamTime, RtAudioStreamStatus status);

  RtAudio                   audio_;
  RtAudio::StreamParameters streamParameters_;
  bool                      playing_;
  unsigned int              sr_;
  unsigned int              fs_;
  ToneGen*                  tg_;
};

int SimplePlayback::rtaudio_callback(void *outBuf, void *inBuf, unsigned int nFrames,
                                     double streamTime, RtAudioStreamStatus status, void *userData) {
  SimplePlayback *sp = reinterpret_cast<SimplePlayback*>(userData);
  return sp->callback(outBuf, inBuf, nFrames, streamTime, status);
}

SimplePlayback::SimplePlayback() : audio_(), streamParameters_(), playing_(false), sr_(0), fs_(512), tg_(NULL) {
}

SimplePlayback::~SimplePlayback() {
  if (playing_)
    stop();
  if (tg_)
    delete tg_;
}

bool SimplePlayback::init(unsigned int sr, unsigned int nch, unsigned int toneGen) {

  sr_                            = sr;
  streamParameters_.deviceId     = audio_.getDefaultOutputDevice();
  streamParameters_.nChannels    = nch;
  streamParameters_.firstChannel = 0;

  try {
    audio_.openStream(&streamParameters_, NULL, RTAUDIO_FLOAT32, sr_, &fs_, &SimplePlayback::rtaudio_callback, (void*)this);
  } catch (RtAudioError& e) {
    e.printMessage();
    return false;
  }

    if (tg_)
      delete tg_;

    switch (toneGen) {
      case 1:
        tg_ = new SquareGen();
        break;
      case 2:
        tg_ = new WhiteNoiseGen();
        break;
      default:
        tg_ = new SineGen();
        break;
    }

  return true;
}

bool SimplePlayback::start() {
  if (playing_)
    return false;
  try {
    audio_.startStream();
    playing_ = true;
  } catch (RtAudioError& e) {
    e.printMessage();
    return false;
  }
  return true;
}

bool SimplePlayback::stop() {
  if (!playing_)
    return false;
  try {
    audio_.stopStream();
    playing_ = false;
    if (audio_.isStreamOpen())
      audio_.closeStream();
  } catch (RtAudioError& e) {
    e.printMessage();
    return false;
  }
  return true;
}

int SimplePlayback::callback(void *outBuf, void *inBuf, unsigned int nFrames,
                             double streamTime, RtAudioStreamStatus status) {
  float *buf = (float*) outBuf;
  for (int i = 0; i < nFrames; ++i) {
    // Note: this will not work if nChannels is > 1
    // Bonus exercise: fix it :-)
    *buf++ = tg_->nextSample();
  }
  return 0;
}

/******************************************************************************/
/* main */
/******************************************************************************/
int main(int argc, char *argv[]) {
  try {
    TCLAP::CmdLine cmd("Tone generator - demo project for RtAudio usage.", ' ', "0.1");

    TCLAP::ValueArg<unsigned int> srArg("s", "samplerate", "Sample rate to use", false, 44100, "unsigned int", cmd);
    TCLAP::ValueArg<unsigned int> chArg("c", "channels", "Number of channels to use", false, 1, "unsigned int", cmd);
    TCLAP::ValueArg<unsigned int> tgArg("t", "tonegen", "Tone generator to use (0=sine - default, 1=square, 2=white noise)", false, 0, "unsigned int", cmd);

    cmd.parse(argc, argv);

    SimplePlayback pb;

    if (!pb.init(srArg.getValue(), chArg.getValue(), tgArg.getValue())) {
      return -1;
    }

    if (pb.start()) {
      char input;
      std::cout << "Now playing.\n Press <enter> to quit...";
      std::cin.get(input);

      pb.stop();
    }
  } catch (TCLAP::ArgException& e) {
    std::cerr << "TCLAP error: " << e.error() << " for arg " << e.argId() << std::endl;
    return -1;
  }

  return 0;
}

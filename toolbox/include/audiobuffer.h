/******************************************************************************/
/*
 *  Project: Voice-over-IP
 *  Author: Matthias Tessmann <matthias.tessmann@th-nuernberg.de
 *  Date: October, 5th 2015
 *
 *  Contents: Very simple audio buffer class (wrapper around std::vector).
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

#ifndef VOIP_TOOLBOX_AUDIOBUFFER_H
#define VOIP_TOOLBOX_AUDIOBUFFER_H

#include <stdint.h>
#include <vector>

namespace voip_toolbox {

/* A very simple implementation of an audio buffer class. */
class AudioBuffer {
public:
  typedef enum sampleformat_ {
    INT16   = 16,
    FLOAT32 = 32,
  } SampleFormat;

  AudioBuffer(uint32_t fs, uint32_t nch, uint32_t sr, AudioBuffer::SampleFormat fmt = FLOAT32);

  uint32_t frameSize() const {
    return frameSize_;
  }

  uint32_t channels() const {
    return nChannels_;
  }

  uint32_t sampleRate() const {
    return sampleRate_;
  }

  SampleFormat format() const {
    return format_;
  }

  uint8_t* data() {
    return &data_[0];
  }

  uint32_t size() const {
    return data_.size();
  }

  uint32_t nSamples() const {
    return frameSize()*channels();
  }

private:
  uint32_t             frameSize_;
  uint32_t             nChannels_;
  uint32_t             sampleRate_;
  SampleFormat         format_;
  std::vector<uint8_t> data_;
};

}

#endif /* VOIP_TOOLBOX_AUDIOBUFFER_H */
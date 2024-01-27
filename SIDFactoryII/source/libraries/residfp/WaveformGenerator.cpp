/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2019 Leandro Nini <drfiemost@users.sourceforge.net>
 * Copyright 2007-2010 Antti Lankila
 * Copyright 2004 Dag Lem <resid@nimrod.no>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#define WAVEFORMGENERATOR_CPP

#include "WaveformGenerator.h"

#include "Dac.h"

namespace reSIDfp
{

/**
 * Number of cycles after which the waveform output fades to 0 when setting
 * the waveform register to 0.
 *
 * FIXME
 * This value has been adjusted aleatorily
 * from the original reSID value (0x4000)
 * to fix /MUSICIANS/H/Hatlelid_Kris/Grand_Prix_Circuit.sid#2
 * and /MUSICIANS/P/PVCF/Thomkat_with_Strange_End.sid;
 * see [VICE Bug #290](http://sourceforge.net/p/vice-emu/bugs/290/)
 * and [VICE Bug #1128](http://sourceforge.net/p/vice-emu/bugs/1128/)
 */
const int FLOATING_OUTPUT_TTL_6581 = 200000;  // ~200ms
const int FLOATING_OUTPUT_TTL_8580 = 5000000; // ~5s;

/**
 * Number of cycles after which the shift register is reset
 * when the test bit is set.
 * Values measured on warm chips (6581R3 and 8580R5).
 * Times vary wildly with temperature and may differ
 * from chip to chip so the numbers here represent
 * only the big difference between the old and new models.
 */
const int SHIFT_REGISTER_RESET_6581 = 200000;  // ~200ms
const int SHIFT_REGISTER_RESET_8580 = 5000000; // ~5s

const int DAC_BITS = 12;

/*
 * This is what happens when the lfsr is clocked:
 *
 * cycle 0: bit 19 of the accumulator goes from low to high, the noise register acts normally,
 *          the output may overwrite a bit;
 *
 * cycle 1: first phase of the shift, the bits are interconnected and the output of each bit
 *          is latched into the following. The output may overwrite the latched value.
 *
 * cycle 2: second phase of the shift, the latched value becomes active in the first
 *          half of the clock and from the second half the register returns to normal operation.
 *
 * When the test or reset lines are active the first phase is executed at every cycle
 * until the signal is released triggering the second phase.
 */
void WaveformGenerator::clock_shift_register(unsigned int bit0)
{
    shift_register = (shift_register >> 1) | bit0;

    // New noise waveform output.
    set_noise_output();
}

unsigned int WaveformGenerator::get_noise_writeback()
{
  return
    ~(
        (1 <<  2) |  // Bit 20
        (1 <<  4) |  // Bit 18
        (1 <<  8) |  // Bit 14
        (1 << 11) |  // Bit 11
        (1 << 13) |  // Bit  9
        (1 << 17) |  // Bit  5
        (1 << 20) |  // Bit  2
        (1 << 22)    // Bit  0
    ) |
    ((waveform_output & (1 << 11)) >>  9) |  // Bit 11 -> bit 20
    ((waveform_output & (1 << 10)) >>  6) |  // Bit 10 -> bit 18
    ((waveform_output & (1 <<  9)) >>  1) |  // Bit  9 -> bit 14
    ((waveform_output & (1 <<  8)) <<  3) |  // Bit  8 -> bit 11
    ((waveform_output & (1 <<  7)) <<  6) |  // Bit  7 -> bit  9
    ((waveform_output & (1 <<  6)) << 11) |  // Bit  6 -> bit  5
    ((waveform_output & (1 <<  5)) << 15) |  // Bit  5 -> bit  2
    ((waveform_output & (1 <<  4)) << 18);   // Bit  4 -> bit  0
}

void WaveformGenerator::write_shift_register()
{
    if (unlikely(waveform > 0x8) && likely(!test) && likely(shift_pipeline != 1))
    {
        // Write changes to the shift register output caused by combined waveforms
        // back into the shift register. This happens only when the register is clocked
        // (see $D1+$81_wave_test [1]) or when the test bit is set.
        // A bit once set to zero cannot be changed, hence the and'ing.
        //
        // [1] ftp://ftp.untergrund.net/users/nata/sid_test/$D1+$81_wave_test.7z
        //
        // FIXME: Write test program to check the effect of 1 bits and whether
        // neighboring bits are affected.

        shift_register &= get_noise_writeback();

        noise_output &= waveform_output;
        no_noise_or_noise_output = no_noise | noise_output;
    }
}

void WaveformGenerator::reset_shift_register()
{
    shift_register = 0x7fffff;
    shift_register_reset = 0;
}

void WaveformGenerator::set_noise_output()
{
    noise_output =
        ((shift_register & (1 <<  2)) <<  9) |  // Bit 20 -> bit 11
        ((shift_register & (1 <<  4)) <<  6) |  // Bit 18 -> bit 10
        ((shift_register & (1 <<  8)) <<  1) |  // Bit 14 -> bit  9
        ((shift_register & (1 << 11)) >>  3) |  // Bit 11 -> bit  8
        ((shift_register & (1 << 13)) >>  6) |  // Bit  9 -> bit  7
        ((shift_register & (1 << 17)) >> 11) |  // Bit  5 -> bit  6
        ((shift_register & (1 << 20)) >> 15) |  // Bit  2 -> bit  5
        ((shift_register & (1 << 22)) >> 18);   // Bit  0 -> bit  4

    no_noise_or_noise_output = no_noise | noise_output;
}

void WaveformGenerator::setWaveformModels(matrix_t* models)
{
    model_wave = models;
}

void WaveformGenerator::setChipModel(ChipModel chipModel)
{
    is6581 = chipModel == MOS6581;

    Dac dacBuilder(DAC_BITS);
    dacBuilder.kinkedDac(chipModel);

    const double offset = dacBuilder.getOutput(is6581 ? 0x380 : 0x9c0);

    for (unsigned int i = 0; i < (1 << DAC_BITS); i++)
    {
        const double dacValue = dacBuilder.getOutput(i);
        dac[i] = static_cast<float>(dacValue - offset);
    }

    model_shift_register_reset = is6581 ? SHIFT_REGISTER_RESET_6581 : SHIFT_REGISTER_RESET_8580;
}

void WaveformGenerator::synchronize(WaveformGenerator* syncDest, const WaveformGenerator* syncSource) const
{
    // A special case occurs when a sync source is synced itself on the same
    // cycle as when its MSB is set high. In this case the destination will
    // not be synced. This has been verified by sampling OSC3.
    if (unlikely(msb_rising) && syncDest->sync && !(sync && syncSource->msb_rising))
    {
        syncDest->accumulator = 0;
    }
}

bool do_pre_writeback(unsigned int waveform_prev, unsigned int waveform, bool is6581)
{
    // no writeback without combined waveforms
    if (likely(waveform_prev <= 0x8))
        return false;
    // no writeback when changing to noise
    if (waveform == 8)
        return false;
    // What's happening here?
    if (is6581 &&
            ((((waveform_prev & 0x3) == 0x1) && ((waveform & 0x3) == 0x2))
            || (((waveform_prev & 0x3) == 0x2) && ((waveform & 0x3) == 0x1))))
        return false;
    if (waveform_prev == 0xc)
        return false;
    // ok do the writeback
    return true;
}

void WaveformGenerator::writeCONTROL_REG(unsigned char control)
{
    const unsigned int waveform_prev = waveform;
    const bool test_prev = test;

    waveform = (control >> 4) & 0x0f;
    test = (control & 0x08) != 0;
    sync = (control & 0x02) != 0;

    // Substitution of accumulator MSB when sawtooth = 0, ring_mod = 1.
    ring_msb_mask = ((~control >> 5) & (control >> 2) & 0x1) << 23;

    if (waveform != waveform_prev)
    {
        // Set up waveform table.
        wave = (*model_wave)[waveform & 0x7];

        // no_noise and no_pulse are used in set_waveform_output() as bitmasks to
        // only let the noise or pulse influence the output when the noise or pulse
        // waveforms are selected.
        no_noise = (waveform & 0x8) != 0 ? 0x000 : 0xfff;
        no_noise_or_noise_output = no_noise | noise_output;
        no_pulse = (waveform & 0x4) != 0 ? 0x000 : 0xfff;

        if (waveform == 0)
        {
            // Change to floating DAC input.
            // Reset fading time for floating DAC input.
            floating_output_ttl = is6581 ? FLOATING_OUTPUT_TTL_6581 : FLOATING_OUTPUT_TTL_8580;
        }
    }

    if (test != test_prev)
    {
        if (test)
        {
            // Reset accumulator.
            accumulator = 0;

            // Flush shift pipeline.
            shift_pipeline = 0;

            // Set reset time for shift register.
            shift_register_reset = model_shift_register_reset;
        }
        else
        {
            // When the test bit is falling, the second phase of the shift is
            // completed by enabling SRAM write.

            // During first phase of the shift the bits are interconnected
            // and the output of each bit is latched into the following.
            // The output may overwrite the latched value.
            if (do_pre_writeback(waveform_prev, waveform, is6581))
            {
                shift_register &= get_noise_writeback();
            }

            // bit0 = (bit22 | test) ^ bit17 = 1 ^ bit17 = ~bit17
            clock_shift_register((~shift_register << 17) & (1 << 22));

            // New noise waveform output.
            set_noise_output();
        }
    }
}

void WaveformGenerator::reset()
{
    // accumulator is not changed on reset
    freq = 0;
    pw = 0;

    msb_rising = false;

    waveform = 0;
    osc3 = 0;

    test = false;
    sync = false;

    wave = model_wave ? (*model_wave)[0] : nullptr;

    ring_msb_mask = 0;
    no_noise = 0xfff;
    no_pulse = 0xfff;
    pulse_output = 0xfff;

    reset_shift_register();
    // when reset is released the shift register is clocked once
    clock_shift_register((~shift_register << 17) & (1 << 22));

    shift_pipeline = 0;

    waveform_output = 0;
    floating_output_ttl = 0;
}

} // namespace reSIDfp

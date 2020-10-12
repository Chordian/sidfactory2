#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

#include "source_mod.h"

//#define DEBUG_OUTPUT

#define ADAPTED_SLIDE_SPEED 30

namespace Converter
{
	SourceMod::SourceMod(SF2::Interface* inInterface, unsigned char* inByteBuffer)
		: m_SF2(inInterface)
		, m_ByteData(inByteBuffer)
		, m_ModHeader()
		, m_ModPatterns()
		, m_ModEvents()
		, m_ModMaxPattern(0)
		, m_ModOrderListLength()
        , m_ModCommandChecked()
        , m_ModCommandsUsed()
	{
        m_SF2->GetCout() << "Converting to \"" << m_SF2->GetDriverName() << "\" now." << std::endl;
	}

	SourceMod::~SourceMod()
	{
	}

    /**
     * Write a warning to the console that the specified command is not supported.
     */
    void SourceMod::Unsupported(const std::string& inModCommand)
    {
        for (const std::string checked_command : m_ModCommandChecked)
            if (inModCommand == checked_command) return;

        m_SF2->GetCout() << "WARNING: MOD command " << inModCommand << " is not supported by the converter." << std::endl;
        m_ModCommandChecked.push_back(inModCommand);
    }

    /**
     * Add the command used to a roster to be output in a report at the end.
     */
    void SourceMod::AddToCommandRoster(unsigned char inModCommand)
    {
        std::stringstream stream;
        stream << std::uppercase << std::hex << (int)inModCommand << "x" << (inModCommand == 0x0 || (inModCommand >= 0x4 && inModCommand <= 0x7) || inModCommand == 0xa || inModCommand == 0xe ? "y" : "x");

        for (const std::string command_used : m_ModCommandsUsed)
            if (stream.str() == command_used) return;

        m_ModCommandsUsed.push_back(stream.str());
    }

    /**
	 * Convert MOD to SF2 format.
	 */
	bool SourceMod::Convert(int inLeaveout, int inTranspose)
	{
        if (inLeaveout < 1 || inLeaveout > 4)
        {
            m_SF2->GetCout() << "\nERROR: Leaving out track " << inLeaveout << " is not valid!" << std::endl;
            return false;
        }
		memcpy(m_ModHeader, m_ByteData, 1084);				// 0	(1084)	Header with song name, sample names, sample ranges, etc.
		m_ModOrderListLength = m_ModHeader[950];			// 950	(1)		Song length (range is 1-128)
		for (int c = 0; c < 128; c++)
			if (m_ModHeader[952 + c] > m_ModMaxPattern)		// 952	(128)	Song positions 0-127; each with pattern 0-63 (or 127)
				m_ModMaxPattern = m_ModHeader[952 + c];
		m_ModMaxPattern++;
		memcpy(m_ModPatterns, &m_ByteData[1084], m_ModMaxPattern * 1024);

#ifdef DEBUG_OUTPUT
		m_SF2->GetCout() << "ModHeader\n---------\n";
		m_Output->HexBlock(0, m_ModHeader, 1084);
        m_SF2->GetCout() << "ModPatterns\n-----------\n";
		m_Output->HexBlock(0, m_ModPatterns, m_ModMaxPattern * 1024);
#endif // DEBUG_OUTPUT

		unsigned short period_table[16][12] =
		{
		  {6848, 6464, 6096, 5760, 5424, 5120, 4832, 4560, 4304, 4064, 3840, 3624},
		  {6800, 6416, 6056, 5720, 5392, 5096, 4808, 4536, 4280, 4040, 3816, 3600},
		  {6752, 6368, 6016, 5672, 5360, 5056, 4776, 4504, 4256, 4016, 3792, 3576},
		  {6704, 6328, 5968, 5632, 5320, 5024, 4736, 4472, 4224, 3984, 3760, 3552},
		  {6656, 6280, 5928, 5592, 5280, 4984, 4704, 4440, 4192, 3960, 3736, 3528},
		  {6608, 6232, 5888, 5552, 5240, 4952, 4672, 4408, 4160, 3928, 3704, 3496},
		  {6560, 6192, 5840, 5512, 5208, 4912, 4640, 4376, 4128, 3896, 3680, 3472},
		  {6512, 6144, 5800, 5472, 5168, 4880, 4600, 4344, 4104, 3872, 3656, 3448},
		  {7256, 6848, 6464, 6096, 5760, 5424, 5120, 4832, 4560, 4304, 4032, 3840},
		  {7200, 6800, 6416, 6056, 5720, 5400, 5088, 4808, 4536, 4280, 4040, 3816},
		  {7152, 6752, 6368, 6016, 5672, 5360, 5056, 4776, 4504, 4256, 4016, 3792},
		  {7096, 6704, 6328, 5968, 5632, 5320, 5024, 4736, 4472, 4224, 3984, 3760},
		  {7048, 6656, 6280, 5928, 5592, 5280, 4984, 4704, 4440, 4192, 3952, 3736},
		  {7000, 6608, 6232, 5888, 5552, 5240, 4952, 4672, 4408, 4160, 3928, 3704},
		  {6944, 6560, 6192, 5840, 5512, 5208, 4912, 4640, 4376, 4128, 3896, 3680},
		  {6896, 6512, 6144, 5800, 5472, 5168, 4880, 4600, 4344, 4104, 3872, 3656}
		};

		ModEvent* destination = m_ModEvents;
		unsigned char* source = m_ModPatterns;

		// Convert patterns into an easier-to-read format
		for (int c = 0; c < m_ModMaxPattern * 256; c++)
		{
			// NOTE: FT2 saves the 13th bit of period into the 5th bit of the sample number, and
			// when loading it cannot read the period back correctly. 13th bit is not used.

#ifdef DEBUG_OUTPUT
			if (c % 256 == 0)
			{
				if (c) m_SF2->GetCout() << std::endl;
                m_SF2->GetCout() << std::setfill('0') << "Pattern #" << std::hex << std::setw(2) << (int)c / 256 << "\n-----------\n";
			}
#endif // DEBUG_OUTPUT

			unsigned short period = ((source[0] & 0x0f) << 8) | source[1];
			unsigned char note = 0, instrument, command;

			if (period)
			{
				int find_note;
				int offset = 0x7fffffff;

				for (find_note = 0; find_note < 96; find_note++)
				{
					if (abs(period - (period_table[0][find_note % 12] >> (find_note / 12))) < offset)
					{
						note = find_note + 1;
						offset = abs(period - (period_table[0][find_note % 12] >> (find_note / 12)));
					}
				}
			}
			instrument = (source[0] & 0xf0) | ((source[2] & 0xf0) >> 4);
			command = source[2] & 0x0f;

			destination->m_ModNote = note;
			destination->m_ModInstr = instrument;
			destination->m_ModCommand = command;
			destination->m_ModData = source[3];

#ifdef DEBUG_OUTPUT
			if (c % 4 == 0) m_SF2->GetCout() << std::hex << std::setw(2) << (int)(c / 4 & 0x3f) << ":  ";
			if (note) m_SF2->GetCout() << c_NotesSharp[(int)(note - 1) % 12] << (int)(note - 1) / 18 << " "; // (Why 18 and not 12?)
			else m_SF2->GetCout() << "--- ";
			if (instrument) m_SF2->GetCout() << std::hex << std::setw(2) << (int)instrument << " ";
			else m_SF2->GetCout() << "-- ";
			if (command || source[3]) m_SF2->GetCout() << std::hex << std::setw(1) << (int)command << std::hex << std::setw(2) << (int)source[3];
			else m_SF2->GetCout() << "---";
			if (c % 4 == 3) m_SF2->GetCout() << std::endl;
			else m_SF2->GetCout() << "  ";
#endif // DEBUG_OUTPUT

			source += 4;
			destination++;
		}

#ifdef DEBUG_OUTPUT
        exit(0);
#endif // DEBUG_OUTPUT

        SF2Event sf2_events[127][65] = { 0 }, temp_notes[65] = { 0 };

        int sf2_channel = 0;
        bool break_note_shown = false;
        unsigned char sf2_sequence = 0x01; // As always leaving sequence #00 empty

        // Set default MOD speed
        m_SF2->EditTableRow(TABLE_TEMPO, 0, { 0x05 });

        // Command #00: Slide stop (used to halt a slide in its tracks)
        // NOTE: Using some 0xff for now to avoid it being overwritten by AppendToTable().
        if (m_SF2->AppendToTable(TABLE_CMDS, { 0x00, 0xff, 0xff }) == 0xff)
            return false;
        m_SF2->EditTableRowText(TABLE_CMDS, 0, "MOD to SF2");
        // Command #01: Portamento reset (can actually reset most effects)
        if (m_SF2->AppendToTable(TABLE_CMDS, { 0x02, 0x80, 0x00 }) == 0xff)
            return false;
        m_SF2->EditTableRowText(TABLE_CMDS, 1, "Stop effect");

        // Add a dummy triangle sound for all instruments
        if (m_SF2->AppendToTable(TABLE_WAVE,
            {
                { 0x11, 0x00 },
                { 0x7f, 0x00 },
            }) == 0xff)
        {
            return false;
        }

        m_SF2->GetCout() << "Ignoring MOD channel [" << inLeaveout << "] thus converting ";
        for (int channel = 1; channel <= 4; channel++)
            if (channel != inLeaveout) m_SF2->GetCout() << "[" << channel << "]";
        m_SF2->GetCout() << " to SF2.\n";

        if (inTranspose != 0)
            m_SF2->GetCout() << "The notes will be transposed +" << inTranspose << " halfsteps in the SF2 version.\n";

        // Add sample names as instrument descriptions in SF2
        for (int instr = 0; instr < 31; instr++)
        {
            std::string description(&m_ModHeader[instr * 30 + 20], &m_ModHeader[instr * 30 + 20] + 22);
            m_SF2->EditTableRowText(TABLE_INSTR, instr + 1, description);
        }

        std::string hex = "0123456789ABCDEF", arp_desc;

        // Push viable MOD data into the SF2 format
        for (int channel = 0; channel < 4; channel++)
        {
            if (channel != inLeaveout - 1)
            {
                for (int orderlist_pos = 0; orderlist_pos < m_ModOrderListLength; orderlist_pos++)
                {
                    int pattern_index = m_ModHeader[952 + orderlist_pos],
                        pattern = pattern_index * 256 + channel;
                    short speed;
                    bool break_pattern = false, cont_effect = false, slide_was_here = false;
                    unsigned char last_mod_data = 0, last_gate = 0x7e, last_instr = 0x80, command_to_set = 0x80, last_cmd = 0x80, actual_data = 0;
                    unsigned char index, this_cmd, vib_speed, vib_depth, arp_first, arp_second;

                    for(int pos = 0; pos < 64; pos++)
                    {
                        /* MOD notes */

                        if (m_ModEvents[pattern + pos * 4].m_ModNote)
                        {
                            temp_notes[pos].m_SF2Note = m_ModEvents[pattern + pos * 4].m_ModNote - 1 + inTranspose;
                            if (temp_notes[pos].m_SF2Note > 0x5d) temp_notes[pos].m_SF2Note = 0x5d; // Max (?)
                            cont_effect = slide_was_here = false;
                            command_to_set = last_cmd = 0x80;
                            last_gate = 0x7e;
                        }
                        // Note ON (or OFF if a C00 command has been issued)
                        else temp_notes[pos].m_SF2Note = last_gate;

                        /* MOD commands */

                        unsigned char mod_command = m_ModEvents[pattern + pos * 4].m_ModCommand;
                        unsigned char mod_data = m_ModEvents[pattern + pos * 4].m_ModData;

                        switch (mod_command)
                        {
                        case 0x0: // Both special case 000 (---) and arpeggio command

                            if (mod_data == 0x00) // This is a 000 (---) event
                            {
                                if (cont_effect && temp_notes[pos].m_SF2Note == last_gate)
                                {
                                    command_to_set = slide_was_here ? 0x00 : 0x01;
                                    last_cmd = command_to_set;
                                    cont_effect = slide_was_here = false;
                                }
                                else
                                    last_cmd = command_to_set = 0x80;
                            }
                            else // 0xy: Arpeggio (x = first halfstep add, y = second)
                            {
                                arp_first = (mod_data & 0xf0) >> 4;
                                arp_second = mod_data & 0x0f;

                                // :: Nibbles at 0 just make for unique arpeggios on their own
                                index = m_SF2->AppendToTable(TABLE_ARP,
                                    {
                                        {{ 0x00 }},
                                        {{ arp_first }},
                                        {{ arp_second }},
                                        {{ 0x70 }},
                                    });
                                if (index == 0xff)
                                    return false;
                                this_cmd = m_SF2->AppendToTable(TABLE_CMDS, { SF2::Interface::Cmd_Arpeggio, 0x00, index });
                                if (this_cmd == 0xff)
                                    return false;
                                m_SF2->EditTableRowText(TABLE_CMDS, this_cmd, "Arp 0"+std::string(1, hex[arp_first])+ std::string(1, hex[arp_second]));
                                cont_effect = true;

                                // If not the same command as last time it must be set
                                if (this_cmd != 0xff && this_cmd != last_cmd)
                                    command_to_set = last_cmd = this_cmd;
                                else
                                    command_to_set = 0x80;
                            }
                            break;

                        case 0x1: // 1xx: Slide up (xx = speed)
                        case 0x2: // 2xx: Slide down (xx = speed)

                            // :: 100, 200 and --- all just halts the slide in its tracks

                            command_to_set = 0x80;

                            if (mod_data == 0x00 && last_cmd)
                            {
                                // Halt the slide in its tracks
                                command_to_set = last_cmd = 0x00;
                                cont_effect = false;
                            }
                            else if (mod_data)
                            {
                                speed = mod_data * ADAPTED_SLIDE_SPEED;
                                if (mod_command == 0x02) speed = -speed;

                                this_cmd = m_SF2->AppendToTable(TABLE_CMDS, { SF2::Interface::Cmd_Slide, (unsigned char)(speed >> 8), (unsigned char)(speed & 0x00ff) });
                                if (this_cmd == 0xff)
                                    return false;
                                m_SF2->EditTableRowText(TABLE_CMDS, this_cmd, "Slide");
                                cont_effect = slide_was_here = true;

                                if (this_cmd != 0xff && this_cmd != last_cmd)
                                    command_to_set = last_cmd = this_cmd;
                            }
                            break;

                        case 0x3: // 3xx: Portamento up/down (xx = speed)

                            // :: 300 repeats the previous portamento speed setting; --- is needed to stop portamento

                            speed = mod_data == 0x00 ? last_mod_data : mod_data;
                            speed *= ADAPTED_SLIDE_SPEED;

                            this_cmd = m_SF2->AppendToTable(TABLE_CMDS, { SF2::Interface::Cmd_Portamento, (unsigned char)((speed & 0xff00) >> 8), (unsigned char)(speed & 0x00ff) });
                            if (this_cmd == 0xff)
                                return false;
                            m_SF2->EditTableRowText(TABLE_CMDS, this_cmd, "Portamento");
                            if (mod_data) last_mod_data = mod_data;
                            cont_effect = true;

                            if (this_cmd != 0xff && this_cmd != last_cmd)
                                command_to_set = last_cmd = this_cmd;
                            else
                                command_to_set = 0x80;
                            break;

                        case 0x4: // 4xy: Vibrato (x = speed, y = depth)

                            // :: Nibbles at 0 repeats the same setting; --- is needed to stop vibrato

                            actual_data = 0x00;
                            actual_data = mod_data & 0xf0 ? mod_data & 0xf0 : last_mod_data & 0xf0;
                            actual_data += mod_data & 0x0f ? mod_data & 0x0f : last_mod_data & 0x0f;
                            last_mod_data = actual_data;

                            // Translate to SF2 vibrato (where depth is upside down)
                            vib_speed = ((actual_data & 0xf0) >> 6) + 1;
                            vib_depth = (-actual_data & 0x0f) - 9;
                            if (!vib_depth) vib_depth = 1;

                            this_cmd = m_SF2->AppendToTable(TABLE_CMDS, { SF2::Interface::Cmd_Vibrato, vib_speed, vib_depth });
                            if (this_cmd == 0xff)
                                return false;
                            m_SF2->EditTableRowText(TABLE_CMDS, this_cmd, "Vibrato");
                            cont_effect = true;

                            if (this_cmd != 0xff && this_cmd != last_cmd)
                                command_to_set = last_cmd = this_cmd;
                            else
                                command_to_set = 0x80;
                            break;

                        case 0x5: // 5xy: Portamento + Volume slide (x = speed up, y = speed down)
                            Unsupported("5xy (Portamento + Volume slide)");
                            command_to_set = last_cmd = 0x80;
                            break;

                        case 0x6: // 6xy: Vibrato + Volume slide (x = speed up, y = speed down)
                            Unsupported("6xy (Vibrato + Volume slide)");
                            command_to_set = last_cmd = 0x80;
                            break;

                        case 0x7: // 7xy: Tremolo (x = speed, y = depth)
                            Unsupported("7xy (Tremolo)");
                            command_to_set = last_cmd = 0x80;
                            break;

                        case 0x8: // 8xx: Set note panning position (00-FF = far left to far right)
                            Unsupported("8xx (Set panning position)");
                            command_to_set = last_cmd = 0x80;
                            break;

                        case 0x9: // 9xx: Set sample offset (23 = 0x2300)
                            Unsupported("9xx (Set sample offset)");
                            command_to_set = last_cmd = 0x80;
                            break;

                        case 0xa: // Axy: Volume slide (x = speed up, y = speed down)
                            Unsupported("Axx (Volume slide)");
                            command_to_set = last_cmd = 0x80;
                            break;

                        case 0xb: // Bxx: Will be handled globally below
                            break;

                        case 0xc: // Cxx: Set volume (xx = 00-40)

                            // :: 00 is off, 40 is loudest; --- does NOT reset the volume last applied

                            if (mod_data == 0x00 && temp_notes[pos].m_SF2Note == 0x7e) // +++
                            {
                                // Convert zero volume to note off (---)
                                temp_notes[pos].m_SF2Note = last_gate = 0x00;
                                command_to_set = 0x80;
                            }
                            else
                            {
                                // Convert volume to sustain/release 06-F6 in ADSR (note) command (won't work 100% - upwards is silence)
                                this_cmd = m_SF2->AppendToTable(TABLE_CMDS, { SF2::Interface::Cmd_ADSR_Note, 0x00, (unsigned char)(((--mod_data << 2) & 0xf0) + 0x06) });
                                if (this_cmd == 0xff)
                                    return false;
                                m_SF2->EditTableRowText(TABLE_CMDS, this_cmd, "ADSR local");
                                last_gate = 0x7e;
                                cont_effect = false;

                                if (this_cmd != 0xff && this_cmd != last_cmd)
                                    command_to_set = last_cmd = this_cmd;
                                else
                                    command_to_set = 0x80;
                            }
                            break;

                        case 0xd: // Dxx: Will be handled globally below
                            break;

                        case 0xe: // E#x: Extended command...
                            Unsupported("E#x (Extended command)");
                            command_to_set = last_cmd = 0x80;
                            break;

                        case 0xf: // Fxx: Set speed (xx = 00-1f = speed (ticks), 20-ff = BPM) - will also be handled globally below

                            // :: 00 stops playback

                            if (mod_data == 0x00)
                                Unsupported("F00 (Stop playback)");
                            else if (mod_data < 0x20)
                            {
                                // Tempo command was added in driver 11.02
                                index = m_SF2->AppendToTable(TABLE_TEMPO,
                                    {
                                        {{ (unsigned char)(mod_data > 1 ? mod_data - 1 : 1) }},
                                        {{ 0x7f }},
                                    });
                                if (index == 0xff)
                                    return false;
                                this_cmd = m_SF2->AppendToTable(TABLE_CMDS, { SF2::Interface::Cmd_Tempo, 0x00, index });
                                if (this_cmd == 0xff)
                                    return false;
                                m_SF2->EditTableRowText(TABLE_CMDS, this_cmd, "Set tempo");
                                cont_effect = false;

                                if (this_cmd != 0xff && this_cmd != last_cmd)
                                    command_to_set = last_cmd = this_cmd;
                                else
                                    command_to_set = 0x80;
                            }
                            else
                            {
                                Unsupported("F20-FFF (BPM tempo)");
                                command_to_set = last_cmd = 0x80;
                            }
                            break;

                        default:
                            // See MOD command 0x0 above about handling 000 (---)
                            break;
                        }

                        // Some commands are global in nature
                        for (int channel = 0; channel < 4; channel++)
                        {
                            switch (m_ModEvents[(pattern_index * 256 + channel) + pos * 4].m_ModCommand)
                            {
                            case 0xb: // Bxx: Position jump (xx = song position)
                            case 0xd: // Dxx: Pattern break (xx = position in next pattern)
                                break_pattern = true;
                                break;

                            default:
                                break;
                            }
                        }

                        if (mod_command != 0 || mod_data != 0)
                            AddToCommandRoster(mod_command);

                        // Set command value whether actual or 0x80 (--)
                        temp_notes[pos].m_SF2Command = command_to_set;

                        /* MOD instruments */

                        const unsigned char mod_instr = m_ModEvents[pattern + pos * 4].m_ModInstr;
                        if (mod_instr && mod_instr != last_instr)
                        {
                            temp_notes[pos].m_SF2Instr = last_instr = mod_instr;
                            m_SF2->EditTableRow(TABLE_INSTR, mod_instr, { 0x00, 0xf6, 0x80, 0x00, 0x00, 0x00 });
                        }
                        // If both a note and a portamento there is a good chance the composer is trying to tie-slide to it
                        else if (temp_notes[pos].m_SF2Note >= 0x01 && temp_notes[pos].m_SF2Note <= 0x6f && mod_command == 0x3)
                            temp_notes[pos].m_SF2Instr = 0x90; // Same as ** (tie note)
                        else
                            temp_notes[pos].m_SF2Instr = 0x80; // Same as --

                        if (break_pattern)
                        {
                            temp_notes[++pos].m_SF2Note = 0x7f;
                            if (!break_note_shown)
                            {
                                m_SF2->GetCout() << "NOTE: The Bxx and Dxx commands only cut their sequence shorter in SF2.\n";
                                break_note_shown = true;
                            }
                            break;
                        }
                    }

                    // Compare sequence data to see if a repeat
                    unsigned char seq, pos;
                    for (seq = 0x01; seq < sf2_sequence; seq++)
                    {
                        for (pos = 0; pos < 64; pos++)
                            if ((sf2_events[seq][pos].m_SF2Instr != temp_notes[pos].m_SF2Instr) ||
                                (sf2_events[seq][pos].m_SF2Command != temp_notes[pos].m_SF2Command) ||
                                (sf2_events[seq][pos].m_SF2Note != temp_notes[pos].m_SF2Note)) break;

                        if (pos == 64) break;
                    }

                    // It is a new sequence
                    if (seq == sf2_sequence)
                    {
                        if (sf2_sequence >= 0x7f)
                        {
                            m_SF2->GetCout() << "\nERROR: Song is too complex; exceeded the 127 available sequences for SID Factory II." << std::endl;
                            return false;
                        }
                        // Remember and then add it
                        for (pos = 0; pos < 64; pos++)
                        {
                            if (temp_notes[pos].m_SF2Note == 0x7f) break;
                            sf2_events[seq][pos] = temp_notes[pos];
                            m_SF2->AppendToSequence(seq, { temp_notes[pos].m_SF2Instr, temp_notes[pos].m_SF2Command, temp_notes[pos].m_SF2Note });
                        }
                        sf2_sequence++;
                    }
                    m_SF2->AppendToOrderList(sf2_channel, { 0xa0, seq });
                }
                sf2_channel++;
            }
        }

        // Replace 0xff with 0x00 in the first command
        m_SF2->EditTableRow(TABLE_CMDS, 0, { 0x00, 0x00, 0x00 });

        // Compile the roster of commands used
        std::sort(m_ModCommandsUsed.begin(), m_ModCommandsUsed.end());
        std::string commands_used;
        for (std::string command : m_ModCommandsUsed)
            commands_used += command + " ";

        m_SF2->GetCout() << std::endl
            << "  MOD commands used:      " << commands_used << "\n"
            << "  MOD patterns found:     " << /*std::setw(3) <<*/ m_ModMaxPattern << "\n"
            << "\n"
            << "  SF2 commands created:   " << /*std::setw(3) <<*/ m_SF2->GetCount(TABLE_CMDS) << "\n"
            << "  SF2 sequences created:  " << /*std::setw(3) <<*/ (int)(--sf2_sequence) << std::endl;

        bool data_pushed = m_SF2->PushAllDataToMemory(true);
        if (!data_pushed)
        {
            m_SF2->GetCout() << "\nERROR: Could not pack the data before saving it.";
            return false;
        }

        return true;
    }
}
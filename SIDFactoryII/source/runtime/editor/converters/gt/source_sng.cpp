#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

#include "source_sng.h"

//#define DEBUG_OUTPUT

namespace Converter
{
	SourceSng::SourceSng(SF2::Interface* inInterface, unsigned char* inByteBuffer)
		: m_SF2(inInterface)
		, m_ByteData(inByteBuffer)
		, m_SngVersion(0)
	{
        // std::cout << "Converting to \"" << m_SF2->GetDriverName() << "\" now." << std::endl;
	}

	SourceSng::~SourceSng()
	{
	}



	const std::string& SourceSng::GetErrorMessage() const
	{
		return m_ErrorMessage;
	}

	/**
	 * Write a warning to the console that the specified command is not supported.
	 */
	void SourceSng::Unsupported(const std::string& inSngCommand)
	{
		for (const std::string checked_command : m_SngCommandChecked)
			if (inSngCommand == checked_command) return;

		std::cerr << "WARNING: SNG command " << inSngCommand << " is not supported by the SF2 driver." << std::endl;
		m_SngCommandChecked.push_back(inSngCommand);
	}

	/**
	 * Add the command used to a roster to be output in a report at the end.
	 */
	void SourceSng::AddToCommandRoster(unsigned char inSngCommand)
	{
		std::stringstream stream;
		stream << std::uppercase << std::hex << (int)inSngCommand << "x" << (inSngCommand == 0x5 || inSngCommand == 0x6 || inSngCommand == 0xb || inSngCommand == 0xd ? "y" : "x");

		for (const std::string command_used : m_SngCommandsUsed)
			if (stream.str() == command_used) return;

		m_SngCommandsUsed.push_back(stream.str());
	}

	/**
	 * Convert SNG (GT2) to SF2 format.
	 */
	bool SourceSng::Convert(int inTranspose)
	{
		// SNG version 
		m_SngVersion = m_ByteData[4] == 0x21 ? 1 : m_ByteData[0x4] - 0x30;

		// Set default GT2 speed
		m_SF2->EditTableRow(TABLE_TEMPO, 0, { 0x05 });

		// Command #00: Slide stop (used to halt a slide in its tracks)
		// NOTE: Using some 0xff for now to avoid it being overwritten by AppendToTable().
		m_SF2->EditTableRow(TABLE_CMDS, 0, { 0x00, 0xff, 0xff });
		m_SF2->EditTableRowText(TABLE_CMDS, 0, "GT2 to SF2");

		// Process data
		bool warning_shown = false;
		for (int pos = 0x65;;)
		{
			/* ---- ORDER LISTS ---- */

			for (unsigned char subtune = 0; subtune < m_ByteData[0x64]; subtune++)
			{
				if (subtune && !warning_shown)
				{
					std::cerr << "WARNING: Multiple sub tunes are not supported by SF2 yet; only the first sub tune will be converted." << std::endl;
					warning_shown = true;
				}

				int repeat = 1, orderlist_size, byte;
				unsigned char value = 0, transpose;

				// Process order lists
				for (int channel = 0; channel < 3; channel++)
				{
					transpose = 0xa0;
					orderlist_size = m_ByteData[pos] + 1;

					for (byte = 0; byte < orderlist_size - 1; byte++)
					{
						value = m_ByteData[pos + byte + 1];
						if ((value & 0xf0) == 0xe0 || (value & 0xf0) == 0xf0)
						{
							transpose = value < 0xf0 ? 0xa0 - (-(value & 0x0f) & 0x0f) : (value & 0x0f) + 0xa0;
						}
						else if ((value & 0xf0) == 0xd0)
						{
							if (repeat == 0) repeat = 16;
							else repeat = (value & 0x0f) + 1;
						}
						else
						{
							while (repeat--)
								if (subtune == 0) m_SF2->AppendToOrderList(channel, { transpose, value });
							repeat = 1;
						}
					}
					pos += orderlist_size + 1;
				}
			}

			/* ---- INSTRUMENTS ---- */

			int instr_count = m_ByteData[pos++], this_instr;
			if (instr_count > 0x1f)
			{
				m_ErrorMessage = "Song is too complex; exceeded the 32 available instruments for SID Factory II.";
				return false;
			}

			bool ignored_message = false;
			unsigned char hr_byte;
			std::vector<std::pair<unsigned char, unsigned char>> instr_adsr;
			instr_adsr.push_back({ 0, 0 }); // First pos is unused in SNG

			for (int instr = 0; instr < instr_count; instr++)
			{
				this_instr = pos + (instr * 25);								// Instruments are 9 bytes data + 16 bytes instrument name
				hr_byte =
					((m_ByteData[this_instr + 7] & 0x80) == 0 ? 0x80 : 0x00) +	// HR on if bit 7 in HR byte in SNG instrument is set
					(m_ByteData[this_instr + 4] ? 0x40 : 0x00) +				// Filter on if filter index in SNG instrument is used
					(m_ByteData[this_instr + 8] == 0x09 ? 0x10 : 0x00);			// Oscillator reset on if waveform $09 used for HR

				// Create matching SF2 instrument
				m_SF2->EditTableRow(TABLE_INSTR, instr + 1,
					{ 
						m_ByteData[this_instr],									// Attack/Decay
						m_ByteData[this_instr + 1],								// Sustain/Release
						hr_byte,												// SF2 HR on/off + filter on/off (see above)
						m_ByteData[this_instr + 4],								// Filter table index
						m_ByteData[this_instr + 3],								// Pulse table index
						m_ByteData[this_instr + 2],								// Wave table index
					});
				if (m_ByteData[this_instr + 6])
				{
					std::cerr << std::setfill('0') << "Ignored vibrato set directly in GT2 instrument $" << std::uppercase << std::setw(2) << std::hex << instr << "; please use SF2 command $01 instead." << std::endl;
					ignored_message = true;
				}

				// Add instrument description
				std::string description(&m_ByteData[this_instr + 9], &m_ByteData[this_instr + 9] + 16);
				m_SF2->EditTableRowText(TABLE_INSTR, instr + 1, description);

				// Remember ADSR for AD and SR pattern commands later
				instr_adsr.push_back(std::make_pair(m_ByteData[this_instr], m_ByteData[this_instr + 1]));
			}
			pos += instr_count * 25;

			/* ---- WAVE TABLE ---- */

			// Row #00 is used to stop table execution
			m_SF2->EditTableRow(TABLE_WAVE, 0x00, { 0x7f, 0x00 });
			
			bool warning_delay_shown = false, warning_cmd_shown = false;
			int wave_table_count = m_ByteData[pos++];
			unsigned char last_waveform = 0x00, last_freq = 0x00, waveform, freq, left, right;

			for (int byte = 0; byte < wave_table_count; byte++)
			{
				left = m_ByteData[pos + byte];

				if (left >= 0x01 && left <= 0x0f)
				{
					if (!warning_delay_shown) std::cerr << "WARNING: Delay values in the wave table are not supported and will be ignored." << std::endl;
					warning_delay_shown = true;
				}
				else if (left >= 0xf0 && left <= 0xfe)
				{
					if (!warning_cmd_shown) std::cerr << "WARNING: Pattern commands in the wave table are not supported and will be ignored." << std::endl;
					warning_cmd_shown = true;
				}
				else
				{
					right = m_ByteData[pos + wave_table_count + byte];

					if (left == 0xff)
					{
						waveform = 0x7f;
						// GT2 ends with 00; SF2 ends by looping to last
						freq = right ? right : byte;
					}
					else
					{
						if (left == 0x00)
							waveform = last_waveform;
						else if (left >= 0xe0 && left <= 0xef)
							waveform = last_waveform = (left & 0x0f);
						else
							waveform = last_waveform = left;

						if (right == 0x80)
							freq = last_freq;
						else
							freq = last_freq = right;
					}
					m_SF2->EditTableRow(TABLE_WAVE, byte + 1, { waveform, freq });
				}
			}
			pos += wave_table_count * 2;

			/* ---- PULSE TABLE ---- */

			// Row #00 is used to stop table execution
			m_SF2->EditTableRow(TABLE_PULSE, 0x00, { 0x7f, 0x00, 0x00 });

			int pulse_table_count = m_ByteData[pos++];
			std::vector<unsigned char> row;

			for (int byte = 0; byte < pulse_table_count; byte++)
			{
				left = m_ByteData[pos + byte];
				right = m_ByteData[pos + pulse_table_count + byte];

				if (left == 0xff)
					// GT2 ends with 00; SF2 ends by looping to end marker
					row = { 0x7f, 0x00, (unsigned char)(right ? right : byte + 1) };
				else if (left >= 0x80)
					row = { (unsigned char)(0x80 + (left & 0x0f)), right, 0x00 };
				else
					row = { (unsigned char)(right >= 0x80 ? 0x0f : 0x00), right , left };
				m_SF2->EditTableRow(TABLE_PULSE, byte + 1, row);
			}
			pos += pulse_table_count * 2;

			/* ---- FILTER TABLE ---- */

			// Row #00 is used to stop table execution
			m_SF2->EditTableRow(TABLE_FILTER, 0x00, { 0x7f, 0x00, 0x00 });

			int filter_table_count = m_ByteData[pos++];
			unsigned char passband = 0x00, resonance_bitmask = 0x00, cutoff = 0x00, this_cutoff;
			unsigned short filter_speed;

			for (int byte = 0; byte < filter_table_count; byte++)
			{
				left = m_ByteData[pos + byte];
				right = m_ByteData[pos + filter_table_count + byte];

				if (left == 0x00)
				{
					cutoff = right;
					row = { (unsigned char)(passband + (cutoff >> 0x4)), (unsigned char)(cutoff << 0x4), resonance_bitmask };
				}
				else if (left == 0xff)
					// GT2 ends with 00; SF2 ends by looping to end marker
					row = { 0x7f, 0x00, (unsigned char)(right ? right : byte + 1) };
				else if (left >= 0x80)
				{
					passband = left;
					resonance_bitmask = right;
					// Use cutoff in the next table row if present or just use most recent
					this_cutoff = m_ByteData[pos + byte + 1] == 0x00 ? m_ByteData[pos + byte + filter_table_count + 1] : cutoff;
					row = { (unsigned char)(passband + (this_cutoff >> 0x4)), (unsigned char)(this_cutoff << 0x4), resonance_bitmask };
				}
				else
				{
					filter_speed = ((right >= 0x80 ? 0x0f : 0x00) * 256 + right) << 4;
					row = { (unsigned char)((filter_speed / 256) & 0x0f), (unsigned char)(filter_speed & 0x00ff), (unsigned char)(left ? left - 1 : 0) };
				}
				m_SF2->EditTableRow(TABLE_FILTER, byte + 1, row);
			}
			pos += filter_table_count * 2;

			// Tidy up the table by removing duplicate filter control rows
			SF2::Interface::Table table = m_SF2->GetTable(SF2::Interface::TableType::Filter);
			SF2::Interface::TableData table_data = m_SF2->GetTableData(SF2::Interface::TableType::Filter);
			SF2::Interface::WrapFormat wrap_info = m_SF2->GetWrapFormat(SF2::Interface::TableType::Filter);
			bool move_mode = false;

			for (int row = 1; row < table.m_RowCount; row++)
			{
				if ((*table_data.m_DataSourceTable)[row * table.m_ColumnCount] > 0x80) move_mode = true; // Filter control row

				if (move_mode)
					// Move row up so the duplicate filter control row is overwritten
					for (int col = 0; col < table.m_ColumnCount; col++)
						(*table_data.m_DataSourceTable)[row * table.m_ColumnCount + col] = (*table_data.m_DataSourceTable)[row * table.m_ColumnCount + col + table.m_ColumnCount];

				// Find wrap ID byte value (e.g. 0x7e, 0x7f, etc.)
				for (auto wrap_id : wrap_info.m_ByteID)
				{
					// Is this a wrap ID byte?
					if ((*table_data.m_DataSourceTable)[(row * table.m_ColumnCount) + wrap_info.m_ByteIDPosition] == wrap_id)
					{
						// Wrap ID byte found; decrease the wrap value by 1
						unsigned char wrap_byte_pos = (row * table.m_ColumnCount) + wrap_info.m_ByteWrapPosition;
						(*table_data.m_DataSourceTable)[wrap_byte_pos] = (((*table_data.m_DataSourceTable)[wrap_byte_pos] & wrap_info.m_ByteWrapMask) - 1) & wrap_info.m_ByteWrapMask;

						// Just clear the row where the wrap marker was before
						for (int col = 0; col < table.m_ColumnCount; col++)
							(*table_data.m_DataSourceTable)[row * table.m_ColumnCount + col + table.m_ColumnCount] = 0x00;

						move_mode = false;
						break;
					}
				}
			}
			m_SF2->PushTableToSource(table_data);

			/* ---- SPEED TABLE ---- */

			int speed_table_count = m_ByteData[pos++];
			if (speed_table_count > 0x3e) // Also need one more for the portamento reset below
			{
				m_ErrorMessage = "Song is too complex; exceeded the 64 available commands for SID Factory II.";
				return false;
			}

			std::vector<std::pair<unsigned char, unsigned char>> speed_table;
			speed_table.push_back({ 0, 0 }); // First pos is unused in SNG

			// Have to refer to the speed table while processing patterns as the bytes in it could be anything
			for (int byte = 0; byte < speed_table_count; byte++)
				speed_table.push_back(std::make_pair(m_ByteData[pos + byte], m_ByteData[pos + speed_table_count + byte]));
			pos += speed_table_count * 2;

			// Have to append a command at the end of speed table size to make sure AppendToTable() won't overwrite any of it
			// but at the same time it can be the effect stopping command that we can use to stop e.g. portamento
			unsigned char command_stop_effect = speed_table_count + 1;
			m_SF2->EditTableRow(TABLE_CMDS, command_stop_effect, { 0x02, 0x80, 0x00 });
			m_SF2->EditTableRowText(TABLE_CMDS, command_stop_effect, "Stop effect");

			/* ---- PATTERNS ---- */

			int pat_count = m_ByteData[pos++], pat_size;
			if (pat_count > 0x7f)
			{
				m_ErrorMessage = "Song is too complex; exceeded the 127 available sequences for SID Factory II.";
				return false;
			}

			int ongoing_effect = 0;
			unsigned char command_to_set, last_cmd, last_instr, this_cmd, sng_command, sng_data,
				instrument, note, vib_speed, vib_depth, approx, rest, index;
			short down_speed;

			unsigned char vib_conversion[256] = {
				8, 7, 6, 6, 5, 5, 5, 4, 4, 4, 3, 3, 3, 3, 3, 3,
				3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
				2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1,
				1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
				1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
				0 }; // See GT2 command 4xx below

			for (int pat = 0; pat < pat_count; pat++)
			{
				pat_size = m_ByteData[pos++] * 4;
				command_to_set = last_cmd = 0x80;
				last_instr = 0x00;
				rest = 0x00;										// Not a perfect rest start but yields less bugs

				for (int pat_event = 0; pat_event < pat_size - 4; pat_event += 4)
				{
					note = m_ByteData[pos + pat_event];
					if (note == 0xbd) note = rest;					// Whatever is currently relevant (note on or off)
					else if (note == 0xbe) note = rest = 0x00;		// Note off - and rest is now note off too
					else if (note == 0xbf) note = rest = 0x7e;		// Note on - and rest is now note on too
					else
					{
						note -= 0x60;								// GT2 notes start at 0x60, SF2 notes start at 0x00
						rest = 0x7e;
					}

					instrument = m_ByteData[pos + pat_event + 1];
					if (instrument == 0x00 || instrument == last_instr) instrument = 0x80;
					else if (instrument < 0x80) last_instr = instrument;

					sng_command = m_ByteData[pos + pat_event + 2];
					sng_data = m_ByteData[pos + pat_event + 3];		// Sometimes this is a speed table index

					/*if (pat == 9) // For debugging
						std::cout
							<< std::setfill('0') << std::setw(2) << std::hex << (int)note << " "
							<< std::setw(2) << std::hex << (int)instrument << " "
							<< std::hex << (int)sng_command
							<< std::setw(2) << std::hex << (int)sng_data << std::endl;*/

					switch (sng_command)
					{
					case 0x0:	// 000: Does nothing (will use this to continue or stop slide/effect)

						if ((ongoing_effect == 0x1 || ongoing_effect == 0x2) && (note == 0x00 || note == 0x7e))
						{
							last_cmd = command_to_set = 0x00;					// Halt slide in its tracks
							ongoing_effect = 0;
						}
						else if (ongoing_effect == 0x3)
						{
							last_cmd = command_to_set = command_stop_effect;	// Stop portamento right there
							ongoing_effect = 0;
						}
						else if (ongoing_effect == 0x4 && (note == 0x00 || note == 0x7e))
						{
							last_cmd = command_to_set = command_stop_effect;	// Stop vibrato on note frequency
							ongoing_effect = 0;
						}
						else
							last_cmd = command_to_set = 0x80;
						break;

					case 0x1:	// 1xx: Slide up   (xx = speed table index for 16-bit value) - repeated each event for MOD compatibility

						if (speed_table[sng_data].first & 0x80)
						{
							Unsupported("1xx (Hi-fi slide up with divisor)");
							ongoing_effect = 0;
						}
						else
						{
							// The speed table row can be reused in the same location in the SF2 command table
							m_SF2->EditTableRow(TABLE_CMDS, sng_data, { SF2::Interface::Cmd_Slide, speed_table[sng_data].first, speed_table[sng_data].second });
							m_SF2->EditTableRowText(TABLE_CMDS, sng_data, "Slide up");
							ongoing_effect = sng_command;

							if (sng_data != last_cmd) command_to_set = last_cmd = sng_data;
							else command_to_set = 0x80;
						}
						break;

					case 0x2:	// 2xx: Slide down (xx = speed table index for 16-bit value) - repeated each event for MOD compatibility

						if (speed_table[sng_data].first & 0x80)
						{
							Unsupported("2xx (Hi-fi slide down with divisor)");
							ongoing_effect = 0;
						}
						else
						{
							down_speed = -(speed_table[sng_data].first * 256 + speed_table[sng_data].second);

							// Because of reversing the speed in SF2 a new row has to be appended (or reused) in the SF2 command table
							this_cmd = m_SF2->AppendToTable(TABLE_CMDS, { SF2::Interface::Cmd_Slide, (unsigned char)(down_speed >> 8), (unsigned char)(down_speed & 0x00ff) });
							m_SF2->EditTableRowText(TABLE_CMDS, this_cmd, "Slide down");
							ongoing_effect = sng_command;

							if (this_cmd != last_cmd) command_to_set = last_cmd = this_cmd;
							else command_to_set = 0x80;
						}
						break;

					case 0x3:	// 3xx: Portamento (xx = speed table index for 16-bit value) - repeated each event for MOD compatibility

						if (sng_data == 0x00)
						{
							if (note >= 0x01 && note <= 0x6f)
							{
								ongoing_effect = 0;

								if (instrument == 0x80)
								{
									// In GT2 the 300 command is used as a tie note
									instrument = 0x90;
									last_cmd = command_to_set = 0x80;
								}
								if (ongoing_effect == 0x3)
									// Better stop the previous portamento too
									last_cmd = command_to_set = command_stop_effect;
							}
						}
						else if (speed_table[sng_data].first & 0x80)
						{
							Unsupported("3xx (Hi-fi portamento with divisor)");
							ongoing_effect = 0;
						}
						else
						{
							// Have to append (or reuse) here too because portamento commands in SF2 have their own identifier
							this_cmd = m_SF2->AppendToTable(TABLE_CMDS, { SF2::Interface::Cmd_Portamento, speed_table[sng_data].first, speed_table[sng_data].second });
							m_SF2->EditTableRowText(TABLE_CMDS, this_cmd, "Portamento");
							ongoing_effect = sng_command;

							// And if there is a note better tie note to it
							if (instrument == 0x80 && note >= 0x01 && note <= 0x6f) instrument = 0x90;

							if (this_cmd != last_cmd) command_to_set = last_cmd = this_cmd;
							else command_to_set = 0x80;
						}
						break;

					case 0x4:	// 4xx: Vibrato (index to xx = speed and yy = depth in speed table) - repeated each event for MOD compatibility

						/*	Vibrato comparison
							------------------

								GT2 depth		SF2 depth
									1				7
									2               6
									4               5
									7				4
									9               3
								   18				2
								   40				1
								   80				0

							GT2 vibrato speed + 1 to reach approximate in SF2. */

						// Translate to SF2 vibrato (where depth is upside down)
						vib_speed = speed_table[sng_data].first + 1;
						vib_depth = vib_conversion[speed_table[sng_data].second];
						if (!vib_depth) vib_depth = 1;

						this_cmd = m_SF2->AppendToTable(TABLE_CMDS, { SF2::Interface::Cmd_Vibrato, vib_speed, vib_depth });
						m_SF2->EditTableRowText(TABLE_CMDS, this_cmd, "Vibrato");
						ongoing_effect = sng_command;

						if (this_cmd != last_cmd) command_to_set = last_cmd = this_cmd;
						else command_to_set = 0x80;
						break;

					case 0x5:	// 5xy: Attack/Decay (x = attack, y = decay)

						approx = sng_data;
						// approx = sng_data >> 2; // Staircase optimization to save command space
						// approx <<= 2;

						command_to_set = m_SF2->AppendToTable(TABLE_CMDS, { SF2::Interface::Cmd_ADSR_Note, approx, instr_adsr[last_instr].second });
						m_SF2->EditTableRowText(TABLE_CMDS, command_to_set, "ADSR local");
						ongoing_effect = 0;
						break;

					case 0x6:	// 6xy: Sustain/Release (x = sustain, y = release)

						approx = sng_data;
						// approx = sng_data >> 2; // Staircase optimization to save command space
						// approx <<= 2;

						command_to_set = m_SF2->AppendToTable(TABLE_CMDS, { SF2::Interface::Cmd_ADSR_Note, instr_adsr[last_instr].first, approx });
						m_SF2->EditTableRowText(TABLE_CMDS, command_to_set, "ADSR local");
						ongoing_effect = 0;
						break;

					case 0x7:	// 7xx: Waveform (overridden by any wave table action)
						Unsupported("7xy (Set waveform)"); // @todo Could be supported by adding a table command
						ongoing_effect = 0;
						break;

					case 0x8:	// 8xx: Wave table index (00 stops execution)
						command_to_set = m_SF2->AppendToTable(TABLE_CMDS, { SF2::Interface::Cmd_Index_Wave, 0x00, sng_data });
						m_SF2->EditTableRowText(TABLE_CMDS, command_to_set, "Wave index");
						ongoing_effect = 0;
						break;

					case 0x9:	// 9xx: Pulse table index (00 stops execution)
						command_to_set = m_SF2->AppendToTable(TABLE_CMDS, { SF2::Interface::Cmd_Index_Pulse, 0x00, sng_data });
						m_SF2->EditTableRowText(TABLE_CMDS, command_to_set, "Pulse index");
						ongoing_effect = 0;
						break;

					case 0xa:	// Axx: Filter table index (00 stops execution)
						command_to_set = m_SF2->AppendToTable(TABLE_CMDS, { SF2::Interface::Cmd_Index_Filter, 0x00, sng_data });
						m_SF2->EditTableRowText(TABLE_CMDS, command_to_set, "Filter index");
						ongoing_effect = 0;
						break;

					case 0xb:	// Bxy: Filter control (x = resonance, y = channel bitmask) - 00 both turns filter off and stops filter table execution
						Unsupported("Bxy (Filter control)"); // @todo Could be supported by adding a table command
						ongoing_effect = 0;
						break;

					case 0xc:	// Cxx: Filter cutoff value (overridden by any filter table action)
						Unsupported("Cxy (Filter cutoff)"); // @todo Could be supported by adding a table command
						ongoing_effect = 0;
						break;

					case 0xd:	// Dxy: Master volume (if 0, set master volume to y - if x != 0, set player address + 0x003F to xy)
						if ((sng_data & 0xf0) == 0)
						{
							command_to_set = m_SF2->AppendToTable(TABLE_CMDS, { SF2::Interface::Cmd_Volume, 0x00, sng_data });
							m_SF2->EditTableRowText(TABLE_CMDS, command_to_set, "Main volume");
						}
						ongoing_effect = 0;
						break;

					case 0xe:	// Exx: Funk tempo (xx = index to byte in speed table where speed alternates between its nibbles)
						Unsupported("Exx (Funk tempo)");
						ongoing_effect = 0;
						break;

					case 0xf:	// Fxx: Set tempo (xx = set tempo 03-7F on all channels - 83-FF set (& 7F) on current channel only - 00-01 recalls funk tempo set by Exx command)

						if (sng_data <= 0x01)
							Unsupported("F00-F01 (Recall funk tempo)");
						else if (sng_data >= 0x83)
							Unsupported("F83-FFF (Set tempo in solo channel only)");
						else
						{
							// Tempo command was added in driver 11.02
							index = m_SF2->AppendToTable(TABLE_TEMPO,
								{
									{{ --sng_data }},
									{{ 0x7f }},
								});
							this_cmd = m_SF2->AppendToTable(TABLE_CMDS, { SF2::Interface::Cmd_Tempo, 0x00, index });
							m_SF2->EditTableRowText(TABLE_CMDS, this_cmd, "Set tempo");
						}
						if (this_cmd != last_cmd) command_to_set = last_cmd = this_cmd;
						else command_to_set = 0x80;
						ongoing_effect = 0;
						break;

					default:
						// See SNG command 0x0 above about handling 000
						break;
					}
					m_SF2->AppendToSequence(pat, { instrument, command_to_set, note });

					if (sng_command != 0)
						AddToCommandRoster(sng_command);
				}
				pos += pat_size;
			}

			break;
		}

		// Replace 0xff with 0x00 in the first command
		m_SF2->EditTableRow(TABLE_CMDS, 0, { 0x00, 0x00, 0x00 });

		// Compile the roster of commands used
		// std::sort(m_SngCommandsUsed.begin(), m_SngCommandsUsed.end());
		// std::string commands_used;
		// for (std::string command : m_SngCommandsUsed)
		// 	commands_used += command + " ";
		// 
		// std::cout << std::endl
		// 	<< "  GT2 commands used:       " << commands_used << "\n"
		// 	<< "  SF2 commands allocated:  " << /*std::setw(3) <<*/ m_SF2->GetCount(TABLE_CMDS) << std::endl;

		bool data_pushed = m_SF2->PushAllDataToMemory(false);
		if (!data_pushed)
		{
			std::cerr << "Could not pack the data before saving it.";
			return false;
		}

		return true;
	}
}
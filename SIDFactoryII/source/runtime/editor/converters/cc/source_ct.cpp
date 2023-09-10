#include "foundation/base/assert.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <map>

#include "source_ct.h"

/*	Issues / Not Done

	No support for multiple sub tunes yet. Last sub tune in focus in CC is converted.

	The A, D, S and R sequence commands are not converted. I fear the command table will
	be filled up in no time if I try to convert those to local ADSR commands. (I could
	of course still do it and then add a command-line switch to override it.)

	The arpeggio speed nibble in the instrument is brought over to the chords in SF2 to
	match the right speed, but unfortunately this is not possible to match for standard
	wave tables too. It always runs at full speed in SF2.

	Delay commands are expanded too, but any loop index targeting a spot AFTER the cluster
	of expanded wave table rows will not be updated (it's rare but happens). All of the
	loop indices in the subsequent wave clusters are of course updated.

	Pulse or filter index 00 is the same as continuing. Check out the chords in the third
	voice of Scarzix's "Cheese Around the Clock" for an example. This is not supported in
	SF2 because it doesn't have a "continue that last effect" as such. However, it can be
	emulated by pointing the continuing instrument to the row that only updates while
	bypassing the row that set the start pulse or filter. (The 00 index is often used in
	CC songs together with a pulse index command in a sequence to change pulsating or
	filtering across several notes.)
*/

#define RAW_SIZE 0x50000 // Uncompress() requires a lot of room to work with

//#define DEBUG_FILE
//#define DEBUG_SEQ
//#define DEBUG_OUTPUT

namespace Converter
{
	SourceCt::SourceCt(SF2::Interface* inInterface, unsigned char* inByteBuffer, long inByteBufferSize)
		: m_SF2(inInterface)
		, m_ByteData(inByteBuffer)
		, m_ByteDataSize(inByteBufferSize)
		, m_CtVersion(0)
		, m_RawData(nullptr)
		, m_WarningShownPulseScope(false)
		, m_WarningShownFilterScope(false)
		, m_UnpackingSucceeded(false)
	{
		m_SF2->GetCout() << "Converting to \"" << m_SF2->GetDriverName() << "\" now." << std::endl;

		// Decompress the CT file
		m_RawDataSize = RAW_SIZE; // Will be adapted to true size when decompressing is complete
		m_RawData = new unsigned char[RAW_SIZE];
		const unsigned char* compressed_song = m_ByteData + 3;
		m_UnpackingSucceeded = uncompress(m_RawData, &m_RawDataSize, compressed_song, m_ByteDataSize - 3) == MZ_OK;
		
		if (!m_UnpackingSucceeded)
			m_SF2->GetCout() << "\nERROR: Could not decompress the CT source file." << std::endl;

#ifdef DEBUG_FILE
		m_SF2->GetCout() << "\nEmulated C64 memory in the decompressed CT source file\n------------------------------------------------------" << std::endl;
		m_Output->HexBlock(0, m_RawData, 0xffff);

		m_SF2->GetCout() << "\nMeta data and order lists in the decompressed CT source file\n------------------------------------------------------------" << std::endl;
		m_Output->HexBlock(0x10000, m_RawData, (unsigned int)(m_RawDataSize & 0xffff));
#endif // DEBUG_FILE
	}

	SourceCt::~SourceCt()
	{
		delete m_RawData;
	}

	/**
	 * Write a warning to the console that the specified command is not supported.
	 */
	void SourceCt::Unsupported(const std::string& inCtCommand)
	{
		for (const std::string checked_command : m_CtCommandChecked)
			if (inCtCommand == checked_command) return;

		m_SF2->GetCout() << "WARNING: CT command " << inCtCommand << " is not supported by the SF2 driver." << std::endl;
		m_CtCommandChecked.push_back(inCtCommand);
	}

	/**
	 * Add the command used to a roster to be output in a report at the end.
	 */
	void SourceCt::AddToCommandRoster(unsigned char inCtCommand)
	{
		std::map <unsigned char, std::string> type;

		type[0x40] = "4x_Pulse";
		type[0x60] = "6x_Filter";
		type[0x80] = "8x_Chord";
		type[0xa0] = "Ax_Att";
		type[0xb0] = "Bx_Dec";
		type[0xc0] = "Cx_Sus";
		type[0xd0] = "Dx_Rel";
		type[0xe0] = "Ex_Vol";
		type[0xf0] = "Fx_Speed";

		std::stringstream stream;
		if (inCtCommand <= 8)
			stream << "Cmd_" << std::uppercase << std::hex << (int)inCtCommand;
		else
			stream << "Seq_" << type[inCtCommand];

		for (const std::string command_used : m_CtCommandsUsed)
			if (stream.str() == command_used) return;

		m_CtCommandsUsed.push_back(stream.str());
	}

	/**
	 * Transform a CC pulse program into a SF2 cluster and add it.
	 */
	unsigned char SourceCt::HandlePulseProgram(unsigned char inIndex)
	{
		unsigned char cc_to_sf2_index_map[0x40], cc_table_row = inIndex, sf2_table_row = 0;
		unsigned char raw_duration, pbyte_duration, pbyte_amount, pbyte_width_or_cont, pbyte_index_or_stop;

		m_Cluster.clear();

		while (true)
		{
			pbyte_duration		= m_PulseTable[cc_table_row][0];	// 00-7F = duration up, 80-FF = duration down
			pbyte_amount		= m_PulseTable[cc_table_row][1];	// 00-FF = amount to +/-
			pbyte_width_or_cont	= m_PulseTable[cc_table_row][2];	// 00-FE = set pulse width (48 = 8400) or FF = continue
			pbyte_index_or_stop	= m_PulseTable[cc_table_row][3];	// 00-3F = pointer to next index, or 7F = stop

			/*m_SF2->GetCout() << "CC pulse row " << std::setfill('0') << std::setw(2) << std::hex << (int)cc_table_row << ": "
				<< std::setw(2) << std::hex << (int)pbyte_duration << " "
				<< std::setw(2) << std::hex << (int)pbyte_amount << " "
				<< std::setw(2) << std::hex << (int)pbyte_width_or_cont << " "
				<< std::setw(2) << std::hex << (int)pbyte_index_or_stop << std::endl;*/

			raw_duration = pbyte_duration & 0x7f;					// Pure duration regardless of direction bit

			if (pbyte_width_or_cont != 0xff)
			{
				// A specific pulse width is set
				m_Cluster.push_back({
					(unsigned char)(0x80 + (pbyte_width_or_cont & 0x0f)),	// SF2 command 0x80 plus CC high pulse width nibble
					(unsigned char)(pbyte_width_or_cont & 0xf0),			// CC low pulse width nibble (placed in high nibble)
					(unsigned char)(pbyte_amount ? 0x00 : raw_duration)		// If no +/- then the duration can be integrated
				});
				cc_to_sf2_index_map[cc_table_row] = sf2_table_row++;

				if (pbyte_amount)
				{
					// The pulse width is also +/- so we have to add another SF2 row
					m_Cluster.push_back({
						(unsigned char)(pbyte_duration & 0x80 ? 0x0f : 0x00),
						(unsigned char)(pbyte_duration & 0x80 ? -pbyte_amount : pbyte_amount),
						(unsigned char)(raw_duration ? raw_duration - 1 : 0x00)		// Duration minus one because of the appending above
					});
					sf2_table_row++;
				}
			}
			else
			{
				// A pulse width is not set, but add duration and +/-
				m_Cluster.push_back({
					(unsigned char)(pbyte_duration & 0x80 ? 0x0f : 0x00),
					(unsigned char)(pbyte_duration & 0x80 ? -pbyte_amount : pbyte_amount),
					raw_duration
				});
				cc_to_sf2_index_map[cc_table_row] = sf2_table_row++;
			}

			if (pbyte_index_or_stop != 0x7f)
			{
				// Is the index just pointing to the next row below it?
				// NOTE: An index of 0x00 almost mean that it should go to next row.
				if (pbyte_index_or_stop && pbyte_index_or_stop != cc_table_row + 1)
				{
					// Is it jumping somewhere else inside the scope of the pulse program?
					if (pbyte_index_or_stop >= inIndex && pbyte_index_or_stop <= cc_table_row)
						m_JumpIndex = cc_to_sf2_index_map[pbyte_index_or_stop];
					else
					{
						if (!m_WarningShownPulseScope)
						{
							m_SF2->GetCout() << "WARNING: One or more pulse programs jumps outside of its own cluster scope; the SF2 equivalent is set to stop instead." << std::endl;
							m_WarningShownPulseScope = true;
						}
						m_JumpIndex = sf2_table_row; // Just loop to self thereby ending the pulse program
					}
					m_Cluster.push_back({ 0x7f, 0x00, m_JumpIndex });
					break;
				}
				cc_table_row++; // Process the next row
			}
			else
			{
				// End row so add a SF2 wrap that loops to self
				m_Cluster.push_back({ 0x7f, 0x00, sf2_table_row });
				break;
			}
		}
		// This automatically reuses any matching cluster
		return m_SF2->AppendClusterToTable(TABLE_PULSE, m_Cluster);
	}

	/**
	 * Transform a CC filter program into a SF2 cluster and add it.
	 */
	unsigned char SourceCt::HandleFilterProgram(unsigned char inIndex)
	{
		unsigned char cc_to_sf2_index_map[0x40], cc_table_row = inIndex, sf2_table_row = 0,
			last_filter_cutoff = 0x00, last_filter_passband = 0x00, last_filter_rb = 0x00;
		unsigned char filter_duration, filter_cutoff, filter_passband, filter_rb,
			fbyte_pb_or_duration, fbyte_rb_or_amount, fbyte_cutoff_or_cont, fbyte_index_or_stop;
		unsigned short amount;

		m_Cluster.clear();

		while (true)
		{
			fbyte_pb_or_duration	= m_FilterTable[cc_table_row][0];	// 00-7F = duration, or 90-F0 = filter passband
			fbyte_rb_or_amount		= m_FilterTable[cc_table_row][1];	// 00-F7 = resonance and channel bitmask if filter passband set, or 01 = +1, FF = -1, etc.
			fbyte_cutoff_or_cont	= m_FilterTable[cc_table_row][2];	// 00-FE = set filter cutoff, or FF = continue
			fbyte_index_or_stop		= m_FilterTable[cc_table_row][3];	// pointer to next index or 7F = stop

			/*m_SF2->GetCout() << "CC filter row " << std::setfill('0') << std::setw(2) << std::hex << (int)cc_table_row << ": "
				<< std::setw(2) << std::hex << (int)fbyte_pb_or_duration << " "
				<< std::setw(2) << std::hex << (int)fbyte_rb_or_amount << " "
				<< std::setw(2) << std::hex << (int)fbyte_cutoff_or_cont << " "
				<< std::setw(2) << std::hex << (int)fbyte_index_or_stop << std::endl;*/

				// We need to always be prepared regarding all filter settings
			filter_passband = last_filter_passband	= fbyte_pb_or_duration <= 0x7f ? last_filter_passband : fbyte_pb_or_duration;
			filter_cutoff = last_filter_cutoff		= fbyte_cutoff_or_cont == 0xff ? last_filter_cutoff : fbyte_cutoff_or_cont;
			filter_rb = last_filter_rb				= fbyte_pb_or_duration <= 0x7f ? last_filter_rb : fbyte_rb_or_amount;

			if (fbyte_pb_or_duration > 0x7f)
			{
				// A filter passband, resonance and channel bitmask is set in CC
				m_Cluster.push_back({
					(unsigned char)((filter_passband & 0xf0) + (filter_cutoff >> 4)),	// SF2 passband nibble plus CC high filter cutoff nibble
					(unsigned char)(filter_cutoff << 4),								// CC low filter cutoff nibble (placed in high nibble)
					filter_rb															// Resonance and channel bitmask
				});
				cc_to_sf2_index_map[cc_table_row] = sf2_table_row++;
			}
			else
			{
				filter_duration = fbyte_pb_or_duration;

				if (fbyte_cutoff_or_cont != 0xff)
				{
					// If the filter cutoff is set in CC we need to set the whole gamut in SF2
					m_Cluster.push_back({
						(unsigned char)((filter_passband & 0xf0) + (filter_cutoff >> 4)),
						(unsigned char)(filter_cutoff << 4),
						filter_rb
					});
					cc_to_sf2_index_map[cc_table_row] = sf2_table_row++;
					// Setting filter control in SF2 takes one frame
					filter_duration ? filter_duration-- : 0x00;
				}

				// A filter passband is not set, but add duration and +/-
				// NOTE: SF2 +/- is four times finer than in CC.
				amount = fbyte_rb_or_amount > 0x7f ? -((fbyte_rb_or_amount ^ 0xff) << 2) : fbyte_rb_or_amount << 2;
				m_Cluster.push_back({
					(unsigned char)((amount >> 8) & 0x0f),
					(unsigned char)(amount & 0x00ff),
					filter_duration
				});
				cc_to_sf2_index_map[cc_table_row] = sf2_table_row++;
			}

			if (fbyte_index_or_stop != 0x7f)
			{
				// Is the index just pointing to the next row below it?
				// NOTE: An index of 0x00 almost mean that it should go to next row.
				if (fbyte_index_or_stop && fbyte_index_or_stop != cc_table_row + 1)
				{
					// Is it jumping somewhere else inside the scope of the filter program?
					if (fbyte_index_or_stop >= inIndex && fbyte_index_or_stop <= cc_table_row)
						m_JumpIndex = cc_to_sf2_index_map[fbyte_index_or_stop];
					else
					{
						if (!m_WarningShownFilterScope)
						{
							m_SF2->GetCout() << "WARNING: One or more filter programs jumps outside of its own cluster scope; the SF2 equivalent is set to stop instead." << std::endl;
							m_WarningShownFilterScope = true;
						}
						m_JumpIndex = sf2_table_row; // Just loop to self thereby ending the filter program
					}
					m_Cluster.push_back({ 0x7f, 0x00, m_JumpIndex });
					break;
				}
				cc_table_row++; // Process the next row
			}
			else
			{
				// End row so add a SF2 wrap that loops to self
				m_Cluster.push_back({ 0x7f, 0x00, sf2_table_row });
				break;
			}
		}
		// This automatically reuses any matching cluster
		return m_SF2->AppendClusterToTable(TABLE_FILTER, m_Cluster);
	}

	/**
	 * Query if conversions is possible 
	 */

	bool SourceCt::CanConvert() const
	{
		return m_UnpackingSucceeded;
	}

	/**
	 * Convert CT (CheeseCutter) to SF2 format.
	 */
	bool SourceCt::Convert(int inTranspose)
	{
		// CT version 
		m_CtVersion = m_RawData[0x10000];
		if (m_CtVersion < 6 || m_CtVersion > 128)
		{
			m_SF2->GetCout() << "\nERROR: " << (m_CtVersion < 6 ? "The version of this CT file is too old." : "This CT file appears to be a 2SID version which is currently not supported.");
			return false;
		}

		// General CC warning
		m_SF2->GetCout() << "NOTE: If there are multiple sub tunes, the one that was last in focus in CC will be converted." << std::endl;

		if (m_RawData[0x10002] != 0x01)
			m_SF2->GetCout() << "WARNING: This multispeed CT file will be converted but it will only play at 1x speed in SF2." << std::endl;

		unsigned char tempo_program_index = 0, tempo_byte;
		unsigned short address_chord_table = m_RawData[0x0fdc] | m_RawData[0x0fdd] << 8,
			address_main_volume = m_RawData[0x0fa2] | m_RawData[0x0fa3] << 8;
				
		// Single CC speed (set even if there is a tempo program that overrides it)
		m_SF2->EditTableRow(TABLE_TEMPO, 0, { m_RawData[0x10005] });

		if (m_RawData[0x10005] < 2)
		{
			// A tempo program is used
			m_Cluster.clear();
			for (int pos = 0; pos < 0x100; pos++)
			{
				tempo_byte = m_RawData[address_chord_table + pos];
				if (tempo_byte & 0x80)
				{
					m_Cluster.push_back({ 0x7f });
					break;
				}
				else
					m_Cluster.push_back({ tempo_byte });
			}
			tempo_program_index = m_SF2->AppendClusterToTable(TABLE_TEMPO, m_Cluster);

			if (tempo_program_index == 0xff)
				return false;

			// Now point the init entry to this tempo program and also set any custom volume
			m_SF2->EditTableRow(TABLE_INIT, 0, { tempo_program_index, m_RawData[address_main_volume] });
		}

#ifdef DEBUG_OUTPUT
		m_Output->HexTable(TABLE_TEMPO);
		m_Output->HexTable(TABLE_INIT);
#endif // DEBUG_OUTPUT

		// Set SID model according to CC source tune
		m_SF2->SetSIDModel(m_RawData[0x10003] ? 8580 : 6581);

		// Command #00 is never used (reserved for HR in CC)
		m_SF2->EditTableRowText(TABLE_CMDS, 0, "CC to SF2");

		/* ---- ORDER LISTS ---- */

		unsigned char previous_transpose = 0xa0, transpose;
		unsigned short address_orderlist;
		for (int channel = 0; channel < 3; channel++)
		{
			address_orderlist = m_RawData[0x0fc6 + channel * 2] | m_RawData[0x0fc7 + channel * 2] << 8;

			for (int pair = 0;;pair += 2)
			{
				transpose = m_RawData[address_orderlist + pair];
				if (transpose == 0xf0) break;
				transpose == 0x80 ? transpose = previous_transpose : previous_transpose = transpose;

				m_SF2->AppendToOrderList(channel, { transpose, m_RawData[address_orderlist + pair + 1] });
			}
		}

#ifdef DEBUG_OUTPUT
		m_SF2->GetCout() << std::endl;
		m_Output->HexOrderList(0);
		m_Output->HexOrderList(1);
		m_Output->HexOrderList(2);
#endif // DEBUG_OUTPUT

		/* ---- COMMAND TABLE ---- */

		unsigned char command, left, right;
		unsigned short speed, address_command_table = m_RawData[0x0fd0] | m_RawData[0x0fd1] << 8;

		// NOTE: This table is a close match but not perfect.
		unsigned char vib_conversion[256] = {
			7, 6, 5, 5, 4, 4, 4, 3, 3, 3, 2, 2, 2, 2, 2, 2,
			2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0 }; // Used for lo-fi vibrato below

		for (int pos = 0; pos < 0x40; pos++)
		{
			command	= m_RawData[address_command_table + pos];
			left	= m_RawData[address_command_table + 0x40 + pos];
			right	= m_RawData[address_command_table + 0x80 + pos];

			if (pos == 0)
				// Special HR case (usually set to 0x0f 0x00 just like in SF2)
				m_SF2->EditTableRow(TABLE_HR, 0, { left, right });
			else if (left || right)
			{
				switch (command)
				{
				case 0: // Slide up (XXXX = speed)
					m_SF2->EditTableRow(TABLE_CMDS, pos, { SF2::Interface::Cmd_Slide, left, right });
					m_SF2->EditTableRowText(TABLE_CMDS, pos, "Slide up");
					break;

				case 1: // Slide down (XXXX = speed)
					speed = -(left * 256 + right);
					m_SF2->EditTableRow(TABLE_CMDS, pos, { SF2::Interface::Cmd_Slide, (unsigned char)(speed >> 8), (unsigned char)(speed & 0x00ff) });
					m_SF2->EditTableRowText(TABLE_CMDS, pos, "Slide down");
					break;

				case 2: // Hi-fi vibrato (0X = feel, Y = speed, Z = depth)
					if (left) Unsupported("4 in table (Hi-fi vibrato) has a feeling value that");
					// The Hi-fi vibrato depth in CC is upside down just like in SF2
					m_SF2->EditTableRow(TABLE_CMDS, pos, { SF2::Interface::Cmd_Vibrato, (unsigned char)(right >> 4), (unsigned char)(right & 0x0f) });
					m_SF2->EditTableRowText(TABLE_CMDS, pos, "Vibrato HIFI");
					break;

				case 3: // Detune(XXXX = amount)
					Unsupported("3 in table (Detune)");
					break;

				case 4: // Set ADSR
					m_SF2->EditTableRow(TABLE_CMDS, pos, { SF2::Interface::Cmd_ADSR_Note, left, right });
					m_SF2->EditTableRowText(TABLE_CMDS, pos, "ADSR local");
					break;

				case 5: // Lo-fi vibrato (XX = speed, YY = depth)

					m_SF2->EditTableRow(TABLE_CMDS, pos, { SF2::Interface::Cmd_Vibrato, (unsigned char)(--left), vib_conversion[right] });
					m_SF2->EditTableRowText(TABLE_CMDS, pos, "Vibrato LOFI");
					break;

				case 6: // Set waveform (XX = waveform)
					Unsupported("6 in table (Waveform)");
					break;

				case 7: // Portamento (XXXX = speed)
					m_SF2->EditTableRow(TABLE_CMDS, pos, { SF2::Interface::Cmd_Portamento, left, right });
					m_SF2->EditTableRowText(TABLE_CMDS, pos, "Portamento");
					break;

				case 8: // Portamento off
					m_SF2->EditTableRow(TABLE_CMDS, pos, { SF2::Interface::Cmd_Portamento, 0x80, 0x00 });
					m_SF2->EditTableRowText(TABLE_CMDS, pos, "Stop effect");
					break;

				default:
					break;
				}
				AddToCommandRoster(command);
			}
		}

#ifdef DEBUG_OUTPUT
		m_Output->HexTable(TABLE_HR);
		m_Output->HexTable(TABLE_CMDS);
#endif // DEBUG_OUTPUT

		/* ---- PULSE TABLE ---- */

		unsigned short address_pulse_table = m_RawData[0x0fc2] | m_RawData[0x0fc3] << 8;

		// We cannot transfer the pulse table as is (too much action per row) so it is parked in an
		// easy-to-access array for easier retrieval when referred to from instruments and sequence
		// commands later. A set of rows (usually larger) in SF2 then have to be built up.
		for (int row = 0; row < 0x40; row++)
			for (int col = 0; col < 4; col++)
				m_PulseTable[row][col] = m_RawData[address_pulse_table + (row * 4) + col];

		/* ---- FILTER TABLE ---- */

		unsigned short address_filter_table = m_RawData[0x0fc0] | m_RawData[0x0fc1] << 8;

		// Same challenge as with the pulse table above
		for (int row = 0; row < 0x40; row++)
			for (int col = 0; col < 4; col++)
				m_FilterTable[row][col] = m_RawData[address_filter_table + (row * 4) + col];

		/* ---- INSTRUMENT TABLE ---- */

		bool has_data;
		unsigned char sf2_pulse_cluster_index;
		unsigned short address_instr_table = m_RawData[0x0fc4] | m_RawData[0x0fc5] << 8;

		for (int row = 0; row < 0x30; row++)
		{
			// Collect columnized instrument data
			has_data = false;
			for (int col = 0; col < 8; col++)
			{
				m_Instrument[col] = m_RawData[address_instr_table + (col * 0x30) + row];
				if (m_Instrument[col]) has_data = true;
			}

			if (!has_data) continue;
			else if (row > 0x1f)
			{
				m_SF2->GetCout() << "\nERROR: Song is too complex; exceeded the 32 available instruments for SID Factory II.";
				return false;
			}

			// Pulse
			if (m_Instrument[5] < 0x80)
				sf2_pulse_cluster_index = HandlePulseProgram(m_Instrument[5]);
			else if (m_Instrument[5] >= 0x80)
				// A simple pulse high nibble is just set
				sf2_pulse_cluster_index = m_SF2->AppendClusterToTable(TABLE_PULSE, {
					{ (unsigned char)(0x80 + (m_Instrument[5] & 0x0f)), 0x00, 0x00 },	// Set to the high pulse width nibble
					{ 0x7f, 0x00, 0x01 },												// Wrap to itself
				});

			if (sf2_pulse_cluster_index == 0xff)
				return false;

			// Create the SF2 instrument
			m_SF2->EditTableRow(TABLE_INSTR, row, {
					m_Instrument[0],											// AD
					m_Instrument[1],											// SR
					(unsigned char)((m_Instrument[2] & 0xf0 ? 0x80 : 0x00) |	// Flag: HR
						(m_Instrument[4] ? 0x40 : 0x00) |						// Flag: Filter
						(m_Instrument[3] == 0x09 ? 0x10 : 0x00)),				// Flag: Osc reset
					HandleFilterProgram(m_Instrument[4]),						// Filter table index
					sf2_pulse_cluster_index,									// Pulse table index
					m_Instrument[7]												// Wave table index
				});
			// And its description
			std::string description(&m_RawData[0x101a5 + (row * 32)], &m_RawData[0x101a5 + (row * 32)] + 32);
			m_SF2->EditTableRowText(TABLE_INSTR, row, description);
		}

#ifdef DEBUG_OUTPUT
		m_Output->HexTable(TABLE_INSTR);
		m_Output->HexTable(TABLE_PULSE);
		m_Output->HexTable(TABLE_FILTER);
#endif // DEBUG_OUTPUT

		/* ---- WAVE TABLE ---- */

		unsigned char last_right, cluster_start = 0x00; // Used to make the wrap values relative
		unsigned short address_wave_left = m_RawData[0x0fbc] | m_RawData[0x0fbd] << 8,
			address_wave_right = m_RawData[0x0fbe] | m_RawData[0x0fbf] << 8;
		std::vector<unsigned char> wave_pair;
		
		for (int row = 0; row <= 0xff; row++)
		{
			left = m_RawData[address_wave_left + row];
			right = m_RawData[address_wave_right + row];

			if (left == 0x7e)
			{
				wave_pair = { 0x7f, (unsigned char)(row - cluster_start - 1) };	// Emulate stop by looping to last row
				cluster_start = row + 1;
			}
			else if (left == 0x7f)
			{
				wave_pair = { 0x7f, (unsigned char)(right - cluster_start) };	// Loop to specific index
				cluster_start = row + 1;
			}
			else
			{
				if (right == 0x00)
					wave_pair = { last_right, left };							// Continuing with the same waveform
				else if (right >= 0x01 && right <= 0x0f)
					wave_pair = { (unsigned char)(0xe0 + right), 0x00 };		// Waveform being repeated X times (which is what the delay does)
				else if (right >= 0xe0 && right <= 0xef)
				{
					wave_pair = { (unsigned char)(right & 0x0f), left };		// Waveform $00-$0F
					last_right = right & 0x0f;
				}
				else
				{
					wave_pair = { right, left };								// It's not me being left/right-blind; they are swapped in CC!
					last_right = right;
				}
			}
			m_SF2->EditTableRow(TABLE_WAVE, row, wave_pair);
		}

		unsigned char repeat;
		std::vector<unsigned char> last_bytes = { 0x01, 0x00 }, wave_bytes, instr_bytes;

		// Expand all the delay commands into repeated waveforms
		// NOTE: There is one corner that I decided to cut here; loop index that points to somewhere BELOW
		// a wave cluster being expanded will not be updated. Users will have to update this themselves.
		for (int row = 0; row <= 0xff; row++)
		{
			wave_bytes = m_SF2->ReadTableRow(TABLE_WAVE, row);

			// Our own repeat command E0-EF has been detected
			if (wave_bytes[0] >= 0xe0 && wave_bytes[0] <= 0xef)
			{
				repeat = (wave_bytes[0] & 0x0f) - 1;

				if (row + repeat > 0xff)
				{
					m_SF2->GetCout() << "\nERROR: Out of wave table memory in SID Factory II while trying to expand wave delay commands.";
					return false;
				}

				// Move the wave table to make room (slow but easier to manage and speed is not an issue)
				// @todo If you want pretty or fast you can always make a genuine move method later.
				for (int pos = 0xff; pos >= row + repeat; pos--)
					m_SF2->EditTableRow(TABLE_WAVE, pos, m_SF2->ReadTableRow(TABLE_WAVE, pos - repeat));

				// Now fill in the blanks
				for (int i = 0; i < repeat + 1; i++)
					m_SF2->EditTableRow(TABLE_WAVE, row + i, last_bytes);

				// Update wave table index in the affected instruments (also a slow way to do this)
				for (int instr = 0; instr < 0x1f; instr++)
				{
					instr_bytes = m_SF2->ReadTableRow(TABLE_INSTR, instr);
					if (instr_bytes[5] >= row)
					{
						instr_bytes[5] += repeat;
						m_SF2->EditTableRow(TABLE_INSTR, instr, instr_bytes);
					}
				}
			}
			last_bytes = wave_bytes;
		}

#ifdef DEBUG_OUTPUT
		m_Output->HexTable(TABLE_WAVE);
#endif // DEBUG_OUTPUT

		/* ---- SEQUENCES ---- */

		std::string hex = "0123456789ABCDEF", arp_desc;
		unsigned char current_instrument, rest, chord_byte, chord_speed, value, index,
			sbyte_instrument, sbyte_tienote, sbyte_note, sbyte_command;
		unsigned short seq_start, seq_pos, address_s00 = m_RawData[0x0fd2] | m_RawData[0x0fd3] << 8,
			address_chord_index = m_RawData[0x0fe0] | m_RawData[0x0fe1] << 8;

		for (int seq = 0; seq <= 0x7f; seq++)
		{
			seq_start = address_s00 + (seq * 256);
			current_instrument = 0xff;
			rest = 0x00;

			if ( // Unused sequence?
				m_RawData[seq_start]	 == 0xf0 &&
				m_RawData[seq_start + 1] == 0xf0 &&
				m_RawData[seq_start + 2] == 0x60 &&
				m_RawData[seq_start + 3] == 0x00 &&
				m_RawData[seq_start + 4] == 0xbf) continue;

			for (int row = 0; row < 0x40; row++)
			{
				seq_pos = seq_start + (row * 4);

				sbyte_instrument	= m_RawData[seq_pos];
				sbyte_tienote		= m_RawData[seq_pos + 1];
				sbyte_note			= m_RawData[seq_pos + 2];
				sbyte_command		= m_RawData[seq_pos + 3];

				// If there is more than what I have researched I want to know about it
				FOUNDATION_ASSERT(sbyte_instrument >= 0xbf && sbyte_instrument <= 0xf0);
				FOUNDATION_ASSERT(sbyte_tienote == 0x5f || sbyte_tienote == 0xf0 || (sbyte_instrument == 0xbf && sbyte_tienote == 0x00));
				FOUNDATION_ASSERT(sbyte_note >= 0x60 && sbyte_note <= 0xbe || (sbyte_instrument == 0xbf && sbyte_note == 0x00));

				// Instrument
				if (sbyte_instrument >= 0xc0 && sbyte_instrument <= 0xef) // Set instrument
					sbyte_instrument = (sbyte_instrument & 0x1f) == current_instrument ? 0x80 : current_instrument = sbyte_instrument & 0x1f;
				else if (sbyte_instrument == 0xbf) break;	// End of sequence
				else sbyte_instrument = 0x80;				// Nothing

				if (sbyte_tienote == 0x5f)					// Tie note (overrides instrument in SF2)
					sbyte_instrument = 0x90;

				// Note
				if (sbyte_note == 0x60)						// Nothing (set whatever is currently relevant - note on or off)
					sbyte_note = rest;
				else if (sbyte_note == 0x61)				// Gate off - and rest is now gate off too
					sbyte_note = rest = 0x00;
				else if (sbyte_note == 0x62)				// Gate on - and rest is now gate on too
					sbyte_note = rest = 0x7e;
				else
				{
					sbyte_note -= 0x60;						// CC notes start at 0x60, SF2 notes start at 0x00
					rest = 0x7e;
				}

				// Command
				if (sbyte_command == 0x00)					// Nothing
					sbyte_command = 0x80;
				else if (sbyte_command < 0x40)				// Set table command (01-3F)
					sbyte_command &= 0x3f;
				else if (sbyte_command >= 0x40 && sbyte_command <= 0x5f)
				{
					// Pulse table index (00-1F) - will be set as a table command in SF2
					index = HandlePulseProgram(sbyte_command & 0x1f);
					if (index == 0xff)
						return false;
					sbyte_command = m_SF2->AppendToTable(TABLE_CMDS, { SF2::Interface::Cmd_Index_Pulse, 0x00, index });
					if (sbyte_command == 0xff)
						return false;

					m_SF2->EditTableRowText(TABLE_CMDS, sbyte_command, "P-index SEQ");
					AddToCommandRoster(0x40);
				}
				else if (sbyte_command >= 0x60 && sbyte_command <= 0x7f)
				{
					// Filter table index (00-1F) - will be set as a table command in SF2
					index = HandleFilterProgram(sbyte_command & 0x1f);
					sbyte_command = m_SF2->AppendToTable(TABLE_CMDS, { SF2::Interface::Cmd_Index_Filter, 0x00, index });
					if (sbyte_command == 0xff)
						return false;

					m_SF2->EditTableRowText(TABLE_CMDS, sbyte_command, "F-index SEQ");
					AddToCommandRoster(0x60);
				}
				else if (sbyte_command >= 0x80 && sbyte_command <= 0x9f)
				{
					// Chord table index (cluster index 0-F) - will be set as a table command in SF2
					index = m_RawData[address_chord_index + (sbyte_command - 0x80)];
					arp_desc = "Arp ";
					m_Cluster.clear();

					for (int pos = 0; pos < 0x100 ;pos++)
					{
						chord_byte = m_RawData[address_chord_table + index + pos];
						if (chord_byte >= 0x80)
						{
							// Wrap pointer is absolute in CC but relative in SF2
							m_Cluster.push_back({ (unsigned char)(0x70 + ((chord_byte & 0x7f) - index)) });
							break;
						}
						else if (chord_byte >= 0x40)
						{
							// Negative chord offsets are not supported by the SF2 driver
							Unsupported(std::to_string(sbyte_command) + " uses a chord with a negative offset which");
							arp_desc.append("x");
							m_Cluster.push_back({ 0x00 }); // Just set it to 0
						}
						else
						{
							arp_desc.append(chord_byte > 16 ? "#" : std::string(1, hex[chord_byte]));
							m_Cluster.push_back({ chord_byte });
						}
					}
					// Add chord cluster in SF2 arpeggio table
					index = m_SF2->AppendClusterToTable(TABLE_ARP, m_Cluster);
					if (index == 0xff)
						return false;

					// Add command with arpeggio speed defined in the current instrument
					chord_speed = m_RawData[address_instr_table + (2 * 0x30) + current_instrument] & 0x0f;
					sbyte_command = m_SF2->AppendToTable(TABLE_CMDS, { 0x03, chord_speed, index });
					if (sbyte_command == 0xff)
						return false;

					if (chord_speed) arp_desc.append(" slow");
					m_SF2->EditTableRowText(TABLE_CMDS, sbyte_command, arp_desc);
					AddToCommandRoster(0x80);
				}
				else if (sbyte_command >= 0xa0 && sbyte_command <= 0xaf)
				{
					// Attack (0-F)
					Unsupported("A0-AF (Attack only set in sequence)");
					sbyte_command = 0x80;
					AddToCommandRoster(0xa0);
				}
				else if (sbyte_command >= 0xb0 && sbyte_command <= 0xbf)
				{
					// Decay (0-F)
					Unsupported("B0-BF (Decay only set in sequence)");
					sbyte_command = 0x80;
					AddToCommandRoster(0xb0);
				}
				else if (sbyte_command >= 0xc0 && sbyte_command <= 0xcf)
				{
					// Sustain (0-F)
					Unsupported("C0-CF (Sustain only set in sequence)");
					sbyte_command = 0x80;
					AddToCommandRoster(0xc0);
				}
				else if (sbyte_command >= 0xd0 && sbyte_command <= 0xdf)
				{
					// Release (0-F)
					Unsupported("D0-DF (Release only set in sequence)");
					sbyte_command = 0x80;
					AddToCommandRoster(0xd0);
				}
				else if (sbyte_command >= 0xe0 && sbyte_command <= 0xef)
				{
					// Main volume (0-F)
					sbyte_command = m_SF2->AppendToTable(TABLE_CMDS, { SF2::Interface::Cmd_Volume, 0x00, (unsigned char)(sbyte_command & 0x0f) });
					if (sbyte_command == 0xff)
						return false;

					m_SF2->EditTableRowText(TABLE_CMDS, sbyte_command, "Volume SEQ");

					AddToCommandRoster(0xe0);
				}
				else if (sbyte_command >= 0xf0)
				{
					// Song speed (0-F; 0 or 1 is swing tempo)
					value = sbyte_command & 0x0f;
					if (value <= 1)
						index = tempo_program_index; // @todo If a tempo program was not initially added this won't work
					else
						index = m_SF2->AppendClusterToTable(TABLE_TEMPO,
							{
								{ value },
								{ 0x7f },
							});
					if (index == 0xff)
						return false;

					sbyte_command = m_SF2->AppendToTable(TABLE_CMDS, { SF2::Interface::Cmd_Tempo, 0x00, index });
					if (sbyte_command == 0xff)
						return false;

					m_SF2->EditTableRowText(TABLE_CMDS, sbyte_command, "Tempo SEQ");

					AddToCommandRoster(0xf0);
				}

				m_SF2->AppendToSequence(seq, { sbyte_instrument, sbyte_command, sbyte_note });
			}
#ifdef DEBUG_SEQ
			m_Output->HexSequence(seq, true);
#endif // DEBUG_SEQ
		}

#ifdef DEBUG_OUTPUT
		m_Output->HexTable(TABLE_ARP);
#endif // DEBUG_OUTPUT

		/* ---- DONE ---- */

		// Compile the roster of commands used
		std::sort(m_CtCommandsUsed.begin(), m_CtCommandsUsed.end());
		std::string commands_used;
		for (std::string command : m_CtCommandsUsed)
			commands_used += command + " ";

		m_SF2->GetCout() << std::endl
			<< "  CC commands used:        " << commands_used << "\n"
			<< "  SF2 commands allocated:  " << m_SF2->GetCount(TABLE_CMDS) << std::endl;

		bool data_pushed = m_SF2->PushAllDataToMemory(true);
		if (!data_pushed)
		{
			m_SF2->GetCout() << "\nERROR: Could not pack the data before saving it.";
			return false;
		}

		return true;
	}
}
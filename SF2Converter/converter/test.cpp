#include <iostream>

#include "test.h"

namespace Converter
{
	Test::Test(SF2::Interface* inInterface, Misc* inOutput)
		: m_SF2(inInterface)
		, m_Output(inOutput)
	{
	}

	Test::~Test()
	{
	}

	void Test::TestAll()
	{
        std::cout << "TESTING CLASS...\n\n";

        // Output driver details
		m_Output->HexSF2Info();

		// Output track 0 order list
        std::cout << "  Should be vanilla - a0 00 ff 00 - (untouched)\n /" << std::endl;
		m_Output->HexOrderList(0);

		// Append to track 0 order list then output it again
        std::cout << "  Should be - 94 05 ff 00\n /" << std::endl;
		m_SF2->AppendToOrderList(0, { 0x94, 0x05 });
		m_Output->HexOrderList(0);

        std::cout << "  Should be - 94 05 aa 07 ff 00\n /" << std::endl;
		m_SF2->AppendToOrderList(0, { 0xaa, 0x07 });
		m_Output->HexOrderList(0);

        std::cout << "  Should be - 94 05 aa 07 aa 12 ff 00\n /" << std::endl;
		m_SF2->AppendToOrderList(0, { 0xaa, 0x12 });
		m_Output->HexOrderList(0);

		// Now append to track 1 order list
        std::cout << "  Should be - 90 24 ff 00\n /" << std::endl;
		m_SF2->AppendToOrderList(1, { 0x90, 0x24 });
		m_Output->HexOrderList(1);

        // And to track 2 order list testing an initial chain of a0 00
        std::cout << "  Should be - a0 00 a0 00 a0 00 ac 15 ff 00\n /" << std::endl;
        m_SF2->AppendToOrderList(2, { 0xa0, 0x00 });
        m_SF2->AppendToOrderList(2, { 0xa0, 0x00 });
        m_SF2->AppendToOrderList(2, { 0xa0, 0x00 });
        m_SF2->AppendToOrderList(2, { 0xac, 0x15 });
        m_Output->HexOrderList(2);

        //unsigned char* block = sf2.GetAllMemory();
        //m_Output->HexBlock(range.m_StartAddress, block, range.m_EndAddress - range.m_StartAddress);
        //m_Output->HexBlock(0x0d00, block, 0x9b00);
        //m_Output->HexBlock(0x2300, block, 0x0300);

        bool data_pushed = m_SF2->PushAllDataToMemory(true);
        if (!data_pushed)
        {
            std::cerr << "\nERROR: Could not push data to emulated C64 memory.";
            exit(0);
        }

        // Output track 0 order list block from C64 memory
        std::cout << "  Should be - 94 05 aa 07 12 ff - in the top of a block of 00 bytes \n /" << std::endl;
        m_Output->HexOrderListInMemory(0);
        m_Output->HexOrderListInMemory(1);
        m_Output->HexOrderListInMemory(2);

        // Output sequence 0d
        std::cout << "  Should be empty (actually - 80 80 00 - but size has temporarily been set to 0)\n /" << std::endl;
        m_Output->HexSequence(0x0d, true);

        m_SF2->AppendToSequence(0x0d, { 0x01, 0x06, 0x40 }); // Instr 01  Cmd 06  Note 40 (E-5)
        m_SF2->AppendToSequence(0x0d, { 0x80, 0x80, 0x3c }); //       --      --       3c (C-5)
        m_SF2->AppendToSequence(0x0d, { 0x80, 0x80, 0x00 }); //       --      --       --
        m_SF2->AppendToSequence(0x0d, { 0x80, 0x80, 0x00 }); //       --      --       --
        m_SF2->AppendToSequence(0x0d, { 0x80, 0x80, 0x0c }); //       --      --       0c (C-1)
        m_SF2->AppendToSequence(0x0d, { 0x80, 0x07, 0x3e }); //       --      07       3e (D-5)

        m_SF2->PushAllDataToMemory(true);

        // Output sequence 0d after the update
        std::cout << "  Should now be - a1 c6 40, 80 80 3c, 80 80 00, 80 80 00, 80 80 0c, 80 c7 3e\n /" << std::endl;
        m_Output->HexSequence(0x0d, true);

        // Output sequence 0d block from C64 memory
        std::cout << "  Should be - c6 a1 80 40 3c 81 00 80 0c c7 3e 7f - in the top of a block of 00 bytes (note how durations are now adapted)\n /" << std::endl;
        m_Output->HexSequenceInMemory(0x0d);
        std::cout << "  Should be vanilla - 80 00 7f - (untouched)\n /" << std::endl;
        m_Output->HexSequenceInMemory(0x0e);

        m_SF2->EditTableRow(TABLE_INSTR, 2, { 0x22, 0xc9, 0x80, 0x00, 0x02, 0x1c }); // This data is reused in AppendToTable() below

        // Output the instruments table after the update
        std::cout << "  Should be 00 bytes in row 00 and 01 - 22 c9 80 00 02 1c - in row 02, and then no more rows\n /" << std::endl;
        m_Output->HexTable(TABLE_INSTR);

        m_SF2->EditTableRow(TABLE_ARP, 0, { 0x00 }); // A 0-3-7 chord
        m_SF2->EditTableRow(TABLE_ARP, 1, { 0x03 });
        m_SF2->EditTableRow(TABLE_ARP, 2, { 0x07 });
        m_SF2->EditTableRow(TABLE_ARP, 3, { 0x70 });

        // Output the arpeggio table after the update
        std::cout << "  Should be one column of four bytes - 00 03 07 70\n /" << std::endl;
        m_Output->HexTable(TABLE_ARP);

        m_SF2->PushAllDataToMemory(true);

        // Output the instruments block from the C64 memory
        std::cout << "  The third column should be six vertical bytes - 22 c9 80 00 02 1c - the rest all 00 bytes\n /" << std::endl;
        m_Output->HexTableInMemory(TABLE_INSTR);

        int row = m_SF2->AppendToTable(TABLE_INSTR, { 0x02, 0xa6, 0x80, 0x00, 0x00, 0x24 });

        std::cout << "Row returned after AppendToTable() row: " << row << " (it's new so it should be 3)\n" << std::endl;

        // Output the instruments table after the update
        std::cout << "  Should be a new - 02 a6 80 00 00 24 - in row 03, and then no more rows\n /" << std::endl;
        m_Output->HexTable(TABLE_INSTR);

        row = m_SF2->AppendToTable(TABLE_INSTR, { 0x22, 0xc9, 0x80, 0x00, 0x02, 0x1c }); // Same data as in EditTableRow(2) above

        // Output the instruments table after the update
        std::cout << "  Identical row data was added so there should be no change here (no new row 04)\n /" << std::endl;
        m_Output->HexTable(TABLE_INSTR);

        std::cout << "Row returned after AppendToTable() row: " << row << " (it's reused so it should be 2)\n" << std::endl;

        row = m_SF2->AppendToTable(TABLE_WAVE,
            {
                { 0x11, 0x24 },
                { 0x41, 0x00 },
                { 0x41, 0x0c },
                { 0x7f, 0x01 }, // Wrap pointer are always zero-based (automatically adapted)
            });

        std::cout << "Row returned after AppendToTable() cluster: " << row << " (it's new so it should be 0)" << std::endl;

        row = m_SF2->AppendToTable(TABLE_WAVE,
            {
                { 0x81, 0xba },
                { 0x11, 0xa5 },
                { 0xf0, 0x00 },
                { 0x7f, 0x02 },
            });

        std::cout << "Row returned after AppendToTable() cluster: " << row << " (it's also new so it should be 4)" << std::endl;

        row = m_SF2->AppendToTable(TABLE_WAVE,
            {
                { 0x11, 0x24 },
                { 0x41, 0x00 },
                { 0x41, 0x0c },
                { 0x7f, 0x01 },
            });

        std::cout << "Row returned after AppendToTable() cluster: " << row << " (it's reused so it should be 0)" << std::endl;

        row = m_SF2->AppendToTable(TABLE_WAVE,
            {
                { 0x81, 0xba },
                { 0x11, 0xa5 },
                { 0xf0, 0x00 },
                { 0x7f, 0x02 },
            });

        std::cout << "Row returned after AppendToTable() cluster: " << row << " (also reused so it should be 4)" << std::endl;

        // Output the wave table after the update
        std::cout << "\n  Should be 8 rows of wave data - 0x11 0x24, 0x41 0x00, 0x41 0x0c, 0x7f 0x01 - and - 0x81 0xba, 0x11 0xa5, 0xf0 0x00, 0x7f 0x02\n /" << std::endl;
        m_Output->HexTable(TABLE_WAVE);

        m_SF2->AppendToTable(TABLE_WAVE,
            {
                { 0x31, 0x00 },
                { 0x7f, 0x00 },
            });

        m_SF2->AppendToTable(TABLE_WAVE,
            {
                { 0x11, 0x18 },
                { 0x61, 0x00 },
                { 0x7f, 0x01 },
            });

        m_SF2->AppendToTable(TABLE_WAVE,
            {
                { 0x31, 0x00 },
                { 0x7f, 0x00 },
            });

        std::cout << "\n  Added three more clusters - 0x31 0x00, 0x7f 0x00 - and - 0x11 0x18, 0x61 0x00, 0x7f, 0x01 - third was equal to first so should be skipped\n /" << std::endl;
        m_Output->HexTable(TABLE_WAVE);

        m_SF2->PushAllDataToMemory(true);
        std::cout << "Pushing data..." << std::endl;

        std::cout << "\n  The wrap makers in the table should now be absolute instead of relative\n /" << std::endl;
        m_Output->HexTable(TABLE_WAVE);

        m_SF2->AppendToTable(TABLE_PULSE,
            {
                { 0x88, 0x00, 0x03 },
                { 0x00, 0x20, 0x10 },
                { 0x0f, 0xe0, 0x10 },
                { 0x7f, 0x00, 0x01 },
            });

        m_SF2->AppendToTable(TABLE_PULSE,
            {
                { 0x81, 0x00, 0x00 },
                { 0x00, 0x08, 0x30 },
                { 0x0f, 0xf8, 0x30 },
                { 0x00, 0x00, 0x00 },
                { 0x7f, 0x00, 0x03 },
            });

        m_SF2->AppendToTable(TABLE_PULSE,
            {
                { 0x81, 0x00, 0x00 },
                { 0x00, 0x08, 0x30 },
                { 0x0f, 0xf8, 0x30 },
                { 0x00, 0x00, 0x00 },
                { 0x7f, 0x00, 0x03 },
            });

        m_SF2->PushAllDataToMemory(true);

        std::cout << "  Pulse table should have 9 rows (two clusters of 4 and 5 rows each) and no more + data was pushed so last wrap marker so be absolute\n /" << std::endl;
        m_Output->HexTable(TABLE_PULSE);

        m_SF2->AppendToTable(TABLE_ARP,
            {
                {{ 0x00 }},
                {{ 0x05 }},
                {{ 0x09 }},
                {{ 0x70 }},
            });

        // Duplicate so should be skipped
        m_SF2->AppendToTable(TABLE_ARP,
            {
                {{ 0x00 }},
                {{ 0x03 }},
                {{ 0x07 }},
                {{ 0x70 }},
            });

        m_SF2->PushAllDataToMemory(true);

        std::cout << "  Arpeggio table only be two chords - 0x00 0x05 0x09 0x70 - and - 0x00 0x03 0x07 0x70 - data was pushed but wrap markers should be unaffected \n /" << std::endl;
        m_Output->HexTable(TABLE_ARP);

	}

    void Test::TestSimpleAndSave()
    {
        // Order lists - sequence 01, 02 and 03 across all tracks
        m_SF2->AppendToOrderList(0, { 0xa0, 0x01 });
        m_SF2->AppendToOrderList(1, { 0xb8, 0x02 });
        m_SF2->AppendToOrderList(2, { 0xa0, 0x03 });

        // Sequence 01 - a bass line
        m_SF2->AppendToSequence(0x01, { 0x01, 0x80, 0x0c }); // Instr 01  Cmd --  Note 0c (C-1)
        m_SF2->AppendToSequence(0x01, { 0x80, 0x80, 0x00 });
        m_SF2->AppendToSequence(0x01, { 0x80, 0x80, 0x00 });
        m_SF2->AppendToSequence(0x01, { 0x80, 0x80, 0x00 });

        m_SF2->AppendToSequence(0x01, { 0x80, 0x80, 0x0c });
        m_SF2->AppendToSequence(0x01, { 0x80, 0x80, 0x00 });
        m_SF2->AppendToSequence(0x01, { 0x80, 0x80, 0x00 });
        m_SF2->AppendToSequence(0x01, { 0x80, 0x80, 0x00 });

        m_SF2->AppendToSequence(0x01, { 0x80, 0x80, 0x18 });
        m_SF2->AppendToSequence(0x01, { 0x80, 0x80, 0x00 });
        m_SF2->AppendToSequence(0x01, { 0x80, 0x80, 0x00 });
        m_SF2->AppendToSequence(0x01, { 0x80, 0x80, 0x00 });

        m_SF2->AppendToSequence(0x01, { 0x80, 0x80, 0x0c });
        m_SF2->AppendToSequence(0x01, { 0x80, 0x80, 0x00 });
        m_SF2->AppendToSequence(0x01, { 0x80, 0x80, 0x00 });
        m_SF2->AppendToSequence(0x01, { 0x80, 0x80, 0x00 });

        m_SF2->AppendToSequence(0x01, { 0x80, 0x00, 0x16 }); // Note held (+++) and vibrato
        m_SF2->AppendToSequence(0x01, { 0x80, 0x80, 0x7e });
        m_SF2->AppendToSequence(0x01, { 0x80, 0x80, 0x7e });
        m_SF2->AppendToSequence(0x01, { 0x80, 0x80, 0x7e });

        m_SF2->AppendToSequence(0x01, { 0x80, 0x80, 0x18 });
        m_SF2->AppendToSequence(0x01, { 0x80, 0x80, 0x00 });
        m_SF2->AppendToSequence(0x01, { 0x80, 0x80, 0x00 });
        m_SF2->AppendToSequence(0x01, { 0x80, 0x80, 0x00 });

        m_SF2->AppendToSequence(0x01, { 0x80, 0x80, 0x13 });
        m_SF2->AppendToSequence(0x01, { 0x80, 0x80, 0x00 });
        m_SF2->AppendToSequence(0x01, { 0x80, 0x80, 0x00 });
        m_SF2->AppendToSequence(0x01, { 0x80, 0x80, 0x00 });

        m_SF2->AppendToSequence(0x01, { 0x80, 0x80, 0x16 });
        m_SF2->AppendToSequence(0x01, { 0x80, 0x80, 0x00 });
        m_SF2->AppendToSequence(0x01, { 0x80, 0x80, 0x00 });
        m_SF2->AppendToSequence(0x01, { 0x80, 0x80, 0x00 });

        // Sequence 02 - chords
        for (int i = 0; i < 4; i++)
        {
            m_SF2->AppendToSequence(0x02, { 0x02, 0x01, 0x24 }); // Instr 02  Cmd 01  Note 24 (C-3)
            for (int j = 0; j < 7; j++)
                m_SF2->AppendToSequence(0x02, { 0x80, 0x80, 0x00 });
        }

        // Sequence 03 - modulation
        m_SF2->AppendToSequence(0x03, { 0x03, 0x02, 0x48 }); // Instr 03  Cmd 02  Note 48 (C-6)
        for (int i = 0; i < 31; i++)
            m_SF2->AppendToSequence(0x03, { 0x80, 0x80, 0x7e });

        // Instruments
        m_SF2->EditTableRow(TABLE_INSTR, 0x01, { 0x00, 0xe8, 0xc0, 0x00, 0x00, 0x00 }); // Bass
        m_SF2->EditTableRow(TABLE_INSTR, 0x02, { 0x00, 0xa8, 0x80, 0x00, 0x00, 0x02 }); // Chords
        m_SF2->EditTableRow(TABLE_INSTR, 0x03, { 0x00, 0x9f, 0x00, 0x00, 0x00, 0x04 }); // Modulation

        // Descriptions for instruments
        m_SF2->EditTableRowText(TABLE_INSTR, 0x00, "This is an empty instrument");
        m_SF2->EditTableRowText(TABLE_INSTR, 0x01, "Bass");
        m_SF2->EditTableRowText(TABLE_INSTR, 0x02, "Chords");
        m_SF2->EditTableRowText(TABLE_INSTR, 0x03, "Ring modulation");

        // Wave table
        m_SF2->EditTableRow(TABLE_WAVE, 0x00, { 0x41, 0x00 }); // Pulse for bass
        m_SF2->EditTableRow(TABLE_WAVE, 0x01, { 0x7f, 0x00 });
        m_SF2->EditTableRow(TABLE_WAVE, 0x02, { 0x11, 0x00 }); // Triangle for chords
        m_SF2->EditTableRow(TABLE_WAVE, 0x03, { 0x7f, 0x00 });
        m_SF2->EditTableRow(TABLE_WAVE, 0x04, { 0x17, 0x00 }); // RM + HS for modulation
        m_SF2->EditTableRow(TABLE_WAVE, 0x05, { 0x7f, 0x00 });

        // Pulse
        m_SF2->EditTableRow(TABLE_PULSE, 0x00, { 0x84, 0x00, 0x00 }); // Pulse 400 for bass
        m_SF2->EditTableRow(TABLE_PULSE, 0x01, { 0x7f, 0x00, 0x00 });

        // Filter
        m_SF2->EditTableRow(TABLE_FILTER, 0x00, { 0x93, 0x00, 0xf1 }); // Bouncy filter for bass
        m_SF2->EditTableRow(TABLE_FILTER, 0x01, { 0x0f, 0x90, 0x04 });
        m_SF2->EditTableRow(TABLE_FILTER, 0x02, { 0x00, 0x00, 0x00 });
        m_SF2->EditTableRow(TABLE_FILTER, 0x03, { 0x7f, 0x00, 0x02 });

        // Arpeggio chord
        m_SF2->EditTableRow(TABLE_ARP, 0x00, { 0x00 }); // 0-3-7 chord
        m_SF2->EditTableRow(TABLE_ARP, 0x01, { 0x03 });
        m_SF2->EditTableRow(TABLE_ARP, 0x02, { 0x07 });
        m_SF2->EditTableRow(TABLE_ARP, 0x03, { 0x70 });

        // Commands
        m_SF2->EditTableRow(TABLE_CMDS, 0x00, { 0x01, 0x03, 0x02 }); // Vibrato for one bass note
        m_SF2->EditTableRow(TABLE_CMDS, 0x01, { 0x03, 0x00, 0x00 }); // 0-3-7 chord
        m_SF2->EditTableRow(TABLE_CMDS, 0x02, { 0x00, 0x00, 0x80 }); // Slide up for modulation

        // Tempo
        m_SF2->EditTableRow(TABLE_TEMPO, 0x00, { 0x02 }); // Tempo 2 instead of 3

        // Push to C64 memory
        bool data_pushed = m_SF2->PushAllDataToMemory(true);
        if (!data_pushed)
        {
            std::cerr << "ERROR: Could not push data to emulated C64 memory." << std::endl;
            exit(0);
        }

        // And save it all
        bool driver_saved = m_SF2->SaveFile("test_sf2conv.sf2");
        if (!driver_saved)
        {
            std::cerr << "ERROR: Test tune failed to save." << std::endl;
            exit(0);
        }

    }
}
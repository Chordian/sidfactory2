#include "utils/configfile.h"
#include "utils/config/configtypes.h"
#include "utils/usercolors.h"
#include "foundation/graphics/viewport.h"
#include "foundation/graphics/color.h"


namespace Utility
{
	class ConfigFile;

	namespace Config
	{
		namespace Details
		{
			void SetUserColor(const ConfigFile& inConfigFile, const std::string& inKey, UserColor inUserColor, Foundation::Viewport& ioViewport, Foundation::Color inDefaultColor);
		}

		void ConfigureColors(const ConfigFile& inConfigFile, Foundation::Viewport& ioViewport)
		{
			using namespace Foundation;

			// Set defaults
			Details::SetUserColor(inConfigFile, "Color.Table.Default", UserColor::TableDefault, ioViewport, Color::LightGrey);
			Details::SetUserColor(inConfigFile, "Color.Table.Highlight", UserColor::TableHighlight, ioViewport, Color::White);
			Details::SetUserColor(inConfigFile, "Color.Table.Text.Background", UserColor::TableTextBackground, ioViewport, Color::DarkGrey);
			Details::SetUserColor(inConfigFile, "Color.Table.Text", UserColor::TableText, ioViewport, Color::White);
			Details::SetUserColor(inConfigFile, "Color.Table.Text.Editing", UserColor::TableTextEditing, ioViewport, Color::LightYellow);
			Details::SetUserColor(inConfigFile, "Color.Button.Text", UserColor::ButtonText, ioViewport, Color::White);
			Details::SetUserColor(inConfigFile, "Color.Button.Default", UserColor::ButtonDefault, ioViewport, Color::Black);
			Details::SetUserColor(inConfigFile, "Color.Button.Default.MouseOver", UserColor::ButtonDefaultMouseOver, ioViewport, Color::DarkGrey);
			Details::SetUserColor(inConfigFile, "Color.Button.Highlight", UserColor::ButtonHighlight, ioViewport, Color::Red);
			Details::SetUserColor(inConfigFile, "Color.Button.Highlight.MouseOver", UserColor::ButtonHighlightMouseOver, ioViewport, Color::LightRed);
			Details::SetUserColor(inConfigFile, "Color.FileSelector.Background", UserColor::FileSelectorBackground, ioViewport, Color::DarkBlue);
			Details::SetUserColor(inConfigFile, "Color.FileSelector.List.Text", UserColor::FileSelectorListText, ioViewport, Color::White);
			Details::SetUserColor(inConfigFile, "Color.FileSelector.Text.Input", UserColor::FileSelectorTextInput, ioViewport, Color::White);
			Details::SetUserColor(inConfigFile, "Color.FileSelector.Cursor.Default", UserColor::FileSelectorCursorNoFocus, ioViewport, Color::DarkGrey);
			Details::SetUserColor(inConfigFile, "Color.FileSelector.Cursor.Focus.Default", UserColor::FileSelectorCursorFocusDefault, ioViewport, Color::LightRed);
			Details::SetUserColor(inConfigFile, "Color.FileSelector.Cursor.Focus.Load.Song", UserColor::FileSelectorCursorFocusLoadSong, ioViewport, Color::DarkGreen);
			Details::SetUserColor(inConfigFile, "Color.FileSelector.Cursor.Focus.Import.Song", UserColor::FileSelectorCursorFocusImportSong, ioViewport, Color::DarkYellow);
			Details::SetUserColor(inConfigFile, "Color.FileSelector.Cursor.Focus.Save.Song", UserColor::FileSelectorCursorFocusSaveSong, ioViewport, Color::Red);
			Details::SetUserColor(inConfigFile, "Color.FileSelector.Cursor.Focus.Save.Packed", UserColor::FileSelectorCursorFocusSavePacked, ioViewport, Color::Blue);
			Details::SetUserColor(inConfigFile, "Color.MarkerList.Background", UserColor::MarkerListBackground, ioViewport, Color::DarkBlue);
			Details::SetUserColor(inConfigFile, "Color.MarkerList.Text", UserColor::MarkerListText, ioViewport, Color::White);
			Details::SetUserColor(inConfigFile, "Color.MarkerList.Cursor.Default", UserColor::MarkerListCursorNoFocus, ioViewport, Color::DarkGrey);
			Details::SetUserColor(inConfigFile, "Color.MarkerList.Cursor.Focus.Default", UserColor::MarkerListCursorFocus, ioViewport, Color::LightBlue);
			Details::SetUserColor(inConfigFile, "Color.SongList.Background", UserColor::SongListBackground, ioViewport, Color::DarkGrey);
			Details::SetUserColor(inConfigFile, "Color.SongList.BackgroundText", UserColor::SongListBackgroundText, ioViewport, Color::DarkerGrey);
			Details::SetUserColor(inConfigFile, "Color.SongList.EventPos", UserColor::SongListEventPos, ioViewport, Color::White);
			Details::SetUserColor(inConfigFile, "Color.SongList.Values", UserColor::SongListValues, ioViewport, Color::White);
			Details::SetUserColor(inConfigFile, "Color.SongList.Cursor", UserColor::SongListCursorDefault, ioViewport, Color::DarkBlue);
			Details::SetUserColor(inConfigFile, "Color.SongList.Cursor.Focus", UserColor::SongListCursorFocus, ioViewport, Color::Blue);
			Details::SetUserColor(inConfigFile, "Color.SongList.Playback.Marker", UserColor::SongListPlaybackMarker, ioViewport, Color::Red);
			Details::SetUserColor(inConfigFile, "Color.SongList.Loop.Marker", UserColor::SongListLoopMarker, ioViewport, Color::LightGreen);
			Details::SetUserColor(inConfigFile, "Color.Track.Background", UserColor::TrackBackground, ioViewport, Color::DarkBlue);
			Details::SetUserColor(inConfigFile, "Color.Track.Background.FocusLine", UserColor::TrackBackgroundFocusLine, ioViewport, Color::DarkerBlue);
			Details::SetUserColor(inConfigFile, "Color.Track.BackgroundMuted", UserColor::TrackBackgroundMuted, ioViewport, Color::DarkGrey);
			Details::SetUserColor(inConfigFile, "Color.Track.BackgroundMuted.FocusLine", UserColor::TrackBackgroundMutedFocusLine, ioViewport, Color::DarkerGrey);
			Details::SetUserColor(inConfigFile, "Color.Track.Name", UserColor::TrackName, ioViewport, Color::Grey);
			Details::SetUserColor(inConfigFile, "Color.Track.Name.Selected", UserColor::TrackNameSelected, ioViewport, Color::White);
			Details::SetUserColor(inConfigFile, "Color.Track.Name.Focus", UserColor::TrackNameFocus, ioViewport, Color::LighterGrey);
			Details::SetUserColor(inConfigFile, "Color.Track.Name.Selected.Focus", UserColor::TrackNameSelectedFocus, ioViewport, Color::Yellow);
			Details::SetUserColor(inConfigFile, "Color.Tracks.EventNumbers.Background", UserColor::TracksEventNumbersBackground, ioViewport, Color::Black);
			Details::SetUserColor(inConfigFile, "Color.Tracks.EventNumbers.Default", UserColor::TracksEventNumbersDefault, ioViewport, Color::LightGrey);
			Details::SetUserColor(inConfigFile, "Color.Tracks.EventNumbers.FocusLine", UserColor::TracksEventNumbersFocusLine, ioViewport, Color::Yellow);
			Details::SetUserColor(inConfigFile, "Color.Tracks.EventNumbers.Highlight", UserColor::TracksEventNumbersHighlight, ioViewport, Color::White);
			Details::SetUserColor(inConfigFile, "Color.Tracks.EventNumbers.Highlight.FocusLine", UserColor::TracksEventNumbersHighlightFocusLine, ioViewport, Color::LightYellow);
			Details::SetUserColor(inConfigFile, "Color.Tracks.EventNumbers.Playback.Marker", UserColor::TracksEventNumbersPlaybackMarker, ioViewport, Color::Red);
			Details::SetUserColor(inConfigFile, "Color.Sequence.Error", UserColor::SequenceError,ioViewport, Color::LightRed);
			Details::SetUserColor(inConfigFile, "Color.Sequence.Instrument.Empty", UserColor::SequenceInstrumentEmpty, ioViewport, Color::LightGrey);
			Details::SetUserColor(inConfigFile, "Color.Sequence.Instrument.Empty.FocusLine", UserColor::SequenceInstrumentEmptyFocusLine, ioViewport, Color::LighterGrey);
			Details::SetUserColor(inConfigFile, "Color.Sequence.Instrument.Tie", UserColor::SequenceInstrumentTie, ioViewport, Color::LightGrey);
			Details::SetUserColor(inConfigFile, "Color.Sequence.Instrument.Tie.FocusLine", UserColor::SequenceInstrumentTieFocusLine, ioViewport, Color::LighterGrey);
			Details::SetUserColor(inConfigFile, "Color.Sequence.Instrument.Value", UserColor::SequenceInstrumentValue, ioViewport, Color::Yellow);
			Details::SetUserColor(inConfigFile, "Color.Sequence.Instrument.Value.FocusLine", UserColor::SequenceInstrumentValueFocusLine, ioViewport, Color::LightYellow);
			Details::SetUserColor(inConfigFile, "Color.Sequence.Instrument.Value.Selected.Highlight", UserColor::SequenceInstrumentValueSelectedHighlight, ioViewport, Color::Green);
			Details::SetUserColor(inConfigFile, "Color.Sequence.Instrument.Value.Selected.Highlight.FocusLine", UserColor::SequenceInstrumentValueSelectedHighlightFocusLine, ioViewport, Color::LightGreen);
			Details::SetUserColor(inConfigFile, "Color.Sequence.Command.Empty", UserColor::SequenceCommandEmpty, ioViewport, Color::LightGrey);
			Details::SetUserColor(inConfigFile, "Color.Sequence.Command.Empty.FocusLine", UserColor::SequenceCommandEmptyFocusLine, ioViewport, Color::LighterGrey);
			Details::SetUserColor(inConfigFile, "Color.Sequence.Command.Value", UserColor::SequenceCommandValue, ioViewport, Color::Yellow);
			Details::SetUserColor(inConfigFile, "Color.Sequence.Command.Value.FocusLine", UserColor::SequenceCommandValueFocusLine, ioViewport, Color::LightYellow);
			Details::SetUserColor(inConfigFile, "Color.Sequence.Command.Value.Selected.Highlight", UserColor::SequenceCommandValueSelectedHighlight, ioViewport, Color::Green);
			Details::SetUserColor(inConfigFile, "Color.Sequence.Command.Value.Selected.Highlight.FocusLine", UserColor::SequenceCommandValueSelectedHighlightFocusLine, ioViewport, Color::LightGreen);
			Details::SetUserColor(inConfigFile, "Color.Sequence.Note.Empty", UserColor::SequenceNoteEmpty, ioViewport, Color::LightGrey);
			Details::SetUserColor(inConfigFile, "Color.Sequence.Note.Empty.FocusLine", UserColor::SequenceNoteEmptyFocusLine, ioViewport, Color::LighterGrey);
			Details::SetUserColor(inConfigFile, "Color.Sequence.Note.Gate.On", UserColor::SequenceNoteGateOn, ioViewport, Color::LightBlue);
			Details::SetUserColor(inConfigFile, "Color.Sequence.Note.Gate.On.FocusLine", UserColor::SequenceNoteGateOnFocusLine, ioViewport, Color::LightBlue);
			Details::SetUserColor(inConfigFile, "Color.Sequence.Note.Value", UserColor::SequenceNoteValue, ioViewport, Color::LightGrey);
			Details::SetUserColor(inConfigFile, "Color.Sequence.Note.Value.FocusLine", UserColor::SequenceNoteValueFocusLine, ioViewport, Color::LighterGrey);
			Details::SetUserColor(inConfigFile, "Color.Sequence.Note.Value.Tied", UserColor::SequenceNoteValueTied, ioViewport, Color::Yellow);
			Details::SetUserColor(inConfigFile, "Color.Sequence.Note.Value.Tied.FocusLine", UserColor::SequenceNoteValueTiedFocusLine, ioViewport, Color::LightYellow);
			Details::SetUserColor(inConfigFile, "Color.Sequence.Note.Value.Selected.Highlight", UserColor::SequenceNoteValueSelectedHighlight, ioViewport, Color::Green);
			Details::SetUserColor(inConfigFile, "Color.Sequence.Note.Value.Selected.Highlight.FocusLine", UserColor::SequenceNoteValueSelectedHighlightFocusLine, ioViewport, Color::LightGreen);
			Details::SetUserColor(inConfigFile, "Color.Orderlist.End.Loop", UserColor::OrderlistEndLoop, ioViewport, Color::White);
			Details::SetUserColor(inConfigFile, "Color.Orderlist.End.Stop", UserColor::OrderlistEndStop, ioViewport, Color::White);
			Details::SetUserColor(inConfigFile, "Color.Orderlist.Value", UserColor::OrderlistValue, ioViewport, Color::LightGrey);
			Details::SetUserColor(inConfigFile, "Color.Orderlist.Value.Loop.Marker", UserColor::OrderlistValueLoopMarker, ioViewport, Color::LightGreen);
			Details::SetUserColor(inConfigFile, "Color.Orderlist.Value.Input", UserColor::OrderlistValueInput, ioViewport, Color::White);
			Details::SetUserColor(inConfigFile, "Color.Dialog.Background", UserColor::DialogBackground, ioViewport, Color::DarkRed);
			Details::SetUserColor(inConfigFile, "Color.Dialog.Header", UserColor::DialogHeader, ioViewport, Color::Red);
			Details::SetUserColor(inConfigFile, "Color.Dialog.Header.Text", UserColor::DialogHeaderText, ioViewport, Color::White);
			Details::SetUserColor(inConfigFile, "Color.Dialog.Text", UserColor::DialogText, ioViewport, Color::White);
			Details::SetUserColor(inConfigFile, "Color.Dialog.ListSelector.Cursor", UserColor::DialogListSelectorCursor, ioViewport, Color::DarkGrey);
			Details::SetUserColor(inConfigFile, "Color.Dialog.ListSelector.Cursor.Focus", UserColor::DialogListSelectorCursorFocus, ioViewport, Color::DarkBlue);
			Details::SetUserColor(inConfigFile, "Color.Dialog.Optimizer.Used", UserColor::DialogOptimizerUsed, ioViewport, Color::White);
			Details::SetUserColor(inConfigFile, "Color.Dialog.Optimizer.Unused", UserColor::DialogOptimizerUnused, ioViewport, Color::LighterGrey);
			Details::SetUserColor(inConfigFile, "Color.StatusBar.Text", UserColor::StatusBarText, ioViewport, Color::White);
			Details::SetUserColor(inConfigFile, "Color.StatusBar.Background.Stopped", UserColor::StatusBarBackgroundStopped, ioViewport, Color::Blue);
			Details::SetUserColor(inConfigFile, "Color.StatusBar.Background.Stopped.MouseOver", UserColor::StatusBarBackgroundStoppedMouseOverRegion, ioViewport, Color::LightBlue);
			Details::SetUserColor(inConfigFile, "Color.StatusBar.Background.Playing", UserColor::StatusBarBackgroundPlaying, ioViewport, Color::DarkGreen);
			Details::SetUserColor(inConfigFile, "Color.StatusBar.Background.Playing.MouseOver", UserColor::StatusBarBackgroundPlayingMouseOverRegion, ioViewport, Color::Green);
			Details::SetUserColor(inConfigFile, "Color.StatusBar.Background.PlayingInput", UserColor::StatusBarBackgroundPlayingInput, ioViewport, Color::DarkYellow);
			Details::SetUserColor(inConfigFile, "Color.StatusBar.Background.PlayingInput.MouseOver", UserColor::StatusBarBackgroundPlayingInputMouseOverRegion, ioViewport, Color::Yellow);
			Details::SetUserColor(inConfigFile, "Color.ScreenEdit.InfoRect.Background", UserColor::ScreenEditInfoRectBackground, ioViewport, Color::DarkerGrey);
			Details::SetUserColor(inConfigFile, "Color.ScreenEdit.InfoRect.Text", UserColor::ScreenEditInfoRectText, ioViewport, Color::DarkerGrey);
			Details::SetUserColor(inConfigFile, "Color.ScreenEdit.InfoRect.Text.Time.PlaybackState", UserColor::ScreenEditInfoRectTextTimePlaybackState, ioViewport, Color::DarkerGrey);
			Details::SetUserColor(inConfigFile, "Color.ScreenEdit.Table.Headline", UserColor::ScreenEditTableHeadline, ioViewport, Color::LightBlue);
			Details::SetUserColor(inConfigFile, "Color.ScreenEdit.Table.Headline.HotkeyLetter", UserColor::ScreenEditTableHeadlineHotkeyLetter, ioViewport, Color::White);
			Details::SetUserColor(inConfigFile, "Color.Converters.Backgroud", UserColor::ConvertersBackground, ioViewport, Color::DarkerGrey);
			Details::SetUserColor(inConfigFile, "Color.Console.Background", UserColor::ConsoleBackground, ioViewport, Color::Black);
			Details::SetUserColor(inConfigFile, "Color.Console.Text", UserColor::ConsoleText, ioViewport, Color::LightGreen);

			Details::SetUserColor(inConfigFile, "Color.FlightRecorder.Background", UserColor::FlightRecorderBackground, ioViewport, Color::DarkerGrey);
			Details::SetUserColor(inConfigFile, "Color.FlightRecorder.Description", UserColor::FlightRecorderDesc, ioViewport, Color::White);
			Details::SetUserColor(inConfigFile, "Color.FlightRecorder.GateOn", UserColor::FlightRecorderGateOn, ioViewport, Color::White);
			Details::SetUserColor(inConfigFile, "Color.FlightRecorder.GateOff", UserColor::FlightRecorderGateOff, ioViewport, Color::LighterGrey);
			Details::SetUserColor(inConfigFile, "Color.FlightRecorder.FilterAndVolume", UserColor::FlightRecorderFilterAndVolume, ioViewport, Color::LightBlue);
			Details::SetUserColor(inConfigFile, "Color.FlightRecorder.CPUUsageLow", UserColor::FlightRecorderCPUUsageLow, ioViewport, Color::White);
			Details::SetUserColor(inConfigFile, "Color.FlightRecorder.CPUUsageMedium", UserColor::FlightRecorderCPUUsageMedium, ioViewport, Color::LightYellow);
			Details::SetUserColor(inConfigFile, "Color.FlightRecorder.CPUUsageHigh", UserColor::FlightRecorderCPUUsageHigh, ioViewport, Color::LightRed);

			Details::SetUserColor(inConfigFile, "Color.FlightRecorder.Visualizer.Background", UserColor::FlightRecorderVisualizerBackground, ioViewport, Color::DarkGrey);
			Details::SetUserColor(inConfigFile, "Color.FlightRecorder.Visualizer.CPUUsageLow", UserColor::FlightRecorderVisualizerCPUUsageLow, ioViewport, Color::White);
			Details::SetUserColor(inConfigFile, "Color.FlightRecorder.Visualizer.CPUUsageMedium", UserColor::FlightRecorderVisualizerCPUUsageMedium, ioViewport, Color::LightYellow);
			Details::SetUserColor(inConfigFile, "Color.FlightRecorder.Visualizer.CPUUsageHigh", UserColor::FlightRecorderVisualizerCPUUsageHigh, ioViewport, Color::LightRed);
			Details::SetUserColor(inConfigFile, "Color.FlightRecorder.Visualizer.HorizontalLine1", UserColor::FlightRecorderVisualizerHorizontalLine1, ioViewport, Color::White);
			Details::SetUserColor(inConfigFile, "Color.FlightRecorder.Visualizer.HorizontalLine2", UserColor::FlightRecorderVisualizerHorizontalLine2, ioViewport, Color::Grey);

			Details::SetUserColor(inConfigFile, "Color.DriverTableColor.Set", UserColor::DriverColorSet, ioViewport, Color::LightGreen);
			Details::SetUserColor(inConfigFile, "Color.DriverTableColor.Jump", UserColor::DriverColorJump, ioViewport, Color::LightYellow);

			Details::SetUserColor(inConfigFile, "Color.DriverTableColor.02", UserColor::DriverColor02, ioViewport, Color::LightRed);
			Details::SetUserColor(inConfigFile, "Color.DriverTableColor.03", UserColor::DriverColor03, ioViewport, Color::LightRed);
			Details::SetUserColor(inConfigFile, "Color.DriverTableColor.04", UserColor::DriverColor04, ioViewport, Color::LightRed);
			Details::SetUserColor(inConfigFile, "Color.DriverTableColor.05", UserColor::DriverColor05, ioViewport, Color::LightRed);
			Details::SetUserColor(inConfigFile, "Color.DriverTableColor.06", UserColor::DriverColor06, ioViewport, Color::LightRed);
			Details::SetUserColor(inConfigFile, "Color.DriverTableColor.07", UserColor::DriverColor07, ioViewport, Color::LightRed);
			Details::SetUserColor(inConfigFile, "Color.DriverTableColor.08", UserColor::DriverColor08, ioViewport, Color::LightRed);
			Details::SetUserColor(inConfigFile, "Color.DriverTableColor.09", UserColor::DriverColor09, ioViewport, Color::LightRed);
			Details::SetUserColor(inConfigFile, "Color.DriverTableColor.0a", UserColor::DriverColor0a, ioViewport, Color::LightRed);
			Details::SetUserColor(inConfigFile, "Color.DriverTableColor.0b", UserColor::DriverColor0b, ioViewport, Color::LightRed);
			Details::SetUserColor(inConfigFile, "Color.DriverTableColor.0c", UserColor::DriverColor0c, ioViewport, Color::LightRed);
			Details::SetUserColor(inConfigFile, "Color.DriverTableColor.0d", UserColor::DriverColor0d, ioViewport, Color::LightRed);
			Details::SetUserColor(inConfigFile, "Color.DriverTableColor.0e", UserColor::DriverColor0e, ioViewport, Color::LightRed);
			Details::SetUserColor(inConfigFile, "Color.DriverTableColor.0f", UserColor::DriverColor0f, ioViewport, Color::LightRed);
			Details::SetUserColor(inConfigFile, "Color.DriverTableColor.10", UserColor::DriverColor10, ioViewport, Color::LightRed);
			Details::SetUserColor(inConfigFile, "Color.DriverTableColor.11", UserColor::DriverColor11, ioViewport, Color::LightRed);
			Details::SetUserColor(inConfigFile, "Color.DriverTableColor.12", UserColor::DriverColor12, ioViewport, Color::LightRed);
			Details::SetUserColor(inConfigFile, "Color.DriverTableColor.13", UserColor::DriverColor13, ioViewport, Color::LightRed);
			Details::SetUserColor(inConfigFile, "Color.DriverTableColor.14", UserColor::DriverColor14, ioViewport, Color::LightRed);
			Details::SetUserColor(inConfigFile, "Color.DriverTableColor.15", UserColor::DriverColor15, ioViewport, Color::LightRed);
			Details::SetUserColor(inConfigFile, "Color.DriverTableColor.16", UserColor::DriverColor16, ioViewport, Color::LightRed);
			Details::SetUserColor(inConfigFile, "Color.DriverTableColor.17", UserColor::DriverColor17, ioViewport, Color::LightRed);
		}

		namespace Details
		{
			void SetUserColor(const ConfigFile& inConfigFile, const std::string& inKey, UserColor inUserColor, Foundation::Viewport& ioViewport, Foundation::Color inDefaultColor)
			{
				if (inConfigFile.HasKey(inKey))
				{
					std::vector<int> default_color_value = { static_cast<int>(inDefaultColor) };
					const std::vector<int> values = Utility::GetConfigurationValues<Config::ConfigValueInt>(inConfigFile, inKey, default_color_value);

					if (values.size() >= 3)
					{
						const unsigned int color = 0xff000000 | ((0xff & values[0]) << 16) | ((0xff & values[1]) << 8) | (0xff & values[2]);
						SetUserColor(ioViewport, inUserColor, color);
						return;
					}
					else if (values.size() == 1)
					{
						const unsigned int color = 0xff000000 | values[0];
						SetUserColor(ioViewport, inUserColor, color);
						return;
					}
				}
			
				SetUserColor(ioViewport, inUserColor, inDefaultColor);
			}
		}
	}
}
#include "runtime/editor/keys/keyhook_setup.h"
#include "foundation/input/keyboard.h"
#include "utils/configfile.h"
#include "utils/config/configtypes.h"

using namespace Foundation;

namespace Editor
{
	KeyHookSetup::KeyHookSetup()
	{
		ConfigureStore();
		ApplyOverrides();
	}


	Utility::KeyHookStore& KeyHookSetup::GetKeyHookStore()
	{
		return m_KeyHookStore;
	}


	void KeyHookSetup::Reset()
	{
		m_KeyHookStore.Clear();

		ConfigureStore();
		ApplyOverrides();
	}


	void KeyHookSetup::ApplyConfigSettings(const Utility::ConfigFile& inConfigFile)
	{
		using namespace Utility;
		using namespace Utility::Config;

		std::vector<std::string> key_hook_names = m_KeyHookStore.GetAllHookNames();

		for (const auto& key_hook_name : key_hook_names)
		{
			if (inConfigFile.HasKeyOfType<ConfigValueKey>(key_hook_name))
			{
				std::vector<KeyHookStore::Key> override_keys;
				const auto& config_keys = inConfigFile.GetValue<ConfigValueKey>(key_hook_name);

				for (size_t i = 0; i < config_keys.GetValueCount(); ++i)
				{
					const auto& key = config_keys.GetValue(i);
					override_keys.push_back({ key.m_Key, key.m_Modifier });
				}

				m_KeyHookStore.OverrideDefinition({ key_hook_name, override_keys });
			}
		}
	}

	void KeyHookSetup::ConfigureStore()
	{
		std::vector<Utility::KeyHookStore::HookDefinition> definitions;

		definitions.push_back({ "Key.ScreenEdit.Stop", {{ SDLK_ESCAPE, Keyboard::None }} });
		definitions.push_back({ "Key.ScreenEdit.Play", {{ SDLK_F1, Keyboard::None }} });
		definitions.push_back({ "Key.ScreenEdit.PlayFromMarker", {{ SDLK_F2, Keyboard::None }} });
		definitions.push_back({ "Key.ScreenEdit.PlayFromLast", {{ SDLK_F2, Keyboard::Shift }} });
		definitions.push_back({ "Key.ScreenEdit.PlayFromCursor", {{ SDLK_F2, Keyboard::Control }} });
		definitions.push_back({ "Key.ScreenEdit.OctaveDown", {{ SDLK_F3, Keyboard::None }} });
		definitions.push_back({ "Key.ScreenEdit.OctaveUp", {{ SDLK_F4, Keyboard::None }} });
		definitions.push_back({ "Key.ScreenEdit.OpenUtilitiesDialog", {{ SDLK_F6, Keyboard::None }} });
		definitions.push_back({ "Key.ScreenEdit.OpenOptionsDialog", {{ SDLK_F6, Keyboard::Control }} });
		definitions.push_back({ "Key.ScreenEdit.Config.Reload", {{ SDLK_F7, Keyboard::Shift }} });
		definitions.push_back({ "Key.ScreenEdit.ToggleSIDModel", {{ SDLK_F9, Keyboard::None }} });
		definitions.push_back({ "Key.ScreenEdit.ToggleRegion", {{ SDLK_F9, Keyboard::Shift }} });
		definitions.push_back({ "Key.ScreenEdit.LoadSong", {{ SDLK_F10, Keyboard::None }} });
		definitions.push_back({ "Key.ScreenEdit.LoadInstrument", {{ SDLK_F10, Keyboard::Shift }} });
		definitions.push_back({ "Key.ScreenEdit.ImportSong", {{ SDLK_F10, Keyboard::Control }} });
		definitions.push_back({ "Key.ScreenEdit.SaveSong", {{ SDLK_F11, Keyboard::None }} });
		definitions.push_back({ "Key.ScreenEdit.SaveInstrument", {{ SDLK_F11, Keyboard::Shift }} });
		definitions.push_back({ "Key.ScreenEdit.ToggleOverlay", {{ SDLK_F12, Keyboard::None }} });
		definitions.push_back({ "Key.ScreenEdit.ToggleFlightRecorderOverlay", {{ SDLK_d, Keyboard::Shift | Keyboard::Control }} });
		definitions.push_back({ "Key.ScreenEdit.ToggleDebugView", {{ SDLK_d, Keyboard::Shift | Keyboard::Alt }} });
		definitions.push_back({ "Key.ScreenEdit.ToggleMuteChannel1", {{ SDLK_1, Keyboard::Control }} });
		definitions.push_back({ "Key.ScreenEdit.ToggleMuteChannel2", {{ SDLK_2, Keyboard::Control }} });
		definitions.push_back({ "Key.ScreenEdit.ToggleMuteChannel3", {{ SDLK_3, Keyboard::Control }} });
		definitions.push_back({ "Key.ScreenEdit.SetMarker", {{ SDLK_m, Keyboard::Control }} });
		definitions.push_back({ "Key.ScreenEdit.GotoMarker", {{ SDLK_g, Keyboard::Control }} });
		definitions.push_back({ "Key.ScreenEdit.QuickSave", {{ SDLK_s, Keyboard::Control }} });
		definitions.push_back({ "Key.ScreenEdit.Undo", {{ SDLK_z, Keyboard::Control }} });
		definitions.push_back({ "Key.ScreenEdit.Redo", {{ SDLK_x, Keyboard::Control }, { SDLK_y, Keyboard::Control }} });
		definitions.push_back({ "Key.ScreenEdit.DecrementEventHighlightInterval", {{ SDLK_UP, Keyboard::Alt }} });
		definitions.push_back({ "Key.ScreenEdit.DecrementEventHighlightOffset", {{ SDLK_UP, Keyboard::Alt | Keyboard::Shift }} });
		definitions.push_back({ "Key.ScreenEdit.IncrementEventHighlightInterval", {{ SDLK_DOWN, Keyboard::Alt }} });
		definitions.push_back({ "Key.ScreenEdit.IncrementEventHighlightOffset", {{ SDLK_DOWN, Keyboard::Alt | Keyboard::Shift }} });
		definitions.push_back({ "Key.ScreenEdit.ToggleFollowPlay", {{ SDLK_p, Keyboard::Control }} });
		definitions.push_back({ "Key.ScreenEdit.ToggleDisplayHexUppercase", {{ SDLK_u, Keyboard::Control }} });
		definitions.push_back({ "Key.ScreenEdit.IncrementInstrumentIndex", {{ SDLK_KP_PLUS, Keyboard::None }, { SDLK_DOWN, Keyboard::Alt | Keyboard::Control }} });
		definitions.push_back({ "Key.ScreenEdit.DecrementInstrumentIndex", {{ SDLK_KP_MINUS, Keyboard::None }, { SDLK_UP, Keyboard::Alt | Keyboard::Control }} });
		definitions.push_back({ "Key.ScreenEdit.IncrementCommandIndex", {{ SDLK_KP_PLUS, Keyboard::Control }} });
		definitions.push_back({ "Key.ScreenEdit.DecrementCommandIndex", {{ SDLK_KP_MINUS, Keyboard::Control }} });
		definitions.push_back({ "Key.ScreenEdit.SetOrderlistLoopPointAll", {{ SDLK_l, Keyboard::Control }} });
		definitions.push_back({ "Key.ScreenEdit.ToggleColorSchemes", {{ SDLK_F7, Keyboard::Control }} });
		definitions.push_back({ "Key.ScreenEdit.RefreshColorSchemes", {{ SDLK_F7, Keyboard::Control | Keyboard::Shift }} });
		definitions.push_back({ "Key.ScreenEdit.FastForward", {{ 189, Keyboard::None }} });
		definitions.push_back({ "Key.ScreenEdit.FasterForward", {{ 189, Keyboard::Shift }} });

		definitions.push_back({ "Key.Track.CursorUp", {{ SDLK_UP, Keyboard::None }} });
		definitions.push_back({ "Key.Track.CursorDown", {{ SDLK_DOWN, Keyboard::None }} });
		definitions.push_back({ "Key.Track.ToggleGateUntilNextEvent", {{ SDLK_DOWN, Keyboard::Shift | Keyboard::Control }} });
		definitions.push_back({ "Key.Track.ToggleGateSincePreviousEvent", {{ SDLK_UP, Keyboard::Shift | Keyboard::Control }} });
		definitions.push_back({ "Key.Track.PageUp", {{ SDLK_PAGEUP, Keyboard::None }} });
		definitions.push_back({ "Key.Track.PageDown", {{ SDLK_PAGEDOWN, Keyboard::None }} });
		definitions.push_back({ "Key.Track.TransposeSemiToneDown", {{ SDLK_F3, Keyboard::Shift }} });
		definitions.push_back({ "Key.Track.TransposeOctaveDown", {{ SDLK_F3, Keyboard::Control }} });
		definitions.push_back({ "Key.Track.TransposeSemiToneUp", {{ SDLK_F4, Keyboard::Shift }} });
		definitions.push_back({ "Key.Track.TransposeOctaveUp", {{ SDLK_F4, Keyboard::Control }} });
		definitions.push_back({ "Key.Track.InsertEventsInSequence", {{ SDLK_F5, Keyboard::Shift }} });
		definitions.push_back({ "Key.Track.ResizeSequence", {{ SDLK_F5, Keyboard::None }} });
		definitions.push_back({ "Key.Track.EraseEventUnderCursor", {{ SDLK_SPACE, Keyboard::None }} });
		definitions.push_back({ "Key.Track.EraseEventLine", {{ SDLK_SPACE, Keyboard::Control }} });
		definitions.push_back({ "Key.Track.ToggleGateUnderCursor", {{ SDLK_SPACE, Keyboard::Shift }} });
		definitions.push_back({ "Key.Track.CursorLeft", {{ SDLK_LEFT, Keyboard::None }} });
		definitions.push_back({ "Key.Track.CursorRight", {{ SDLK_RIGHT, Keyboard::None }} });
		definitions.push_back({ "Key.Track.CursorLeftColumn", {{ SDLK_LEFT, Keyboard::Control }} });
		definitions.push_back({ "Key.Track.CursorRightColumn", {{ SDLK_RIGHT, Keyboard::Control }} });
		definitions.push_back({ "Key.Track.Insert", {{ SDLK_INSERT, Keyboard::None }} });
		definitions.push_back({ "Key.Track.InsertAndGrow", {{ SDLK_INSERT, Keyboard::Control }} });
		definitions.push_back({ "Key.Track.Delete", {{ SDLK_DELETE, Keyboard::None }} });
		definitions.push_back({ "Key.Track.InsertAndShrink", {{ SDLK_DELETE, Keyboard::Control }} });
		definitions.push_back({ "Key.Track.Backspace", {{ SDLK_BACKSPACE, Keyboard::None }} });
		definitions.push_back({ "Key.Track.BackspaceAndShrink", {{ SDLK_BACKSPACE, Keyboard::Control }} });
		definitions.push_back({ "Key.Track.ShiftBackspace", {{ SDLK_BACKSPACE, Keyboard::Shift }} });
		definitions.push_back({ "Key.Track.ShiftBackspaceAndGrow", {{ SDLK_BACKSPACE, Keyboard::Shift | Keyboard::Control }} });
		definitions.push_back({ "Key.Track.Home", {{ SDLK_HOME, Keyboard::None }} });
		definitions.push_back({ "Key.Track.End", {{ SDLK_END, Keyboard::None }} });
		definitions.push_back({ "Key.Track.ApplyOrderlistInput", {{ SDLK_RETURN, Keyboard::None }} });
		definitions.push_back({ "Key.Track.CancelOrderlistInput", {{ SDLK_ESCAPE, Keyboard::None }} });
		definitions.push_back({ "Key.Track.ToggleTieNote", {{ SDLK_RETURN, Keyboard::Shift }} });
		definitions.push_back({ "Key.Track.Copy", {{ SDLK_c, Keyboard::Control }} });
		definitions.push_back({ "Key.Track.Paste", {{ SDLK_v, Keyboard::Control }} });
		definitions.push_back({ "Key.Track.DuplicateSequence", {{ SDLK_d, Keyboard::Control }} });
		definitions.push_back({ "Key.Track.InsertFirstFreeSequence", {{ SDLK_f, Keyboard::Control }} });
		definitions.push_back({ "Key.Track.InsertFirstEmptySequence", {{ SDLK_f, Keyboard::Shift | Keyboard::Control  }} });
		definitions.push_back({ "Key.Track.SplitSequenceAtEventPosition", {{ SDLK_b, Keyboard::Control }} });
		definitions.push_back({ "Key.Track.SetSelectedInstrumentIndexValue", {{ SDLK_i, Keyboard::Control }} });
		definitions.push_back({ "Key.Track.SetSelectedCommandIndexValue", {{ SDLK_o, Keyboard::Control }} });
		definitions.push_back({ "Key.Track.SetOrderlistLoopPoint", {{ SDLK_l, Keyboard::Shift | Keyboard::Control }} });
		definitions.push_back({ "Key.Track.Debug", {{ SDLK_d, Keyboard::Alt }} });

		m_KeyHookStore.PassBaseDefinitions(definitions);
	}

	void KeyHookSetup::ApplyOverrides()
	{
#ifndef _SF2_WINDOWS
		m_KeyHookStore.OverrideDefinition({ "Key.ScreenEdit.IncrementInstrumentIndex", {{ SDLK_DOWN, Keyboard::Cmd }} });
		m_KeyHookStore.OverrideDefinition({ "Key.ScreenEdit.DecrementInstrumentIndex", {{ SDLK_UP, Keyboard::Cmd }} });
		m_KeyHookStore.OverrideDefinition({ "Key.ScreenEdit.IncrementCommandIndex", {{ SDLK_DOWN, Keyboard::Cmd | Keyboard::Shift }} });
		m_KeyHookStore.OverrideDefinition({ "Key.ScreenEdit.PlayFromCursor", {{ SDLK_F2, Keyboard::Cmd }} });
		m_KeyHookStore.OverrideDefinition({ "Key.ScreenEdit.DecrementCommandIndex", {{ SDLK_UP, Keyboard::Cmd | Keyboard::Shift }} });
		m_KeyHookStore.OverrideDefinition({ "Key.ScreenEdit.FastForward", {{ SDLK_MINUS, Keyboard::None }} });
		m_KeyHookStore.OverrideDefinition({ "Key.ScreenEdit.FasterForward", {{ SDLK_MINUS, Keyboard::Shift }} });
		m_KeyHookStore.OverrideDefinition({ "Key.Track.TransposeOctaveDown", {{ SDLK_F3, Keyboard::Cmd | Keyboard::Shift }} });
		m_KeyHookStore.OverrideDefinition({ "Key.Track.TransposeOctaveUp", {{ SDLK_F4, Keyboard::Cmd | Keyboard::Shift }} });
		m_KeyHookStore.OverrideDefinition({ "Key.Track.CursorLeftColumn", {{ SDLK_LEFT, Keyboard::Cmd }} });
		m_KeyHookStore.OverrideDefinition({ "Key.Track.CursorRightColumn", {{ SDLK_RIGHT, Keyboard::Cmd }} });
#endif //_SF2_WINDOWS
	}
}

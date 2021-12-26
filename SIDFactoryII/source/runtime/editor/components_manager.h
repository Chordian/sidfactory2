#pragma once

#include "foundation/base/types.h"
#include "runtime/editor/cursor_control.h"
#include <memory>
#include <vector>
#include <functional>

#define MAX_TAB_GROUP_COUNT 16

namespace Foundation
{
	class Viewport;
	class Mouse;
	class Keyboard;
}

namespace Emulation
{
	class CPUMemory;
}

namespace Editor
{
	class ComponentBase;
	class VisualizerComponentBase;
	class ScreenBase;
	class DialogBase;
	class DriverInfo;
	class DisplayState;

	class ComponentsManager final
	{
	public:
		ComponentsManager(Foundation::Viewport* inViewport, CursorControl* inCursorControl);
		~ComponentsManager();

		void Clear();

		void Suspend();
		void Resume();

		void StartDialog(std::shared_ptr<DialogBase> inDialog);
		bool IsDisplayingDialog() const;
		void CancelDialog();
		bool IsNoteInputSilenced() const;
		bool IsFastForwardAllowed() const;

		void PullDataFromAllSources(const bool inFromUndo);

		void AddComponent(std::shared_ptr<ComponentBase> inComponent);
		void RemoveComponent(std::shared_ptr<ComponentBase> inComponent);

		ComponentBase* GetComponent(int inComponentID);
		const ComponentBase* GetComponent(int inComponentID) const;

		void SetGroupEnabledForInput(unsigned int inGroup, bool inEnable);
		bool IsGroupEnabledForInput(unsigned int inGroup) const;

		void SetGroupEnabledForTabbing(unsigned int inGroup);
		unsigned int GetEnabledTabGroup() const;
		bool IsTabGroupEnabled(unsigned int inGroup) const;

		void SetComponentInFocus(int inComponentID);
		void SetComponentInFocus(std::shared_ptr<ComponentBase> inComponent);
		bool IsComponentInFocus(int inComponentID) const;

		void AddVisualizerComponent(std::shared_ptr<VisualizerComponentBase> inComponent);
		void RemoveVisualizerComponent(std::shared_ptr<VisualizerComponentBase> inComponent);

		bool ConsumeInput(const Foundation::Keyboard& inKeyboard, const Foundation::Mouse& inMouse);
		void Update(int inDeltaTick, Emulation::CPUMemory* inCPUMemory);
		void Refresh(const DisplayState& inDisplayState);

		void SetNextTabComponentFocus();
		void SetTabPreviousComponentFocus();
		void ForceRefresh();

	private:
		void SetComponentInFocus(ComponentBase* inFocusComponent);
		void SetComponentInFocusByTabbing(ComponentBase* inFocusComponent, bool inWasForwardTab);
		void SortComponents();

		ComponentBase* GetComponentAfter(ComponentBase* inComponent) const;
		ComponentBase* GetComponentBefore(ComponentBase* inComponent) const;
		ComponentBase* GetComponentAt(const Foundation::Point& inPosition) const;

		bool m_Suspended;

		Foundation::Viewport* m_Viewport;

		std::shared_ptr<DialogBase> m_ActiveDialog;

		std::vector<std::shared_ptr<ComponentBase>> m_Components;
		std::vector<std::shared_ptr<VisualizerComponentBase>> m_VisualizerComponents;

		unsigned int m_EnabledInputGroups;
		unsigned int m_EnabledTabGroup;

		ComponentBase* m_TabGroupFocusComponent[MAX_TAB_GROUP_COUNT];
		ComponentBase* m_FocusComponent;
		CursorControl* m_CursorControl;
	};
}